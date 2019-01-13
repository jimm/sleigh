#include "test_helper.h"
#include "sledge_program_test.h"
#include "../src/init_program.h"

void test_sledge_program_set_program_number() {
  SledgeProgram sp;
  init_program(&sp);

  sp.set_program_number(0);
  tassert(sp.prog_high == 0, 0);
  tassert(sp.prog_low == 0, 0);

  sp.set_program_number(0x7f);
  tassert(sp.prog_high == 0, 0);
  tassert(sp.prog_low == 0x7f, 0);

  sp.set_program_number(0x81);
  tassert(sp.prog_high == 1, 0);
  tassert(sp.prog_low == 1, 0);
}

void test_sledge_program_program_number() {
  SledgeProgram sp;
  init_program(&sp);

  sp.set_program_number(0);
  tassert(sp.program_number() == 0, 0);

  sp.set_program_number(0x7f);
  tassert(sp.program_number() == 0x7f, 0);
  tassert(sp.prog_low == 0x7f, 0);

  sp.set_program_number(0x81);
  tassert(sp.program_number() == 0x81, 0);
}

void test_sledge_program_update() {
  SledgeProgram sp;
  init_program(&sp);

  memcpy(sp.name, "abcdefghijklmnopqrstuvwxyz", SLEDGE_NAME_LEN);
  sp.category = 3;
  sp.update();
  tassert(strcmp(sp.name_str(), "abcdefghijklmnop") == 0, sp.name_str());
  tassert(strcmp(sp.category_str(), "BASS") == 0, 0);

  memcpy(sp.name, "abc def         ", SLEDGE_NAME_LEN);
  sp.category = 9;
  sp.update();
  tassert(strcmp(sp.name_str(), "abc def") == 0, 0);
  tassert(strcmp(sp.category_str(), "PAD") == 0, 0);
}


void test_sledge_program() {
  test_run(test_sledge_program_set_program_number);
  test_run(test_sledge_program_program_number);
  test_run(test_sledge_program_update);
}
