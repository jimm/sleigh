#ifndef SLEIGH_H
#define SLEIGH_H

#include <pthread.h>
#include "sledge.h"

typedef unsigned char byte;

struct opts {
  bool list_devices;
  bool testing;
  bool debug;
  int channel;
  int input_num;
  int output_num;

  opts() : list_devices(false), testing(false), debug(false),
           channel(0), input_num(0), output_num(0) {}
};

class Editor;

class Sleigh {
public:
  Sledge sledge;
  Editor *editor;
  bool running;
  MIDIClientRef my_client_ref;
  MIDIPortRef my_in_port;
  MIDIPortRef my_out_port;
  MIDIEndpointRef sledge_in_end_ref;
  MIDIEndpointRef sledge_out_end_ref;

  ~Sleigh();

  void parse_command_line(int argc, char * const *argv, struct opts *opts);
  void print_sources_and_destinations();
  void initialize(struct opts *opts);
  void start(struct opts *opts);
  void stop();

private:
  char * copy_cfstring(CFStringRef cf_str);
  const char * cfstring_cstr_ptr(CFStringRef cf_str);
  CFStringRef cstr_to_cfstring(const char *str);
  void name_of(MIDIObjectRef ref, char *buf);

  void init_midi(struct opts *opts);
  void usage(const char *prog_name);
};

#endif /* SLEIGH_H */
