#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ncurses.h>

#include "debug.h"
#include "colors.h"
#include "map_gen.h"

/* GLOBAL variables >>use extern? */
typedef enum State {RENDER_SHOOT, MENU_SHOOT, WAIT, EXIT} State;
struct map_info *map_data;
map_t g_map;
extern int dx, dy;
extern int angle, power;
struct player players[MAX_PLAYERS];
extern u_int16_t players_size;

/* game.c */
int camera_move(char input_character);

/* cl_proto.c */
void fetch_map(int sock);
void fetch_changes(int sock);
int join_game(int sock, char *nickname);

/* render.c */
void put_col_str(Color color, int y, int x, const char *str);
void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle);
void draw_map(map_t map, int pos_x, int pos_y, int width, int height);
void render_map();
void render_tanks();

#endif /* CLIENT_H */
