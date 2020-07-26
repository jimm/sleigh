/*
 * Manages Sledge program loading from files, saving to files,
 * copying/moving programs around, and transmitting programs to the Sledge.
 */

#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "editor.h"
#include "utils.h"

#define toggle(x) ((x) = !(x))

Editor::Editor(Sledge &s, const char * const sysex_dir)
  : sledge(s)
{
  if (sysex_dir)
    default_sysex_dir = sysex_dir;
}

// Loads into `from_file`.
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
    memcpy((void *)&file_programs[prog_num], &prog, SLEDGE_PROGRAM_SYSEX_LEN);
    file_programs[prog_num].update();
  }
  fclose(fp);

  return 0;
}

// Saves `synth` programs into a file.
int Editor::save(const char * const path) {
  FILE *fp = fopen(expand_and_save_path((char *)path), "w");

  if (fp == 0) {
    sprintf(error_message, "error opening %s for write: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  for (int i = 0; i < NUM_SLEDGE_PROGRAMS; ++i)
    if (sledge.programs[i].sysex != 0)
      fwrite(&sledge.programs[i], SLEDGE_PROGRAM_SYSEX_LEN, 1, fp);
  fclose(fp);
  return 0;
}

void Editor::copy_file_to_synth(std::vector<int> &selected, int prog_num) {
  copy_or_move(&file_programs[0], &sledge.programs[0], selected, prog_num, false);
}

void Editor::copy_within_synth(std::vector<int> &selected, int prog_num) {
  copy_or_move(&sledge.programs[0], &sledge.programs[0], selected, prog_num, false);
}

void Editor::move_within_synth(std::vector<int> &selected, int prog_num) {
  copy_or_move(&sledge.programs[0], &sledge.programs[0], selected, prog_num, true);
}

void Editor::copy_or_move(SledgeProgram *from, SledgeProgram *to,
                          std::vector<int> &selected, int prog_num, bool init_src)
{
  for (auto i : selected) {
    SledgeProgram &src = from[i];
    SledgeProgram &dest = to[prog_num];

    memcpy((void *)&dest, (void *)&src, SLEDGE_PROGRAM_SYSEX_LEN);
    dest.set_program_number(prog_num);
    dest.update();
    if (init_src) {
      src.initialize();
      src.update();
    }
    ++prog_num;
  }
  sledge.changed();
}

void Editor::transmit_selected(std::vector<int> &selected) {
  for (auto i : selected) {
    usleep(10);
    sledge.send_sysex((const byte *)&sledge.programs[i], SLEDGE_PROGRAM_SYSEX_LEN);
    last_transmitted_prog = i;
  }
  last_transmitted_prog = EDITOR_TRANSMIT_DONE;
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
