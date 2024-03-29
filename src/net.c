#include "net.h"

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"

i32 netTCPListen(i32 port) {
  i32 sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    return -1;
  }

  i32 yes;
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

i32 netAcceptConnection(i32 server_socket) {
  struct sockaddr_in sa; // Remote address. Currently unsed.
  socklen_t len = sizeof(sa);

  // TODO(nk2ge5k): add hard limit on the loop.
  while (1) {
    i32 in = accept(server_socket, (struct sockaddr *)&sa, &len);
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

i32 netTCPDial(char *host, u16 port) {
  char portstr[6] = {0};
  struct addrinfo hints, *servinfo, *p;

  snprintf(portstr, 5, "%d", port);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, portstr, &hints, &servinfo) != 0) {
    return -1;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    i32 sock;
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) {
      freeaddrinfo(servinfo);
      return sock;
    }

    close(sock);
  }

  freeaddrinfo(servinfo);
  return -1;
}
