#include "client.h"
#include "client_draw.h"
#define RENDER_MARGIN 0

void put_col_str(Color color, int y, int x, const char *str)
{
    attron(COLOR_PAIR((int) color));
    mvprintw(y, x, str);
    attroff(COLOR_PAIR((int) color));
}

void draw_tank(Color color, int pos_x, int pos_y, int x, int y, int angle)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (yy > 0 && yy < (LINES-1)) {
        if (xx > 0 && xx < (COLS-1))
            put_col_str(color, yy, xx-1, "<*>");
        else
            if (xx == 0)
                put_col_str(color, yy, xx, "*>");
            else if (xx == (COLS-1))
                put_col_str(color, yy, xx-1, "<*");
    }
}

void draw_map(map_t map, int pos_x, int pos_y, int width, int height)
{
    int end = pos_x+width;
    for (int i = pos_x; i < end && i < map_data->length; i++) {
        int yy = map[i]-pos_y;
        int xx = i-pos_x;
        put_col_str(COL_G, yy, xx, "V");
        put_col_str(COL_C, yy+1, xx, "H");
        put_col_str(COL_M, yy+2, xx, "W");
    }
}

ScreenMove draw_bullet(int pos_x, int pos_y, int x, int y)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (xx < RENDER_MARGIN)
        return SCR_LEFT;
    if (xx > COLS-RENDER_MARGIN)
        return SCR_RIGHT;
    if (yy > LINES-RENDER_MARGIN)
        return SCR_DOWN;
    if (yy >= 0)
        put_col_str(COL_R, yy, xx, "#");
    else
        put_col_str(COL_R, 0, xx, "^");
    return SCR_OK;
}

void draw_blank_bullet(int pos_x, int pos_y, int x, int y)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (xx >= 0 && xx <= COLS && yy <= LINES) {
        if (yy >= 0)
            put_col_str(COL_B, yy, xx, " ");
        else
            put_col_str(COL_B, 0, xx, " ");
    }
}

void draw_shoot_menu()
{
    attron(COLOR_PAIR((int) COL_W));
    /* print values to the screen, add spaces to overwrite previous values */
    mvprintw(1, 2, "ANGLE: %d   ", angle);
    mvprintw(2, 2, "POWER: %d   ", power);
    attroff(COLOR_PAIR((int) COL_W));
}

void draw_bullet_explosion(int pos_x, int pos_y, int x, int y)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (xx > 0 && xx < COLS && yy > 0 && yy < LINES) {
        put_col_str(COL_WW, yy, xx, "X");
        put_col_str(COL_YY, yy-1, xx, "X");
        put_col_str(COL_YY, yy+1, xx, "X");
        put_col_str(COL_YY, yy, xx+1, "X");
        put_col_str(COL_YY, yy, xx-1, "X");
        put_col_str(COL_RR, yy-1, xx-1, "X");
        put_col_str(COL_RR, yy+1, xx+1, "X");
        put_col_str(COL_RR, yy-1, xx+1, "X");
        put_col_str(COL_RR, yy+1, xx-1, "X");
    }
}

void draw_stats()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        attron(COLOR_PAIR((int) (cur_pl->state == PS_DEAD) ? COL_R :
                    (cur_pl == loc_player) ? COL_W : COL_Y));
        mvprintw(1+i, COLS-strlen(cur_pl->nickname)-6,
                "%s:%d    ", cur_pl->nickname, cur_pl->hitpoints);
        attroff(COLOR_PAIR((int) (cur_pl->state == PS_DEAD) ? COL_R :
                    (cur_pl == loc_player) ? COL_W : COL_Y));
    }
}

void draw_post_game()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        attron(COLOR_PAIR((int) (cur_pl->state == PS_WINNER) ? COL_G : COL_Y));
        mvprintw(1+i, 1, "%s : %s", cur_pl->nickname,
            (cur_pl->state == PS_WINNER) ? "WINNER" : "LOSER");
        attroff(COLOR_PAIR((int) (cur_pl->state == PS_WINNER) ? COL_G : COL_Y));
    }
}

void draw_lobby()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        /* render player's nickname */
        put_col_str((cur_pl->state == PS_READY) ? COL_G :
                (cur_pl == loc_player) ? COL_W : COL_Y,
                1+i, 1, cur_pl->nickname);
        /* render player's state */
        put_col_str(COL_W, 1+i, 30,
                cur_pl->state == PS_READY ? "READY" : "NOT READY");
        /* render player's ability */
        struct ability *cur_a = find_ability(cur_pl->ability_id);
        if (cur_a != NULL)
            put_col_str((cur_pl == loc_player) ? COL_W : COL_Y,
                    1+i, 50, cur_a->name);
    }
    /* render help */
    put_col_str(COL_W, LINES-3, 1,
            "Press space to mark yourself as ready");
}
