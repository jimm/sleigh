#include <stdio.h>
#include <stdlib.h>
#include "../consts.h"
#include "../utils.h"
#include "list_window.h"

ListWindow::ListWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix), programs(0), curr_index(0),
    sel_min(-1), sel_max(-1), offset(0)
{
}

void ListWindow::set_contents(const char *title_str, SledgeProgram *sp,
                              int curr_idx, int selection_min, int selection_max)
{
  title = title_str;
  programs = sp;
  curr_index = curr_idx;
  sel_min = selection_min;
  sel_max = selection_max;
}

void ListWindow::draw() {
  Window::draw();
  if (programs == 0)
    return;

  int vis_height = visible_height();
  if (curr_index < offset)
    offset = curr_index;
  else if (curr_index >= offset + vis_height)
    offset = curr_index - vis_height + 1;

  int row = 1;
  for (int i = offset; i < 1000 && i < offset + vis_height; ++i, ++row) {
    SledgeProgram *prog = &programs[i];
    if (prog->sysex != SYSEX)
      continue;
    wmove(win, row, 1);
    if (i >= sel_min && i <= sel_max)
      wattron(win, A_REVERSE);
    waddch(win, ' ');
    waddstr(win, prog_num_str(i));
    waddstr(win, c_name(prog));
    waddch(win, ' ');
    if (i >= sel_min && i <= sel_max)
      wattroff(win, A_REVERSE);
  }
}

// Uses utils c_str_buf.
char * ListWindow::prog_num_str(int i) {
  sprintf(c_str_buf, "%03d: ", i+1);
  return c_str_buf;
}
