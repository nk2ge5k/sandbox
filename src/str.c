#include "str.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void stringFree(String str) { free(str.v); }

// TODO(nk2ge5k): may be should replace with int64_t?
int stringIndexOf(String str, char ch) {
  for (size_t i = 0; i < str.len; i++) {
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
