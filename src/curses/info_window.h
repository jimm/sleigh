#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include "window.h"

using namespace std;

class InfoWindow : public Window {
public:
  InfoWindow(struct rect, const char *);
  ~InfoWindow();

  void draw();
};

#endif /* INFO_WINDOW_H */
