#include "common.h"

/* calculate position of the bullet for specified t */
struct f_pair shot_pos(struct f_pair init_pos, struct f_pair init_v, float t)
{
    struct f_pair return_pos = {init_pos.x + (WIND ? init_v.x*t + t*t*WIND/2 : init_v.x*t),
            init_pos.y + init_v.y*t + t*t*GRAVITY/2};
    return return_pos;
}
