#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "editor.h"
#include "utils.h"

Editor::Editor(Sledge *s)
  : sledge(s),
    curr_program(0), programs_sel_min(-1), programs_sel_max(-1),
    curr_sledge(0), sledge_sel_min(-1), sledge_sel_max(-1)
{
  memset((void *)programs, 1000, SLEDGE_PROGRAM_SYSEX_LEN);
  for (int i = 0; i < 1000; ++i)
    programs[i].sysex = 0;
}

int Editor::load(const char * const path) {
  SledgeProgram prog;

  FILE *fp = fopen(save_and_expand_path((char *)path), "r");
  if (fp == 0) {
    sprintf(error_message, "error opening %s for read: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  while (fread(&prog, SLEDGE_PROGRAM_SYSEX_LEN, 1, fp) == 1) {
    int prog_num = prog.prog_high * 0x80 + prog.prog_low;
    memcpy(&programs[prog_num], &prog, SLEDGE_PROGRAM_SYSEX_LEN);
  }
  fclose(fp);

  curr_program = 0;
  programs_sel_min = -1;
  programs_sel_max = -1;
  return 0;
}

int Editor::save(const char * const path) {
  FILE *fp = fopen(path, "w");

  if (fp == 0) {
    sprintf(error_message, "error opening %s for write: %s", path, strerror(errno));
    debug("%s\n", error_message);
    return errno;
  }
  for (int i = 0; i < 1000; ++i)
    if (programs[i].sysex != 0)
      fwrite(&programs[i], SLEDGE_PROGRAM_SYSEX_LEN, 1, fp);
  fclose(fp);
  return 0;
}

// Expands tilde or env var at beginning of path and writes expanded path
// into loaded_file_path. Returns loaded_file_path.
const char * Editor::save_and_expand_path(char *path) {
  char *p = loaded_file_path;
  if (path[0] == '~') {
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
    char env_name[1024], *env_p = path+1, *n;

    for (n = env_name, env_p = path+1; *env_p && *env_p != '/'; ++n, ++env_p)
      *n = *env_p;
    *n = 0;
    sprintf(loaded_file_path, "%s", getenv(env_name));
    p += strlen(loaded_file_path);
    path = env_p;
  }

  strcpy(p, path);
  return loaded_file_path;
}
