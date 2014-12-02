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

int shoot_menu(int input_character)
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
        default:
            return 0;
    }
    return 1;
}
