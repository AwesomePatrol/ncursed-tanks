#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>

int sendall(int socket, char *msg, int len);
int recvall(int socket, char *msg, int len);

#endif /* COMMON_H */
