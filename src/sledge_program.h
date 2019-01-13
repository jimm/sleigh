#ifndef SLEDGE_PROGRAM_H
#define SLEDGE_PROGRAM_H

#define SLEDGE_PROGRAM_SYSEX_LEN 0x188
#define SLEDGE_NAME_LEN 16
#define SLEDGE_MIDDLE_DATA_LEN \
  (SLEDGE_PROGRAM_SYSEX_LEN - 7 - SLEDGE_NAME_LEN - 17 - 3 - 16 - 6)

#define WALDORF_MANUFACTURER_ID 0x3e

#define SLEDGE_CATEGORY_INIT 0x00
#define SLEDGE_CATEGORY_ARP  0x01
#define SLEDGE_CATEGORY_ATMO 0x02
#define SLEDGE_CATEGORY_BASS 0x03
#define SLEDGE_CATEGORY_DRUM 0x04
#define SLEDGE_CATEGORY_FX   0x05
#define SLEDGE_CATEGORY_KEYS 0x06
#define SLEDGE_CATEGORY_LEAD 0x07
#define SLEDGE_CATEGORY_MONO 0x08
#define SLEDGE_CATEGORY_PAD  0x09
#define SLEDGE_CATEGORY_PERC 0x0a
#define SLEDGE_CATEGORY_POLY 0x0b
#define SLEDGE_CATEGORY_SEQ  0x0c

typedef unsigned char byte;

class SledgeProgram {
public:
  byte sysex;                   // f0
  byte waldorf;                 // WALDORF_MANUFACTURER_ID
  byte header[3];               // 15 00 10
  byte prog_high;               // 00-7f; bits 7-13
  byte prog_low;                // 00-7f; bits 0-6
  byte data[SLEDGE_MIDDLE_DATA_LEN];
  byte sixteen_fours[16];       // filled with 04
  byte seventeen_forty_fours[17]; // filled with 44
  byte three_zeroes[3];         // 00
  byte name[SLEDGE_NAME_LEN];   // ASCII padded with spaces
  byte category;                // SLEDGE_CATEGORY_*
  byte data2[4];                // three zeroes, then one byte
  byte eox;                     // f7

  void update();                // Call this when data changes
  const char * const name_str() { return name_string; }
  const char * const category_str() { return category_string; }

  void set_program_number(int n) { prog_high = (n & 0x3f80) >> 7; prog_low = n & 0x7f; }
  int program_number() { return (prog_high << 7) + prog_low; }

private:
  char name_string[SLEDGE_NAME_LEN + 1];
  char category_string[5];
};


#endif /* SLEDGE_PROGRAM_H */
