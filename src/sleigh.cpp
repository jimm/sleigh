#include <CoreMIDI/MIDIServices.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "sleigh.h"
#include "sledge.h"
#include "editor.h"
#include "utils.h"
#include "curses/gui.h"
#include "init_program.h"

#define CFSTRING_BUF_SIZE 512

Sleigh sleigh;

void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con,
                    void *src_conn_ref_con)
{
  ((Sledge *)ref_con)->receive_midi(pktlist);
}

void cleanup_midi() {
  OSStatus err;

  err = MIDIPortDisconnectSource(sleigh.my_in_port, sleigh.sledge_out_end_ref);
  if (err != 0)
    printf("MIDIPortDisconnectSource error: %d\n", err);

  err = MIDIPortDispose(sleigh.my_in_port);
  if (err != 0)
    printf("MIDIPortDispose error: %d\n", err);

  err = MIDIPortDispose(sleigh.my_out_port);
  if (err != 0)
    printf("MIDIPortDispose error: %d\n", err);
}

void cleanup() {
  cleanup_midi();
  cleanup_debug();
}

// ****************************************************************

Sleigh::~Sleigh() {
  delete sledge;
}

// Returns a C string that is a newly allocated copy of cf_str.
char * Sleigh::copy_cfstring(CFStringRef cf_str) {
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
const char * Sleigh::cfstring_cstr_ptr(CFStringRef cf_str) {
  return CFStringGetCStringPtr(cf_str, kCFStringEncodingASCII);
}

// Returns new CFString ref. Don't forget to call CFRelease(cf_str) when
// you're done with it.
CFStringRef Sleigh::cstr_to_cfstring(const char *str) {
  CFStringRef cf_str;
  return CFStringCreateWithCString(kCFAllocatorDefault, str, kCFStringEncodingASCII);
}

// Copies name property of MIDIObject into buf.
void Sleigh::name_of(MIDIObjectRef ref, char *buf) {
  CFStringRef pvalue;
  MIDIObjectGetStringProperty(ref, kMIDIPropertyName, &pvalue);
  CFStringGetCString(pvalue, buf, CFSTRING_BUF_SIZE, 0);
  CFRelease(pvalue);
}

void Sleigh::print_sources_and_destinations() {
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

void Sleigh::init_midi(struct opts *opts) {
  OSStatus err;
  CFStringRef cf_str;

  sledge = new Sledge(opts->channel);

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
  err = MIDIInputPortCreate(my_client_ref, cf_str, midi_read_proc, sledge, &my_in_port);
  if (err != 0)
    printf("MIDIInputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // My output port
  cf_str = cstr_to_cfstring("Sleigh Sledge Program Mapper Output");
  err = MIDIOutputPortCreate(my_client_ref, cf_str, &my_out_port);
  if (err != 0)
    printf("MIDIOutputPortCreate error: %d\n", err);
  CFRelease(cf_str);

  // Connect Sledge output to my input
  // 0 is conn ref_con
  err = MIDIPortConnectSource(my_in_port, sledge_out_end_ref, 0);
  if (err != 0)
    printf("MIDIPortConnectSource error: %d\n", err);

  sledge->set_output(my_out_port, sledge_in_end_ref);
}

void Sleigh::initialize(struct opts *opts) {
  init_debug(opts->debug);
  debug("Sleigh::initialize\n");
  atexit(cleanup);
  init_init_program();
  init_midi(opts);
}

void Sleigh::run() {
  debug("Sleigh::run\n");
  Editor editor(sledge, getenv("SLEIGH_SYSEX_DIR"));
  GUI gui(&editor);
  // The editor must be added as an observer before the GUI is, so that
  // programs received from the Sledge update the editor's current index
  // before the GUI is redrawn.
  sledge->add_observer(&editor);
  sledge->add_observer(&gui);
  editor.add_observer(&gui);
  gui.run();
  sledge->remove_observer(&gui);
}

void Sleigh::usage(const char *prog_name) {
  printf("usage: %s\n", basename((char *)prog_name));
  puts("[-l] [-i N] [-o N] [-n] [-h] file");
  puts("");
  puts("    -l or --list-ports  List all attached MIDI ports");
  puts("    -i or --input N     Input number");
  puts("    -o or --output N    Output number");
  puts("    -c or --channel N   Sledge MIDI channel (1-16)");
  puts("    -n or --no-midi     No MIDI (ignores bad/unknown MIDI ports)");
  puts("    -d or --debug       Debug");
  puts("    -h or --help        This help");
}

void Sleigh::parse_command_line(int argc, char * const *argv, struct opts *opts) {
  int ch;
  char *prog_name = argv[0];
  static struct option longopts[] = {
    {"list", no_argument, 0, 'l'},
    {"channel", required_argument, 0, 'c'},
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"no-midi", no_argument, 0, 'n'},
    {"debug", no_argument, 0, 'd'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  while ((ch = getopt_long(argc, argv, "lc:i:o:ndh", longopts, 0)) != -1) {
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
    case 'd':
      opts->debug = true;
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

  sleigh.parse_command_line(argc, argv, &opts);
  argc -= optind;
  argv += optind;

  if (opts.list_devices) {
    sleigh.print_sources_and_destinations();
    exit(0);
  }

  sleigh.initialize(&opts);
  sleigh.run();

  exit(0);
  return 0;
}
