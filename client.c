#include "client.h"

int dx = 0, dy = 0;
int angle = 90, power = 50;
u_int16_t players_size = 0; 

void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */
    keypad(stdscr, TRUE); /* get special keys (arrows) */
    cbreak(); /* get one char at the time */
    timeout(2000); /* timeout getch(), don't wait forever */

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
    init_pair(COL_Y, COLOR_YELLOW, -1);
    init_pair(COL_B, -1, -1);
}

int main(int argc, char *argv[])
{
    /* Be sure we have proper arguments */
    if ( argc <= 2 || strlen(argv[1]) < 7
            || strlen(argv[2]) < 3)
    {
        if (DEBUG <= 5) puts("Wrong command-line arguments!");
        return EXIT_FAILURE;
    }
    
    /* Open debug_file */
    char *debug_filename = malloc(strlen(argv[2])+strlen(".debug")+1);
    strcpy(debug_filename, argv[2]);
    strcat(debug_filename, ".debug");
    debug_open(debug_filename);
    
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
    
    /* for now on we use sock (a global variable) instead of cl_sock */
    sock = cl_sock;
    
    /* join_game */
    if (join_game(argv[2]) < 0)
        return EXIT_FAILURE;/* some errors occured */

    /* Init ncurses */
    init_curses();
    
    debug_d( 1, "lines", LINES);
    debug_d( 1, "columns", COLS);

    lobby_scene();
    
    /*testing*/
    if (players[0].state) players[0].state = PS_WAITING;
    /*end of testing*/

    while (players[0].state)
    {
        center_camera(&players[0]);
        wait_scene();
        shoot_menu_scene();
    }

    free(debug_filename);
    free(map_data);
    /* Close connection */
    close(cl_sock);
    endwin();
    return EXIT_SUCCESS;
}
