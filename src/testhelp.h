#if TEST_BUILD

#ifndef TESTHELP_H
#define TESTHELP_H

#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "types.h"

// TODO(nk2ge5k): why in redis these need to be extern?
int failed_tests_count;
int total_tests_count;

#define TEST(name) void test##name(void)
#define RUN_TEST(name) test##name();

#define CLI_GREEN(val) "\x1b[32m" val "\x1b[0m"
#define CLI_RED(val) "\x1b[31m" val "\x1b[0m"

#define test_expect(cond, format, ...)                                         \
  do {                                                                         \
    total_tests_count++;                                                       \
    if (cond) {                                                                \
      printf("[" CLI_GREEN("PASSED") "] ");                                    \
    } else {                                                                   \
      printf("[" CLI_RED("FAILED") "] ");                                      \
      failed_tests_count++;                                                    \
    }                                                                          \
    put_prefix_(stdout);                                                       \
    printf(format "\n", ##__VA_ARGS__);                                        \
  } while (0)

#define test_report()                                                          \
  do {                                                                         \
    printf("%d tests, %d passed, %d failed\n", total_tests_count,              \
           total_tests_count - failed_tests_count, failed_tests_count);        \
    if (failed_tests_count) {                                                  \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#endif // TESTHELP_H
#endif // TEST_BUILD
