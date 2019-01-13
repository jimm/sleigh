#include <sstream>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#ifndef CLOCK_REALTIME
#include <sys/time.h>
#endif
#include "../consts.h"
#include "../utils.h"
#include "gui.h"
#include "geometry.h"
#include "info_window.h"
#include "list_window.h"
#include "prompt_window.h"

#define DEBOUNCE_MSECS 100
#define LEFT_WINDOW 0
#define RIGHT_WINDOW 1

GUI::GUI(Editor *e)
  : editor(e), active_window(0), clear_msg_id(0), last_mouse_click_msecs(0)
{
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
  bool done = FALSE, needs_refresh = true;
  int ch, prev_cmd = 0;
  MEVENT event;

  while (!done) {
    if (needs_refresh)
      refresh_all();
    needs_refresh = true;
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
      send_program_change();
      break;
    case 'j':
      jump_in_window();
      break;
    case 'q':
      done = TRUE;
      break;
    case '\t':
      active_window->set_active(false);
      active_window = active_window == file_list ? synth_list : file_list;
      active_window->set_active(true);
      break;
    case KEY_MOUSE:
      if (getmouse(&event) == OK)
        handle_mouse(&event);
      break;
    case KEY_PPAGE:
      page_up();
      break;
    case KEY_NPAGE:
      page_down();
      break;
    case KEY_RESIZE:
      resize_windows();
      break;
    case 'r':
      break;
    default:
      needs_refresh = false;
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
  active_window = file_list;
  active_window->set_active(true);
  info = new InfoWindow(geom_info_rect(), "");
  prompt = 0;

  scrollok(stdscr, false);
}

void GUI::resize_windows() {
  file_list->move_and_resize(geom_file_rect());
  synth_list->move_and_resize(geom_synth_rect());
  info->move_and_resize(geom_info_rect());
}

void GUI::free_windows() {
  delete file_list;
  delete synth_list;
  delete info;
}

void GUI::refresh_all() {
  set_window_data();

  file_list->draw();
  synth_list->draw();
  info->draw();

  wnoutrefresh(stdscr);
  wnoutrefresh(file_list->win);
  wnoutrefresh(synth_list->win);
  wnoutrefresh(info->win);
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

void GUI::send_program_change() {
  prompt = new PromptWindow("Send Program Change");
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

void GUI::jump_in_window() {
  prompt = new PromptWindow("Jump To");
  string num_str = prompt->gets();
  delete prompt;
  prompt = 0;

  int num = atoi(num_str.c_str()) - 1;
  if (num < 0 || num > 1000)
    return;

  active_window->jump_to(num);
}

void GUI::handle_mouse(MEVENT *event) {
  if (mouse_click_too_soon())
    return;

  rect left_rect = geom_file_rect();
  rect right_rect = geom_synth_rect();
  bool shifted = (event->bstate & BUTTON_SHIFT) != 0;
  int index_at_mouse;

  if (point_in_rect(event->x, event->y, left_rect)) {
    index_at_mouse = file_list->index_at(event->y - left_rect.row,
                                         event->x - left_rect.col);
    editor->select(EDITOR_FILE, index_at_mouse, shifted);
    active_window = file_list;
  }
  else if (point_in_rect(event->x, event->y, right_rect)) {
    index_at_mouse = synth_list->index_at(event->y - left_rect.row,
                                          event->x - left_rect.col);
    editor->select(EDITOR_SLEDGE, index_at_mouse, shifted);
    active_window = synth_list;
  }
}

bool GUI::mouse_click_too_soon() {
  long now_msecs;
#ifdef CLOCK_REALTIME
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  now_msecs = now.tv_sec * 1000L + (now.tv_nsec/1000000L);
#else
  timeval now;
  gettimeofday(&now, 0);
  now_msecs = now.tv_sec * 1000L + (now.tv_usec/1000L);
#endif

  long diff_msecs = now_msecs - last_mouse_click_msecs;
  last_mouse_click_msecs = now_msecs;
  return diff_msecs < DEBOUNCE_MSECS;
}

void GUI::page_up() {
  if (active_window == 0)
    return;
  active_window->page_up();
}

void GUI::page_down() {
  if (active_window == 0)
    return;
  active_window->page_down();
}

void GUI::show_message(string msg) {
  debug("message: %s\n", msg.c_str());
  mvaddstr(LINES-1, 1, msg.c_str());
  clrtoeol();
  refresh();
  doupdate();
}

void GUI::clear_message() {
  move(LINES-1, 1);
  clrtoeol();
  refresh();
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
