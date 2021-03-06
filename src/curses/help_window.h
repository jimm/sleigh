#ifndef HELP_WINDOW_H
#define HELP_WINDOW_H

#include <vector>
#include "window.h"

class HelpWindow : public Window {
public:
  vector<char *> *lines;

  HelpWindow(struct rect, const char *);
  ~HelpWindow();

  void draw();

private:
  const char * read_help();
  vector<char *> *text_to_lines(const char *);
};


#endif /* HELP_WINDOW_H */
