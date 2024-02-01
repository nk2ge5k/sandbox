#ifndef STRING_H
#define STRING_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define STR(data) stringMakeFrom(data)

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

// stringMakeFromLen returns new string from char array with given length.
String stringMakeFromLen(char *data, size_t len);

// stringMakeFromFile creates string that contains entire file's content.
// Otherwise returns empty string if command failed to read file.
String stringMakeFromFile(FILE *file);

// stringCopy copies string's content into character array.
// It probably will SEGFAULT if dst is to short.
void stringCopy(char *dst, String src);

// stringFree frees the memory space pointed by String v
void stringFree(String str);

// stringEqual checks if two strings are equal to each other.
bool stringEqual(String a, String b);

// stringHasPrefix checks if string starts with prefix
bool stringHasPrefix(String str, String prefix);

// stringIndexOf returns first index of the character ch or -1 if character
// not found.
int stringIndexOf(String str, char ch);

// stringIndexOfAfter returns first index of the character ch after offset or
// -1 if character not found.
int stringIndexOfAfter(String str, size_t offset, char ch);

// stringIndexOfString searches for the start of given substring.
// Returns -1 if needle is not found in the haystack or needle's length greater
// then haystack's length.
int stringIndexOfString(String haystack, String needle);

// stringSlice returns slice of the string.
String stringSlice(String str, size_t start, size_t len);

// stringCharAt returns character at the index.
char stringCharAt(String str, size_t index);

// stringIsEmpty checks if string is empty.
bool stringIsEmpty(String str);

// stringTrimPrefix removes prefix from the string if string starts from
// given prefix otherwise does nothing and returns original string
String stringTrimPrefix(String str, String prefix);

void stringPrintln(String str);

void stringDebugPrintln(String str);

#endif
