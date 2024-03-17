#include <stdio.h>

#include "command_server.h"

#include <netinet/in.h>
#include <unistd.h>

#include "debug.h"
#include "net.h"
#include "str.h"
#include "http.h"

#define BUFSIZE 1024

i32 commandServer(i32 argc, char **argv) {
  i32 server_socket = netTCPListen(8080);
  if (server_socket == -1) {
    errorf("Failed to listen\n");
    return 1;
  }

  char buffer[BUFSIZE] = {0};

  while (true) {
    i32 client_socket = netAcceptConnection(server_socket);
    if (client_socket == -1) {
      continue;
    }

    i32 nread = read(client_socket, &buffer, BUFSIZE - 1);
    if (nread < 0) {
      debugf("could not read from socket: %s\n", strerror(errno));
      goto cleanup;
    }

    String str = stringMakeFromLen(buffer, nread);
    Request request = httpParseRequest(str);

    httpRequetPrint(request);

    char *response = "HTTP/1.1 200 OK\r\n";
    send(client_socket, response, strlen(response), 0);

  cleanup:
    close(client_socket);
  }

  return 0;
}
