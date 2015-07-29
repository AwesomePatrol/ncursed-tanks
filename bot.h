#ifndef BOT_H
#define BOT_H

#include "common.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>

#include "debug.h"
#include "dyn_arr.h"
#include "map.h"
#include "config.h"

/* GLOBAL VARIABLES */
extern struct dyn_arr Players;
extern struct dyn_arr Abilities;
extern struct player *loc_player;
extern int16_t loc_player_id; 
struct map_info *map_data;
map_t g_map;
extern int angle, power;
extern u_int16_t players_size;
float g_impact_t;
int sock;
struct update s_update;
char *g_servername;

/* bot.c */
bool client_connect(char *servername);

/* bot_game.c */
struct ability *find_ability(int16_t id);
bool check_end_game_state();
void wait_state();
void shoot();

/* bot_net.c */
void fetch_map();
void process_saved_updates();
void process_update(struct update *UpdateNet);
void fetch_changes();
void update_loc_player();
int find_player(u_int16_t player_id);
int join_game(char *nickname);
void send_shoot();
void send_ready();

#endif /* BOT_H */
