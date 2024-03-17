#include "command_test_csv.h"

#include <assert.h>
#include <stdlib.h>

#include "csv.h"
#include "debug.h"
#include "file.h"
#include "str.h"
#include "types.h"

int commandTestCsv(int argc, char **argv) {
  if (argc < 2) {
    return 1;
  }

  FILE *file = fopen(argv[1], "r");
  if (file == NULL) {
    errorf("Failed to open file %s\n", argv[1]);
    return 1;
  }

  i64 file_size = fileSize(file);
  if (file_size <= 0) {
    errorf("Failed to get file size\n");
    goto error;
  }

  String content = stringMake(file_size); // @leak
  i64 nread = fileReadIntoString(&content, file);

  if (nread < 0) {
    errorf("Failed to read file\n");
    goto error;
  } else if (nread != file_size) {
    errorf("Short read " Fi64 " != " Fi64 "\n", nread, file_size);
    goto error;
  }

  String header_line = csvGetLine(&content);
  size_t ncolumns = csvCountColumns(header_line, COMMA);
  if (ncolumns == 0) {
    errorf("No columns found in the first line\n");
    goto error;
  }

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

  return 0;

error:
  fclose(file);
  return 1;
}
