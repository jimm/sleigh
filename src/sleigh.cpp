#include <CoreMIDI/MIDIServices.h>
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "sledge.h"

#define CFSTRING_BUF_SIZE 512

using namespace std;

typedef unsigned char byte;

struct opts {
  bool list_devices;
  bool testing;
  int channel;
  int input_num;
  int output_num;
} opts;

MIDIClientRef my_client_ref;
MIDIPortRef my_in_port;
MIDIEndpointRef sledge_in_end_ref;
MIDIEndpointRef sledge_out_end_ref;

void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con,
                    void *src_conn_ref_con)
{
  ((Sledge *)ref_con)->receive_midi(pktlist);
}

// Returns a C string that is a newly allocated copy of cf_str.
char * copy_cfstring(CFStringRef cf_str) {
  if (cf_str == 0)
    return 0;

  CFIndex length = CFStringGetLength(cf_str);
  CFIndex max_size =
    CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingASCII) + 1;
  char *buffer = (char *)malloc(max_size);
  if (CFStringGetCString(cf_str, buffer, max_size, kCFStringEncodingASCII))
    return buffer;

  free(buffer);
  return 0;
}

// Returns pointer to C string inside CFString.
const char * cfstring_cstr_ptr(CFStringRef cf_str) {
  return CFStringGetCStringPtr(cf_str, kCFStringEncodingASCII);
}

// Returns new CFString ref. Don't forget to call CFRelease(cf_str) when
// you're done with it.
CFStringRef cstr_to_cfstring(const char *str) {
  CFStringRef cf_str;
  return CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingASCII);
}

// Copies name property of MIDIObject into buf.
void name_of(MIDIObjectRef ref, char *buf) {
  CFStringRef pvalue;
  MIDIObjectGetStringProperty(ref, kMIDIPropertyName, &pvalue);
  CFStringGetCString(pvalue, buf, CFSTRING_BUF_SIZE, 0);
  CFRelease(pvalue);
}

void print_sources_and_destinations() {
  char val[CFSTRING_BUF_SIZE];

  ItemCount i, ndev = MIDIGetNumberOfSources();
  printf("Inputs:\n");
  for (i = 0; i < ndev; ++i) {
    MIDIEndpointRef end_ref = MIDIGetSource(i);
    name_of(end_ref, val);
    printf("%3ld: %s\n", i, val);
  }

  ndev = MIDIGetNumberOfDestinations();
  printf("Outputs\n");
  for (i = 0; i < ndev; ++i) {
    MIDIEndpointRef end_ref = MIDIGetDestination(i);
    name_of(end_ref, val);
    printf("%3ld: %s\n", i, val);
  }
}

void cleanup_midi() {
  OSStatus err;

  err = MIDIPortDisconnectSource(my_in_port, sledge_out_end_ref);
  if (err != 0)
    printf("MIDIPortDisconnectSource error: %d\n", err);

  err = MIDIPortDispose(my_in_port);
  if (err != 0)
    printf("MIDIPortDispose error: %d\n", err);
}

void cleanup() {
  cleanup_midi();
}

Sledge * init_midi(struct opts *opts) {
  OSStatus err;
  CFStringRef cf_str;

  Sledge *s = new Sledge(opts->channel);

  cf_str = cstr_to_cfstring("Sleigh Sledge Program Mapper");
  err = MIDIClientCreate(cf_str, 0, 0, &my_client_ref);
  if (err != 0)
    printf("MIDIClientCreate error: %d\n", err);
  CFRelease(cf_str);

  // Sledge endpoints
  sledge_in_end_ref = MIDIGetDestination(opts->input_num);
  sledge_out_end_ref = MIDIGetSource(opts->output_num);
  if (sledge_in_end_ref == 0)
    printf("error getting input destination %d\n", opts->input_num);
  if (sledge_out_end_ref == 0)
    printf("error getting output destination %d\n", opts->output_num);

  // My input port
  cf_str = cstr_to_cfstring("Sleigh Sledge Program Mapper Input");
  err = MIDIInputPortCreate(my_client_ref, cf_str, midi_read_proc, s, &my_in_port);
  if (err != 0)
    printf("MIDIInputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // Connect Sledge output to my input
  // 0 is conn ref_con
  err = MIDIPortConnectSource(my_in_port, sledge_out_end_ref, 0);
  if (err != 0)
    printf("MIDIPortConnectSource error: %d\n", err);

  s->set_input(my_in_port);
  s->set_output(sledge_in_end_ref);

  return s;
}

Sledge * initialize(struct opts *opts) {
  atexit(cleanup);
  return init_midi(opts);
}

void run(Sledge *s) {
  puts("not yet implemented");
}

void usage(const char *prog_name) {
  cerr << "usage: " << basename((char *)prog_name)
       << "[-l] [-i N] [-o N] [-n] [-h] file" << endl
       << endl
       << "    -l or --list-ports" << endl
       << "        List all attached MIDI ports" << endl
       << endl
       << "    -i or --input N" << endl
       << "        Input number" << endl
       << endl
       << "    -o or --output N" << endl
       << "        Output number" << endl
       << endl
       << "    -n or --no-midi" << endl
       << "        No MIDI (ignores bad/unknown MIDI ports)" << endl
       << endl
       << "    -h or --help" << endl
       << "        This help" << endl;
}

void parse_command_line(int argc, char * const *argv, struct opts *opts) {
  int ch, testing = false;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"list", no_argument, 0, 'l'},
    {"channel", required_argument, 0, 'c'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"no-midi", no_argument, 0, 'n'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  opts->list_devices = opts->testing = false;
  while ((ch = getopt_long(argc, argv, "lc:i:o:nh", longopts, 0)) != -1) {
    switch (ch) {
    case 'l':
      opts->list_devices = true;
      break;
    case 'c':
      opts->channel = atoi(optarg) - 1;
      break;
    case 'i':
      opts->input_num = atoi(optarg);
    case 'o':
      opts->output_num = atoi(optarg);
      break;
    case 'n':
      opts->testing = true;
      break;
    case 'h': default:
      usage(prog_name);
      exit(ch == '?' || ch == 'h' ? 0 : 1);
    }
  }
}

int main(int argc, char * const *argv) {
  struct opts opts;
  const char *prog_name = argv[0];

  parse_command_line(argc, argv, &opts);
  argc -= optind;
  argv += optind;

  if (opts.list_devices) {
    print_sources_and_destinations();
    exit(0);
  }

  Sledge *s = initialize(&opts);
  run(s);
  delete s;

  exit(0);
  return 0;
}
