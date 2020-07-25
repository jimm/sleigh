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

  EVT_MENU(ID_TransmitSelected, Frame::transmit_selected)
  EVT_MENU(ID_SendProgramChange, Frame::send_program_change)
  EVT_MENU(ID_CopyFileToSynth, Frame::copy_file_to_synth)
  EVT_MENU(ID_CopyWithinSynth, Frame::copy_within_synth)
  EVT_MENU(ID_MoveWithinSynth, Frame::move_within_synth)

  EVT_BUTTON(ID_TransmitSelected, Frame::transmit_selected)
  EVT_BUTTON(ID_SendProgramChange, Frame::send_program_change)
  EVT_BUTTON(ID_CopyFileToSynth, Frame::copy_file_to_synth)
  EVT_BUTTON(ID_CopyWithinSynth, Frame::copy_within_synth)
  EVT_BUTTON(ID_MoveWithinSynth, Frame::move_within_synth)

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
  show_user_message("No Sledge file loaded", 15);
}

void Frame::make_frame_panels() {
  wxGridBagSizer *const sizer = new wxGridBagSizer();

  // TODO lists should be reports: prog num, name, category

  file_programs_wxlist = new wxListBox(this, wxID_ANY, wxDefaultPosition,
                                       wxSize(200, 400), 0, nullptr, wxLB_MULTIPLE);
  sizer->Add(file_programs_wxlist, POS(0, 0), SPAN(8, 3), wxEXPAND);

  int button_row = 3;
  sizer->Add(new wxButton(this, ID_TransmitSelected, "Xmit Selected"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(new wxButton(this, ID_TransmitSelected, "Program Change"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(new wxButton(this, ID_TransmitSelected, ">>"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(new wxButton(this, ID_TransmitSelected, "Copy"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(new wxButton(this, ID_TransmitSelected, "Move"), POS(button_row++, 3), SPAN(1, 1));

  sledge_programs_wxlist = new wxListBox(this, wxID_ANY, wxDefaultPosition,
                                         wxSize(200, 400), 0, nullptr, wxLB_MULTIPLE);
  sizer->Add(sledge_programs_wxlist, POS(0, 4), SPAN(8, 3), wxEXPAND);

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

// ================ actions ================

void Frame::transmit_selected() {
}

void Frame::send_program_change() {
}

void Frame::copy_file_to_synth() {
}

void Frame::copy_within_synth() {
}

void Frame::move_within_synth() {
}

// ================ standard menu items ================

void Frame::OnAbout(wxCommandEvent &_event) {
  wxMessageBox("Sleigh, the Studologic Sledge patch organizer.\n"
               "v1.0.0\n"
               "Jim Menard, jim@jimmenard.com\n"
               "https://github.com/jimm/sledge/wiki",
               "About Sleigh", wxOK | wxICON_INFORMATION);
}

void Frame::OnNew(wxCommandEvent &_event) {
}

void Frame::OnOpen(wxCommandEvent &_event) {
  wxFileName fname(file_path);

  wxFileDialog openFileDialog(this, "Open Sledge system exclusive file",
                              fname.GetPath(), fname.GetFullName(),
                              "Sledge sysex files|*",
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
  update_lists();
  update_buttons();
  update_menu_items();
}

void Frame::update_lists() {
  update_list(file_programs_wxlist, &sleigh.editor->file_programs[0]);
  update_list(sledge_programs_wxlist, &sleigh.editor->sledge.programs[0]);
}

void Frame::update_list(wxListBox *lbox, SledgeProgram *programs) {
  lbox->Clear();

  wxArrayString names;
  for (int i = 0; i < NUM_SLEDGE_PROGRAMS; ++i)
    names.Add(programs[i].name_str());
  lbox->InsertItems(names, 0);
}

void Frame::update_buttons() {
  // TODO disable move from file to synth if nothing selected on either side
  // TODO disable transmit if nothing selecdted on synth side
}

void Frame::update_menu_items() {
  // file menu
  menu_bar->FindItem(wxID_NEW, nullptr)->Enable(false);
  menu_bar->FindItem(wxID_SAVEAS, nullptr)->Enable(!file_path.empty());

  // sledge menu
  // if no file load
  // if no progs selected, disable xmit selected
}
