#include "command_test_csv.h"

#include <assert.h>
#include <stdlib.h>

#include "csv.h"
#include "debug.h"
#include "file.h"
#include "str.h"

int commandTestCsv(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    errorf("Failed to open file %s\n", argv[1]);
    return 1;
  }

  int64_t file_size = fileSize(file);
  if (file_size <= 0) {
    errorf("Failed to get file size\n");
    goto error;
  }

  String content = stringMake(file_size); // @leak
  int64_t nread = fileReadIntoString(&content, file);

  if (nread < 0) {
    errorf("Failed to read file\n");
    goto error;
  } else if (nread != file_size) {
    errorf("Short read %ld != %ld\n", nread, file_size);
    goto error;
  }

  String header_line = csvGetLine(&content);
  size_t ncolumns = csvCountColumns(header_line, COMMA);
  assert(ncolumns > 0);

  String *header = malloc(sizeof(String) * ncolumns); // @leak
  String *line = malloc(sizeof(String) * ncolumns);   // @leak

  size_t nvalues = csvGetValues(header, ncolumns, header_line, COMMA);
  for (size_t i = 1; !stringIsEmpty(content); i++) {
    nvalues = csvGetValues(line, ncolumns, csvGetLine(&content), COMMA);

    printf("========== LINE %ld ==========\n", i);
    for (size_t j = 0; j < nvalues; j++) {
      printf(PRSTR_RPAD(10) " | " PRSTR "\n", STRING_FMT(header[j]),
             STRING_FMT(line[j]));
    }
  }

  fclose(file);
  return 0;

error:
  fclose(file);
  return 1;
}
