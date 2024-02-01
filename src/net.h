#ifndef NET_H
#define NET_H

// netTCPListen attempts to open server socket on the given port.
// Retruns -1 if any errors occurs.
int netTCPListen(int port);

// netAcceptConnection attempts to accept incoming connection.
// If the listening socket signaled there is a new connection ready to
// be accepted, we accept(2) it and return -1 on error or the new client
// socket on success.
int netAcceptConnection(int server_socket);

#endif
