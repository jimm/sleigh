// Defines positions and sizes of windows.
#include <ncurses.h>
#include "geometry.h"

#define MAX_PROMPT_WINDOW_WIDTH 80

inline int INFO_HEIGHT() { return 4; }
inline int TOP_HEIGHT() { return LINES - (INFO_HEIGHT() + 1); }
inline int TOP_WIDTH() { return COLS / 2; }

rect geom_file_rect() {
  rect r;

  r.row = 0;
  r.col = 0;
  r.height = TOP_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_synth_rect() {
  rect r;

  r.row = 0;
  r.col = TOP_WIDTH();
  r.height = TOP_HEIGHT();
  r.width = TOP_WIDTH();
  return r;
}

rect geom_info_rect() {
  rect r;

  r.row = TOP_HEIGHT();
  r.col = 0;
  r.height = INFO_HEIGHT();
  r.width = COLS;
  return r;
}

rect geom_help_rect() {
  rect r;

  r.row = 3;
  r.col = 3;
  r.height = LINES - 6;
  r.width = COLS - 6;
  return r;
}

rect geom_prompt_rect() {
  rect r;

  r.height = 3;
  r.width = COLS - 8;
  if (r.width > MAX_PROMPT_WINDOW_WIDTH)
    r.width = MAX_PROMPT_WINDOW_WIDTH;
  r.row = LINES / 3;
  r.col = (COLS - r.width) / 2;
  return r;
}

rect geom_message_rect() {
  rect r;

  r.row = LINES - 1;
  r.col = 0;
  r.height = 1;
  r.width = COLS;
  return r;
}
