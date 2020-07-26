#include <wx/defs.h>
#include <wx/filename.h>
#include <wx/gbsizer.h>
#include <wx/numdlg.h>
#include <unistd.h>
#include "frame.h"
#include "frame_list_ctrl.h"
#include "../sleigh.h"
#include "../editor.h"

#define CW 10
#define POS(row, col) wxGBPosition(row, col)
#define SPAN(rowspan, colspan) wxGBSpan(rowspan, colspan)

#define LIST_WIDTH 200
#define TALL_LIST_HEIGHT 300
#define SHORT_LIST_HEIGHT 200
#define CLOCK_BPM_HEIGHT 16
#define NOTES_WIDTH 200

wxDEFINE_EVENT(Frame_StateUpdate, wxCommandEvent);

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

  EVT_COMMAND(wxID_ANY, Frame_StateUpdate, Frame::state_changed)
wxEND_EVENT_TABLE()

const char * const COLUMN_HEADERS[] = {
  "Prog #", "Name", "Category"
};
const int COLUMN_WIDTHS[] = {
  4*CW, 16*CW, 4*CW
};

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

  file_programs_wxlist = new FrameListCtrl(this, ID_FileList, wxDefaultPosition,
                                           wxSize(200, 400), wxLC_REPORT);
  sizer->Add(file_programs_wxlist, POS(0, 0), SPAN(8, 3), wxEXPAND);

  int button_row = 3;
  sizer->Add(xmit_button = new wxButton(this, ID_TransmitSelected, "Xmit Selected"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(pc_button = new wxButton(this, ID_SendProgramChange, "Program Change"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(xfer_button = new wxButton(this, ID_CopyFileToSynth, ">>"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(copy_button = new wxButton(this, ID_CopyWithinSynth, "Copy"), POS(button_row++, 3), SPAN(1, 1));
  sizer->Add(move_button = new wxButton(this, ID_MoveWithinSynth, "Move"), POS(button_row++, 3), SPAN(1, 1));

  sledge_programs_wxlist = new FrameListCtrl(this, ID_SynthList, wxDefaultPosition,
                                             wxSize(200, 400), wxLC_REPORT);
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
  vector<int> selected = selected_indexes(file_programs_wxlist);
  sleigh.editor->transmit_selected(selected);
}

void Frame::send_program_change() {
  int prog_num = get_program_number("Send Program Change");
  if (prog_num >= 0 && prog_num <= 999)
    sleigh.sledge.program_change(prog_num);
}

void Frame::copy_file_to_synth() {
  int prog_num = get_program_number("Copy from File to Sledge");
  if (prog_num < 0)
    return;

  vector<int> selected = selected_indexes(file_programs_wxlist);
  sleigh.editor->copy_file_to_synth(selected, prog_num);
}

void Frame::copy_within_synth() {
  int prog_num = get_program_number("Copy Within Sledge");
  if (prog_num < 0)
    return;

  vector<int> selected = selected_indexes(file_programs_wxlist);
  sleigh.editor->copy_within_synth(selected, prog_num);
}

void Frame::move_within_synth() {
  int prog_num = get_program_number("Move Within Synth");
  if (prog_num < 0)
    return;

  vector<int> selected = selected_indexes(file_programs_wxlist);
  sleigh.editor->move_within_synth(selected, prog_num);
}

// ================ standard menu items ================

void Frame::OnAbout(wxCommandEvent &_event) {
  wxMessageBox("Sleigh, the Studologic Sledge patch organizer\n"
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

  update();
}

void Frame::save() {
  if (file_path.empty())
    return;

  sleigh.editor->save(file_path);
}

void Frame::state_changed(wxCommandEvent &event) {
  update_buttons();
  update_menu_items();
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

void Frame::update_list(wxListCtrl *lbox, SledgeProgram *programs) {
  lbox->ClearAll();
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    lbox->InsertColumn(i, COLUMN_HEADERS[i]);
    lbox->SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }
  for (int i = 0; i < NUM_SLEDGE_PROGRAMS; ++i) {
    lbox->InsertItem(i, wxString::Format("%03d", i+1));
    lbox->SetItem(i, 1, programs[i].name_str());
    lbox->SetItem(i, 2, programs[i].category_str());
  }
}

void Frame::update_buttons() {
  bool any_file_selected = file_programs_wxlist->GetSelectedItemCount() > 0;
  bool any_synth_selected = sledge_programs_wxlist->GetSelectedItemCount() > 0;
  xfer_button->Enable(any_file_selected);
  xmit_button->Enable(any_synth_selected);
  copy_button->Enable(any_synth_selected);
  move_button->Enable(any_synth_selected);
}

void Frame::update_menu_items() {
  // file menu
  menu_bar->FindItem(wxID_NEW, nullptr)->Enable(false);
  menu_bar->FindItem(wxID_SAVEAS, nullptr)->Enable(!file_path.empty());

  // sledge menu
  bool any_file_selected = file_programs_wxlist->GetSelectedItemCount() > 0;
  bool any_synth_selected = sledge_programs_wxlist->GetSelectedItemCount() > 0;
  menu_bar->FindItem(ID_TransmitSelected, nullptr)->Enable(any_synth_selected);
  menu_bar->FindItem(ID_CopyFileToSynth, nullptr)->Enable(any_file_selected);
  menu_bar->FindItem(ID_CopyWithinSynth, nullptr)->Enable(any_synth_selected);
  menu_bar->FindItem(ID_MoveWithinSynth, nullptr)->Enable(any_synth_selected);
}

// Returns -1 if cancelled
int Frame::get_program_number(const char * const prompt) {
  wxNumberEntryDialog dialog(this, prompt, "Destination", "Target Sledge Program", 0, 0, 999);
  return (dialog.ShowModal() == wxID_OK) ? dialog.GetValue() : -1;
}

std::vector<int> Frame::selected_indexes(FrameListCtrl *list) {
  vector<int> selected;
  long index = -1;

  while (true) {
    index = list->GetNextItem(index, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (index == -1)
        break;
    selected.push_back((int)index);
}
  return selected;
}
