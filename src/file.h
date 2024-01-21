#ifndef FILE_H
#define FILE_H

#include <stddef.h>
#include <stdint.h>

// file_size returns file size.
// TODO(nk2ge5k): returns 0 if any error encountered, and its kinda missleading.
size_t file_size(const char *filename);

// file_read_all reads entire file and returns its contents as zero-terminated
// string.
uint8_t * file_read_all_uint8(const char *filename, size_t *size);

#endif
