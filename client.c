#include "client.h"

int dx = 0, dy = 0;
int angle = 90, power = 50;
int16_t loc_player_id =0;

void print_help()
{
    puts("Usage: client [OPTION]... SERVER_IP PLAYER_NAME");
    puts("OPTION may be:");
    puts("-w, --width WIDTH\t specify width (number of columns) of a window");
    puts("-h, --height HEIGHT\t specify height (number of lines) of a window");
    puts("");
    puts("Run client --help to see this message");
    puts("Terminal size must be at least 24x20");
    puts("");
    puts("For complete documentation look for ./doc in project's files");
}

void init_curses()
{
    initscr(); /* initialize screen to draw on */
    noecho(); /* do not echo any keypress */
    curs_set(FALSE); /* do not show cursor */
    keypad(stdscr, TRUE); /* get special keys (arrows) */
    cbreak(); /* get one char at the time */
    timeout(DEFAULT_TIMEOUT); /* timeout getch(), don't wait forever */

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
    unsigned int cmd_lines=0, cmd_cols=0;

    /* Help and help only */
    if ( argc > 1 )
        if ( strcmp(argv[1],"--help") == 0) {
            print_help();
            return EXIT_SUCCESS;
        }
    
    /* Too few arguments error */
    if ( argc <= 2 )
    {
        if (DEBUG <= 5) puts("Too few arguments!");
        return EXIT_FAILURE;
    }

    /* In case we have at least two extra command line arguments */
    if ( argc >= 5 )
    {
        if (strcmp(argv[1],"--width") == 0 || strcmp(argv[1],"-w") == 0)
            cmd_cols=atoi(argv[2]);
        if (strcmp(argv[1],"--height") == 0 || strcmp(argv[1],"-h") == 0)
            cmd_lines=atoi(argv[2]);
    }

    /* In case we have exactly four extra command line arguments */
    if ( argc == 7 )
    {
        if (strcmp(argv[3],"--width") == 0 || strcmp(argv[3],"-w") == 0)
            cmd_cols=atoi(argv[4]);
        if (strcmp(argv[3],"--height") == 0 || strcmp(argv[3],"-h") == 0)
            cmd_lines=atoi(argv[4]);
    }

    /* Ignore values that are not proper */
    if (cmd_cols < 24) cmd_cols=0;
    if (cmd_lines < 20) cmd_lines=0;

    /* Open debug_file */
    char *debug_filename = malloc(strlen(argv[argc-1])+strlen(".debug")+1);
    strcpy(debug_filename, argv[argc-1]);
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
    dest.sin_addr.s_addr = inet_addr(argv[argc-2]);

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
    if (join_game(argv[argc-1]) < 0)
        return EXIT_FAILURE;/* some errors occured */

    /* Init ncurses */
    init_curses();
    
    /* Set window width and height from cmd line */
    if (cmd_cols) COLS=cmd_cols;
    if (cmd_lines) LINES=cmd_lines;

    debug_d( 1, "lines", LINES);
    debug_d( 1, "columns", COLS);

    lobby_scene();
    
    while (loc_player->state)
    {
        wait_scene();
        shoot_menu_scene();
    }

    /*free!*/
    for (int i=0; i<Players.count; i++)
        clear_player(dyn_arr_get(&Players,i));
    dyn_arr_clear(&Players);
    free(debug_filename);
    free(map_data);

    /* Close connection */
    close(cl_sock);
    endwin();
    return EXIT_SUCCESS;
}
