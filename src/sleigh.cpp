#include <CoreMIDI/MIDIServices.h>
#include <getopt.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include "sleigh.h"
#include "sledge.h"
#include "editor.h"
#include "utils.h"

#define CFSTRING_BUF_SIZE 512

void midi_read_proc(const MIDIPacketList *pktlist, void *ref_con,
                    void *src_conn_ref_con)
{
  ((Sledge *)ref_con)->receive_midi(pktlist);
}

// ****************************************************************

Sleigh::~Sleigh() {
  delete editor;
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

void Sleigh::start(struct opts *opts) {
  debug("Sleigh::start\n");
  init_midi(opts);
  editor = new Editor(sledge, getenv("SLEIGH_SYSEX_DIR"));
}

void Sleigh::stop() {
  debug("Sleigh::stop\n");
  OSStatus err = MIDIPortDisconnectSource(my_in_port, sledge_out_end_ref);
  if (err != 0)
    fprintf(stderr, "MIDIPortDisconnectSource error: %d\n", err);

  err = MIDIPortDispose(my_in_port);
  if (err != 0)
    fprintf(stderr, "MIDIPortDispose error: %d\n", err);

  err = MIDIPortDispose(my_out_port);
  if (err != 0)
    fprintf(stderr, "MIDIPortDispose error: %d\n", err);

  cleanup_debug();
}

void Sleigh::init_midi(struct opts *opts) {
  OSStatus err;
  CFStringRef cf_str;

  sledge.channel = opts->channel;

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
  err = MIDIInputPortCreate(my_client_ref, cf_str, midi_read_proc, &sledge, &my_in_port);
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

  sledge.set_output(my_out_port, sledge_in_end_ref);
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
