#ifndef COMMON_H
#define COMMON_H

#include <sys/socket.h>

/*
 * command        args        reply
 *
 * GET_MAP        none        struct map_info
 */
typedef enum Command {GET_STATE = 'S',
    SHOOT = 'F', GET_MAP = 'M', ERROR = 'E'} Command;

#define PORTNUM 7979 /* Port Number */
#define MAXRCVLEN 128 /* Maximal Length of Received Value */

struct map_info
{
    u_int32_t seed;
    u_int16_t length;
    u_int16_t height;
};


int sendall(int socket, void *data, int len);
int recvall(int socket, void *data, int len);

#endif /* COMMON_H */
