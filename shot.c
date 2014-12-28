#include "shot.h"

/* calculate position of the bullet for specified t */
struct f_pair shot_pos(struct f_pair init_pos, struct f_pair init_v, float t)
{
     return (struct f_pair) {
         init_pos.x + (WIND ? init_v.x*t + t*t*WIND/2 : init_v.x*t),
         init_pos.y + init_v.y*t + t*t*GRAVITY/2
     };
}

struct f_pair initial_v(struct shot *shot)
{
    double angle_rad = deg_to_rad(shot->angle);

    return (struct f_pair) {
        shot->power * cos(angle_rad) / C_POWER,
        -shot->power * sin(angle_rad) / C_POWER
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
        floor(pos.y)
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
