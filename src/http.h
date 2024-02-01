#ifndef HTTP_H
#define HTTP_H

#include "str.h"

typedef struct Request {
  // TODO(nk2ge5k): probably should parse as an integer
  String proto;

  // HTTP method
  String method;
  // Request path
  String path;

  // All of the headers
  String headers;
  // Number of headers
  size_t headers_len;

  // Request body
  String body;
} Request;

// httpParseAttempts to parse string as http request.
// TODO(nk2ge5k): return error somehow.
Request httpParseRequest(String data);

// httpRequetPrintln prints request data into stderr
void httpRequetPrint(Request req);

#endif
