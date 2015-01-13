#include "server_data.h"

pthread_key_t thread_data;
pthread_mutex_t clients_array_mutex = PTHREAD_MUTEX_INITIALIZER;

struct p_dyn_arr clients = {0};

struct map_info map_info;
map_t map = NULL;
map_t tanks_map = NULL;
bool game_started = false;


void lock_clients_array(void)
{
    //debug_s(0, "lock clients array", "Locking");
    pthread_mutex_lock(&clients_array_mutex);
}

void unlock_clients_array(void)
{
    //debug_s(0, "unlock clients array", "Unlocking");
    pthread_mutex_unlock(&clients_array_mutex);
}

void lock_updates(struct client *cl)
{
    pthread_mutex_lock(&cl->updates_mutex);
}

void unlock_updates(struct client *cl)
{
    pthread_mutex_unlock(&cl->updates_mutex);
}

/* frees upd */
void add_update(struct client *cl, struct update *upd)
{
    struct update upd_copy = copy_update(upd);

    lock_updates(cl);                                            /* {{{ */
    uq_append(cl->updates, &upd_copy);
    unlock_updates(cl);                                          /* }}} */

    free(upd);
}

/* frees upd */
/* clients_array must be locked already, doesn't do it on its own */
void all_add_update(struct update *upd)
{
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        struct update upd_copy = copy_update(upd);
        uq_append(cl->updates, &upd_copy);
    }

    free(upd);
}

void add_client(struct client *cl)
{
    p_dyn_arr_append(&clients, cl);
}

/* Changes player state and adds a player update to all the clients */
void player_change_state(struct player *player, PlayerState state)
{
    player->state = state;
    all_add_update(new_player_update(U_PLAYER, player));
}

void player_deal_damage(struct player *player, int16_t damage)
{
    player->hitpoints -= damage;
    if (player->hitpoints <= 0)
        /* adds an update as well */
        player_die(player);
    else
        all_add_update(new_player_update(U_PLAYER, player));
    debug_s(3, "damage: player", player->nickname);
    debug_d(3, "damage", damage);
}

void player_die(struct player *player)
{
    player_change_state(player, PS_DEAD);
}

void change_map(int16_t x, map_height_t new_height)
{
    map[x] = new_height;
    all_add_update(new_map_update(x, new_height));
}

/* Returns the pointer to the array element of the client that satisfies test.
 * Returns NULL if no such element found */
struct client **find_client_loc_by(int (*test)(struct client *))
{
    for (int i = 0; i < clients.count; i++)
    {
        struct client **cl =
            (struct client **)p_dyn_arr_get_location(&clients, i);
        if (test(*cl))
        {
            return cl;
        }
    }
    return NULL;
}

struct client **find_client_loc(client_id_t id)
{
    int test(struct client *cl)
    {
        return cl->id == id;
    }

    return find_client_loc_by(test);
}

struct client *find_client_by_nickname(char *nickname)
{
    int test(struct client *cl)
    {
        return strcmp(cl->player->nickname, nickname) == 0;
    }

    struct client **result = find_client_loc_by(test);
    return result ? *result : NULL;
}


struct client *new_client(char *nickname)
{
    struct client *result = malloc(sizeof(*result));
    client_id_t id = new_client_id();

    *result = (struct client) {
        .id = id,
        .player = new_player(nickname, id),
        .updates = new_uq(),
        .updates_mutex = PTHREAD_MUTEX_INITIALIZER,
    };

    return result;
}

client_id_t new_client_id(void)
{
    static client_id_t player_id_counter = 0;

    client_id_t result = ++player_id_counter;
    /* TODO fix it somehow */
    if (player_id_counter == 0)
        debug_s( 5, "player id",
"Player ID counter overflowed to 0 (\"empty\" value)! Expect breakage!");

    return result;
}

void clear_client(struct client *cl)
{
    if (uq_is_nonempty(cl->updates))
        free_uq(cl->updates);
    pthread_mutex_destroy(&cl->updates_mutex);
    clear_player(cl->player);
    free(cl->player);
}

void free_client(struct client *cl)
{
    clear_client(cl);
    free(cl);
}

struct player *new_player(char *nickname, client_id_t id)
{
    int player_x = new_player_x();
    struct player *result = malloc(sizeof(*result));

    *result = (struct player) {
        .state = PS_JOINED,
        .is_connected = true,
        .id = id,
        .nickname = nickname,
        .hitpoints = config_get("tank_hp"),
        .pos = { player_x,
                 new_player_y(player_x) }
    };

    return result;
}

map_height_t new_player_y(int16_t x)
{
    return map[x] - 1;
}

struct update *new_player_update(UpdateType type, struct player *player)
{
    struct update *result = malloc(sizeof(*result));
    *result = (struct update) {
        .type = type,
        .player = *player,
    };
    return result;
}

struct update *new_config_update(struct config_item *opt)
{
    struct update *result = malloc(sizeof(*result));
    *result = (struct update) {
        .type = U_CONFIG,
        .opt_name = opt->name,
        .opt_value = opt->value,
    };
    return result;
}

struct update *new_shot_update(struct shot *shot, client_id_t id)
{
    struct update *result = malloc(sizeof(*result));
    *result = (struct update) {
        .type = U_SHOT,
        .shot = *shot,
        .player_id = id,
    };
    return result;
}

struct update *new_shot_impact_update(double impact_t)
{
    struct update *result = malloc(sizeof(*result));
    *result = (struct update) {
        .type = U_SHOT_IMPACT,
        .impact_t = round(impact_t * IMPACT_T_NET_PRECISION),
    };
    return result;
}

struct update *new_map_update(int16_t x, int16_t new_height)
{
    struct update *result = malloc(sizeof(*result));
    *result = (struct update) {
        .type = U_MAP,
        .x = x,
        .new_height = new_height,
    };
    return result;
}

struct update copy_update(struct update *u)
{
    struct update result = *u;

    switch (u->type)
    {
    case U_PLAYER: case U_ADD_PLAYER: case U_DEL_PLAYER:
        ;
        struct player player_copy = u->player;
        player_copy.nickname = strdup(u->player.nickname);

        result.player = player_copy;

        break;
    }

    return result;
}

void clear_update(struct update *u)
{
    switch (u->type)
    {
    case U_PLAYER: case U_ADD_PLAYER: case U_DEL_PLAYER:
        clear_player(&u->player);

        break;
    }
}
