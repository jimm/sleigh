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
  waddstr(win, "Help will go here");
}
