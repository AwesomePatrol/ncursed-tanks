/*Ncursed-tanks bot, writen by patrol (Aleksander Mistewicz),
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
#include "bot.h"

/* initialize GLOBAL variables */
int angle = 90, power = 50;
struct player *loc_player = NULL;
int16_t loc_player_id =0;

char default_portnum[] = "7979";
char *portnum = default_portnum;

int games_num = 1;
int difficulty = 2;
char *nick;
char *debug_filename;

void print_usage(char *argv[])
{
    printf(
"Usage: %s [OPTION]... SERVER_ADDR PLAYER_NAME\n\
OPTION may be:\n\
	-g, --games GAMES_NUM  	number of games bot will play,\n\
				default: 1\n\
	-d, --difficulty DIFF	difficulty of the bot, 0 will snipe you\n\
				every time, 10 will be easy; 2 is the default\n\
	-p, --port PORT		specify on what port client connects to a server\n\
	-h, -?, --help		show this message\n\
\n\
For complete documentation look for ./doc in project's files\n\
",
argv[0]);

    // exit required because we'll be called by getopt on wrong arguments
    exit(EXIT_FAILURE);
}

void parse_commandline(int argc, char *argv[])
{
    const char *opt_string = "g:d:p:h?";
    const struct option long_opts[] = {
        { "games",      required_argument, NULL, 'g' },
        { "difficulty", required_argument, NULL, 'd' },
        { "port",       required_argument, NULL, 'p' },
        { "help",       no_argument,       NULL, 'h' },
    };

    int opt = 0;
    do
    {
        opt = getopt_long(argc, argv, opt_string, long_opts, NULL);

        switch (opt)
        {
        case 'g':
            games_num = atoi(optarg);
            break;

        case 'd':
            difficulty = atoi(optarg);
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

    g_servername=argv[optind];
    optind++;

    nick = argv[optind];

    debug_filename = malloc(strlen(nick) +
                            strlen(".debug") + 1);
    strcpy(debug_filename, nick);
    strcat(debug_filename, ".debug");
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
        return EXIT_FAILURE; /* some errors occured */

    while (loc_player->state && games_num > 0) {
        if (loc_player->state == PS_JOINED) {
            if (Players.count > 1) send_ready();
            else wait_state();
        }
        if (loc_player->state == PS_READY ||
            loc_player->state == PS_WAITING ||
            loc_player->state == PS_DEAD)
           wait_state();
        if (loc_player->state == PS_ACTIVE)
            shoot();
        if (loc_player->state == PS_WINNER ||
            loc_player->state == PS_LOSER)
           loc_player->state=PS_NO_PLAYER;
            /* above states are invalid
             * because bot does not save the updates
             * so it processes them immediately */
    }

    /* free! */
    for (int i=0; i<Players.count; i++)
        clear_player(dyn_arr_get(&Players,i));
    dyn_arr_clear(&Players);
    free(debug_filename);
    free(map_data);

    /* Close connection */
    close(sock);
    return EXIT_SUCCESS;
}
