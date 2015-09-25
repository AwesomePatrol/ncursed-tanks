/*Ncursed-tanks server, writen by vozhyk (Vitaut Bajaryn), licensed under the MIT license. :
 * Copyright (c) 2015, Vitaut Bajaryn
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
#include "server.h"

void parse_commandline(int argc, char *argv[]);
void init_signals(void);

void exit_cleanup(void);
void terminate_handler(int signum);

int main(int argc, char *argv[])
{
    parse_commandline(argc, argv);

    debug_open("server.debug");

    read_config();
    read_abilities();

    init_signals();

    init_game();

    pthread_key_create(&thread_data, NULL);

    init_server();

    server_listen();

    pthread_exit(NULL);
}

/* Prints usage and exits */
void print_usage(char *argv[])
{
    printf(
"Usage: %s [OPTION]...\n\
OPTION may be:\n\
	-p, --port PORT		Listen on PORT\n\
	-h, -?, --help		Display this message\n\
",
argv[0]);

    // exit required because we'll be called by getopt on wrong arguments
    exit(EXIT_FAILURE);
}

void parse_commandline(int argc, char *argv[])
{
    const char *opt_string = "p:h?";
    const struct option long_opts[] = {
        { "port", required_argument, NULL, 'p' },
        { "help", no_argument,       NULL, 'h' },
    };

    server_port = 7979;

    int opt = 0;
    do
    {
        opt = getopt_long(argc, argv, opt_string, long_opts, NULL);

        switch (opt)
        {
        case 'p':
            server_port = atoi(optarg);
            break;

        case 'h':
        case '?':
            print_usage(argv);
            break;
        }
    } while (opt != -1);
}

void init_signals(void)
{
    struct sigaction sigaction_new = {{0}};

    /* Cleanup on normal exit */
    if (atexit(exit_cleanup) != 0)
        debug_s( 5, "atexit",
"Warning! Couldn't register exit handler. Won't be able to clean up on exit!");

    /* Handle SIGINT (Control-c) */
    sigaction_new.sa_handler = terminate_handler;
    sigemptyset(&sigaction_new.sa_mask);
    if (sigaction(SIGINT, &sigaction_new, NULL) == -1)
    {
        debug_s( 5, "sigaction",
"Warning! Couldn't assign handler to SIGINT. If the server is terminated, \
won't be able to clean up!");
        debug_errno("sigaction");
    }

    /* Handle SIGTERM with the same handler */
    if (sigaction(SIGTERM, &sigaction_new, NULL) == -1)
    {
        debug_s( 5, "sigaction",
"Warning! Couldn't assign handler to SIGINT. If the server is terminated, \
won't be able to clean up!");
        debug_errno("sigaction");
    }
}

void exit_cleanup(void)
{
    game_cleanup();

    pthread_mutex_destroy(&clients_array_mutex);
    /* TODO close sockets from threads */
    /* for every player */
    debug_d(0, "cleanup: number of clients", clients.count);
    /* Don't need to lock, because everything should be dead by now */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        debug_d(0, "cleanup: freeing client #", cl->id);
        free_client(cl);
    }
    p_dyn_arr_clear(&clients);

    /* free all abilities */
    for (int i = 0; i < abilities.count; i++)
    {
        struct ability *a = dyn_arr_get(&abilities, i);

        clear_ability(a);
    }
    dyn_arr_clear(&abilities);

    close(server_socket);
}

void terminate_handler(int signum)
{
    debug_s( 3, "terminate", "Received SIGINT or SIGTERM, cleaning up...");

    exit(EXIT_SUCCESS);
}
