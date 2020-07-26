#ifndef WX_FRAME_H
#define WX_FRAME_H

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
 #include <wx/listctrl.h>
#endif
#include "events.h"
#include "../sleigh.h"
#include "../observer.h"

enum {
  ID_TransmitSelected = 1,
  ID_SendProgramChange,
  ID_CopyFileToSynth,
  ID_CopyWithinSynth,
  ID_MoveWithinSynth,
  ID_FileList,
  ID_SynthList
};

class wxTextCtrl;
class SledgeProgram;
class FrameListView;

class Frame: public wxFrame, public Observer {
public:
  Frame(const wxString& title, Sleigh &sleigh);
  virtual ~Frame() {}

  void initialize();
  void load(wxString path);
  void save();


  void update(wxCommandEvent& event) { update(); }
  virtual void update(Observable *o) { update(); }
  void update();

  void update_menu_items(wxCommandEvent& event) { update_menu_items(); }
  void update_menu_items();

  void show_user_message(std::string);
  void show_user_message(std::string, int);
  void clear_user_message();
  void clear_user_message_after(int);
  int clear_user_message_seconds() { return clear_msg_secs; }
  int clear_user_message_id() { return clear_msg_id; }

  void transmit_selected();
  void send_program_change();
  void copy_file_to_synth();
  void copy_within_synth();
  void move_within_synth();

private:
  Sleigh &sleigh;
  wxString file_path;
  wxMenuBar *menu_bar;
  FrameListView *file_programs_wxlist;
  FrameListView *sledge_programs_wxlist;
  wxButton *xmit_button;
  wxButton *pc_button;
  wxButton *xfer_button;
  wxButton *copy_button;
  wxButton *move_button;
  int clear_msg_secs;
  int clear_msg_id;

  void OnNew(wxCommandEvent& event);
  void OnOpen(wxCommandEvent& event);
  void OnSave(wxCommandEvent& event);
  void OnSaveAs(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  void transmit_selected(wxCommandEvent &_event) { transmit_selected(); }
  void send_program_change(wxCommandEvent &_event) { send_program_change(); }
  void copy_file_to_synth(wxCommandEvent &_event) { copy_file_to_synth(); }
  void copy_within_synth(wxCommandEvent &_event) { copy_within_synth(); }
  void move_within_synth(wxCommandEvent &_event) { move_within_synth(); }

  void make_frame_panels();
  void make_menu_bar();

  void state_changed(wxCommandEvent &event);

  void update_lists();
  void update_buttons();

  int get_program_number(const char * const prompt);

  wxDECLARE_EVENT_TABLE();
};

#endif /* WX_FRAME_H */
