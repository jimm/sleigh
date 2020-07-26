#ifndef SLEDGE_PROGRAM_H
#define SLEDGE_PROGRAM_H

#include "consts.h"
#include "types.h"

class SledgeProgram {
public:
  byte sysex;                   // f0
  byte waldorf;                 // WALDORF_MANUFACTURER_ID
  byte header[3];               // 15 00 10
  byte prog_high;               // 00-7f; bits 7-13
  byte prog_low;                // 00-7f; bits 0-6
  byte data[SLEDGE_PROGRAM_MIDDLE_DATA_LEN];
  byte sixteen_fours[16];       // filled with 04
  byte seventeen_forty_fours[17]; // filled with 44
  byte three_zeroes[3];         // 00
  byte name[SLEDGE_PROGRAM_NAME_LEN]; // ASCII padded with spaces
  byte category;                // SLEDGE_CATEGORY_*
  byte data2[4];                // three zeroes, then one byte
  byte eox;                     // f7

  SledgeProgram();     // Calls initialize()

  void initialize();   // Modifies this program to become the "Init" program
  void update();       // Call this when data changes
  const char * const name_str() { return name_string; }
  const char * const category_str() { return category_string; }

  void set_program_number(int n) { prog_high = (n & 0x3f80) >> 7; prog_low = n & 0x7f; }
  int program_number() { return (prog_high << 7) + prog_low; }

private:
  char name_string[SLEDGE_PROGRAM_NAME_LEN + 1];
  char category_string[5];
};


#endif /* SLEDGE_PROGRAM_H */
