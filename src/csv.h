/**
 * =============================================================================
 * TODO
 * =============================================================================
 * - May be will be good idea to define some CSV structure that will store
 *   delimeter and cursor and all other helpful things.
 * - Allow to parse CSV from the file.
 */
#ifndef CSV_H
#define CSV_H

#include "str.h"

#define COMMA ','

// csvGetLine returns string that contains first line of the csv, and modifes
// src String so it points past the ending of this line. If no line end found
// then entire string will return.
String csvGetLine(String *src);

// csvGetLine returns string that contains first value of the csv, and modifes
// src String so it points past the delimiter position.
String csvGetValue(String *line, char delim);

// csvCountColumns reutrns number of columns in the first line of the src
// string.
// FIXME(nk2ge5k): this function cannot handle quoted values.
size_t csvCountColumns(String src, char delim);

// csvCountLines return number of lines in the string
// NOTE(nk2ge5k): O(N) complexity where N = src.len
size_t csvCountLines(String src);

// csvGetValue poulates dst with maximum max values from the csv line
// returns number of the values parsed.
size_t csvGetValues(String *dst, size_t max, String line, char delim);

#endif
