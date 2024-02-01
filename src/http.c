#include "http.h"

#include "debug.h"
#include "types.h"

internal String consumeLine(String *str, String eol) {
  String line;

  int off = stringIndexOfString(*str, eol);
  if (off == -1) {
    line = stringSlice(*str, 0, str->len);
    *str = stringSlice(*str, str->len, 0);
  } else {
    line = stringSlice(*str, 0, off);
    *str = stringSlice(*str, off + eol.len, str->len - (off + eol.len));
  }

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
  String eol = STR("\r\n"); // CRLF

  Request request = {0};

  String line = consumeLine(&data, eol);
  request.method = consumeWord(&line);
  request.path = consumeWord(&line);
  request.proto = stringSlice(line, 0, line.len - eol.len);

  size_t nheaders = 0;
  String headers = stringSlice(data, 0, 0);

  while (!stringIsEmpty(data) && !stringHasPrefix(data, eol)) {
    String header = consumeLine(&data, eol);
    headers.len += header.len + eol.len;
    nheaders++;
  }

  request.headers_len = nheaders;
  request.headers = headers;
  request.body = stringTrimPrefix(data, eol);

  return request;
}

void httpRequetPrint(Request request) {
  debugf("Protocol: " PRSTR "\n", STRING_FMT(request.proto));
  debugf("Method:   " PRSTR "\n", STRING_FMT(request.method));
  debugf("Path:     " PRSTR "\n", STRING_FMT(request.path));
  debugf("Headers:  (len=%lu)\n" PRSTR "\n", request.headers_len,
         STRING_FMT(request.headers));
  debugf("Body:\n" PRSTR "\n", STRING_FMT(request.body));
}
