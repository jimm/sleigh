#ifndef SM_WINDOW_H
#define SM_WINDOW_H

#include <string>
#include <ncurses.h>
#include "geometry.h"

using namespace std;

class Window {
public:
  WINDOW *win;
  string title;
  rect rect;
  bool active;

  Window(struct rect r, const char *title);
  virtual ~Window();

  void set_active(bool a) { active = a; }
  void move_and_resize(struct rect);
  void draw();
  int visible_height();
  void make_fit(string &, int);
  void make_fit(const char *, int, char *);
};

#endif /* SM_WINDOW_H */
