#ifndef WX_APP_H
#define WX_APP_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
 #include <wx/wx.h>
#endif
#include "../sleigh.h"

class Frame;

class App: public wxApp {
public:
  App();
  ~App();

  void show_user_message(std::string msg);
  void show_user_message(std::string msg, int clear_secs);

private:
  Sleigh sleigh;
  Frame *frame;
  wxString command_line_path;
  struct opts opts;

  virtual bool OnInit();
  virtual void OnInitCmdLine(wxCmdLineParser &);
  virtual bool OnCmdLineParsed(wxCmdLineParser &);
  virtual int OnExit();
};

App *app_instance();

#endif /* WX_APP_H */
