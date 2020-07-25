#ifndef SLEDGE_H
#define SLEDGE_H

#include <CoreMIDI/MIDIServices.h>
#include "consts.h"
#include "observable.h"
#include "sledge_program.h"

using namespace std;

typedef unsigned char byte;

class Sledge : public Observable {
public:
  byte channel;
  SledgeProgram programs[NUM_SLEDGE_PROGRAMS];

  Sledge(byte channel);         // channel 0-15
  Sledge() : channel(0) {}
  ~Sledge();

  void set_output(MIDIPortRef app_output_ref, MIDIEndpointRef synth_input_ref) {
    app_output_port = app_output_ref;
    sledge_input_endpoint = synth_input_ref;
  }

  void receive_midi(const MIDIPacketList *pktlist);

  void load_file(const char * const path);
  void save_file(const char * const path);
  void send_programs(int min, int max);

  OSStatus program_change(int prog_num); // prog_num 0-998

  OSStatus send_sysex(const byte * const sysex, UInt32 bytes_to_send);
  void dump_sysex(const char * const msg);

  int last_received_program() { return last_received_prog_num; }

private:
  MIDIPortRef app_output_port;
  MIDIEndpointRef sledge_input_endpoint;
  bool receiving_sysex;
  byte *sysex;
  size_t sysex_allocated_size;
  size_t sysex_length;
  int last_received_prog_num;

  void sysex_received();
  void clear_sysex_buffer();
  void append_sysex_byte(byte b);
  void send_notification();
};

#endif /* SLEDGE_H */
