#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include "sleigh_test.h"
#include "sledge_program_test.h"

// ================ running tests ================

using namespace std;

test_results results;
vector<void (*)()> test_funcs;

void test_results_init() {
  results.num_tests = results.num_errors = 0;
}

void test_passed() {
  ++results.num_tests;
  printf(".");
}

void test_failed() {
  ++results.num_tests;
  ++results.num_errors;
  printf("*");
}

int test_num_errors() {
  return results.num_errors;
}

void print_time() {
  rusage usage;
  getrusage(RUSAGE_SELF, &usage);
  printf("\n\nFinished in %ld.%06ld seconds\n", usage.ru_utime.tv_sec,
         (long)usage.ru_utime.tv_usec);
}

void print_results() {
  printf("\nTests run: %d, tests passed: %d, tests failed: %d\n",
         results.num_tests, results.num_tests - results.num_errors,
         results.num_errors);
  printf("done\n");
}

void run_tests() {
  vector<void (*)()>::iterator i;

  for (i = test_funcs.begin(); i != test_funcs.end(); ++i)
    (*i)();
}

void register_test(void (*test_func)()) {
  test_funcs.push_back(test_func);
}

void run_tests_and_print_results() {
  run_tests();
  print_time();
  print_results();
}

// ================ main ================

int main(int argc, const char **argv) {
  register_test(test_sledge_program);
  run_tests_and_print_results();
  exit(results.num_errors == 0 ? 0 : 1);
  return 0;
}
