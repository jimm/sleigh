#ifndef LIST_WINDOW_H
#define LIST_WINDOW_H

#include "../sledge_program.h"
#include "../editor.h"
#include "window.h"

using namespace std;

class ListWindow : public Window {
public:
  ListWindow(struct rect, const char *);

  void set_contents(Editor *editor, int editor_type);
  void draw();

  int index_at(int row, int col) { return row + offset - 1; }

private:
  Editor *editor;
  int editor_type;
  int offset;

  char * prog_num_str(int i);
};

#endif /* LIST_WINDOW_H */
