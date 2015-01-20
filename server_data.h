#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include "server_lib_includes.h"
#include "server_game.h"

/* type of (struct player).id */
typedef __typeof__((struct player) {0}.id) client_id_t;

struct thread_data
{
    pthread_t thread;
    int socket;

    struct client *client;
    int clients_lock_count;
};

struct client
{
    client_id_t id;
    struct player *player;
    struct ability *ability;
    struct updates_queue *updates;

    pthread_mutex_t updates_mutex;
};


extern pthread_key_t thread_data;
extern pthread_mutex_t clients_array_mutex;

/* You must lock_clients_array() when using this variable
 * unless you have a good reason not to */
extern struct p_dyn_arr clients;

extern struct map_info map_info;
extern map_t map;
extern map_t tanks_map;
extern bool game_started;

void player_change_state(struct player *player, PlayerState state);
void player_set_connected(struct player *player, bool is_connected);
void player_deal_damage(struct player *player, int16_t damage);
void player_die(struct player *player);
void change_map(int16_t x, int16_t new_height);

void lock_clients_array(void);
void unlock_clients_array(void);
void lock_updates(struct client *cl);
void unlock_updates(struct client *cl);

void add_update(struct client *cl, struct update *upd);
void all_add_update(struct update *upd);
void add_client(struct client *cl);
struct client **find_client_loc(client_id_t id);
struct client *find_client_by_nickname(char *nickname);

struct client *new_client(char *nickname);
client_id_t new_client_id(void);
void clear_client(struct client *cl);
void free_client(struct client *cl);
struct player *new_player(char *nickname, client_id_t id);
map_height_t new_player_y(int16_t x);
struct update *new_player_update(UpdateType type, struct player *player);
struct update *new_config_update(struct config_item *opt);
struct update *new_add_ability_update(struct ability *ability);
struct update *new_shot_update(struct shot *shot, client_id_t id);
struct update *new_shot_impact_update(double impact_t);
struct update *new_map_update(int16_t x, int16_t new_height);
struct update copy_update(struct update *u);
void clear_update(struct update *u);

#endif /* SERVER_DATA_H */
