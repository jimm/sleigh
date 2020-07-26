#include "../consts.h"
#include "../sledge_program.h"
#include "frame_list_view.h"
#include "events.h"

#define CW 10

const char * const COLUMN_HEADERS[] = {
  "Prog #", "Name", "Category"
};
const int COLUMN_WIDTHS[] = {
  5*CW, 16*CW, 6*CW
};

/*
 * A FrameListView is a wxListView that tells the frame to update its menu
 * whenever an item is selected/deselected.
 */
FrameListView::FrameListView(SledgeProgram *progs, wxWindow *parent, wxWindowID id,
                             wxPoint pos, wxSize size, long style)
  : programs(progs), wxListView(parent, id, pos, size, style)
{
}

std::vector<int> FrameListView::selected_indexes() {
  std::vector<int> selected;
  long index = GetFirstSelected();
  while (index != -1) {
    selected.push_back((int)index);
    index = GetNextSelected(index);
  }
  return selected;
}

void FrameListView::select_indexes(int start, int len) {
  for (int i = 0; i < NUM_SLEDGE_PROGRAMS-1; ++i) {
    bool selected = i >= start && i < (start + len);
    Select((long)i, selected);
  }
}

void FrameListView::update() {
  ClearAll();
  for (int i = 0; i < sizeof(COLUMN_HEADERS) / sizeof(const char * const); ++i) {
    InsertColumn(i, COLUMN_HEADERS[i]);
    SetColumnWidth(i, COLUMN_WIDTHS[i]);
  }
  for (int i = 0; i < NUM_SLEDGE_PROGRAMS-1; ++i) {
    InsertItem(i, wxString::Format("%03d", i+1));
    SetItem(i, 1, programs[i].name_str());
    SetItem(i, 2, programs[i].category_str());
  }
}

// If `event` is a list (de)selection event, broadcast a
// Frame_SelectionUpdate event.
bool FrameListView::TryAfter(wxEvent &event) {
  wxEventType type = event.GetEventType();
  if (type == wxEVT_LIST_ITEM_SELECTED || type == wxEVT_LIST_ITEM_DESELECTED) {
    wxCommandEvent e(Frame_SelectionUpdate, GetId());
    wxPostEvent(GetParent(), e);
  }
  return wxListView::TryAfter(event);
}
