#include "client.h"

void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (yy > 0 && yy < (LINES-1))
        if (xx > 0 && xx < (COLS-1))
            put_col_str(color, yy, xx-1, "<*>");
        else
            if (xx == 0)
                put_col_str(color, yy, xx, "*>");
            else if (xx == (COLS-1))
                put_col_str(color, yy, xx-1, "<*");
}

void draw_map(map_t map, int pos_x, int pos_y, int width, int height)
{
    int end = pos_x+width;
    for (int i = pos_x; i < end && i < map_data->length; i++)
    {
        int yy = map[i]-pos_y;
        int xx = i-pos_x;
        put_col_str(COL_G, yy, xx, "V");
        put_col_str(COL_C, yy+1, xx, "H");
        put_col_str(COL_M, yy+2, xx, "W");
    }
}

void draw_shoot_menu()
{
    attron(COLOR_PAIR((int) COL_W));
    /* print values to the screen, add spaces to overwrite previous values */
    mvprintw(2, 2, "ANGLE: %d   ", angle);
    mvprintw(4, 2, "POWER: %d   ", power);
    attroff(COLOR_PAIR((int) COL_W));
}

void draw_stats()
{
    attron(COLOR_PAIR((int) COL_R));
    mvprintw(2, COLS-strlen(players[0].nickname)-6,
            "%s:%d    ", players[0].nickname, players[0].hitpoints);
    attroff(COLOR_PAIR((int) COL_R));
    attron(COLOR_PAIR((int) COL_W));
    for (int i=1; i<players_size; i++)
        mvprintw(2 + 2*i, COLS-strlen(players[i].nickname)-6,
                "%s:%d    ", players[i].nickname, players[i].hitpoints);
    attroff(COLOR_PAIR((int) COL_W));
}

void render_tanks()
{
    for (int i=0; i<players_size; i++)
        draw_tank( i == 0 ? COL_R : COL_W,
                dx, dy, players[i].pos_x, players[i].pos_y, 0);
}

void render_map()
{
    draw_map(g_map, dx, dy, COLS, LINES);
}
