#include "bot.h"

#define MAX_POWER 100

struct dyn_arr Players = { sizeof(struct player) };
struct dyn_arr Abilities = { sizeof(struct ability) };

/* find ability by id */
struct ability *find_ability(int16_t id)
{
    for (int i=0; i<Abilities.count; i++) {
        struct ability *cur_a = dyn_arr_get(&Abilities, i);
        if (cur_a->id == id)
            return cur_a;
    }
    return NULL;
}

bool check_end_game_state()
{
    for (int i=0; i<Players.count; i++) {
        struct player *tmp_p = dyn_arr_get(&Players, i);
        if (tmp_p->state == PS_WINNER || tmp_p->state == PS_LOSER)
            continue;
        else
            return false;
    }
    return true;
}

void wait_state()
{
    while (loc_player->state == PS_READY ||
            loc_player->state == PS_WAITING ||
            loc_player->state == PS_DEAD) {
        fetch_changes();
        sleep(1);
    }
}

void shoot()
{
    angle=50;
    power=50;
    send_shoot();
    while (loc_player->state == PS_ACTIVE) {
        fetch_changes();
        sleep(1);
    }
}
