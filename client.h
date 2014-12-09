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

/* GLOBAL */
typedef enum State {RENDER_SHOOT, MENU_SHOOT, WAIT, EXIT} State;
struct map_info *map_data;
map_t g_map;
extern int dx, dy;
extern int angle, power;
struct player players[MAX_PLAYERS];
extern u_int16_t players_size;
extern int camera_focus;
int sock;

/* game.c */
int camera_move(int input_character);
int change_camera_focus(int input_character);
void center_camera(struct player *tank);
int quit_key(int input_character);
int shoot_menu(int input_character);
void shoot_menu_scene();
void lobby_scene();
void wait_scene();

/* cl_proto.c */
void fetch_map();
void fetch_changes();
int join_game(char *nickname);
void send_shoot();

/* draw.c */
void put_col_str(Color color, int y, int x, const char *str);

/* render.c */
void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle);
void draw_map(map_t map, int pos_x, int pos_y, int width, int height);
void draw_bullet(int pos_x, int pos_y, int x, int y);
void draw_shoot_menu();
void draw_stats();
void draw_lobby();
void render_map();
void render_tanks();

#endif /* CLIENT_H */
