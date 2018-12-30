#ifndef GUI_H
#define GUI_H

#include "../editor.h"
#include "list_window.h"

using namespace std;

class InfoWindow;
class Window;

class GUI {
public:
  GUI(Editor *);
  ~GUI();

  void run();

  void clear_message();
  int clear_message_seconds() { return clear_msg_secs; }
  int clear_message_id() { return clear_msg_id; }

private:
  Editor *editor;
  ListWindow *file_list;
  ListWindow *synth_list;
  InfoWindow *info;
  Window *message;
  int clear_msg_secs;
  int clear_msg_id;

  void event_loop();
  void config_curses();
  void create_windows();
  void resize_windows();
  void free_windows();
  void refresh_all();
  void set_window_data();
  void close_screen();
  void help();
  void prog_changes_seen();
  void load();
  void save();
  void reload();
  void show_message(string);
  void clear_message_after(int);
};

#endif /* GUI_H */
