#include "../consts.h"
#include "frame_list_view.h"
#include "events.h"

/*
 * A FrameListView is a wxListView that tells the frame to update its menu
 * whenever an item is selected/deselected.
 */
FrameListView::FrameListView(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size, long style)
  : wxListView(parent, id, pos, size, style)
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

// If `event` is a list (de)selection event, broadcast a Frame_StateUpdate
// event.
bool FrameListView::TryAfter(wxEvent &event) {
  wxEventType type = event.GetEventType();
  if (type == wxEVT_LIST_ITEM_SELECTED || type == wxEVT_LIST_ITEM_DESELECTED) {
    wxCommandEvent e(Frame_StateUpdate, GetId());
    wxPostEvent(GetParent(), e);
  }
  return wxListView::TryAfter(event);
}
