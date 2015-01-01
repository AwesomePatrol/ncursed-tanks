#include "map_gen.h"

map_t generate_map(struct map_info *info)
{
    // TODO map terrain types, for now just flat terrain
    map_t map = malloc(info->length * sizeof(map_elt_t));
    int cur_height = info->height / 2;

    srand(info->seed);

    for (int i=0; i < info->length; i++)
    {
        if (rand() % 2 == 1)
            map[i] = cur_height - (rand() % 2);
        else
            map[i] = cur_height + (rand() % 2);
        cur_height = map[i];
    }

    return map;
}

map_t copy_map(map_t map, struct map_info *info)
{
    size_t map_size = info->length * sizeof(map_elt_t);
    map_t new_map = malloc(map_size);

    memcpy(new_map, map, map_size);

    return new_map;
}

bool_t is_inside_map(struct map_position pos, struct map_info *info)
{
    return pos.x >= 0 && pos.x < info->length;
}
