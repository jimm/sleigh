#include <stdio.h>
#include "sledge.h"
#include "utils.h"

FILE *debug_fp;
char c_str_buf[SLEDGE_NAME_LEN + 1];

// Copies program name to the pre-allocated buffer c_str_buf, strips off the
// trailing spaces, and returns a pointer to that buffer. Obviously not
// thread safe.
char * c_name(SledgeProgram *prog) {
  memcpy(c_str_buf, prog->name, SLEDGE_NAME_LEN);
  for (char *p = &c_str_buf[SLEDGE_NAME_LEN-1]; p >= c_str_buf && *p == ' '; --p)
    *p = 0;
  return c_str_buf;
}

void dump_hex(byte *bytes, size_t size, const char * const msg) {
  printf("%s\n", msg);
  if (bytes == 0) {
    puts("<null>");
    return;
  }
  if (size == 0) {
    puts("<empty>");
    return;
  }
  size_t offset = 0;
  while (size > 0) {
    int chunk_len = 8 > size ? size : 8;
    printf("%08lx:", offset);
    for (int i = 0; i < chunk_len; ++i) {
      printf(" %02x", bytes[i]);
    }
    for (int i = chunk_len; i < 8; ++i) {
      printf("   ");
    }
    printf(" ");
    for (int i = 0; i < chunk_len; ++i) {
      printf("%c", (bytes[i] >= 32 && bytes[i] <= 127) ? bytes[i] : '.');
    }
    puts("");
    bytes += chunk_len;
    size -= chunk_len;
    offset += chunk_len;
  }
}

void init_debug(bool debug) {
  debug_fp = fopen(debug ? "/tmp/sleigh_debug.log" : "/dev/null", "a");
}

void cleanup_debug() {
  fclose(debug_fp);
}

void debug_timestamp() {
  char tstamp[32];
  time_t t = time(0);
  
  strftime(tstamp, 32, "%Y-%m-%d %H:%M:%S", localtime(&t));
  fprintf(debug_fp, "[%s] ", tstamp);
}
