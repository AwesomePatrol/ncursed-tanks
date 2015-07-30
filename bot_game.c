#include "bot.h"

#define MAX_POWER 100
#define MIN_DIFFICULTY 1

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
    do {
        fetch_changes();
        sleep(1);
    } while (loc_player->state == PS_READY ||
            loc_player->state == PS_WAITING ||
            loc_player->state == PS_DEAD);
}

void shoot()
{
    /* choose target */
    struct player *target;
    do {
        target = dyn_arr_get(&Players, rand() % Players.count);
    } while (target == loc_player);
    debug_s(1, "Target", target->nickname);
    
    /* randomize power and angle */
    power = rand() % MAX_POWER/2 +50;
    int16_t target_x = target->pos.x;
    debug_d(1, "TargetX", target_x);
    if (target->pos.x > target_x)
        angle = rand() % 40 + 40;
    else
        angle = rand() % 40 + 100;
    
    /* calculate angle */
    int16_t impact_x = get_impact_x();
    while (abs(impact_x - target_x)  > difficulty+MIN_DIFFICULTY) {
        if (impact_x > target_x) {
            if (angle > 90)
                if (rand() % 2)
                    angle+=rand() % 5 + 1;
                else
                    power+=rand() % 5 + 1;
            else
                if (rand() % 2)
                    angle+=rand() % 5 + 1;
                else
                    power-=rand() % 5 + 1;
        } else {
            if (angle > 90)
                if (rand() % 2)
                    angle-=rand() % 5 + 1;
                else
                    power-=rand() % 5 + 1;
            else
                if (rand() % 2)
                    angle-=rand() % 5 + 1;
                else
                    power+=rand() % 5 + 1;
        }
        if (power > MAX_POWER) power=MAX_POWER;
        impact_x = get_impact_x();
    }

    /* shoot */
    send_shoot();
    /* wait for result */
    while (loc_player->state == PS_ACTIVE) {
        fetch_changes();
        sleep(1);
    }
}
