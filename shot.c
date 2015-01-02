#include "shot.h"

/* calculate position of the bullet for specified t */
struct f_pair shot_pos(struct f_pair init_pos,
                       struct f_pair init_v, struct f_pair acc, float t)
{
     return (struct f_pair) {
         init_pos.x + (acc.x ? init_v.x*t + t*t*acc.x/2 : init_v.x*t),
         init_pos.y + init_v.y*t + t*t*acc.y/2
     };
}

struct f_pair initial_v(struct shot *shot)
{
    int power_c = config_get("power_c");
    double angle_rad = deg_to_rad(shot->angle);

    return (struct f_pair) {
        shot->power * cos(angle_rad) / power_c,
        -shot->power * sin(angle_rad) / power_c
    };
}

struct f_pair acceleration(void)
{
    return (struct f_pair) {
        (double)config_get("wind") / 1000,
        (double)config_get("gravity") / 1000
    };
}

struct f_pair initial_pos(struct player *player)
{
    return (struct f_pair) { player->pos.x + 0.5, player->pos.y + 0.5 };
}

struct map_position round_to_map_pos(struct f_pair pos)
{
    return (struct map_position) {
        round(pos.x - 0.5),
        /* Is using floor right?
         * why - 0.5? IMO without this it'll be alright*/
        floor(pos.y - 0.5)
    };
}

/* converts degrees to radians */
double deg_to_rad(int deg)
{
    switch (deg)
    {
        case 0:
            return 0;
        case 90:
            return M_PI/2;
        case 180:
            return M_PI;
        default:
            return deg * M_PI / 180;
    }
}

/* calculate distance between two specified points (f_pair)*/
double distance(struct f_pair a, struct f_pair b)
{
    if (a.x == b.x)
        return a.y > b.y ? a.y-b.y : b.y-a.y;
    if (a.y == b.y)
        return a.x > b.x ? a.x-b.x : b.x-a.x;
    double px=a.x-b.x, py=a.y-b.y;
    return sqrt(px*px+py*py);
}

/* calculate damage to specified player */
int damage(struct player *p, struct f_pair s)
{
    struct f_pair t_pos = initial_pos(p);
    double dis = distance(t_pos, s);
    if (dis > config_get("dmg_radius"))
        return 0;
    if (dis <= 1)
        return config_get("dmg_cap");
    return config_get("dmg_cap")/dis;
}
