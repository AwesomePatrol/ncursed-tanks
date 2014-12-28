#ifndef TANK_MENU_H
#define TANK_MENU_H

#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <menu.h>

#include "debug.h"
#include "config.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

ITEM **my_items;
int c;
MENU *my_menu;
int n_choices, i;

struct choice_info
{
    char *description;
    char *type_description;
};

extern const struct choice_info choices[];

#endif /* TANK_MENU_H */
