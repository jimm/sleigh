#ifndef SLEDGE_H
#define SLEDGE_H

#include <CoreMIDI/MIDIServices.h>
#include "sledge_program.h"

using namespace std;

typedef unsigned char byte;

class Sledge {
public:
  byte channel;
  SledgeProgram programs[1000];

  Sledge(byte channel);
  ~Sledge();

  void set_input(MIDIEndpointRef input_ref) { input = input_ref; }
  void set_output(MIDIEndpointRef output_ref) { output = output_ref; }

  void receive_midi(const MIDIPacketList *pktlist);

  void load_file(const char * const path);
  void save_file(const char * const path);
  void send_programs(int min, int max);

  OSStatus send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  void dump_sysex(const char * const msg);

private:
  MIDIEndpointRef input;
  MIDIEndpointRef output;
  bool receiving_sysex;
  byte *sysex;
  size_t sysex_allocated_size;
  size_t sysex_length;

  void sysex_received();
  void clear_sysex_buffer();
  void append_sysex_byte(byte b);
};

#endif /* SLEDGE_H */
