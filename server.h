#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
 
#define PORTNUM 7979 /* Port Number */
#define MAXRCVLEN 100 /* Maximal Length of Received Value */

#define DEBUG 0 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/

typedef enum Command {GET_STATE, SHOOT, GET_MAP, ERR} Command;

void parse(const char *buffer, size_t length);
#endif /* SERVER_H */
