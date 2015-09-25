/*Ncursed-tanks client, writen by patrol (Aleksander Mistewicz),
 * licensed under the MIT license. :
 * Copyright (c) 2015, Aleksander Mistewicz
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * */
#include "client.h"
#include "colors.h"

/* initialize GLOBAL variables */
int dx = 0, dy = 0;
int angle = 90, power = 50;
struct player *loc_player = NULL;
int16_t loc_player_id =0;

/* initialize LOCAL variables with DEFAULT values */
unsigned int cmd_lines=0, cmd_cols=0;
char default_portnum[] = "7979";
char *portnum = default_portnum;
char *nick;
char *debug_filename;

/* Prints usage and exits */
void print_usage(char *argv[])
{
    printf(
"Usage: %s [OPTION]... SERVER_ADDR PLAYER_NAME\n\
OPTION may be:\n\
	-c, --cols COLS		specify width (number of columns) of a window\n\
	-l, --lines LINES	specify height (number of lines) of a window\n\
	-p, --port PORT		specify on what port client connects to a server\n\
	-h, -?, --help		show this message\n\
\n\
Terminal size must be at least 24x20\n\
Wrong values will be ignored\n\
\n\
For complete documentation look for ./doc in project's files\n\
",
argv[0]);

    // exit required because we'll be called by getopt on wrong arguments
    exit(EXIT_FAILURE);
}

void parse_commandline(int argc, char *argv[])
{
    const char *opt_string = "c:l:p:h?";
    const struct option long_opts[] = {
        { "cols",  required_argument, NULL, 'c' },
        { "lines", required_argument, NULL, 'l' },
        { "port",  required_argument, NULL, 'p' },
        { "help",  no_argument,       NULL, 'h' },
    };

    int opt = 0;
    do
    {
        opt = getopt_long(argc, argv, opt_string, long_opts, NULL);

        switch (opt)
        {
        case 'c':
            cmd_cols = atoi(optarg);
            break;

        case 'l':
            cmd_lines = atoi(optarg);
            break;

        case 'p':
            portnum = optarg;
            break;

        case 'h':
            print_usage(argv);
            break;
        }
    } while (opt != -1);

    /* We require 2 arguments */
    if (argc - optind < 2) {
        if (DEBUG <= 5) fputs("2 arguments are required!\n", stderr);
        print_usage(argv);
    }

    /* Ignore values that are not proper */
    if (cmd_cols < 24) cmd_cols=0;
    if (cmd_lines < 20) cmd_lines=0;

    g_servername=argv[optind];
    optind++;

    nick = argv[optind];

    debug_filename = malloc(strlen(nick) +
                            strlen(".debug") + 1);
    strcpy(debug_filename, nick);
    strcat(debug_filename, ".debug");
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

bool client_connect(char *servername)
{
    /* Get connection to server */
    int cl_sock;
    struct addrinfo hints, *servlist, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(servername, portnum, &hints, &servlist) != 0)
        return false;

    for (p = servlist; p != NULL; p = p->ai_next) {
           cl_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (cl_sock == -1)
                continue;
            if (connect(cl_sock, p->ai_addr, p->ai_addrlen) != -1)
                break; /* for now on we have connection with server */
    }

    /* in case socket or connection is broken we should fail here */
    if ( p == NULL ) {
        if (DEBUG <= 5) puts("Socket or connection is broken!");
        return false;
    }
    
    /* for now on we use sock (a global variable) instead of cl_sock */
    sock = cl_sock;

    /* this is no longer needed, free! */
    freeaddrinfo(servlist);
    return true;
}

int main(int argc, char *argv[])
{
    parse_commandline(argc, argv);

    /* Open debug_file */
    if (DEBUG <= 5)
        debug_open(debug_filename);
    
    /* Get connection to server */
    if (!client_connect(g_servername))
        return EXIT_FAILURE;

    /* join_game */
    if (join_game(nick) < 0)
        return EXIT_FAILURE;/* some errors occured */

    /* Init ncurses */
    init_curses();
    
    /* Set window width and height from cmd line */
    if (cmd_cols) COLS=cmd_cols;
    if (cmd_lines) LINES=cmd_lines;

    debug_d( 1, "lines", LINES);
    debug_d( 1, "columns", COLS);

    /* main loop */
    while (loc_player->state) {
        if (loc_player->state == PS_READY ||
            loc_player->state == PS_JOINED) lobby_scene();
        if (loc_player->state == PS_WAITING ||
            loc_player->state == PS_DEAD) wait_scene();
        if (loc_player->state == PS_ACTIVE) shoot_menu_scene();
        if (loc_player->state == PS_WINNER ||
            loc_player->state == PS_LOSER) post_game_scene();
    }

    /* free! */
    for (int i=0; i<Players.count; i++)
        clear_player(dyn_arr_get(&Players,i));
    dyn_arr_clear(&Players);
    free(debug_filename);
    free(map_data);

    /* Close connection */
    close(sock);
    endwin();
    return EXIT_SUCCESS;
}
