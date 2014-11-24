#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ncurses.h>

#include "debug.h"
#include "colors.h"
#include "map_gen.h"
#include "draw.h"

/* GLOBAL variables >>use extern? */
typedef enum State {RENDER_SHOOT, MENU_SHOOT, WAIT} State;
struct map_info map_data;
map_t g_map;

/* cl_proto.c */
void fetch_map(int sock);

#endif /* CLIENT_H */
