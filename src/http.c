#include "http.h"

#include "types.h"

internal String consumeLine(String *str, String eol) {
  int off = stringIndexOfString(*str, eol);
  if (off == -1) {
    return stringMake(0);
  }

  String line = stringSlice(*str, 0, off);
  *str = stringSlice(*str, off + eol.len, str->len - (off + eol.len));

  return line;
}

internal String consumeWord(String *str) {
  int off = stringIndexOf(*str, ' ');
  if (off < 0) {
    return (String){0};
  }

  String result = stringSlice(*str, 0, off);
  *str = stringSlice(*str, off + 1, str->len - off);

  return result;
}

Request httpParseRequest(String data) {
  String eol = stringMakeFrom("\r\n"); // CRLF

  Request request = {0};

  String line = consumeLine(&data, eol);
  request.method = consumeWord(&line);
  request.path = consumeWord(&line);
  request.proto = stringSlice(line, 0, line.len - eol.len);

  size_t nheaders = 0;
  String headers = stringSlice(data, 0, 0);

  while (!stringHasPrefix(data, eol)) {
    String header = consumeLine(&data, eol);
    headers.len += header.len + eol.len;
    nheaders++;
  }
  if (nheaders != 0) {
    headers.len -= eol.len;
  }

  request.headers_len = nheaders;
  request.headers = headers;

  request.body = stringSlice(data, eol.len, data.len - eol.len);

  return request;
}
