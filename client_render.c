#include "client.h"
/* the best value of RENDER_MARGIN is 0 for time being
 * other values cause SCR_ALL multiple times */

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
        switch (draw_bullet(dx, dy, map_pos.x, map_pos.y)) {
            case SCR_OK:
                break;
            case SCR_UP:
            case SCR_DOWN:
            case SCR_LEFT:
            case SCR_RIGHT:
                center_camera(map_pos);
                clear();
                render_map();
                render_tanks();
                draw_bullet(dx, dy, map_pos.x, map_pos.y);
                break;
            default:
                debug_s(5, "ScreenMove(shot)", "Wrong ScrMove value");
        }
        refresh();
        if (map_pos.x > map_data->length  || map_pos.x < 0)
            break;
        if (t > g_impact_t) {
            b_pos = shot_pos(init_pos, init_v, acc, g_impact_t);
            map_pos = round_to_map_pos(b_pos);
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

void render_post_game()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        attron(COLOR_PAIR((int) (cur_pl->state == PS_WINNER) ? COL_G : COL_Y));
        mvprintw(1+i, 1, "%s : %s", cur_pl->nickname,
            (cur_pl->state == PS_WINNER) ? "WINNER" : "LOSER");
        attroff(COLOR_PAIR((int) (cur_pl->state == PS_WINNER) ? COL_G : COL_Y));
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

void render_tanks()
{
    for (int i=0; i<Players.count; i++) {
        struct player *cur_pl = dyn_arr_get(&Players, i);
        if (cur_pl->state != PS_DEAD)
            draw_tank( cur_pl == loc_player ? COL_W : COL_Y,
                dx, dy, cur_pl->pos.x, cur_pl->pos.y, 0);
    }
}

void render_map()
{
    draw_map(g_map, dx, dy, COLS, LINES);
}
