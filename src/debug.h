#ifndef DEBUG_H

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define __FILENAME__                                                           \
  (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define put_prefix_(dst) fprintf(dst, "[%s:%d]: ", __FILENAME__, __LINE__)

#define STD_ERROR strerror(errno)

#define errorf(format, ...)                                                    \
  do {                                                                         \
    put_prefix_(stderr);                                                       \
    fprintf(stderr, "ERROR " format, ##__VA_ARGS__);                           \
  } while (0)

#if !DEBUG

#define debugf(format, ...)                                                    \
  do {                                                                         \
  } while (0)

#define assertf(expression, format, ...)                                       \
  do {                                                                         \
  } while (0)

#else

#include <stdlib.h>

#define debugf(format, ...)                                                    \
  do {                                                                         \
    put_prefix_(stderr);                                                       \
    fprintf(stderr, "DEBUG " format, ##__VA_ARGS__);                           \
  } while (0)

#define assertf(expression, format, ...)                                       \
  do {                                                                         \
    if (!(expression)) {                                                       \
      errorf("Assertion (" #expression ") failed: " format, ##__VA_ARGS__);    \
      abort();                                                                 \
    }                                                                          \
  } while (0)

#endif

#endif
