#ifndef GUI_H
#define GUI_H

#include <CoreMIDI/MIDIServices.h>
#include <ncurses.h>
#include <time.h>
#include "../editor.h"
#include "../sledge.h"
#include "list_window.h"
#include "prompt_window.h"

using namespace std;

class InfoWindow;
class Window;

class GUI : public Observer {
public:
  GUI(Editor *);
  ~GUI();

  void run();

  void clear_message();
  int clear_message_seconds() { return clear_msg_secs; }
  int clear_message_id() { return clear_msg_id; }

  virtual void update(Observable *o);

private:
  Editor *editor;
  ListWindow *file_list;
  ListWindow *synth_list;
  InfoWindow *info;
  PromptWindow *prompt;
  ListWindow *active_window;
  int clear_msg_secs;
  int clear_msg_id;
  mmask_t old_mouse_mask;
  long last_mouse_click_msecs;

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
  void transmit();
  void file_to_synth();
  void copy_within_synth();
  void move_within_synth();
  void transmit_selected();
  void send_program_change();
  void jump_in_window();
  void handle_mouse(MEVENT *event);
  void page_up();
  void page_down();
  bool mouse_click_too_soon();
  void show_message(string);
  void clear_message_after(int);
};

#endif /* GUI_H */
