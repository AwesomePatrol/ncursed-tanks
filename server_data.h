#ifndef SERVER_DATA_H
#define SERVER_DATA_H

#include "server_lib_includes.h"

/* type of (struct player).id */
typedef __typeof__((struct player) {0}.id) client_id_t;

struct thread_data
{
    pthread_t thread;
    int socket;

    client_id_t client_id;
};

struct client
{
    client_id_t id;
    struct player *player;
    struct updates_queue *updates;
};


extern pthread_key_t thread_data;
extern pthread_mutex_t clients_mutex;

extern struct dyn_arr clients;

extern struct map_info map_info;
extern map_t map;
extern int game_started; /* bool */

void lock_clients(void);
void unlock_clients(void);

void add_update(struct client *cl, struct update *upd);
void all_add_update(struct update *upd);
void add_client(struct client *cl);
struct client *find_client(client_id_t id);
struct client *find_client_by_nickname(char *nickname);

struct client *new_client(char *nickname);
client_id_t new_client_id(void);
void clear_client(struct client *cl);
struct player *new_player(char *nickname, client_id_t id);
int new_player_x(void);
struct update *new_player_update(UpdateType type, struct player *player);
struct update copy_update(struct update *u);
void clear_update(struct update *u);

#endif /* SERVER_DATA_H */
