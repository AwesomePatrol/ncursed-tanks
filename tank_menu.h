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

const char *choices[] = {
                        "number of players",
                        "map width",
                        "map height",
                        "tank HP",
                        "dmg radius",
                        "dmg cap",
                        "gravity",
                        "power/coefficient",
                        "map margin with no tanks",
                        "distance between tanks",
                  };

char *explanation[] = {
                        "INT {1..16}",
                        "INT {48..1024}",
                        "INT {48..1024}",
                        "INT {1..1000}",
                        "INT {2..16}",
                        "INT {1..1000}",
                        "INT {1..10}",
                        "INT {20..100}",
                        "INT {2..128}",
                        "INT {1..128}",
                  };

#endif /* TANK_MENU_H */
