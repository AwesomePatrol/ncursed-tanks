#include "client.h"

#define MAX_POWER 100

/* move camera using i,k,j,l keys */
int camera_move(int input_character)
{
    switch (input_character)
    {
        case 'i':
            if (dy > 0)
                dy--;
            break;
        case 'k':
            if (dy < (map_data->height-LINES))
                dy++;
            break;
        case 'j':
            if (dx > 0)
                dx--;
            break;
        case 'l':
            if (dx < (map_data->length-COLS))
                dx++;
            break;
        default:
            return 0;
    }
    debug_d(1, "dx", dx);
    debug_d(1, "dy", dx);
    return 1;
}

void shoot(int sock)
{
    /*
    send_shoot(int sock);
    render_shoot();
    */ 
}

/* start quit or write debug, always run on the end of all key functions*/
int quit_key(int input_character)
{
    switch(input_character)
    {
        case 'q':
            players[0].state = PS_NO_PLAYER;
            return 1;
            break;
        default:
            /* in this case we shouldn't redraw the screen */
            debug_c(1, "unsupported key", input_character);
    }
    return 0;
}
/* manage up,down,lef,right keys in shoot_menu */
int shoot_menu(int input_character, int sock)
{
    switch (input_character)
    {
        case KEY_UP:
            if (power < MAX_POWER) power++;
            break;
        case KEY_DOWN:
            if (power > 0) power--;
            break;
        case KEY_RIGHT:
            if (angle > 0) angle--;
            break;
        case KEY_LEFT:
            if (angle < 180) angle++;
            break;
        case KEY_ENTER:
            shoot(sock);
            break;
        default:
            return 0;
    }
    return 1;
}

void shoot_menu_scene(int sock)
{
    int input_ch;
    while (players[0].state == PS_SHOOT)
    {
        fetch_changes(sock);
        clear();
        render_map();
        render_tanks();
        draw_stats();
        refresh();
        input_ch = getch();
        if (camera_move(input_ch) || shoot_menu(input_ch, sock))
            continue;
        quit_key(input_ch);
    }
}

void wait_scene(int sock)
{
    int input_ch;
    while (players[0].state == PS_WAIT)
    {
        fetch_changes(sock);
        clear();
        render_map();
        render_tanks();
        draw_stats();
        refresh();
        input_ch = getch();
        if (camera_move(input_ch))
            continue;
        quit_key(input_ch);
    }
}
