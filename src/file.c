#include "file.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

size_t file_size_from_fd(const int fd) {
  struct stat statbuf;
  if (fstat(fd, &statbuf) == -1) {
    return 0;
  }
  return statbuf.st_size;
}

size_t file_size(const char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
    return 0;
  }

  size_t size = file_size_from_fd(fd);
  close(fd);

  return size;
}

size_t file_read_into(void *dst, FILE *file, const size_t length) {
  size_t nread = 0;
  size_t n;

  while (nread < length) {
    n = fread(dst + nread, 1, length - nread, file);
    if (n == 0) {
      if (ferror(file)) {
        fprintf(stderr, "[ERROR]: failed to read file\n");
        return 0;
      }
      break;
    }
    nread += n;
  }

  return nread;
}

uint8_t *file_read_all_uint8(const char *filename, size_t *size) {
  size_t fsize = file_size(filename);
  uint8_t *data = malloc(fsize + 1);
  memset(data, 0, fsize + 1);

  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "[ERROR]: failed to open the file %s: %s\n", filename,
            strerror(errno));
    return NULL;
  }

  ssize_t nread = file_read_into(data, file, fsize);
  if (nread < 0) {
    fprintf(stderr, "[ERROR]: Failed to read file %s: %s\n", filename,
            strerror(errno));
    goto error;
  }

  if ((size_t)nread != fsize) {
    fprintf(stderr, "[ERROR]: Short-read: %ld != %lu\n", nread, fsize);
    goto error;
  }

  // NOTE(nk2ge5k): zero-terminated string
  if (size != NULL) {
    *size = nread;
  }

  fclose(file);
  return data;

error:
  fprintf(stderr, "[EROR]: failed to read the file %s\n", filename);

  fclose(file);
  free(data);
  return NULL;
}
