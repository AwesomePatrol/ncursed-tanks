#include "server.h"

void init_signals(void);

void exit_cleanup(void);
void terminate_handler(int signum);

int main(int argc, char *argv[])
{
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

void init_signals(void)
{
    struct sigaction sigaction_new = {0};

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

    close(server_socket);
}

void terminate_handler(int signum)
{
    debug_s( 3, "terminate", "Received SIGINT or SIGTERM, cleaning up...");

    exit(EXIT_SUCCESS);
}
