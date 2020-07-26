#include <string.h>
#include "test_helper.h"
#include "../src/sledge_program.h"

#define CATCH_CATEGORY "[program]"

TEST_CASE("initialize", CATCH_CATEGORY) {
  SledgeProgram sp;

  REQUIRE(sp.sysex == SYSEX);
  REQUIRE(strncmp((const char *)sp.name, "Init            ", SLEDGE_PROGRAM_NAME_LEN) == 0);
}

TEST_CASE("set program number", CATCH_CATEGORY) {
  SledgeProgram sp;

  sp.set_program_number(0);
  REQUIRE(sp.prog_high == 0);
  REQUIRE(sp.prog_low == 0);

  sp.set_program_number(0x7f);
  REQUIRE(sp.prog_high == 0);
  REQUIRE(sp.prog_low == 0x7f);

  sp.set_program_number(0x81);
  REQUIRE(sp.prog_high == 1);
  REQUIRE(sp.prog_low == 1);
}

TEST_CASE("program number", CATCH_CATEGORY) {
  SledgeProgram sp;

  sp.set_program_number(0);
  REQUIRE(sp.program_number() == 0);

  sp.set_program_number(0x7f);
  REQUIRE(sp.program_number() == 0x7f);
  REQUIRE(sp.prog_low == 0x7f);

  sp.set_program_number(0x81);
  REQUIRE(sp.program_number() == 0x81);
}

TEST_CASE("program update", CATCH_CATEGORY) {
  SledgeProgram sp;

  memcpy(sp.name, "abcdefghijklmnopqrstuvwxyz", SLEDGE_PROGRAM_NAME_LEN);
  sp.category = 3;
  sp.update();
  REQUIRE(strcmp(sp.name_str(), "abcdefghijklmnop") == 0);
  REQUIRE(strcmp(sp.category_str(), "BASS") == 0);

  memcpy(sp.name, "abc def         ", SLEDGE_PROGRAM_NAME_LEN);
  sp.category = 9;
  sp.update();
  REQUIRE(strcmp(sp.name_str(), "abc def") == 0);
  REQUIRE(strcmp(sp.category_str(), "PAD") == 0);
}
