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

  if (len == 0) {
    return result;
  }

  char *data = malloc(sizeof(char) * len);
  if (data == NULL) {
    return result;
  }

  result.v = data;
  result.len = len;

  return result;
}

String stringMakeFrom(char *data) {
  assert(data != NULL);

  String result = {
      .offset = 0,
      .len = strlen(data),
      .v = data,
  };

  return result;
}

String stringMakeFromLen(char *data, size_t len) {
  assert(data != NULL);

  String result = {
      .offset = 0,
      .len = len,
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

  i64 file_size = fileSize(file);
  if (file_size <= 0) {
    return result;
  }

  result = stringMake(file_size);

  i64 nread = fileReadIntoString(&result, file);
  if (nread < 0) {
    errorf("Failed to read file\n");
  }
  if (nread != file_size) {
    errorf("Short read " Fi64 " != " Fi64 "\n", nread, file_size);
  }

  result.len = nread;

  return result;
}

void stringCopy(char *dst, String src) {
  memcpy(dst, src.v + src.offset, src.len);
  dst[src.len] = '\0';
}

void stringFree(String str) { free(str.v); }

bool stringEqual(String a, String b) {
  return a.len == b.len && strncmp(a.v + a.offset, b.v + b.offset, a.len) == 0;
}

bool stringHasPrefix(String str, String prefix) {
  return str.len >= prefix.len &&
         stringEqual(stringSlice(str, 0, prefix.len), prefix);
}

i32 stringIndexOf(String str, char ch) {
  for (size_t i = 0; i < str.len; i++) {
    if (ch == str.v[str.offset + i]) {
      return i;
    }
  }
  return -1;
}

i32 stringIndexOfAfter(String str, size_t offset, char ch) {
  for (size_t i = offset + 1; i < str.len; i++) {
    if (ch == str.v[str.offset + i]) {
      return i;
    }
  }
  return -1;
}

i32 stringIndexOfString(String haystack, String needle) {
  if (haystack.len < needle.len || needle.len == 0) {
    // errorf("haystack.len < needle.len || needle.len == 0");
    return -1;
  }

  size_t start_offset = haystack.offset;

  char first = stringCharAt(needle, 0);
  while (haystack.len >= needle.len) {
    i32 cur = stringIndexOf(haystack, first);
    if (cur == -1) {
      // errorf("stringIndexOf(" PRSTR ", first) == -1\n",
      // STRING_FMT(haystack));
      return -1;
    }

    haystack.offset += cur;
    haystack.len -= cur;

    if (stringHasPrefix(haystack, needle)) {
      return (i32)(haystack.offset - start_offset);
    }

    haystack.offset += 1;
    haystack.len -= 1;
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
  assertf(start <= str.len, "String{.offfset = %ld, .len = %ld} %ld <= %ld",
          str.offset, str.len, start, str.len);

  String result = {
      .offset = str.offset + start,
      .len = len,
      .v = str.v,
  };

  return result;
}

String stringTrimPrefix(String str, String prefix) {
  if (stringHasPrefix(str, prefix)) {
    return stringSlice(str, prefix.len, str.len - prefix.len);
  }
  return str;
}

char stringCharAt(String str, size_t index) {
  assertf(index < str.len, "%ld < %ld", index, str.len);
  return str.v[str.offset + index];
}

bool stringIsEmpty(String str) { return str.len == 0; }

void stringPrintln(String str) { printf(PRSTR "\n", STRING_FMT(str)); }

void stringDebugPrintln(String str) {
  printf("String{ .offset = %lu, .len = %lu, .data = \"%s\"}\n", str.offset,
         str.len, str.v);
}
