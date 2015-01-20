#include "server_game.h"

/* Helper for new_player_x */
bool player_x_too_close(int16_t x)
{
    int tank_distance = config_get("tank_distance");
    bool result = false;

    /* Find a player that is too close to x,
     * return true if found. */
    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        if (abs(cl->player->pos.x - x) < tank_distance)
        {
            result = true;
            goto end;
        }
    }
 end:
    unlock_clients_array();                                      /* }}} */
    return result;
}

int16_t new_player_x(void)
{
    int notank_margin = config_get("map_margin");
    int16_t x;

    /* Generate position until it isn't too close to other tanks */
    do
        x = notank_margin
            + rand() % (map_info.length - 2 * notank_margin);
    while (player_x_too_close(x));

    return x;
}

/* helper for get_impact_pos() */
double get_t_step(double prev_delta_x, double prev_t,
                  double *x_step, bool *one_side_clear,
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
            *one_side_clear = true;
            *x_step = -(*x_step);
        }
        else
        {
            debug_s(5, "wtf", "Haven't found a valid t_step!");
        }
    }
}

/* Move to server_game.c or something? */
/* Sets *impact_t to impact time, returns impact position */
struct map_position get_impact_pos(struct player *player, struct shot *shot,
                                   double *impact_t)
{
    struct f_pair init_v = initial_v(shot);
    struct f_pair acc = acceleration();
    short init_direction = fabs(init_v.x) / init_v.x;

    struct f_pair init_pos = map_pos_to_float(player->pos);
    double x_step = (double)init_direction / COLLISION_X_PRECISION;

    bool one_side_clear = false;
    double cur_delta_x = 0;
    double cur_t = 0;

    debug_f(0, "shot: wind", acc.x);

    while (true) /* exit with return */
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
            {
                *impact_t = cur_t;
                return map_pos;
            }
        }
        cur_delta_x += x_step;
    }
}

map_t map_with_tanks(void)
{
    map_t new_map = copy_map(map, &map_info);

    lock_clients_array();                                        /* {{{ */
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
    unlock_clients_array();                                      /* }}} */

    return new_map;
}

int16_t damage_to_player(struct f_pair impact_pos, struct f_pair player_pos)
{
    config_value_t damage_cap = config_get("dmg_cap");
    config_value_t radius = config_get("dmg_radius");
    struct f_pair diff = { player_pos.x - impact_pos.x,
                           player_pos.y - impact_pos.y };
    double distance = sqrt(diff.x*diff.x + diff.y*diff.y);
    if (distance > radius) return 0;
    if (distance <= 2) return damage_cap;
    return damage_cap/(distance-1);
    /* damage in the impact point - damage_cap, on the edge of the radius - 0 */
    //return damage_cap - distance * ((double)damage_cap / radius);
}


void init_game(void)
{
    unsigned int random_seed;

    /* Initialize random number renerator with current time */
    random_seed = time(NULL);
    srand(random_seed);

    map_info.seed = rand();

    map_info.length = config_get("map_width");
    map_info.height = config_get("map_height");

    map = generate_map(&map_info);
}

/* Cleans up the game state */
void game_cleanup(void)
{
    free(map);       map = NULL;
    free(tanks_map); tanks_map = NULL;

    game_started = false;
}

void start_game(void)
{
    /* TODO move tanks_map creation to process_shoot_command? */
    tanks_map = map_with_tanks();

    lock_clients_array();                                        /* {{{ */
    /* Mark all players as waiting for their turns */
    for (int i = 1; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        player_change_state(cl->player, PS_WAITING);
    }

    /* Give turn to the first player */
    /* Assume the first player is the first client.
     * May become not true in the future?
     */
    struct client *cl = p_dyn_arr_get(&clients, 0);
    unlock_clients_array();                                      /* }}} */
    player_change_state(cl->player, PS_ACTIVE);

    game_started = true;
}

/* Advances turn to the next player */
void next_turn(void)
{
    /* TODO Change / make the algorithm easier to read */
    /* Returns 1 if made active */
    int make_active_if_not(struct player *player)
    {
        if (player->state != PS_WAITING)
            return 0;
        player_change_state(player, PS_ACTIVE);
        return 1;
    }

    bool made_inactive = false;

    lock_clients_array();                                        /* {{{ */

    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;

        if (!made_inactive)
        {
            if (player->state == PS_ACTIVE)
            {
                player_change_state(player, PS_WAITING);
                made_inactive = true;
            }
        }
        else
        {
            if (make_active_if_not(player)) goto end;
        }
    }
    /* Player made inactive but the next player still not made active */
    /* Continue to look for the next inactive client from the beginning */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        if (make_active_if_not(cl->player)) goto end;
    }

 end:
    unlock_clients_array();                                      /* }}} */
}

bool end_game_if_needed(void)
{
    /* Check for end of game */
    int num_living_players = 0;

    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        if (cl->player->state != PS_DEAD)
            num_living_players++;
        if (num_living_players > 1)
        {
            unlock_clients_array();                              /* }}} 1 */
            return false;
        }
    }
    unlock_clients_array();                                      /* }}} 2 */
    /* At this point, only one living player remains */

    /***** GAME OVER *****/
    debug_s(3, "only one living player remains", "Game over");

    end_game();

    return true;
}

void end_game(void)
{
    /* Mark dead players as PS_LOSER and living players as PS_WINNER */
    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;

        player_change_state(player,
                            player->state != PS_DEAD ?
                                PS_WINNER : PS_LOSER);
    }
    unlock_clients_array();                                      /* }}} 2 */

    game_cleanup();
    reset_game();
}

/* Returns the game back to lobby */
void reset_game(void)
{
    init_game();

    /* Set all players as PS_JOINED */
    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        player_change_state(cl->player, PS_JOINED);
    }
    unlock_clients_array();                                      /* }}} 2 */
}

double min(double a, double b)
{
    return a <= b ? a : b;
}

void make_smooth(int x, short int direction)
{
    while ( x < map_info.length && x > 0
            && (map[x]-map[direction ? x+1 : x-1]) >= 3 )
    {
        change_map(direction ? x+1 : x-1, map[x]-2);
        direction ? x++ : x--;
    }
}

/* helper for shot_update_map() */
void update_map_at(struct f_pair pos, struct map_position map_pos,
                   struct f_pair orig_pos,
                   config_value_t radius)/* too many arguments */
{
    double change_amount = 1;/* this could scale with radius */
    debug_f(0, "update_map_at: change amount", change_amount);

    change_map(map_pos.x, map[map_pos.x] + change_amount);
    if (radius >= 3 && map_pos.x > 0 && map_pos.x < map_info.length) {
        change_map(map_pos.x-1, map[map_pos.x-1] + change_amount);
        change_map(map_pos.x+1, map[map_pos.x+1] + change_amount);
        make_smooth(map_pos.x+1, 1);
        make_smooth(map_pos.x-1, 0);
    } else {
        make_smooth(map_pos.x, 1);
        make_smooth(map_pos.x, 0);
    }
}

void shot_update_map(struct map_position impact_pos)
{
    config_value_t radius = config_get("dmg_radius");
    debug_d(0, "damage radius", radius);

    if (!is_inside_map(impact_pos, &map_info))
        return;

    struct f_pair orig_pos = map_pos_to_float(impact_pos);
    double left_x = orig_pos.x - radius;
    double right_x = orig_pos.x + radius;
    debug_f(0, "left x", left_x);
    debug_f(0, "right x", right_x);

    struct f_pair pos = { left_x, orig_pos.y };
    for (; pos.x <= right_x; pos.x += 1)
    {
        debug_f(0, "update map: current x", pos.x);
        struct map_position map_pos = round_to_map_pos(pos);

        if (is_inside_map(map_pos, &map_info))
            update_map_at(pos, map_pos, orig_pos, radius);
    }

    /* Place all tanks above ground back onto the ground */
    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;
        map_height_t map_y = map[player->pos.x];

        if (player->pos.y < map_y)
            player->pos.y = new_player_y(player->pos.x);
    }
    unlock_clients_array();                                      /* }}} */
}

void shot_deal_damage(struct map_position impact_pos)
{
    if (!is_inside_map(impact_pos, &map_info))
        return;

    struct f_pair f_impact_pos = map_pos_to_float(impact_pos);

    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;

        if (player->state != PS_DEAD)
        {
            struct f_pair f_player_pos = map_pos_to_float(player->pos);

            int16_t damage = damage_to_player(f_impact_pos, f_player_pos);
            if (damage > 0)
                player_deal_damage(player, damage);
        }
    }
    unlock_clients_array();                                      /* }}} */
}

void process_impact(struct map_position impact_pos)
{
    shot_deal_damage(impact_pos);

    shot_update_map(impact_pos);

    if (!end_game_if_needed())
    {
        /* TODO change it so that it just doesn't allocate each time? */
        free(tanks_map);
        tanks_map = map_with_tanks();

        next_turn();
    }
}
