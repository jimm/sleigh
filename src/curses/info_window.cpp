#include "info_window.h"

InfoWindow::InfoWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix)
{
}

InfoWindow::~InfoWindow() {
}

void InfoWindow::draw() {
  char fitted[BUFSIZ];

  Window::draw();
  wmove(win, 1, 1);
  add_help_str('l', "load");
  add_help_str('s', ", save");
  add_help_str('t', ", transmit to Sledge");
  add_help_str('r', ", refresh");
  add_help_str('q', ", quit");
  wmove(win, 2, 1);
  add_help_str('>', "copy ->");
  add_help_str('<', ", <- copy");
}

void InfoWindow::add_help_str(char ch, const char * const str) {
  bool bolded = false;

  for (const char *p = str; *p; ++p) {
    if (!bolded && *p == ch)
      wattron(win, A_REVERSE);
    waddch(win, *p);
    if (!bolded && *p == ch) {
      wattroff(win, A_REVERSE);
      bolded = true;
    }
  }
}
