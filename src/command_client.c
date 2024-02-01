#include "command_client.h"

#include <sys/errno.h>
#include <unistd.h>

#include "debug.h"
#include "net.h"

int commandClient(int argc, char **argv) {
  int client_socket = netTCPDial("localhost", 8080);
  if (client_socket == -1) {
    errorf("dial failed\n");
    return 1;
  }

  debugf("Client socket: %d\n", client_socket);

  char *payload = "GET /hello HTTP/1.1\r\n";

  int nwrite = write(client_socket, payload, strlen(payload));
  if (nwrite < 0) {
    errorf("could not write request: %s\n", strerror(errno));
    goto cleanup;
  }

  debugf("Writen: %d\n", nwrite);

cleanup:
  close(client_socket);
  debugf("Connection closed\n");
  return 0;
}
