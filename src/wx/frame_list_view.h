#ifndef FRAME_LIST_VIEW_H
#define FRAME_LIST_VIEW_H

#include <vector>
#include "wx/listctrl.h"

class FrameListView : public wxListView {
public:
  FrameListView(wxWindow *parent, wxWindowID id, wxPoint pos, wxSize size,
                long style);

  std::vector<int> selected_indexes();
  void select_indexes(int start, int len);

protected:
  virtual bool TryAfter(wxEvent &event);
};

#endif /* FRAME_LIST_VIEW_H */
