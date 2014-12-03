#include "client.h"

inline void put_col_str(Color color, int y, int x, const char *str)
{
    attron(COLOR_PAIR((int) color));
    mvprintw(y, x, str);
    attroff(COLOR_PAIR((int) color));
}
