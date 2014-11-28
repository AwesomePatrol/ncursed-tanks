#ifndef TANK_MENU_H
#define TANK_MENU_H

#include <stdlib.h>
#include <ncurses.h>
#include <menu.h>

#include "debug.h"
#include "config.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

ITEM **my_items;
int c;
MENU *my_menu;
int n_choices, i;

char *choices[] = {
                        "number of players",
                        "map width",
                        "map height",
                        "tank HP",
                        "dmg radius",
                        "dmg cap",
                  };

char *explanation[] = {
                        "INT {1..16}",
                        "INT {48..1024}",
                        "INT {48..1024}",
                        "INT {1..1000}",
                        "INT {2..16}",
                        "INT {1..1000}",
                  };

#endif /* TANK_MENU_H */
