#include "server_data.h"

pthread_key_t thread_data;
pthread_mutex_t clients_array_mutex = PTHREAD_MUTEX_INITIALIZER;

struct p_dyn_arr clients = {0};

struct map_info map_info;
map_t map = NULL;
map_t tanks_map = NULL;
bool_t game_started = FALSE;


/* helper for get_impact_pos() */
double get_t_step(double prev_delta_x, double prev_t,
                  double *x_step, bool_t *one_side_clear,
                  struct f_pair init_v, struct f_pair acc)
{
    double c1, c2;
    double t_step1, t_step2;

    if (acc.x)
    {
        double D = sqrt(init_v.x*init_v.x
                        + 2*acc.x*(prev_delta_x + *x_step));
        if (!isnan(D))
        {
            c1 = -init_v.x - acc.x*prev_t;
            c2 = acc.x;

            t_step1 = (c1 - D) / c2;
            t_step2 = (c1 + D) / c2;

            debug_f(0, "t step (1)", t_step1);
            debug_f(0, "t step (2)", t_step2);
        }
        else
        {
            /* Need to turn around to the opposite direction */
            *x_step = -(*x_step);
            return get_t_step(prev_delta_x, prev_t,
                              x_step, one_side_clear,
                              init_v, acc);
        }
    }
    else
    {
        c1 = prev_delta_x + *x_step - init_v.x*prev_t;
        c2 = init_v.x;

        t_step1 = t_step2 = c1 / c2;

        debug_f(0, "t step", t_step1);
    }
    /* t_step1 <= t_step2 */

    if (t_step1 >= 0)
    {
        return t_step1;
    }
    else if (t_step2 >= 0)
    {
        return t_step2;
    }
    else
    {
        /* No valid t_step found, turn back */
        if (!one_side_clear)
        {
            *one_side_clear = TRUE;
            *x_step = -(*x_step);
        }
        else
        {
            debug_s(5, "wtf", "Haven't found a valid t_step!");
        }
    }
}

/* Move to server_game.c or something? */
struct map_position get_impact_pos(struct player *player, struct shot *shot)
{
    struct f_pair init_v = initial_v(shot);
    struct f_pair acc = acceleration();
    short init_direction = fabs(init_v.x) / init_v.x;

    struct f_pair init_pos = map_pos_to_float(player->pos);
    double x_step = (double)init_direction / COLLISION_X_PRECISION;

    bool_t one_side_clear = FALSE;
    double cur_delta_x = 0;
    double cur_t = 0;

    debug_f(0, "shot: wind", acc.x);

    while (TRUE) /* exit with return */
    {
        debug_f(0, "current delta_x", cur_delta_x);
        double t_step = get_t_step(cur_delta_x, cur_t,
                                   &x_step, &one_side_clear,
                                   init_v, acc);

        if (t_step != 0)
        {
            cur_t += t_step;
            struct f_pair f_pos = shot_pos(init_pos, init_v, acc, cur_t);
            debug_f(0, "current x", f_pos.x);
            debug_f(0, "current y", f_pos.y);
            struct map_position map_pos = round_to_map_pos(f_pos);
            map_height_t map_y;

            /* The bullet might return from the edge of the map,
             * so stop only when it falls to the bottom */
            if (!is_inside_map(map_pos, &map_info))
                map_y = map_info.height;
            else
                map_y = tanks_map[map_pos.x];

            if (map_pos.y >= map_y)
                return map_pos;
        }
        cur_delta_x += x_step;
    }
}

/* Doesn't lock clients array, must be already locked */
map_t map_with_tanks(void)
{
    map_t new_map = copy_map(map, &map_info);

    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        /* Assume a client always has a player */
        struct player *player = cl->player;

        /* Raise the map where there is a tank */
        /* Don't for a dead player */
        if (player->state != PS_DEAD)
            new_map[player->pos.x]--;
    }

    return new_map;
}

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

void player_do_damage(struct player *player, int16_t damage)
{
    player->hitpoints -= damage;
    if (player->hitpoints <= 0)
        player_die(player);
    all_add_update(new_player_update(U_PLAYER, player));
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
        .id = id,
        .nickname = nickname,
        .hitpoints = config_get("tank_hp"),
        .pos = { player_x,
                 map[player_x] - 1 }
    };

    return result;
}

int new_player_x(void)
{
    int notank_margin = config_get("map_margin");
    return notank_margin
        + rand() % (map_info.length - 2 * notank_margin);
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
