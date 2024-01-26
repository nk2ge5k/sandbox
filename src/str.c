#include "str.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "file.h"

String stringMake(size_t len) {
  String result = {
      .offset = 0,
      .len = 0,
      .v = NULL,
  };

  char *data = malloc(sizeof(char) * len);

  if (data == NULL) {
    return result;
  }

  result.v = data;
  result.len = len;

  return result;
}

String stringMakeFrom(char *data) {
  String result = {
      .offset = 0,
      .len = strlen(data),
      .v = data,
  };

  return result;
}

String stringMakeFromFile(FILE *file) {
  String result = {
      .offset = 0,
      .len = 0,
      .v = NULL,
  };

  if (NULL == file) {
    return result;
  }

  size_t file_size = fileSize(file);
  if (file_size <= 0) {
    return result;
  }

  result = stringMake(file_size);

  int64_t nread = fileReadIntoString(&result, file);
  if (nread < 0) {
    errorf("Failed to read file\n");
  }
  if ((size_t)nread != file_size) {
    errorf("Short read %lld != %ld\n", nread, file_size);
  }

  result.len = nread;

  return result;
}

void stringCopy(char *dst, String src) {
  memcpy(dst, src.v + src.offset, src.len);
  dst[src.len] = '\0';
}

void stringFree(String str) { free(str.v); }

// TODO(nk2ge5k): may be should replace return vaule type with int64_t?
int stringIndexOf(String str, char ch) {
  for (size_t i = 0; i < str.len; i++) {
    if (ch == str.v[str.offset + i]) {
      return i;
    }
  }
  return -1;
}

int stringIndexOfAfter(String str, size_t offset, char ch) {
  for (size_t i = offset + 1; i < str.len; i++) {
    if (ch == str.v[str.offset + i]) {
      return i;
    }
  }
  return -1;
}

String stringSliceAt(String str, size_t at) {
  assert(at < str.len);

  String result = {
      .offset = str.offset + at,
      .len = str.len - at,
      .v = str.v,
  };

  return result;
}

String stringSlice(String str, size_t start, size_t len) {
  assert(start < str.len);

  String result = {
      .offset = str.offset + start,
      .len = len,
      .v = str.v,
  };

  return result;
}

char stringCharAt(String str, size_t index) {
  assert(index < str.len);
  return str.v[str.offset + index];
}

bool stringIsEmpty(String str) { return str.len == 0; }

void stringPrintln(String str) { printf(PRSTR "\n", STRING_FMT(str)); }
