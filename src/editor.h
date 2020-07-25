#ifndef EDITOR_H
#define EDITOR_H

#include <string>
#include "sledge.h"
#include "sledge_program.h"

#define EDITOR_TRANSMIT_DONE -1

class Editor {
public:
  Sledge &sledge;
  SledgeProgram file_programs[NUM_SLEDGE_PROGRAMS];
  char recent_file_path[1024];
  char error_message[1024];
  string default_sysex_dir;

  Editor(Sledge &s, const char * const default_sysex_dir);

  // Returns -1 if all done sending
  int last_transmitted_program() { return last_transmitted_prog; }

  int load(const char * const path); // returns 0 if OK, else error_message set
  int save(const char * const path); // ditto

  void copy_file_to_synth(int prog_num);
  void copy_within_synth(int prog_num);
  void move_within_synth(int prog_num);
  void transmit_selected();
  OSStatus send_program_change(int prog_num) {
    return sledge.program_change(prog_num);
  }

private:
  int last_transmitted_prog;

  const char * expand_and_save_path(char *path);
  void copy_or_move(SledgeProgram *from, SledgeProgram *to,
                    int prog_num, bool init_src);
};

#endif /* EDITOR_H */
