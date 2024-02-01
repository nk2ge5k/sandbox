#include <stdio.h>

#include "command_server.h"

#include <assert.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "debug.h"
#include "net.h"
#include "str.h"
#include "http.h"

#define BUFSIZE 1024

int commandServer(int argc, char **argv) {
  int server_socket = netTCPListen(8080);
  if (server_socket == -1) {
    errorf("Failed to listen\n");
    return 1;
  }

  char buffer[BUFSIZE] = {0};

  while (true) {
    int client_socket = netAcceptConnection(server_socket);
    if (client_socket == -1) {
      continue;
    }

    int nread = read(client_socket, &buffer, BUFSIZE - 1);
    if (nread < 0) {
      debugf("could not read from socket: %s\n", strerror(errno));
      goto cleanup;
    }

    String str = stringMakeFromLen(buffer, nread);
    Request request = httpParseRequest(str);

    debugf("Protocol: " PRSTR "\n", STRING_FMT(request.proto));
    debugf("Method:   " PRSTR "\n", STRING_FMT(request.method));
    debugf("Path:     " PRSTR "\n", STRING_FMT(request.path));
    debugf("Headers: (%lu)\n" PRSTR "\n", request.headers_len,
           STRING_FMT(request.headers));
    debugf("Body:\n" PRSTR "\n", STRING_FMT(request.body));

    char *response = "HTTP/1.1 200 OK\n";
    send(client_socket, response, strlen(response), 0);

  cleanup:
    close(client_socket);
  }

  return 0;
}
