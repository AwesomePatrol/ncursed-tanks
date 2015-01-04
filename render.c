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

void draw_bullet(int pos_x, int pos_y, int x, int y)
{
    int xx = x-pos_x;
    int yy = y-pos_y;
    if (xx >= 0 && xx <= COLS && yy <= LINES) {
        if (yy >= 0)
            put_col_str(COL_R, yy, xx, "#");
        else
            put_col_str(COL_R, 0, xx, "^");
    }
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

void render_shot(struct shot *shot, int s_id)
{
    struct player *shoot_pl = dyn_arr_get(&Players, s_id);
    center_camera(shoot_pl->pos);
    clear();
    render_map();
    render_tanks();
    debug_d(1, "RenderShotX", shoot_pl->pos.x);
    debug_d(1, "RenderShotY", shoot_pl->pos.y);
    debug_d(1, "RenderShot Angle", shot->angle);
    debug_d(1, "RenderShot Power", shot->power);
    int input_ch;
    struct f_pair init_v = initial_v(shot);
    struct f_pair acc = acceleration();
    /* position (x,y) must be either double or float */
    struct f_pair init_pos = map_pos_to_float(shoot_pl->pos);
    timeout(SHOOT_TIMEOUT);
    float t=1;
    /* this part is duplicated, because it's initial */
    struct f_pair b_pos = shot_pos(init_pos, init_v, acc, t);
    struct map_position map_pos = round_to_map_pos(b_pos);
    draw_bullet(dx, dy, map_pos.x, map_pos.y);
    refresh();
    /* end */
    input_ch = getch();
    if (input_ch != ERR)
        quit_key(input_ch);
    while (loc_player->state)
    {
        /* remove drew bullet */
        draw_blank_bullet(dx, dy, map_pos.x, map_pos.y);
        debug_d(1, "BulletX", map_pos.x);
        debug_d(1, "BulletY", map_pos.y);
        t+=(float)SHOOT_TIMEOUT/100;
        b_pos = shot_pos(init_pos, init_v, acc, t);
        map_pos = round_to_map_pos(b_pos);
        /* draw a new one */
        draw_bullet(dx, dy, map_pos.x, map_pos.y);
        refresh();
        if (map_pos.x > map_data->length  || map_pos.x < 0)
            break;
        if (map_pos.y > map_data->height || map_pos.y >= g_map[map_pos.x]) {
            draw_bullet_explosion(dx, dy, map_pos.x, map_pos.y);
            refresh();
            break;
        }
        /* let player see the change */
        input_ch = getch();
        if (input_ch != ERR)
            quit_key(input_ch);
    }
    input_ch = getch(); // let players
    input_ch = getch(); // see the explosion
    struct player *c_player = dyn_arr_get(&Players, camera_focus);
    center_camera(c_player->pos);
    /* SCR_ALL is already in screen update queue by center_camera*/
    timeout(DEFAULT_TIMEOUT); //back to original
}
void draw_stats()
{
    attron(COLOR_PAIR((int) COL_W));
    mvprintw(1, COLS-strlen(loc_player->nickname)-6,
            "%s:%d    ", loc_player->nickname, loc_player->hitpoints);
    attroff(COLOR_PAIR((int) COL_W));
    attron(COLOR_PAIR((int) COL_Y));
    for (int i=1; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        mvprintw(1+i, COLS-strlen(cur_pl->nickname)-6,
                "%s:%d    ", cur_pl->nickname, cur_pl->hitpoints);
    }
    attroff(COLOR_PAIR((int) COL_Y));
}

void draw_lobby()
{
    put_col_str(loc_player->state == PS_READY ? COL_G : COL_W,
            1, 1, loc_player->nickname);
    for (int i=1; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        put_col_str(cur_pl->state == PS_READY ? COL_G : COL_Y,
                1+i, 1, cur_pl->nickname);
    }
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        put_col_str(COL_W, 1+i, 30,
                cur_pl->state == PS_READY ? "READY" : "NOT READY");
    }
    put_col_str(COL_W, LINES-3, 1,
            "Press space to mark yourself as ready");
}

void render_tanks()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        if (cur_pl->state != PS_DEAD)
            draw_tank( i == 0 ? COL_W : COL_Y,
                dx, dy, cur_pl->pos.x, cur_pl->pos.y, 0);
    }
}

void render_map()
{
    draw_map(g_map, dx, dy, COLS, LINES);
}
