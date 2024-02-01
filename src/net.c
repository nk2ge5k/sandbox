#include "net.h"

#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

int netTCPListen(int port) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return -1;
  }

  int yes;
  // allow local address reuse
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  // TODO(nk2ge5k): configure limit on the incoming connections through the
  // second argument of listen.
  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1 ||
      listen(sock, 1) == -1) {
    close(sock);
    return -1;
  }

  return sock;
}

int netAcceptConnection(int server_socket) {
  struct sockaddr_in sa; // Remote address. Currently unsed.
  socklen_t len = sizeof(sa);

  // TODO(nk2ge5k): add hard limit on the loop.
  while (1) {
    int in = accept(server_socket, (struct sockaddr *)&sa, &len);
    if (in == -1) {
      if (errno == EINTR) {
        continue;
      } else {
        return -1;
      }
    }
    return in;
  }
}
