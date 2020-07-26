#ifndef FRAME_LIST_VIEW_H
#define FRAME_LIST_VIEW_H

#include <vector>
#include "wx/listctrl.h"

class SledgeProgram;

class FrameListView : public wxListView {
public:
  FrameListView(SledgeProgram *programs, wxWindow *parent, wxWindowID id,
                wxPoint pos, wxSize size, long style);

  std::vector<int> selected_indexes();
  void select_indexes(int start, int len);

  void update();

protected:
  SledgeProgram *programs;
  bool first_update;

  virtual bool TryAfter(wxEvent &event);
};

#endif /* FRAME_LIST_VIEW_H */
