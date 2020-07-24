#include <wx/defs.h>
#include <wx/filename.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/gbsizer.h>
#include <unistd.h>
#include "frame.h"
#include "../sleigh.h"
#include "../editor.h"

#define POS(row, col) wxGBPosition(row, col)
#define SPAN(rowspan, colspan) wxGBSpan(rowspan, colspan)

#define LIST_WIDTH 200
#define TALL_LIST_HEIGHT 300
#define SHORT_LIST_HEIGHT 200
#define CLOCK_BPM_HEIGHT 16
#define NOTES_WIDTH 200

wxDEFINE_EVENT(Frame_MenuUpdate, wxCommandEvent);

wxBEGIN_EVENT_TABLE(Frame, wxFrame)
  EVT_MENU(wxID_NEW,  Frame::OnNew)
  EVT_MENU(wxID_OPEN,  Frame::OnOpen)
  EVT_MENU(wxID_SAVE,  Frame::OnSave)
  EVT_MENU(wxID_SAVEAS,  Frame::OnSaveAs)
  EVT_MENU(wxID_ABOUT, Frame::OnAbout)

  EVT_COMMAND(wxID_ANY, Frame_MenuUpdate, Frame::update_menu_items)
wxEND_EVENT_TABLE()

void *frame_clear_user_message_thread(void *gui_vptr) {
  Frame *gui = (Frame *)gui_vptr;
  int clear_user_message_id = gui->clear_user_message_id();

  sleep(gui->clear_user_message_seconds());

  // Only clear the window if the id hasn't changed
  if (gui->clear_user_message_id() == clear_user_message_id)
    gui->clear_user_message();
  return nullptr;
}

Frame::Frame(const wxString& title, Sleigh &sleigh_ref)
  : wxFrame(NULL, wxID_ANY, title), sleigh(sleigh_ref)
{
  make_frame_panels();
  make_menu_bar();
  CreateStatusBar();
  show_user_message("No KeyMaster file loaded", 15);
}

void Frame::make_frame_panels() {
  wxGridBagSizer *const sizer = new wxGridBagSizer();

  // sizer->Add(make_set_list_songs_panel(this), POS(0, 0), SPAN(3, 1), wxEXPAND);
  // sizer->Add(make_song_patches_panel(this), POS(0, 1), SPAN(3, 1), wxEXPAND);
  // sizer->Add(make_clock_panel(this), POS(0, 2), SPAN(1, 1), wxEXPAND);
  // sizer->Add(make_notes_panel(this), POS(1, 2), SPAN(2, 1), wxEXPAND);

  // sizer->Add(make_patch_conns_panel(this), POS(3, 0), SPAN(1, 3), wxEXPAND);

  // sizer->Add(make_set_lists_panel(this), POS(4, 0), SPAN(1, 1), wxEXPAND);
  // sizer->Add(make_messages_panel(this), POS(4, 1), SPAN(1, 1), wxEXPAND);
  // sizer->Add(make_triggers_panel(this), POS(4, 2), SPAN(1, 1), wxEXPAND);


  wxBoxSizer * const outer_border_sizer = new wxBoxSizer(wxVERTICAL);
  outer_border_sizer->Add(sizer, wxSizerFlags().Expand().Border());
  SetSizerAndFit(outer_border_sizer);
}


void Frame::make_menu_bar() {
  wxMenu *menu_file = new wxMenu;
  menu_file->Append(wxID_NEW, "&New Project\tCtrl-Shift-N", "Create a new project");
  menu_file->Append(wxID_OPEN);
  menu_file->Append(wxID_SAVE);
  menu_file->Append(wxID_SAVEAS);
  menu_file->AppendSeparator();
  menu_file->Append(wxID_EXIT);

  wxMenu *menu_edit = new wxMenu;

  wxMenu *menu_sledge = new wxMenu;
  menu_sledge->Append(ID_TransmitSelected, "&Transmit Selected Programs\nCtrl-T", "Transmit selected programs to the Sledge");
  menu_sledge->Append(ID_SendProgramChange, "Send a &Program Change\n", "Send a program change to the Sledge");
  menu_file->AppendSeparator();
  menu_sledge->Append(ID_CopyFileToSynth, "Copy &File to Sledge\n", "Copy file contents to Sledge list");
  menu_sledge->Append(ID_CopyWithinSynth, "&Copy Within Sledge\n", "Copy programs to another place in the Sledge list");
  menu_sledge->Append(ID_MoveWithinSynth, "&Move Within Sledge\n", "Move programs to another place in the Sledge list");

  wxMenu *menu_help = new wxMenu;
  menu_help->Append(wxID_ABOUT);

  menu_bar = new wxMenuBar;
  menu_bar->Append(menu_file, "&File");
  menu_bar->Append(menu_edit, "&Edit");
  menu_bar->Append(menu_sledge, "&Sledge");
  menu_bar->Append(menu_help, "&Help");
  SetMenuBar(menu_bar);
#if defined(__WXMAC__)
  menu_bar->OSXGetAppleMenu()->SetTitle("Sleigh");
#endif
}

// ================ messaging ================

void Frame::show_user_message(string msg) {
  SetStatusText(msg.c_str());
}

void Frame::show_user_message(string msg, int secs) {
  SetStatusText(msg.c_str());
  clear_user_message_after(secs);
}

void Frame::clear_user_message() {
  SetStatusText("");
}

void Frame::clear_user_message_after(int secs) {
  clear_msg_secs = secs;
  clear_msg_id++;

  pthread_t pthread;
  pthread_create(&pthread, 0, frame_clear_user_message_thread, this);
}

// ================ standard menu items ================

void Frame::OnAbout(wxCommandEvent &_event) {
  wxMessageBox("KeyMaster, the MIDI processing and patching system.\n"
               "v1.0.0\n"
               "Jim Menard, jim@jimmenard.com\n"
               "https://github.com/jimm/keymaster/wiki",
               "About KeyMaster", wxOK | wxICON_INFORMATION);
}

void Frame::OnNew(wxCommandEvent &_event) {
}

void Frame::OnOpen(wxCommandEvent &_event) {
  wxFileName fname(file_path);

  wxFileDialog openFileDialog(this, "Open KeyMaster file",
                              fname.GetPath(), fname.GetFullName(),
                              "KeyMaster files (*.km)|*.km",
                              wxFD_OPEN|wxFD_FILE_MUST_EXIST);
  if (openFileDialog.ShowModal() != wxID_CANCEL) {
    file_path = openFileDialog.GetPath();
    load(file_path);
  }
  update_menu_items();
}

void Frame::OnSave(wxCommandEvent &event) {
  if (file_path.empty()) {
    OnSaveAs(event);
    return;
  }
  save();
  update_menu_items();
}

void Frame::OnSaveAs(wxCommandEvent &_event) {
  wxFileName fname(file_path);

  wxFileDialog file_dialog(this, "Save KeyMaster file",
                           fname.GetPath(), fname.GetFullName(),
                           "KeyMaster files (*.km)|*.km",
                           wxFD_SAVE);
  if (file_dialog.ShowModal() != wxID_CANCEL) {
    file_path = file_dialog.GetPath();
    save();
    update_menu_items();
  }
}

// ================ helpers ================

void Frame::initialize() {
  update();
}

void Frame::load(wxString path) {
  if (access(path, F_OK) != 0) {
    wxString err = wxString::Format("File '%s' does not exist", path);
    wxLogError(err);
    return;
  }

  sleigh.editor->load(path);
  show_user_message(string(wxString::Format("Loaded %s", path).c_str()), 15);
  file_path = path;

  update();                     // must come after start
}

void Frame::save() {
  if (file_path.empty())
    return;

  sleigh.editor->save(file_path);
}

void Frame::update() {
  update_menu_items();
}

void Frame::update_menu_items() {
  // file menu
  menu_bar->FindItem(wxID_SAVEAS, nullptr)->Enable(!file_path.empty());

  // sledge menu
  // if no file load
  // if no progs selected, disable xmit selected
}
