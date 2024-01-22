#include "csv.h"

#include "constans.h"
#include "types.h"

String csvGetLine(String *src) {
  String result = {
      .offset = src->offset,
      .len = 0,
      .v = src->v,
  };

  int eol_pos = stringIndexOf(*src, EOL);
  if (eol_pos < 0) {
    // If have not been able to find end of the line then returning entire strig
    result.offset = src->offset;

    src->offset += src->len;
    src->len = 0;

    return result;
  }

  result.len = eol_pos;

  if ((size_t)(eol_pos + 1) > src->len) {
    // TODO(nk2ge5k): is it correct?
    src->offset = src->len - 1;
    src->len = 0;
  } else {
    src->offset += eol_pos + 1;
    src->len -= eol_pos + 1;
  }

  return result;
}

#define DOUBLE_QUOTE '"'

internal bool isCharOrEnd(String str, size_t index, char ch) {
  return (index >= str.len) || stringCharAt(str, index) == ch;
}

// findEndQuote
internal int findEndQuote(String line, char delim) {
  if (stringCharAt(line, 0) != DOUBLE_QUOTE) {
    return 0;
  }

  line = stringSlice(line, 1, line.len - 1);

  int end_quote_pos = stringIndexOf(line, DOUBLE_QUOTE);
  if (end_quote_pos >= 0 && isCharOrEnd(line, end_quote_pos + 1, delim)) {
    // NOTE(nk2ge5k): +1 because of the offset increment before.
    return end_quote_pos + 1;
  }

  return 0;
}

String csvGetValue(String *line, char delim) {
  int end_quote_pos = findEndQuote(*line, delim);
  if (end_quote_pos > 0) {
    String result = stringSlice(*line, 1, end_quote_pos - 1);
    line->offset += end_quote_pos + 2;
    line->len -= end_quote_pos + 2;

    return result;
  }

  int delim_pos = stringIndexOf(*line, delim);
  if (delim_pos < 0) {
    String result = {
        .offset = line->offset,
        .len = line->len,
        .v = line->v,
    };

    line->len = 0;
    line->offset = line->len - 1;

    return result;
  }

  String result = stringSlice(*line, 0, delim_pos);
  line->offset += delim_pos + 1;
  line->len -= delim_pos + 1;

  return result;
}

size_t csvCountColumns(String src, char delim) {
  size_t eol_pos = src.len;

  { // Find out where is the line ending
    int actual_eol_pos = stringIndexOf(src, EOL);
    if (actual_eol_pos == 0) {
      return 0;
    }
    if (actual_eol_pos > 0 && (size_t)actual_eol_pos < src.len) {
      eol_pos = (size_t)actual_eol_pos;
    }
  }

  size_t ncolumns = 1;
  for (size_t i = 0; i < eol_pos; i++) {
    if (stringCharAt(src, i) == delim) {
      ncolumns++;
    }
  }

  return ncolumns;
}

size_t csvCountLines(String src) {
  if (stringIsEmpty(src)) {
    // No lines in the empty string
    return 0;
  }

  int index = stringIndexOf(src, EOL);
  if (index < 0) {
    // No new line means that we have only one line
    return 1;
  }

  size_t count = 1;
  do {
    index = stringIndexOfAfter(src, index, EOL);
    count++;
  } while (index >= 0);

  return count;
}

size_t csvGetValues(String *dst, size_t max, String line, char delim) {
  size_t i;
  for (i = 0; i < max && !stringIsEmpty(line); i++) {
    dst[i] = csvGetValue(&line, delim);
  }
  return i;
}
