#include "common.h"

typedef short map_elt_t;
typedef map_elt_t *map_t;

map_t generate_map(struct map_info *info);
map_t copy_map(map_t map, struct map_info *info);

bool_t is_inside_map(struct map_position pos, struct map_info *info);
