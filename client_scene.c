#include "client.h"

void map_update()
{
    for (int i=0; i<MapUpdates.count; i++)
    {
        struct map_position *map_u = dyn_arr_get(&MapUpdates, i);
        g_map[map_u->x]=map_u->y;
    }
    dyn_arr_clear(&MapUpdates);
}

void render_scene()
{
    /* TODO: shorten this code */
    bool shoot=false, lobby=false, stats=false, tanks=false, shoot_menu=false, map=false;
    for (int i = false; i < ScrUpdates.count; i++)
    {
        ScreenUpdate *scr_u = dyn_arr_get(&ScrUpdates, i);
        switch (*scr_u) {
            case SCR_SHOOT:
                shoot=true;
                break;
            case SCR_LOBBY:
                lobby=true;
                break;
            case SCR_STATS:
                stats=true;
                break;
            case SCR_TANKS:
                tanks=true;
                break;
            case SCR_SHOOT_MENU:
                shoot_menu=true;
                break;
            case SCR_MAP:
                map=true;
                break;
            case SCR_ALL:
                lobby=true;
                stats=true;
                tanks=true;
                shoot_menu=true;
                map=true;
                break;
            default:
                debug_d(5, "UnknownValueScrUpdate", *scr_u);
        }
    }
    dyn_arr_clear(&ScrUpdates);
    if (shoot || lobby || stats || tanks || shoot_menu || map) {
        if (shoot) { /* it's a VERY special case */
            render_shot(&s_update.shot,
                        find_player(s_update.player_id));
            map_update();
        } else {
            switch (loc_player->state) {
                case PS_JOINED:
                case PS_READY:
                    if (lobby) {
                        clear();
                        draw_lobby();
                    }
                    break;
                case PS_DEAD:
                case PS_WAITING:
                    if (map) {
                        clear();
                        render_map();
                    }
                    if (tanks) render_tanks();
                    if (stats) draw_stats();
                    break;
                case PS_ACTIVE:
                    if (map) {
                        clear();
                        render_map();
                    }
                    if (tanks) render_tanks();
                    if (stats) draw_stats();
                    if (shoot_menu) draw_shoot_menu();
                    break;
                default:
                    debug_d(5, "UnknownState", loc_player->state);
            }
            refresh();
        }
    }
}

void lobby_scene()
{
    int input_ch;
    /* initial render */
    clear();
    draw_lobby();
    refresh();
    while (loc_player->state == PS_JOINED
            || loc_player->state == PS_READY)
    {
        fetch_changes();
        render_scene();
        input_ch = getch();
        if (input_ch == ERR)
            continue;
        if (loc_player->state == PS_JOINED && lobby_menu(input_ch))
            continue;
        quit_key(input_ch);
    }
}


void shoot_menu_scene()
{
    int input_ch;
    /* initial render */
    center_camera(loc_player->pos);
    clear();
    render_map();
    render_tanks();
    draw_stats();
    draw_shoot_menu();
    refresh();
    while (loc_player->state == PS_ACTIVE)
    {
        fetch_changes();
        render_scene();
        input_ch = getch();
        if (input_ch == ERR)
            continue;
        if (camera_move(input_ch) || shoot_menu(input_ch)
                || change_camera_focus(input_ch))
            continue;
        quit_key(input_ch);
    }
}

void wait_scene()
{
    int input_ch;
    /* initial render */
    struct player *c_player = dyn_arr_get(&Players, camera_focus);
    center_camera(c_player->pos);
    clear();
    render_map();
    render_tanks();
    draw_stats();
    refresh();
    while (loc_player->state == PS_WAITING
            || loc_player->state == PS_DEAD)
    {
        fetch_changes();
        render_scene();
        input_ch = getch();
        if (input_ch == ERR)
            continue;
        if (camera_move(input_ch) || change_camera_focus(input_ch))
            continue;
        quit_key(input_ch);
    }
}

void post_game_scene()
{
    int input_ch;
    /* initial render */
    clear();
    render_post_game();
    refresh();
    while (loc_player->state == PS_WINNER
            || loc_player->state == PS_LOSER)
    {
        fetch_changes();
        input_ch = getch();
        if (input_ch == ERR)
            continue;
        quit_key(input_ch);
        break;
    }
}
