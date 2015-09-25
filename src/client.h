#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ncurses.h>

#include "platform.h"
#include "debug.h"
#include "dyn_arr.h"
#include "map.h"
#include "config.h"

#define DEFAULT_TIMEOUT 2000
#define SHOOT_TIMEOUT 100 //200 = 5fps, 100 = 10fps

/* GLOBAL VARIABLES */
typedef enum ScreenUpdate {SCR_SHOOT, SCR_LOBBY, SCR_STATS,
    SCR_TANKS, SCR_SHOOT_MENU, SCR_MAP, SCR_ALL} ScreenUpdate;
typedef enum ScreenMove {SCR_OK, SCR_UP, SCR_DOWN, SCR_LEFT,
    SCR_RIGHT} ScreenMove;
extern struct dyn_arr ScrUpdates;
extern struct dyn_arr NetUpdates;
extern bool save_updates;
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
char *g_servername;

/* client.c */
bool client_connect(char *servername);

/* client_game.c */
struct ability *find_ability(int16_t id);
bool check_end_game_state();
int camera_move(int input_character);
int change_camera_focus(int input_character);
void center_camera(struct map_position d_pos);
int quit_key(int input_character);
int shoot_menu(int input_character);
int lobby_menu(int input_character);

/* client_scene.c */
void render_scene();
void shoot_menu_scene();
void lobby_scene();
void wait_scene();
void post_game_scene();

/* client_net.c */
void fetch_map();
void process_saved_updates();
void process_update(struct update *UpdateNet);
void fetch_changes();
void update_loc_player();
int find_player(u_int16_t player_id);
int join_game(char *nickname);
void send_shoot();

/* client_render.c */
void render_shot(struct shot *shot, int s_id);
void render_shoot_menu();
void render_post_game();
void render_map();
void render_tanks();
void render_stats();
void render_lobby();

#endif /* CLIENT_H */
