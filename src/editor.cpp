#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "editor.h"
#include "utils.h"
#include "init_program.h"

#define toggle(x) ((x) = !(x))

Editor::Editor(Sledge *s, const char * const sysex_dir)
  : sledge(s), default_sysex_dir(sysex_dir)
{
  pstate[EDITOR_FILE].programs = programs;
  pstate[EDITOR_SLEDGE].programs = sledge->programs;
  for (int i = 0; i < 2; ++i) {
    pstate[i].curr = 0;
    for (int j = 0; j < 1000; ++j) {
      pstate[i].programs[j].sysex = 0;
      pstate[i].selected[j] = false;
    }
  }
}

void Editor::update(Observable *_o) {
  pstate[EDITOR_SLEDGE].curr = sledge->last_received_program();
}

int Editor::load(const char * const path) {
  SledgeProgram prog;

  FILE *fp = fopen(expand_and_save_path((char *)path), "r");
  if (fp == 0) {
    sprintf(error_message, "error opening %s for read: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  while (fread(&prog, SLEDGE_PROGRAM_SYSEX_LEN, 1, fp) == 1) {
    int prog_num = prog.prog_high * 0x80 + prog.prog_low;
    memcpy((void *)&pstate[EDITOR_FILE].programs[prog_num], &prog, SLEDGE_PROGRAM_SYSEX_LEN);
    pstate[EDITOR_FILE].programs[prog_num].update();
  }
  fclose(fp);

  pstate[EDITOR_FILE].curr = 0;
  for (int i = 0; i < 1000; ++i)
    pstate[EDITOR_FILE].selected[i] = false;
  return 0;
}

int Editor::save(const char * const path) {
  FILE *fp = fopen(path, "w");

  if (fp == 0) {
    sprintf(error_message, "error opening %s for write: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  ProgramState *state = &pstate[EDITOR_SLEDGE];
  for (int i = 0; i < 1000; ++i)
    if (state->programs[i].sysex != 0)
      fwrite(&state->programs[i], SLEDGE_PROGRAM_SYSEX_LEN, 1, fp);
  fclose(fp);
  return 0;
}

void Editor::file_to_synth(int prog_num) {
  copy_or_move(EDITOR_FILE, EDITOR_SLEDGE, prog_num, false);
}

void Editor::copy_within_synth(int prog_num) {
  copy_or_move(EDITOR_SLEDGE, EDITOR_SLEDGE, prog_num, false);
}

void Editor::move_within_synth(int prog_num) {
  copy_or_move(EDITOR_SLEDGE, EDITOR_SLEDGE, prog_num, true);
}

void Editor::copy_or_move(int from_type, int to_type, int prog_num, bool init_src)
{
  for (int i = 0; i < 1000 && prog_num < 1000; ++i) {
    if (pstate[from_type].selected[i]) {
      SledgeProgram *src = &pstate[from_type].programs[i];
      SledgeProgram *dest = &pstate[to_type].programs[prog_num];

      memcpy((void *)dest, (void *)src, SLEDGE_PROGRAM_SYSEX_LEN);
      dest->prog_high = (prog_num & 0x3f8) >> 7;
      dest->prog_low = prog_num & 0x3f;
      if (init_src) {
        memcpy((void *)src, (void *)&init_program, SLEDGE_PROGRAM_SYSEX_LEN);
        src->update();
      }
      dest->update();
      ++prog_num;
    }
  }
  sledge->changed();
}

void Editor::transmit_selected() {
  for (int i = 0; i < 1000; ++i)
    if (pstate[EDITOR_SLEDGE].selected[i])
      sledge->send_sysex((const byte *)&pstate[EDITOR_SLEDGE].programs[i],
                         SLEDGE_PROGRAM_SYSEX_LEN);
}

void Editor::select(int which, int index, bool shifted) {
  ProgramState *state = &pstate[which];
  int old_curr = state->curr;

  state->curr = index;

  if (!shifted) {
    bool was_selected = state->selected[state->curr];
    for (int i = 0; i < 1000; ++i)
      state->selected[i] = false;
    state->selected[state->curr] = !was_selected;
    return;
  }

  if (old_curr == state->curr) {
    toggle(state->selected[state->curr]);
    return;
  }

  int curr_min = 1001, curr_max = -1;
  for (int i = 0; i < 1000; ++i) {
    if (state->selected[i]) {
      if (i < curr_min) curr_min = i;
      if (i > curr_max) curr_max = i;
    }
  }

  if (state->curr >= curr_min && state->curr <= curr_max) {
    toggle(state->selected[state->curr]);
  }
  else if (state->curr < curr_min) {
    for (int i = state->curr; i < curr_min; ++i)
      state->selected[i] = true;
  }
  else if (state->curr > curr_max) {
    for (int i = curr_max + 1; i <= state->curr; ++i)
      state->selected[i] = true;
  }
}

// Expands tilde or env var at beginning of path and writes expanded path
// into loaded_file_path. If path does not start with '~' or '$' and if
// default_sysex_dir is not 0, then prepends default_sysex_dir. Returns
// loaded_file_path.
const char * Editor::expand_and_save_path(char *path) {
  char *p = loaded_file_path;
  if (path[0] == '~') {
    // Expand tilde
    char *home = getenv("HOME");
    if (path[1] == '/') {
      sprintf(loaded_file_path, "%s", home);
      p += strlen(loaded_file_path);
      ++path;
    }
    else {
      dirname_r(home, p);
      p += strlen(p);
      *p++ = '/';
      ++path;
    }
  }
  else if (path[0] == '$') {
    // Replace env var with value
    char env_name[1024], *env_p = path+1, *n;

    for (n = env_name, env_p = path+1; *env_p && *env_p != '/'; ++n, ++env_p)
      *n = *env_p;
    *n = 0;
    sprintf(loaded_file_path, "%s", getenv(env_name));
    p += strlen(loaded_file_path);
    path = env_p;
  }
 else if (*path != '/' && default_sysex_dir != 0) {
   // Prepend default_sysex_dir if defined
    strncpy(loaded_file_path, default_sysex_dir, 1024);
    if (loaded_file_path[strlen(loaded_file_path)-1] != '/')
      strncat(loaded_file_path, "/", 1024);
    p += strlen(loaded_file_path);
  }

  strcpy(p, path);

  return loaded_file_path;
}
