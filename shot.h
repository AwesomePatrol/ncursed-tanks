#ifndef SHOT_H
#define SHOT_H

#include <math.h>

#include "common.h"
#include "config.h"

/*support for M_PI was dropped in C99, we don't need better value than here*/
#define M_PI 3.1415

double deg_to_rad(int deg);

struct f_pair shot_pos(struct f_pair init_pos,
                       struct f_pair init_v, struct f_pair acc, float t);
struct f_pair initial_v(struct shot *shot);
struct f_pair acceleration(void);
struct f_pair initial_pos(struct player *player);
struct map_position round_to_map_pos(struct f_pair pos);

double distance(struct f_pair a, struct f_pair b);
int damage(struct player *p, struct f_pair s);

#endif /* SHOT_H */
