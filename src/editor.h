#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "sledge.h"
#include "sledge_program.h"

#define EDITOR_FILE   0
#define EDITOR_SLEDGE 1
#define EDITOR_TRANSMIT_DONE -1

class ProgramState {
public:
  Sledge &sledge;
  bool selected[1000];
  int curr;

  ProgramState(Sledge &s);

  SledgeProgram *programs() { return sledge.programs; }
  bool is_selected(int i) { return selected[i]; }
};

class Editor : public Observer, public Observable {
public:
  ProgramState synth;
  ProgramState from_file;
  char recent_file_path[1024];
  char error_message[1024];
  string default_sysex_dir;

  Editor(Sledge &s, const char * const default_sysex_dir);

  virtual void update(Observable *o);

  // Returns -1 if all done sending
  int last_transmitted_program() { return last_transmitted_prog; }

  int load(const char * const path); // returns 0 if OK, else error_message set
  int save(const char * const path); // ditto

  void copy_file_to_synth(int prog_num);
  void copy_within_synth(int prog_num);
  void move_within_synth(int prog_num);
  void transmit_selected();
  OSStatus send_program_change(int prog_num) {
    return synth.sledge.program_change(prog_num);
  }

  void select(int which, int index, bool shifted);

private:
  int last_transmitted_prog;

  const char * expand_and_save_path(char *path);
  void copy_or_move(ProgramState &from, ProgramState &to,
                    int prog_num, bool init_src);
};

#endif /* EDITOR_H */
