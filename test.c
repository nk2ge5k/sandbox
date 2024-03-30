#if TEST_BUILD

#define STB_DS_IMPLEMENTATION
#include <stb_ds.h>

#include "testhelp.h"
#include "lnglat.h"

int main() {
  // TODO(nk2ge5k): this file must be easy to generate from source parsing
  RUN_TEST(LNGLAT_H);
  test_report();
}

#else

#include <stdio.h>

int main() {
  fprintf(stderr,
          "Running tests for non test binary - build with -DTEST_BUILD flag\n");
  return 1;
}

#endif
