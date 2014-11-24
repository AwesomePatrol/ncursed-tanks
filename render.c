#include "draw.h"

void draw_map(map_t map, int pos_x, int pos_y, int width, int height)
{
    int end = pos_x+width;
    for (int i = pos_x; i < end && i < map_data.length; i++)
    {
        int yy = map[i]-pos_y;
        put_col_str(COL_G, yy, i, "V");
        put_col_str(COL_C, yy, i, "H");
        put_col_str(COL_M, yy, i, "W");
    }
}

void render_map()
{
    draw_map(g_map, 0, 0, COLS, LINES);
}
