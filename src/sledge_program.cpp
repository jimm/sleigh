#include <string.h>
#include "sledge_program.h"

const char * const SLEDGE_CATEGORY_NAMES[] = {
  "INIT", "ARP", "ATMO", "BASS",
  "DRUM", "FX", "KEYS", "LEAD",
  "MONO", "PAD", "PERC", "POLY",
  "SEQ "
};

void SledgeProgram::update() {
  memcpy(name_string, name, SLEDGE_NAME_LEN);
  for (char *p = &name_string[SLEDGE_NAME_LEN-1]; p >= name_string && *p == ' '; --p)
    *p = 0;
  strncpy(category_string, SLEDGE_CATEGORY_NAMES[category], 5);
}
