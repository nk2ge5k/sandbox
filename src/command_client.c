#include "command_client.h"

#include <sys/errno.h>
#include <unistd.h>

#include "debug.h"
#include "net.h"
#include "str.h"
#include "http.h"

i32 commandClient(i32 argc, char **argv) {
  i32 client_socket = netTCPDial("localhost", 8080);
  if (client_socket == -1) {
    errorf("dial failed\n");
    return 1;
  }

  char *payload = "GET /hello HTTP/1.1\r\n";

  i32 nwrite = write(client_socket, payload, strlen(payload));
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
