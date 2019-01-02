#include <string.h>
#include "consts.h"
#include "init_program.h"
#include "sledge_program.h"

// MUST initialize this by calling init_init_program.
SledgeProgram init_program;

static byte ip_header[3] = {0x15, 0x00, 0x10};
static byte data[] = {
    0x02,0x40,0x40,0x40,0x42,0x60,0x00,0x00,
    0x01,0x40,0x00,0x40,0x00,0x00,0x00,0x00,
    0x00,0x40,0x40,0x40,0x42,0x60,0x01,0x00,
    0x01,0x40,0x00,0x40,0x00,0x00,0x00,0x00,
    0x00,0x34,0x40,0x40,0x42,0x60,0x02,0x00,
    0x01,0x40,0x00,0x40,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,
    0x00,0x14,0x00,0x00,0x00,0x7f,0x01,0x7f,
    0x01,0x7f,0x01,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x01,0x7f,0x40,
    0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x40,
    0x40,0x00,0x40,0x00,0x00,0x40,0x00,0x40,
    0x00,0x00,0x7f,0x40,0x00,0x00,0x00,0x00,
    0x00,0x00,0x40,0x40,0x40,0x00,0x40,0x00,
    0x40,0x40,0x00,0x40,0x00,0x00,0x03,0x00,
    0x00,0x7f,0x72,0x00,0x40,0x00,0x00,0x00,
    0x00,0x00,0x14,0x40,0x40,0x00,0x7f,0x7f,
    0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,
    0x00,0x10,0x35,0x40,0x64,0x00,0x40,0x64,
    0x00,0x64,0x6e,0x00,0x0f,0x40,0x7f,0x7f,
    0x00,0x32,0x40,0x00,0x00,0x00,0x00,0x40,
    0x00,0x40,0x40,0x00,0x00,0x28,0x40,0x00,
    0x00,0x00,0x00,0x40,0x00,0x40,0x40,0x00,
    0x00,0x1e,0x40,0x00,0x00,0x00,0x00,0x40,
    0x00,0x40,0x40,0x01,0x00,0x40,0x00,0x00,
    0x7f,0x32,0x00,0x00,0x7f,0x00,0x00,0x00,
    0x00,0x40,0x00,0x00,0x7f,0x34,0x7f,0x00,
    0x7f,0x00,0x00,0x00,0x00,0x40,0x00,0x00,
    0x40,0x40,0x40,0x40,0x40,0x40,0x00,0x00,
    0x00,0x40,0x00,0x00,0x40,0x40,0x40,0x40,
    0x40,0x40,0x00,0x00,0x01,0x10,0x0d,0x06,
    0x40,0x19,0x05,0x02,0x40,0x00,0x00,0x00,
    0x40,0x00,0x00,0x00,0x40,0x01,0x01,0x40,
    0x01,0x00,0x40,0x01,0x00,0x40,0x03,0x00,
    0x40,0x03,0x00,0x40,0x03,0x00,0x40,0x1a,
    0x00,0x40,0x1a,0x00,0x40,0x1a,0x00,0x40,
    0x00,0x00,0x40,0x00,0x00,0x40,0x00,0x00,
    0x40,0x00,0x00,0x40,0x00,0x00,0x40,0x00,
    0x00,0x40,0x00,0x00,0x40,0x10,0x64,0x00,
    0x00,0x0f,0x08,0x05,0x00,0x00,0x00,0x00,
    0x0c,0x00,0x00,0x0f,0x00,0x00,0x37
};
static byte data2[] = {0x00, 0x00, 0x00, 0x52};

void init_init_program() {
  init_program.sysex = SYSEX;
  init_program.waldorf = WALDORF_MANUFACTURER_ID;
  memcpy(&init_program.header, ip_header, sizeof(ip_header));
  init_program.prog_high = 0;
  init_program.prog_low = 0;
  memcpy(&init_program.data, data, sizeof(data));
  memset(&init_program.sixteen_fours, 0x04, sizeof(init_program.sixteen_fours));
  memset(&init_program.seventeen_forty_fours, 0x44,
         sizeof(init_program.seventeen_forty_fours));
  memset(&init_program.three_zeroes, 0, sizeof(init_program.three_zeroes));
  strncpy((char *)&init_program.name, "Init            ", 16);
  init_program.category = SLEDGE_CATEGORY_INIT;
  memcpy(&init_program.data2, &data2, sizeof(data2));
  init_program.eox = EOX;
}
