#ifndef NET_H
#define NET_H

typedef enum Command {GET_STATE = 'S',
    SHOOT = 'F', GET_MAP = 'M', ERR = 'E'} Command;

#define PORTNUM 7979 /* Port Number */
#define MAXRCVLEN 128 /* Maximal Length of Received Value */

#endif /* NET_H */
