#ifndef UTILS_H
#define UTILS_H

#include "sledge.h"

typedef unsigned char byte;

char * c_name(SledgeProgram *prog);

void dump_hex(byte *bytes, size_t size, const char * const msg);

#endif /* UTILS_H */