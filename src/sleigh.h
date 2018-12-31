#ifndef SLEIGH_H
#define SLEIGH_H

#include <pthread.h>
#include "sledge.h"
#include "portmidi.h"

typedef unsigned char byte;

struct opts {
  bool list_devices;
  bool testing;
  bool debug;
  int channel;
  int input_num;
  int output_num;
} opts;

class Sleigh {
public:
  Sledge *sledge;
  bool running;
  MIDIClientRef my_client_ref;
  MIDIPortRef my_in_port;
  MIDIEndpointRef sledge_in_end_ref;
  MIDIEndpointRef sledge_out_end_ref;

  ~Sleigh();

  void parse_command_line(int argc, char * const *argv, struct opts *opts);
  void print_sources_and_destinations();
  void initialize(struct opts *opts);
  void run();

private:
  char * copy_cfstring(CFStringRef cf_str);
  const char * cfstring_cstr_ptr(CFStringRef cf_str);
  CFStringRef cstr_to_cfstring(const char *str);
  void name_of(MIDIObjectRef ref, char *buf);

  void list_devices(const char *title, const PmDeviceInfo *infos[], int num_devices);
  void init_midi(struct opts *opts);
  void usage(const char *prog_name);
};

#endif /* SLEIGH_H */