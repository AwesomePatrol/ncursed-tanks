#include "tank_menu.h"

void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */
    keypad(stdscr, TRUE); /* get special keys (arrows) */
    cbreak(); /* get one char at the time */
}

int main(int argv, char *argc[])
{
    /* Open debug_file */
    debug_open("menu.debug");

    init_curses();
    
    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

    for(i = 0; i < n_choices; ++i)
            my_items[i] = new_item(choices[i], explanation[i]);
    my_items[n_choices] = (ITEM *)NULL;

    my_menu = new_menu((ITEM **)my_items);
    mvprintw(LINES - 2, 0, "EXIT or q to quit");
    post_menu(my_menu);
    refresh();

    while((c = getch()) != 'q')
    {   switch(c)
        {   case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            default:
                debug_c(1, "unsupported key", c);
        }
    }

    unpost_menu(my_menu);
    for(i = 0; i < n_choices; ++i)
                free_item(my_items[i]);
        free_menu(my_menu);

    endwin();
    return EXIT_SUCCESS;
}
