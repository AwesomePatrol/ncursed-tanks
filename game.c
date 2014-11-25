#include "client.h"

/* move camera using i,k,j,l keys */
int camera_move(char input_character)
{
    switch (input_character)
    {
        case 'i':
            if (dy > 0)
                dy--;
            break;
        case 'k':
            if (dy < (map_data.height-LINES))
                dy++;
            break;
        case 'j':
            if (dx > 0)
                dx--;
            break;
        case 'l':
            if (dx < (map_data.length-COLS))
                dx++;
            break;
        default:
            return 0;
    }
    debug_d(1, "dx", dx);
    debug_d(1, "dy", dx);
    return 1;
}
