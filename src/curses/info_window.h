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

vector<char *> *info_window_text_to_lines(const char *);
void info_window_free_lines(vector<char *> *);

#endif /* INFO_WINDOW_H */
