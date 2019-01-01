#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "consts.h"
#include "sledge.h"
#include "utils.h"

#define SYSEX_CHUNK_SIZE 1024

static Sledge *sledge_instance;

Sledge *Sledge_instance() {
  return sledge_instance;
}

// ================ allocation ================

// channel 0-15
Sledge::Sledge(byte chan) : channel(chan), receiving_sysex(false) {
  sysex = (byte *)malloc(SYSEX_CHUNK_SIZE);
  sysex_allocated_size = SYSEX_CHUNK_SIZE;
  sysex_length = 0;
  sledge_instance = this;
}

Sledge::~Sledge() {
  free(sysex);
  if (sledge_instance == this)
    sledge_instance = 0;
}

void Sledge::receive_midi(const MIDIPacketList *packet_list) {
  const MIDIPacket *packet = &packet_list->packet[0];
  for (int i = 0; i < packet_list->numPackets; ++i) {
    for (int j = 0; j < packet->length; ++j) {
      byte b = packet->data[j];
      switch (b) {
      case SYSEX:
        clear_sysex_buffer();
        receiving_sysex = true;
        append_sysex_byte(b);
        break;
      case EOX:
        receiving_sysex = false;
        append_sysex_byte(b);
        sysex_received();
        break;
      default:
        if (receiving_sysex)
          append_sysex_byte(b);
        break;
      }
    }
    packet = MIDIPacketNext(packet);
  }
}

void Sledge::sysex_received() {
  if (sysex_length != SLEDGE_PROGRAM_SYSEX_LEN ||
      sysex[1] != WALDORF_MANUFACTURER_ID ||
      sysex[2] != 0x15 || sysex[3] != 0x00 || sysex[4] != 0x10)
    return;

  last_received_prog_num = sysex[5] * 0x80 + sysex[6];
  memcpy(&programs[last_received_prog_num], sysex, SLEDGE_PROGRAM_SYSEX_LEN);

  changed();
}

void Sledge::clear_sysex_buffer() {
  sysex_length = 0;
  receiving_sysex = false;
}

void Sledge::append_sysex_byte(byte b) {
  if (sysex_length == sysex_allocated_size) {
    sysex_allocated_size += sysex_allocated_size;
    sysex = (byte *)realloc(sysex, sysex_allocated_size);
  }
  sysex[sysex_length++] = b;
}

void Sledge::send_programs(int min, int max) {
  for (int i = min; i <= max; ++i)
    send_sysex((byte *)&programs[i], SLEDGE_PROGRAM_SYSEX_LEN);
}

OSStatus Sledge::send_sysex(const byte * const data, const UInt32 bytes_to_send) {
  MIDISysexSendRequest req;

  req.destination = sledge_input_endpoint;
  req.data = data;
  req.bytesToSend = bytes_to_send;
  req.complete = false;
  req.completionProc = 0;
  req.completionRefCon = 0;

  while (req.bytesToSend > 0) {
    OSStatus result = MIDISendSysex(&req);
    if (result != 0)
      return result;
  }
  return 0;
}

OSStatus Sledge::program_change(int prog_num) {
  int prog_high = (prog_num & 0x3f80) >> 7;
  int prog_low = prog_num & 0x7f;

  MIDIPacketList packet_list;
  packet_list.numPackets = 1;
  MIDIPacket *p = &packet_list.packet[0];
  p->timeStamp = 0;
  p->length = 5;
  p->data[0] = CONTROLLER + channel;
  p->data[1] = CC_BANK_SELECT_LSB;
  p->data[2] = prog_high;
  p->data[3] = PROGRAM_CHANGE + channel;
  p->data[4] = prog_low;

  debug("program change high %d, low %d, channel %d\n", prog_high, prog_low, channel);

  return MIDISend(app_output_port, sledge_input_endpoint, &packet_list);
  
}

void Sledge::dump_sysex(const char * const msg) {
  dump_hex(sysex, sysex_length, msg);
}
