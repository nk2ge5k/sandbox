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

Request httpParseRequest(String data);

#endif
