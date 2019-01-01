#ifndef EDITOR_H
#define EDITOR_H

#include "sledge.h"
#include "sledge_program.h"

class Editor : public Observer {
public:
  SledgeProgram programs[1000];
  Sledge *sledge;
  int curr_program;
  int programs_sel_min;
  int programs_sel_max;
  int curr_sledge;
  int sledge_sel_min;
  int sledge_sel_max;
  char loaded_file_path[1024];
  char error_message[1024];
  const char * const default_sysex_dir;

  Editor(Sledge *s, const char * const default_sysex_dir);

  virtual void update(Observable *o);

  int load(const char * const path); // returns 0 if OK, else error_message set
  int save(const char * const path); // ditto

  OSStatus send_program_change(int prog_num) {
    return sledge->program_change(prog_num);
  }

private:
  const char * expand_and_save_path(char *path);
};

#endif /* EDITOR_H */
