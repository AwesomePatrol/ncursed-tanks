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
int difficulty = 0;

void print_help()
{
    puts("Usage: bot [OPTION]... SERVER_IP PLAYER_NAME");
    puts("OPTION may be:");
    puts("-p, --port PORT\t specify on what port client connects a server");
    puts("-g, --games GAMES_NUM\t number of games bot will play,");
    puts("\t\t 1 (default)");
    puts("-d, --difficulty DIFF\t difficulty of the bot, 0 will snipe you");
    puts("\t\t every time, 10 will be easy");
    puts("--help\t show this message");
    puts("");
    puts("For complete documentation look for ./doc in project's files");
}

bool parse_cmd(int argc, char *argv[])
{
    if (argc == 2 && strcmp(argv[1],"--help")) {
        print_help();
        return true;
    }
    for (int i=1; i<(argc-2); i++) {
        if (strcmp(argv[i],"--help") == 0) {
            print_help();
            return 1;
        }
        if (strcmp(argv[i],"--port") == 0
                || strcmp(argv[i],"-p") == 0)
            portnum=argv[i+1];
        if (strcmp(argv[i],"--games") == 0
                || strcmp(argv[i],"-g") == 0)
            games_num=atoi(argv[i+1]);
        if (strcmp(argv[i],"--difficulty") == 0
                || strcmp(argv[i],"-d") == 0)
            difficulty=atoi(argv[i+1]);
    }
    return false;
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
    /* Too few arguments error */
    if ( argc <= 2 ) {
        if (DEBUG <= 5) puts("Too few arguments!");
        return EXIT_FAILURE;
    }

    /* Parse command line arguments */
    if (parse_cmd(argc, argv))
        return EXIT_SUCCESS;

    /* Open debug_file */
    char *debug_filename = malloc(strlen(argv[argc-1])+strlen(".debug")+1);
    strcpy(debug_filename, argv[argc-1]);
    strcat(debug_filename, ".debug");
    if (DEBUG <= 5)
        debug_open(debug_filename);
    
    /* Get connection to server */
    if (!client_connect(argv[argc-2]))
        return EXIT_FAILURE;

    g_servername=argv[argc-2];

    /* join_game */
    if (join_game(argv[argc-1]) < 0)
        return EXIT_FAILURE;/* some errors occured */

    while (loc_player->state && games_num > 0) {
        if (loc_player->state == PS_JOINED) send_ready();
        if (loc_player->state == PS_READY ||
            loc_player->state == PS_WAITING ||
            loc_player->state == PS_DEAD) wait_state();
        if (loc_player->state == PS_ACTIVE) shoot();
        if (loc_player->state == PS_WINNER ||
            loc_player->state == PS_LOSER) loc_player->state=PS_NO_PLAYER;
            /* above states are invalid
             * because bot does not save the updates
             * so it process them immediately */
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
