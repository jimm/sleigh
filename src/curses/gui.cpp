#include <sstream>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include "../consts.h"
#include "../utils.h"
#include "gui.h"
#include "geometry.h"
#include "info_window.h"
#include "list_window.h"
#include "prompt_window.h"

#define DEBOUNCE_MSECS 100

GUI::GUI(Editor *e)
  : editor(e), clear_msg_id(0)
{
  clock_gettime(CLOCK_REALTIME, &last_mouse_click_time);
}

GUI::~GUI() {
}

void GUI::run() {
  config_curses();
  create_windows();
  event_loop();
  clear();
  refresh();
  close_screen();
  free_windows();
}

void GUI::event_loop() {
  bool done = FALSE;
  int ch, prev_cmd = 0;
  MEVENT event;

  while (!done) {
    refresh_all();
    ch = getch();
    switch (ch) {
    case 'l':
      load();
      break;
    case 's':
      save();
      break;
    case 't':
      transmit_selected();
      break;
    case '>':
      file_to_synth();
      break;
    case 'c':
      copy_within_synth();
      break;
    case 'm':
      move_within_synth();
      break;
    case 'p':
      goto_program();
      break;
    case 'r':
      break;
    case 'q':
      done = TRUE;
      break;
    case KEY_MOUSE:
      if (getmouse(&event) == OK)
        handle_mouse(&event);
      break;
    case KEY_RESIZE:
      resize_windows();
      break;
    }
    prev_cmd = ch;
  }
}

void GUI::update(Observable *o) {
  Sledge *sledge = (Sledge *)o;
  int prog_num = sledge->last_received_program();

  synth_list->draw();
  wnoutrefresh(synth_list->win);

  if (prompt)
    wnoutrefresh(prompt->win);

  doupdate();
}

void GUI::config_curses() {
  initscr();
  cbreak();                     /* unbuffered input */
  noecho();                     /* do not show typed keys */
  keypad(stdscr, true);         /* enable arrow keys and window resize as keys */
  nl();                         /* return key => newline, \n => \r\n */
  mousemask(BUTTON1_PRESSED | BUTTON_SHIFT, &old_mouse_mask);
  curs_set(0);                  /* cursor: 0 = invisible, 1 = normal */
}

void GUI::create_windows() {
  file_list = new ListWindow(geom_file_rect(), "Loaded File");
  synth_list = new ListWindow(geom_synth_rect(), "Sledge Programs / Save to File");
  info = new InfoWindow(geom_info_rect(), "");
  message = new Window(geom_message_rect(), "");
  prompt = 0;

  scrollok(stdscr, false);
  scrollok(message->win, false);
}

void GUI::resize_windows() {
  file_list->move_and_resize(geom_file_rect());
  synth_list->move_and_resize(geom_synth_rect());
  info->move_and_resize(geom_info_rect());
  message->move_and_resize(geom_message_rect());
}

void GUI::free_windows() {
  delete file_list;
  delete synth_list;
  delete info;
  delete message;
}

void GUI::refresh_all() {
  set_window_data();

  file_list->draw();
  synth_list->draw();
  info->draw();
  message->draw();

  wnoutrefresh(stdscr);
  wnoutrefresh(file_list->win);
  wnoutrefresh(synth_list->win);
  wnoutrefresh(info->win);
  wnoutrefresh(message->win);
  if (prompt != 0)
    wnoutrefresh(prompt->win);

  doupdate();
}

void GUI::set_window_data() {
  file_list->set_contents(editor, EDITOR_FILE);
  synth_list->set_contents(editor, EDITOR_SLEDGE);
}

void GUI::close_screen() {
  mmask_t discarded;

  curs_set(1);
  mousemask(old_mouse_mask, &discarded);
  echo();
  nl();
  noraw();
  nocbreak();
  refresh();
  endwin();
}

void GUI::load() {
  prompt = new PromptWindow("Load File");
  string path = prompt->gets();
  delete prompt;
  prompt = 0;

  if (path.length() == 0) {
    show_message("error: no file loaded");
    return;
  }

  if (editor->load(path.c_str()) == 0) {
    ostringstream ostr;
    ostr << "loaded file " << path;
    show_message(ostr.str());
  }
  else
    show_message(editor->error_message);
}

void GUI::save() {
  prompt = new PromptWindow("Save File");
  string path = prompt->gets();
  delete prompt;
  prompt = 0;

  if (path.length() == 0) {
    show_message("error: no file saved");
    return;
  }

  if (editor->save(path.c_str()) == 0) {
    ostringstream ostr;
    ostr << "saved file " << path;
    show_message(ostr.str());
  }
  else
    show_message(editor->error_message);
}

void GUI::file_to_synth() {
  prompt = new PromptWindow("Copy From File to Synth, Starting at Program Number");
  string prog_num_str = prompt->gets();
  delete prompt;
  prompt = 0;

  int prog_num = atoi(prog_num_str.c_str()) - 1;
  if (prog_num < 0 || prog_num > 998)
    return;
  editor->file_to_synth(prog_num);
}

void GUI::copy_within_synth() {
  prompt = new PromptWindow("Copy Within Synth, Starting at Program Number");
  string prog_num_str = prompt->gets();
  delete prompt;
  prompt = 0;

  int prog_num = atoi(prog_num_str.c_str()) - 1;
  if (prog_num < 0 || prog_num > 998)
    return;
  editor->copy_within_synth(prog_num);
}

void GUI::move_within_synth() {
  prompt = new PromptWindow("Move Within Synth, Starting at Program Number");
  string prog_num_str = prompt->gets();
  delete prompt;
  prompt = 0;

  int prog_num = atoi(prog_num_str.c_str()) - 1;
  if (prog_num < 0 || prog_num > 998)
    return;
  editor->move_within_synth(prog_num);
}

void GUI::transmit_selected() {
  editor->transmit_selected();
}

void GUI::goto_program() {
  prompt = new PromptWindow("Go To Program");
  string prog_num_str = prompt->gets();
  delete prompt;
  prompt = 0;

  int prog_num = atoi(prog_num_str.c_str()) - 1;
  if (prog_num < 0 || prog_num > 998)
    return;

  OSStatus status = editor->send_program_change(prog_num);
  if (status != 0) {
    ostringstream ostr;
    ostr << "error sending program change: " << status;
    show_message(ostr.str());
  }
}

void GUI::handle_mouse(MEVENT *event) {
  // Don't do anything if previous click was within DEBOUNCE_MSECS milliseconds.
  timespec now;
  long then_msecs, now_msecs, diff_msecs;
  clock_gettime(CLOCK_REALTIME, &now);
  then_msecs = last_mouse_click_time.tv_sec * 1000L +
    (last_mouse_click_time.tv_nsec/1000000L);
  now_msecs = now.tv_sec * 1000L + (now.tv_nsec/1000000L);
  diff_msecs = now_msecs - then_msecs;

  clock_gettime(CLOCK_REALTIME, &last_mouse_click_time);

  if (diff_msecs < DEBOUNCE_MSECS) {
    clock_gettime(CLOCK_REALTIME, &last_mouse_click_time);
    return;
  }

  rect left_rect = geom_file_rect();
  rect right_rect = geom_synth_rect();
  bool shifted = (event->bstate & BUTTON_SHIFT) != 0;
  int index_at_mouse;

  if (point_in_rect(event->x, event->y, left_rect)) {
    index_at_mouse = file_list->index_at(event->y - left_rect.row,
                                         event->x - left_rect.col);
    editor->select(EDITOR_FILE, index_at_mouse, shifted);
  }
  else if (point_in_rect(event->x, event->y, right_rect)) {
    index_at_mouse = synth_list->index_at(event->y - left_rect.row,
                                          event->x - left_rect.col);
    editor->select(EDITOR_SLEDGE, index_at_mouse, shifted);
  }
}

void GUI::show_message(string msg) {
  debug("message: %s\n", msg.c_str());

  WINDOW *win = message->win;
  wclear(win);
  message->make_fit(msg, 0);
  waddstr(win, msg.c_str());
  wrefresh(win);
  doupdate();
}

void GUI::clear_message() {
  WINDOW *win = message->win;
  wclear(win);
  wrefresh(win);
  doupdate();
}

void *clear_message_thread(void *gui_vptr) {
  GUI *gui = (GUI *)gui_vptr;
  int clear_message_id = gui->clear_message_id();

  sleep(gui->clear_message_seconds());

  // Only clear the window if the id hasn't changed
  if (gui->clear_message_id() == clear_message_id)
    gui->clear_message();
  return 0;
}

void GUI::clear_message_after(int secs) {
  clear_msg_secs = secs;
  clear_msg_id++;

  pthread_t pthread;
  pthread_create(&pthread, 0, clear_message_thread, this);
}
