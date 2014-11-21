#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>
#include "map_gen.h"
#include "colors.h"

void put_col_str(Color color, int y, int x, const char *str);
void draw_map(map_t map, int pos_x, int pos_y, int width, int height);

#endif /* DRAW_H */
