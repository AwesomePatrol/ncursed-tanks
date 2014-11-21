#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ncurses.h>

#include "net.h"
#include "colors.h"
#include "draw.h"

typedef enum State {RENDER_SHOOT, MENU_SHOOT, WAIT} State;

#define DEBUG 0 /* DEBUG: set 5 for ERROR, set 3 for INFO, set 0 for DEBUG
                   VALUES higher than 5 will result in silent execution*/

#endif /* CLIENT_H */
