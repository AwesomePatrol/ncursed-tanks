#include "client.h"
#include "client_keys.h"

#define MAX_POWER 100
#define SHOOT_FAST_STEP 2
int camera_focus=0;
struct dyn_arr ScrUpdates = { sizeof(ScreenUpdate) };
struct dyn_arr MapUpdates = { sizeof(struct map_position) };
struct dyn_arr Players = { sizeof(struct player) };
struct dyn_arr Abilities = { sizeof(struct ability) };
int shoot_last_key=0;
unsigned int shoot_press_key=0;

/* find ability by id */
struct ability *find_ability(int16_t id)
{
    for (int i=0; i<Abilities.count; i++) {
        struct ability *cur_a = dyn_arr_get(&Abilities, i);
        if (cur_a->id == id)
            return cur_a;
    }
    return NULL;
}

/* move camera using i,k,j,l keys */
int camera_move(int input_character)
{
    switch (input_character) {
        case K_CAM_UP:
            if (dy > 0)
                dy--;
            break;
        case K_CAM_DOWN:
            if (dy < (map_data->height - LINES/2))
                dy++;
            break;
        case K_CAM_LEFT:
            if (dx > 0)
                dx--;
            break;
        case K_CAM_RIGHT:
            if (dx < (map_data->length - COLS))
                dx++;
            break;
        default:
            return 0;
    }
    /* add SCR_ALL to screen update queue */
    ScreenUpdate camera_move = SCR_ALL;
    dyn_arr_append(&ScrUpdates, &camera_move);
    debug_d(1, "dx", dx);
    debug_d(1, "dy", dx);
    return 1;
}

/* use m,n keys to change camera focus */
int change_camera_focus(int input_character)
{
    switch (input_character) {
        case K_NEXT_PLAYER:
            camera_focus++;
            break;
        case K_PREV_PLAYER:
            camera_focus--;
            break;
        default:
            return 0;
    }
    if (camera_focus > Players.count-1) camera_focus=0;
    if (camera_focus < 0) camera_focus=Players.count;
    struct player *c_player = dyn_arr_get(&Players, camera_focus);
    center_camera(c_player->pos);
    return 1;
}

void center_camera(struct map_position d_pos)
{
    dx = d_pos.x - COLS/2;
    if (dx > (map_data->length - COLS))
        dx = map_data->length - COLS;
    else if (dx < 0)
        dx = 0;
    dy = d_pos.y - LINES/2;
    if (dy > (map_data->height - LINES/2))
        dy = map_data->height - LINES/2;
    else if (dy < 0)
        dy = 0;
    /* add SCR_ALL to screen update queue */
    ScreenUpdate camera_move = SCR_ALL;
    dyn_arr_append(&ScrUpdates, &camera_move);
}

/* start quit or write debug, always run on the end of all key functions*/
int quit_key(int input_character)
{
    switch(input_character) {
        case K_EXIT:
            loc_player->state = PS_NO_PLAYER;
            return 1;
            break;
        default:
            /* in this case we shouldn't redraw the screen */
            debug_c(1, "unsupported key", input_character);
    }
    return 0;
}

/* manage up,down,lef,right keys in shoot_menu */
int shoot_menu(int input_character)
{
    switch (input_character) {
        case KEY_UP:
            if (power < MAX_POWER) power++;
            break;
        case KEY_DOWN:
            if (power > 1) power--;
            break;
        case KEY_RIGHT:
            if (angle > 0) angle--;
            break;
        case KEY_LEFT:
            if (angle < 180) angle++;
            break;
        case K_SHOOT: /* KEY_ENTER does not work */
            debug_s(1, "send", "shoot");
            send_shoot();
            break;
        default:
            if (shoot_press_key>0) shoot_press_key--;
            return 0;
    }
    if (shoot_last_key == input_character) {
        if (shoot_press_key > 3) {
            switch (input_character) {
            case KEY_UP:
                if (power < MAX_POWER-SHOOT_FAST_STEP) power+=SHOOT_FAST_STEP;
                break;
            case KEY_DOWN:
                if (power > SHOOT_FAST_STEP) power-=SHOOT_FAST_STEP;
                break;
            case KEY_RIGHT:
                if (angle > SHOOT_FAST_STEP) angle-=SHOOT_FAST_STEP;
                break;
            case KEY_LEFT:
                if (angle < 180-SHOOT_FAST_STEP) angle+=SHOOT_FAST_STEP;
                break;
            }
        }
        else
            shoot_press_key++;
    } else {
        shoot_last_key=input_character;
        shoot_press_key=0;
    }
    /* add SCR_SHOOT_MENU to screen update queue */
    ScreenUpdate u_shoot_menu = SCR_SHOOT_MENU;
    dyn_arr_append(&ScrUpdates, &u_shoot_menu);
    return 1;
}

/* manage space in lobby */
int lobby_menu(int input_character)
{
    switch (input_character) {
        case KEY_LEFT:
            loc_player->ability_id--;
            if (Abilities.count == 0) {
                loc_player->ability_id++;
                break;
            }
            if (find_ability(loc_player->ability_id) == NULL) {
                struct ability *tmp_a = dyn_arr_get(&Abilities,
                        Abilities.count-1);
                loc_player->ability_id = tmp_a->id;
            }
            break;
        case KEY_RIGHT:
            loc_player->ability_id++;
            if (Abilities.count == 0) {
                loc_player->ability_id--;
                break;
            }
            if (find_ability(loc_player->ability_id) == NULL) {
                struct ability *tmp_a = dyn_arr_get(&Abilities, 0);
                loc_player->ability_id = tmp_a->id;
            }
            break;
        case K_SET_READY:
            if (loc_player->state == PS_JOINED) {
                send_int8(sock, C_READY);
                fetch_changes();
            }
            break;
        default:
            return 0;
    }
    return 1;
}
