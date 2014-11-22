#ifndef NET_H
#define NET_H

/*
 * command        args        reply
 *
 * GET_MAP        none        map_seed map_length map_height
 */
typedef enum Command {GET_STATE = 'S',
    SHOOT = 'F', GET_MAP = 'M', ERROR = 'E'} Command;

#define PORTNUM 7979 /* Port Number */
#define MAXRCVLEN 128 /* Maximal Length of Received Value */

#endif /* NET_H */
