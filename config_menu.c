#include "config_menu.h"

/* Dependent on the order of config variables. Very bad, needs to be fixed. */
/* TODO Get the min/max values from config[] directly */
const struct choice_info choices[] = {
    {"number of players",        "INT {1..16}"},
    {"map width",                "INT {48..1024}"},
    {"map height",               "INT {48..1024}"},
    {"tank HP",                  "INT {1..1000}"},
    {"dmg radius",               "INT {2..16}"},
    {"dmg cap",                  "INT {1..1000}"},
    /* TODO change that or something */
    {"gravity",                  "INT {1..10000}"},
    {"wind",                     "INT {-10000..10000}"},
    {"power/coefficient",        "INT {20..100}"},
    {"map margin with no tanks", "INT {2..128}"},
    {"distance between tanks",   "INT {1..128}"},
};


void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */
    keypad(stdscr, TRUE); /* get special keys (arrows) */
    cbreak(); /* get one char at the time */
}

void draw_values()
{
    for (int i=0; i<n_choices; i++)
    {
        mvprintw(i, 50, " %d    ", config[i].value);
        /* spaces are added to overdraw prevoius output */
    }
}

void item_change(ITEM *curr_it, const char cr)
{
    int index_it = item_index(curr_it);
    if (cr == '+' && config[index_it].value < config[index_it].max)
        config[index_it].value++;
    else if (cr == '-' && config[index_it].value > config[index_it].min)
        config[index_it].value--;
    debug_d(1, "item_index", index_it);
    debug_c(1, "item_char", cr);
    draw_values();
} 

int main(int argv, char *argc[])
{
    /* Open debug_file */
    debug_open("menu.debug");

    init_curses();

    if (LINES < 24 || COLS < 36) {
        endwin();
        puts("Your screen is too small!");
        return EXIT_FAILURE;
    }
    
    read_config();
    
    debug_s(1, "opened file", "server.conf");

    n_choices = ARRAY_SIZE(choices);
    my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

    for(i = 0; i < n_choices; ++i)
            my_items[i] = new_item(choices[i].description,
                                   choices[i].type_description);
    my_items[n_choices] = (ITEM *)NULL;

    my_menu = new_menu((ITEM **)my_items);
    mvprintw(LINES - 6, 1, "left and right arrow to change values");
    mvprintw(LINES - 4, 1, "enter to write server.conf");
    mvprintw(LINES - 2, 1, "q to quit");
    post_menu(my_menu);
    draw_values();
    refresh();

    while((c = getch()) != 'q')
    {   switch(c)
        {   case KEY_DOWN:
                menu_driver(my_menu, REQ_DOWN_ITEM);
                break;
            case KEY_UP:
                menu_driver(my_menu, REQ_UP_ITEM);
                break;
            case KEY_RIGHT:
                item_change(current_item(my_menu),'+');
                break;
            case KEY_LEFT:
                item_change(current_item(my_menu),'-');
                break;
            case 10:
                write_config();
                mvprintw(LINES - 8, 1,
                         "Configuration saved in \""
                         SERVER_CONFIG_FILENAME "\"");
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
