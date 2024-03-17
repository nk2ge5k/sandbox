#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "str.h"
#include "types.h"

// fileSize returns file size. Otherwise, -1 is returned and errno is set
// to indicate the error.
i64 fileSize(FILE *file);

// fileReadInto reads entire file into dst, returns number of bytes read from
// file. Otherwise, -1 is returned and errno is set to indicate the error.
i64 fileReadInto(void *dst, size_t length, FILE *file);

// fileReadInto reads entire file into dst string, returns number of bytes
// read from file. Otherwise, -1 is returned and errno is set to indicate the
// error.
// NOTE(nk2ge5k): function wont modify length of the dst string in case of the
// short write.
i64 fileReadIntoString(String *dst, FILE *file);

#endif
