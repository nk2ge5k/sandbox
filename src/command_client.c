#include "command_client.h"

#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#include "debug.h"
#include "str.h"
#include "types.h"

internal void doRequest(const char *host, const char *port, String request) {
  struct addrinfo hints, *servinfo, *p;

  // char portstr[6]; /* Max 16 bit number string length. */
  // snprintf(portstr,sizeof(portstr),"%d",port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(host, port, &hints, &servinfo) != 0) {
    errorf("No address\n");
    return;
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    int sock;
    if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      continue;
    }

    printf("sa data: %d\n", p->ai_addr->sa_family);
    if (connect(sock, p->ai_addr, p->ai_addrlen) < 0) {
      errorf("could not connect to srv: %s\n", strerror(errno));
      return;
    }

    int nwrite = write(sock, request.v, request.len);
    if (nwrite < 0) {
      errorf("could not write request: %s\n", strerror(errno));
      return;
    }

    close(sock);
    return;
  }
}

int commandClient(int argc, char **argv) {

  doRequest("localhost", "8080", stringMakeFrom("GET /hello HTTP/1.1\r\n\r\n"));

  return 0;
}
