#include "client.h"

int dx=0, dy=0;
State state = WAIT;

void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */
    keypad(stdscr, TRUE); /* get special keys (arrows) */
    cbreak(); /* get one char at the time */

    /* Initialize colors */
    start_color();
    use_default_colors();
    init_pair(COL_W, COLOR_WHITE, -1);
    init_pair(COL_R, COLOR_RED, -1);
    init_pair(COL_RR, COLOR_RED, COLOR_RED);
    init_pair(COL_YY, COLOR_YELLOW, COLOR_YELLOW);
    init_pair(COL_WW, COLOR_WHITE, COLOR_WHITE);
    init_pair(COL_G, COLOR_GREEN, -1);
    init_pair(COL_C, COLOR_CYAN, -1);
    init_pair(COL_M, COLOR_MAGENTA, -1);
    init_pair(COL_B, -1, -1);
}

int main(int argc, char *argv[])
{
    /* Open debug_file */
    debug_open("client.debug"); 

    /* Be sure we have proper arguments */
    if ( argc <= 2 || strlen(argv[1]) < 7
            || strlen(argv[2]) < 3)
    {
        if (DEBUG <= 5) puts("Wrong command-line arguments!");
        return EXIT_FAILURE;
    }

    /* Init ncurses */
    init_curses();
    
    /* Get connection to server */
    int cl_sock;
    struct sockaddr_in dest; 
 
    cl_sock = socket(AF_INET, SOCK_STREAM, 0);

    /* zero the struct */
    memset(&dest, 0, sizeof(dest));
    
    /* support for IPv4 */
    dest.sin_family = AF_INET;
    
    /* set destination IP number */ 
    dest.sin_addr.s_addr = inet_addr(argv[1]);

    /* set destination port number */
    dest.sin_port = htons(PORTNUM);

    /* in case socket or connection is broken we should fail here */
    if ( cl_sock == -1 ||
         connect(cl_sock, (struct sockaddr *)&dest, sizeof(struct sockaddr))
         == -1 )
    {
        if (DEBUG <= 5) puts("Socket or connection is broken!");
        return EXIT_FAILURE;
    }
    
    fetch_map(cl_sock);
    debug_d( 1, "lines", LINES);
    debug_d( 1, "columns", COLS);

    short int change = 1;
    while (state == WAIT)
    {
        if (change) clear();
        render_map();
        refresh();
        char input_ch = getch();
        change = 1;
        switch (input_ch)
        {
            case 'i':
                if (dy > 0)
                    dy--;
                else change = 0;
                break;
            case 'k':
                if (dy < (map_data.height-LINES))
                    dy++;
                else change = 0;
                break;
            case 'j':
                if (dx > 0)
                    dx--;
                else change = 0;
                break;
            case 'l':
                if (dx < (map_data.length-COLS))
                    dx++;
                else change = 0;
                break;
            case 'q':
                state = EXIT;
                break;
            default:
                debug_c(1, "unsupported key", input_ch);
        }
        debug_d(1, "dx", dx);
        debug_d(1, "dy", dx);
    }


    /* Close connection */
    close(cl_sock);
    endwin();
    return EXIT_SUCCESS;
}
