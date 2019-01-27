#include <stdio.h>
#include <stdlib.h>
#include "../consts.h"
#include "../utils.h"
#include "list_window.h"

ListWindow::ListWindow(struct rect r, const char *title, ProgramState &pstate)
  : Window(r, title), offset(0), program_state(pstate)
{
}

void ListWindow::draw() {
  Window::draw();

  SledgeProgram *programs = program_state.programs();
  int vis_height = visible_height();

  int row = 1;
  for (int i = offset; i < 1000 && i < offset + vis_height; ++i, ++row) {
    SledgeProgram *prog = &programs[i];
    wmove(win, row, 1);
    if (program_state.is_selected(i))
      wattron(win, A_REVERSE);
    waddch(win, ' ');
    wprintw(win, "%03d: ", i+1);
    if (prog->sysex == SYSEX)
      waddstr(win, prog->name_str());
    waddch(win, ' ');
    if (program_state.is_selected(i))
      wattroff(win, A_REVERSE);
  }
}

void ListWindow::page_up() {
  int vis_height = visible_height();
  if (offset > vis_height-1)
    offset -= vis_height;
}

void ListWindow::page_down() {
  int vis_height = visible_height();
  if (offset < 1000 - vis_height)
    offset += vis_height;
}

void ListWindow::jump_to(int n) {
  offset = n;
}
