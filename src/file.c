#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

i64 fileSize(FILE *file) {
  i64 size = -1;
  i64 cur = ftell(file); // saving current position

  if (fseek(file, 0, SEEK_END) < 0) {
    goto finish;
  }

  if ((size = ftell(file)) < 0) {
    goto finish;
  }

finish:
  // NOTE(nk2ge5k): no error cheking because i do not understand how to
  // handle this error if any other error occured.
  fseek(file, cur, SEEK_SET);
  return size;
}

i64 fileReadInto(void *dst, size_t length, FILE *file) {
  size_t nread = 0;

  while (nread < length) {
    size_t n = fread(dst + nread, 1, length - nread, file);
    if (n == 0) {
      break;
    }
    nread += n;
  }

  return nread;
}

i64 fileReadIntoString(String *dst, FILE *file) {
  i64 nread = fileReadInto(dst->v, dst->len, file);
  return nread;
}
