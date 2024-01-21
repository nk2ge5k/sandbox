#ifndef DEBUG_H

#include <stdio.h>

#ifndef DEBUG

#define debugf(format, ...)                                                    \
  do {                                                                         \
  } while (0)

#else

#define debugf(format, ...)                                                    \
  do {                                                                         \
    fprintf(stderr, "[" __FILE__ ":%d]: ", __LINE__);                          \
    fprintf(stderr, format, ##__VA_ARGS__);                                    \
  } while (0)

#endif

#endif
