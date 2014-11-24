#include "map_gen.h"

map_t generate_map(struct map_info info)
{
    // TODO map terrain types, for now just flat terrain
    map_t map = malloc(info.length * sizeof(map_elt_t));
    int cur_height = info.height / 2;

    srand(info.seed);

    for (int i=0; i < info.length; i++)
    {
        if (rand() % 2 == 1)
            map[i] = cur_height - (rand() % 2);
        else
            map[i] = cur_height + (rand() % 2);
        cur_height = map[i];
    }

    return map;
}
