#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include "window.h"

using namespace std;

class InfoWindow : public Window {
public:
  InfoWindow(struct rect, const char *);
  ~InfoWindow();

  void draw();

private:
  void add_help_str(char ch, const char * const str);
};

#endif /* INFO_WINDOW_H */
