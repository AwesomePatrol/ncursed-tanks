#include "client.h"
#include "math.h" //required by render_shot

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
    center_camera(players[s_id].pos);
    clear();
    render_map();
    render_tanks();
    debug_d(1, "RenderShotX", players[s_id].pos.x);
    debug_d(1, "RenderShotY", players[s_id].pos.y);
    debug_d(1, "RenderShot Angle", shot->angle);
    debug_d(1, "RenderShot Power", shot->power);
    int input_ch;
    struct f_pair init_v = initial_v(shot);
    struct f_pair acc = acceleration();
    /* position (x,y) must be either double or float */
    struct f_pair init_pos = initial_pos(&players[s_id]);
    timeout(SHOOT_TIMEOUT);
    float t=1;
    /* this part is duplicated, use do...while? */
    struct f_pair b_pos = shot_pos(init_pos, init_v, acc, t);
    struct map_position map_pos = round_to_map_pos(b_pos);
    draw_bullet(dx, dy, map_pos.x, map_pos.y);
    refresh();
    /* end */
    input_ch = getch();
    if (input_ch != ERR)
        quit_key(input_ch);
    while (players[0].state)
    {
        draw_blank_bullet(dx, dy, map_pos.x, map_pos.y);
        debug_d(1, "BulletX", map_pos.x);
        debug_d(1, "BulletY", map_pos.y);
        t+=SHOOT_TIMEOUT/100;
        b_pos = shot_pos(init_pos, init_v, acc, t);
        map_pos = round_to_map_pos(b_pos);
        draw_bullet(dx, dy, map_pos.x, map_pos.y);
        refresh();
        if (map_pos.x > map_data->length  || map_pos.x < 0)
            break;
        if (map_pos.y > map_data->height || map_pos.y >= g_map[map_pos.x]) {
            draw_bullet_explosion(dx, dy, map_pos.x, map_pos.y);
            refresh();
            break;
        }
        input_ch = getch();
        if (input_ch != ERR)
            quit_key(input_ch);
    }
    input_ch = getch(); // let players
    input_ch = getch(); // see the explosion
    center_camera(players[camera_focus].pos);
    /* SCR_ALL is already in screen update queue by center_camera*/
    timeout(DEFAULT_TIMEOUT); //back to original
}
void draw_stats()
{
    attron(COLOR_PAIR((int) COL_W));
    mvprintw(1, COLS-strlen(players[0].nickname)-6,
            "%s:%d    ", players[0].nickname, players[0].hitpoints);
    attroff(COLOR_PAIR((int) COL_W));
    attron(COLOR_PAIR((int) COL_Y));
    for (int i=1; i<players_size; i++)
        mvprintw(1+i, COLS-strlen(players[i].nickname)-6,
                "%s:%d    ", players[i].nickname, players[i].hitpoints);
    attroff(COLOR_PAIR((int) COL_Y));
}

void draw_lobby()
{
    put_col_str(players[0].state == PS_READY ? COL_G : COL_W,
            1, 1, players[0].nickname);
    for (int i=1; i<players_size; i++)
        put_col_str(players[i].state == PS_READY ? COL_G : COL_Y,
                1+i, 1, players[i].nickname);
    for (int i=0; i<players_size; i++)
        put_col_str(COL_W, 1+i, 30,
                players[i].state == PS_READY ? "READY" : "NOT READY");
    put_col_str(COL_W, LINES-3, 1,
            "Press space to mark yourself as ready");
}

void render_tanks()
{
    for (int i=0; i<players_size; i++)
        draw_tank( i == 0 ? COL_W : COL_Y,
                dx, dy, players[i].pos.x, players[i].pos.y, 0);
}

void render_map()
{
    draw_map(g_map, dx, dy, COLS, LINES);
}
