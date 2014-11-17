#include "map_gen.h"

map_t generate_map(unsigned int seed, int length, int height)
{
    // TODO map terrain types, for now just flat terrain
    map_t map = malloc(length * sizeof(map_elt_t));
    int cur_height = height / 2;

    srand(seed);

    for (int i=0; i < length; i++)
    {
        if (rand() % 2 == 1)
            map[i] = cur_height - (rand() % 2);
        else
            map[i] = cur_height + (rand() % 2);
        cur_height = map[i];
    }

    return map;
}
