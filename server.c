#include "server.h"

int server_socket; /* socket used to listen for incoming connections */
/* socket info about the machine connecting to us */
struct sockaddr_in client_sa;
socklen_t socksize = sizeof(struct sockaddr_in);

struct map_info map_info = {.length = 80, .height = 24};
map_t map = NULL;

void init_signals(void);
void init_game(void);
void init_server(void);
void server_listen(void);

void *connection_thread(void *consock);

void process_command(int socket, Command cmd);

void exit_cleanup(void);
void terminate_handler(int signum);

int main(int argc, char *argv[])
{
    debug_open("server.debug");

    init_signals();

    init_game();

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
    if (sigaction(SIGINT, &sigaction_new, NULL) == -1)
        debug_s( 5, "sigaction",
"Warning! Couldn't assign handler to SIGINT. If the server is terminated, \
won't be able to clean up!");

    /* Handle SIGTERM with the same handler */
    if (sigaction(SIGTERM, &sigaction_new, NULL) == -1)
        debug_s( 5, "sigaction",
"Warning! Couldn't assign handler to SIGINT. If the server is terminated, \
won't be able to clean up!");
}

void init_game(void)
{
    unsigned int random_seed;

    /* Initialize random number renerator with current time */
    random_seed = time(NULL);
    debug_d( 0, "initial random seed", (signed long long)random_seed);
    srand(random_seed);

    map_info.seed = rand();
    debug_d( 0, "map seed", map_info.seed);
}

void init_server(void)
{
    struct sockaddr_in serv = {0}; /* socket info about our server */

    /* set the type of connection to TCP/IP */
    serv.sin_family = AF_INET;
    /* set our address to any interface */
    serv.sin_addr.s_addr = htonl(INADDR_ANY);
    /* set the server port number */ 
    serv.sin_port = htons(PORTNUM);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    /* bind serv information to server_socket */
    bind(server_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
}

void server_listen(void)
{
    /* start listening, allowing a queue of up to 16 pending connection */
    listen(server_socket, 16);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        conn_sockets[i] = accept(server_socket,
                                 (struct sockaddr *)&client_sa,
                                 &socksize);

        if (!conn_sockets[i])
        {
            debug_s( 5, "accept", "Couldn't accept connection!");
            continue;
        }

        debug_s( 3, "incoming connection", inet_ntoa(client_sa.sin_addr));

        pthread_create(&threads[i], NULL,
                       connection_thread, (void *)&conn_sockets[i]);
    }

    debug_s( 5, "listen",
"Maximum thread number exceeded! No new connections will be accepted");
}

void *connection_thread(void *consock)
{
    int consocket = *(int *)consock;

    char buffer[MAXRCVLEN];
    int len;

    /* receive command - 1 char */
    /* process commands until disconnect */
    while ((len = recvall(consocket, buffer, 1)) != 0)
    {
        debug_c( 3, "received command", buffer[0]);
        process_command(consocket, buffer[0]);
    }
    close(consocket);
}

void exit_cleanup(void)
{
    if (map) free(map);
    /* close connection sockets from every thread */
    for (int i = 0; i < NUM_THREADS; i++)
        close(conn_sockets[i]);
    close(server_socket);
}

void terminate_handler(int signum)
{
    debug_s( 3, "terminate", "Received SIGINT or SIGTERM, cleaning up...");

    exit(EXIT_SUCCESS);
}

void process_command(int socket, Command cmd)
{
    char reply[MAXRCVLEN + 1];

    switch (cmd)
    {
    case GET_MAP:
        debug_s( 0, "send map", "Received GET_MAP. Sending map...");
        /* TODO check sent length */
        struct map_info map_info_net = map_info_to_net(&map_info);
        sendall(socket, &map_info_net, sizeof(map_info_net));

        map = generate_map(map_info);

        break;
    default:
        debug_c( 5, "unrecognized command", cmd);
    }
}
