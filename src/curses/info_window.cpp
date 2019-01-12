#include "info_window.h"

const char * const info_text[] = {
  "*l*oad, *s*ave, *t*ransmit to Sledge, *tab* toggle window, *r*efresh, *q*uit",
  "*j*ump in window, copy -*>*, *c*opy within synth, *m*ove within synth, *p*rogram change"
};

InfoWindow::InfoWindow(struct rect r, const char *title_prefix)
  : Window(r, title_prefix)
{
}

InfoWindow::~InfoWindow() {
}

void InfoWindow::draw() {
  bool reversed;
  const char * p;
  int i;

  Window::draw();
  for (int i = 0; i < sizeof(info_text) / sizeof(char *); ++i) {
    reversed = false;
    wmove(win, i+1, 1);
    p = info_text[i];
    while (*p) {
      if (*p == '*') {
        if (reversed)
          wattroff(win, A_REVERSE);
        else
          wattron(win, A_REVERSE);
        reversed = !reversed;
      }
      else
        waddch(win, *p);
      ++p;
    }
  }
}
