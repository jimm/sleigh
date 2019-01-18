/*
 * Manages Sledge program loading from files, saving to files,
 * copying/moving programs around, and transmitting programs to the Sledge.
 */

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
  if (sysex_dir)
    default_sysex_dir = sysex_dir;
}

void Editor::update(Observable *_o) {
  pstate[EDITOR_SLEDGE].curr = sledge->last_received_program();
}

// Loads into EDITOR_FILE programs.
int Editor::load(const char * const path) {
  SledgeProgram prog;

  FILE *fp = fopen(expand_and_save_path((char *)path), "r");
  if (fp == 0) {
    sprintf(error_message, "error opening %s for read: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  while (fread(&prog, SLEDGE_PROGRAM_SYSEX_LEN, 1, fp) == 1) {
    int prog_num = prog.program_number();
    memcpy((void *)&pstate[EDITOR_FILE].programs[prog_num], &prog, SLEDGE_PROGRAM_SYSEX_LEN);
    pstate[EDITOR_FILE].programs[prog_num].update();
  }
  fclose(fp);

  pstate[EDITOR_FILE].curr = 0;
  for (int i = 0; i < 1000; ++i)
    pstate[EDITOR_FILE].selected[i] = false;
  return 0;
}

// Saves EDITOR_SLEDGE programs into a file.
int Editor::save(const char * const path) {
  FILE *fp = fopen(expand_and_save_path((char *)path), "w");

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

void Editor::copy_file_to_synth(int prog_num) {
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
      dest->set_program_number(prog_num);
      if (init_src) {
        memcpy((void *)src, (void *)&sledge_init_program, SLEDGE_PROGRAM_SYSEX_LEN);
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
    if (pstate[EDITOR_SLEDGE].selected[i]) {
      usleep(10);
      sledge->send_sysex((const byte *)&pstate[EDITOR_SLEDGE].programs[i],
                         SLEDGE_PROGRAM_SYSEX_LEN);
      last_transmitted_prog = i;
      changed();
    }
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

  // curr is in range of already-selected items; toggle it.
  if (state->curr >= curr_min && state->curr <= curr_max) {
    toggle(state->selected[state->curr]);
    return;
  }

  // curr is less than current min, extend range down.
  if (state->curr < curr_min) {
    for (int i = state->curr; i < curr_min; ++i)
      state->selected[i] = true;
    return;
  }

  // curr is greater than current min, extend range up.
  if (state->curr > curr_max) {
    for (int i = curr_max + 1; i <= state->curr; ++i)
      state->selected[i] = true;
  }
}

// Expands tilde or env var at beginning of path and writes expanded path
// into recent_file_path. If path does not start with '~' or '$' and if
// default_sysex_dir is not empty, then prepends default_sysex_dir. Saves
// final directdory to default_sysex_dir. Returns recent_file_path.
const char * Editor::expand_and_save_path(char *path) {
  char *p = recent_file_path;
  if (path[0] == '~') {
    // Expand tilde
    char *home = getenv("HOME");
    if (path[1] == '/') {
      sprintf(recent_file_path, "%s", home);
      p += strlen(recent_file_path);
      ++path;
    }
    else {
      strcpy(p, dirname(home));
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
    sprintf(recent_file_path, "%s", getenv(env_name));
    p += strlen(recent_file_path);
    path = env_p;
  }
 else if (*path != '/' && default_sysex_dir != "") {
   // Prepend default_sysex_dir if defined
   strncpy(recent_file_path, default_sysex_dir.c_str(), 1024);
    if (recent_file_path[strlen(recent_file_path)-1] != '/')
      strncat(recent_file_path, "/", 1024);
    p += strlen(recent_file_path);
  }

  strcpy(p, path);

  // Copy final directory to default_sysex_dir
  default_sysex_dir = dirname(recent_file_path);

  return recent_file_path;
}
