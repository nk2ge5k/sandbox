#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>

// Format helpers for printf/sprintf... functions
#define PRSTR "%.*s"
#define PRSTR_RPAD(N) "%-" #N ".*s"
#define STRING_FMT(str) (int)str.len, (str.v + str.offset)
#define PSTRING_FMT(str) (int)str->len, (str->v + str->offset)

typedef struct String {
  // Offset in the data array where string starts
  //
  // NOTE(nk2ge5k): The idea behind this field is simple - reduce allocations
  // from creating new string while splitting string into chunks, but I am
  // not completely sold on this concept it may be possible that it makes
  // all operations unnecessary more complicated.
  //
  // TODO(nk2ge5k): may be use new string?
  size_t offset;
  // Length of the string
  size_t len;
  // String data
  char *v;
} String;

// stringMake allocates new string.
String stringMake(size_t len);

// stringMakeFrom creates new string from zero-terminated string.
String stringMakeFrom(char *data);

// stringMakeFromFile creates string that contains entire file's content.
// Otherwise returns empty string if command failed to read file.
String stringMakeFromFile(FILE *file);

// stringCopy copies string's content into character array.
// It probably will SEGFAULT if dst is to short.
void stringCopy(char *dst, String src);

// stringFree frees the memory space pointed by String v
void stringFree(String str);

// stringIndexOf returns first index of the character ch or -1 if character
// not found.
int stringIndexOf(String str, char ch);

// stringIndexOfAfter returns first index of the character ch after offset or 
// -1 if character not found.
int stringIndexOfAfter(String str, size_t offset, char ch);

// stringSlice returns slice of the string.
String stringSlice(String str, size_t start, size_t len);

// stringCharAt returns character at the index.
char stringCharAt(String str, size_t index);

// stringIsEmpty checks if string is empty.
bool stringIsEmpty(String str);

void stringPrintln(String str);

#endif
