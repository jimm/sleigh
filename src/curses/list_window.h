#ifndef LIST_WINDOW_H
#define LIST_WINDOW_H

#include "../sledge_program.h"
#include "window.h"

using namespace std;

class ListWindow : public Window {
public:
  SledgeProgram *programs;
  int sel_min;
  int sel_max;
  int offset;
  int curr_index;

  ListWindow(struct rect, const char *);

  void set_contents(const char *title, SledgeProgram *programs,
                    int curr_idx, int sel_min, int sel_max);
  void draw();

private:
  char * prog_num_str(int i);
};

#endif /* LIST_WINDOW_H */
