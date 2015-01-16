#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ncurses.h>

#include "debug.h"
#include "dyn_arr.h"
#include "colors.h"
#include "map.h"
#include "shot.h"

#define DEFAULT_TIMEOUT 2000
#define SHOOT_TIMEOUT 100 //200 = 5fps, 100 = 10fps

/* GLOBAL VARIABLES */
typedef enum ScreenUpdate {SCR_SHOOT, SCR_LOBBY, SCR_STATS,
    SCR_TANKS, SCR_SHOOT_MENU, SCR_MAP, SCR_ALL} ScreenUpdate;
typedef enum ScreenMove {SCR_OK, SCR_UP, SCR_DOWN, SCR_LEFT,
    SCR_RIGHT} ScreenMove;
extern struct dyn_arr ScrUpdates;
extern struct dyn_arr MapUpdates;
extern struct dyn_arr Players;
extern struct dyn_arr Abilities;
extern struct player *loc_player;
extern int16_t loc_player_id; 
struct map_info *map_data;
map_t g_map;
extern int dx, dy;
extern int angle, power;
extern u_int16_t players_size;
int camera_focus;
float g_impact_t;
int sock;
struct update s_update;

/* client_game.c */
int camera_move(int input_character);
int change_camera_focus(int input_character);
void center_camera(struct map_position d_pos);
int quit_key(int input_character);
int shoot_menu(int input_character);
int lobby_menu(int input_character);

/* client_scene.c */
void map_update();
void render_scene();
void shoot_menu_scene();
void lobby_scene();
void wait_scene();
void post_game_scene();

/* client_net.c */
void fetch_map();
void fetch_changes();
void update_loc_player();
int find_player(u_int16_t player_id);
int join_game(char *nickname);
void send_shoot();

/* client_draw.c */
void put_col_str(Color color, int y, int x, const char *str);

/* client_render.c */
void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle);
void draw_map(map_t map, int pos_x, int pos_y, int width, int height);
ScreenMove draw_bullet(int pos_x, int pos_y, int x, int y);
void draw_blank_bullet(int pos_x, int pos_y, int x, int y);
void draw_shoot_menu();
void draw_bullet_explosion(int pos_x, int pos_y, int x, int y);
void render_shot(struct shot *shot, int s_id);
void render_post_game();
void draw_stats();
void draw_lobby();
void render_map();
void render_tanks();

#endif /* CLIENT_H */
