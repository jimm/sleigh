#include <wx/cmdline.h>
#include "app.h"
#include "frame.h"
#include "../utils.h"
#include "../sleigh.h"

static const wxCmdLineEntryDesc g_cmdLineDesc [] = {
  { wxCMD_LINE_SWITCH, "l", "list-devices", "Display MIDI Devices" },
  { wxCMD_LINE_SWITCH, "t", "testing", "Testing mode" },
  { wxCMD_LINE_SWITCH, "d", "debug", "Debug mode" },
  { wxCMD_LINE_OPTION, "c", "channel", "Sledge MIDI channel (default is 1)", wxCMD_LINE_VAL_NUMBER },
  { wxCMD_LINE_OPTION, "i", "in-device", "Input device number", wxCMD_LINE_VAL_NUMBER },
  { wxCMD_LINE_OPTION, "o", "out-device", "Output device number", wxCMD_LINE_VAL_NUMBER },
  { wxCMD_LINE_PARAM, nullptr, nullptr, "Sledge system exclusive file", wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL },
  { wxCMD_LINE_NONE }
};

static App *a_instance = nullptr;

App *app_instance() {
  return a_instance;
}

App::App() {
  a_instance = this;
}

App::~App() {
  if (a_instance == this)
    a_instance = nullptr;
}

void App::show_user_message(string msg) {
  frame->show_user_message(msg);
}

void App::show_user_message(string msg, int clear_secs) {
  frame->show_user_message(msg, clear_secs);
}

// This is the wxWidgets equivalent of the "main" function.
bool App::OnInit() {
  if (!wxApp::OnInit())
    return false;

  init_debug(opts.debug);
  sleigh.start(&opts);
  frame = new Frame("Sleigh", sleigh);
  frame->Show(true);
  SetTopWindow(frame);
  if (!command_line_path.IsEmpty())
    frame->load(command_line_path);
  return true;
}

void App::OnInitCmdLine(wxCmdLineParser& parser) {
  parser.SetDesc(g_cmdLineDesc);
  parser.SetSwitchChars (wxT("-"));
}

bool App::OnCmdLineParsed(wxCmdLineParser& parser) {
  long long_val;

  if (parser.Found("l")) {
    sleigh.print_sources_and_destinations();
    return false;
  }
  opts.testing = parser.Found("t");
  opts.debug = parser.Found("d");
  if (parser.Found("c", &long_val))
      opts.channel = (int)long_val - 1;
  if (parser.Found("i", &long_val))
    opts.input_num = (int)long_val;
  if (parser.Found("o", &long_val))
    opts.output_num = (int)long_val;

  if (parser.GetParamCount() > 0)
    command_line_path = parser.GetParam(0);

  return true;
}

int App::OnExit() {
  sleigh.stop();
  return wxApp::OnExit();
}
