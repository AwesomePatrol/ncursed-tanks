#include "draw.h"

void put_col_str(Color color, int y, int x, const char *str)
{
    attron((int) color);
    mvprintw(y, x, str);
    attroff((int) color);
}
