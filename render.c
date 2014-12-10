#include "client.h"
#include "math.h" //required by render_shot

#define GRAV 0.05 //TODO move to common.h
#define M_PI 3.1415

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
    if (xx >= 0 && xx <= LINES && yy <= COLS) {
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
    if (xx >= 0 && xx <= LINES && yy <= COLS) {
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

void render_shot(int s_angle, int s_power, int s_id)
{
    debug_d(1, "RenderShotX", players[s_id].pos.x);
    debug_d(1, "RenderShotY", players[s_id].pos.y);
    debug_d(1, "RenderShot Angle", s_angle);
    debug_d(1, "RenderShot Power", s_power);
    int input_ch;
    //TODO for specific values be more 
    double radians = s_angle * M_PI / 180;
    float v_x = s_power * cos(radians) / 100;
    float v_y = s_power * sin(radians) / 100;
    int x = players[s_id].pos.x;
    int y = players[s_id].pos.y;
    timeout(200); //5 fps
    int fly=1;
    while (fly)
    {
        draw_blank_bullet(dx, dy, x, y);
        x += v_x;
        y -= v_y;
        v_y -= GRAV;
        debug_d(1, "BulletX", x);
        debug_d(1, "BulletY", y);
        draw_bullet(dx, dy, x, y);
        refresh();
        if (x > map_data->length  || x < 0 || y > map_data->height ||
                y > g_map[x])
            fly = 0;
        input_ch = getch();
        if (input_ch != ERR)
            quit_key(input_ch);
    }
    /* add SCR_ALL to screen update queue */
    ScreenUpdate shot_done = SCR_ALL;
    dyn_arr_append(&ScrUpdates, &shot_done);
    timeout(2000); //back to original
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
