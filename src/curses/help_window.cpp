#include <stdlib.h>
#include <string.h>
#include "help_window.h"
#include "info_window.h"

HelpWindow::HelpWindow(struct rect r, const char *title)
  : Window(r, title)
{
  lines = text_to_lines(read_help());
}

HelpWindow::~HelpWindow() {
  if (lines == 0)
    return;

  if (!lines->empty())
    free((*lines)[0]);
  delete lines;
}

void HelpWindow::draw() {
  Window::draw();
  for (vector<char *>::iterator i = lines->begin(); i != lines->end(); ++i) {
    wmove(win, i - lines->begin() + 1, 1);
    waddstr(win, *i);
  }
}

const char * HelpWindow::read_help() {
  return "\
    l - Load\n\
    s - Save\n\
    t - Transmit selection to Sledge\n\
    > - Copy file patch to synth patch\n\
    c - Copy within synth\n\
    m - Move within synth\n\
    p - Send program change\n\
    j - Jump to patch\n\
  TAB - Alternate between file and Sledge lists\n\
Mouse - Select/deselect. Shift to extend selection, Control to select\n\
PG UP - Up one page\n\
PG DN - Down one page\n\
 h, ? - This help\n\
    q - Quit\
";
}

/*
 * Splits `text` into lines and returns a list containing the lines. When
 * you are done with the list, only the first entry should be freed.
 */
vector<char *> * HelpWindow::text_to_lines(const char *text) {
  vector<char *> *l = new vector<char *>();

  char *line;
  char *s = strdup(text);
  while ((line = strsep(&s, "\n")) != NULL)
    l->push_back(line);

  return l;
}
