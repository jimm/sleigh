#ifndef EDITOR_H
#define EDITOR_H

#include "sledge.h"
#include "sledge_program.h"

#define EDITOR_FILE   0
#define EDITOR_SLEDGE 1

class ProgramState {
public:
  SledgeProgram *programs;
  bool selected[1000];
  int curr;
};

class Editor : public Observer {
public:
  Sledge *sledge;
  SledgeProgram programs[1000];
  ProgramState pstate[2];
  char loaded_file_path[1024];
  char error_message[1024];
  const char * const default_sysex_dir;

  Editor(Sledge *s, const char * const default_sysex_dir);

  virtual void update(Observable *o);

  SledgeProgram * programs_for_type(int type) { return pstate[type].programs; }
  int curr_index_for_type(int type) { return pstate[type].curr; }
  bool is_selected(int type, int i) { return pstate[type].selected[i]; }

  int load(const char * const path); // returns 0 if OK, else error_message set
  int save(const char * const path); // ditto

  void file_to_synth(int prog_num);
  void copy_within_synth(int prog_num);
  void move_within_synth(int prog_num);

  OSStatus send_program_change(int prog_num) {
    return sledge->program_change(prog_num);
  }

  void select(int which, int index, bool shifted);

private:
  const char * expand_and_save_path(char *path);
  void copy_or_move(int from_type, int to_type, int prog_num, bool init_src);
};

#endif /* EDITOR_H */
