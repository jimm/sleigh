#include <stdio.h>
#include <stdlib.h>
#include "../consts.h"
#include "../utils.h"
#include "list_window.h"

ListWindow::ListWindow(struct rect r, const char *title)
  : Window(r, title), editor(0), offset(0)
{
}

void ListWindow::set_contents(Editor *ed, int ed_type) {
  editor = ed;
  editor_type = ed_type;
}

void ListWindow::draw() {
  Window::draw();
  if (editor == 0)
    return;

  SledgeProgram *programs = editor->programs_for_type(editor_type);
  int vis_height = visible_height();

  int row = 1;
  for (int i = offset; i < 1000 && i < offset + vis_height; ++i, ++row) {
    SledgeProgram *prog = &programs[i];
    if (prog->sysex != SYSEX)
      continue;
    wmove(win, row, 1);
    if (editor->is_selected(editor_type, i))
      wattron(win, A_REVERSE);
    waddch(win, ' ');
    wprintw(win, "%03d: ", i+1);
    waddstr(win, prog->name_str());
    waddch(win, ' ');
    if (editor->is_selected(editor_type, i))
      wattroff(win, A_REVERSE);
  }
}

void ListWindow::page_up() {
  int vis_height = visible_height();
  if (offset > vis_height)
    offset -= vis_height;
}

void ListWindow::page_down() {
  int vis_height = visible_height();
  if (offset < 1000 - vis_height)
    offset += vis_height;
}
