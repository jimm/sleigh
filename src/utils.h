#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include "sledge.h"

typedef unsigned char byte;

extern FILE *debug_fp;

void dump_hex(byte *bytes, size_t size, const char * const msg);

void init_debug(bool debug);
void cleanup_debug();
void debug_timestamp();

#define debug(fmt, ...) { \
    debug_timestamp(); \
    fprintf(debug_fp, fmt, ##__VA_ARGS__); \
    fflush(debug_fp); \
  }

#endif /* UTILS_H */
