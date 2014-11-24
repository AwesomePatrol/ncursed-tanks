#include "server.h"

int server_socket; /* socket used to listen for incoming connections */
int consocket;
/* socket info about the machine connecting to us */
struct sockaddr_in client_sa;
socklen_t socksize = sizeof(struct sockaddr_in);

struct map_info map_info = {0, 80, 24};
map_t map = NULL;

void init_signals(void);
void init_game(void);
void init_server(void);
void server_listen(void);

void process_command(Command cmd);

void exit_cleanup(void);
void sigchld_handler(int signum);
void sigint_handler(int signum);

int main(int argc, char *argv[])
{
    debug_open("server.debug");

    init_signals();

    init_game();

    init_server();

    server_listen();

    return EXIT_SUCCESS;
}

void init_signals(void)
{
    struct sigaction sigaction_new = {0};

    /* Cleanup on normal exit */
    if (atexit(exit_cleanup) != 0)
        debug_s( 5, "atexit",
"Warning! Couldn't register exit handler. Won't be able to clean up on exit!");

    /* Handle SIGINT (Control-c) */
    sigaction_new.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sigaction_new, NULL) == -1)
        debug_s( 5, "sigaction",
"Warning! Couldn't assign handler to SIGINT. If the server is terminated, \
won't be able to clean up!");

    sigaction_new = (struct sigaction) {0};
    /* reap all dead processes */
    sigaction_new.sa_handler = sigchld_handler;
    sigemptyset(&sigaction_new.sa_mask);
    sigaction_new.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigaction_new, NULL) == -1)
        debug_s( 5, "sigaction",
                 "Warning! Couldn't assign handler to SIGCHLD!");
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
    char buffer[MAXRCVLEN + 1]; /* +1 so we can add null terminator */
    int len;

    /* start listening, allowing a queue of up to 16 pending connection */
    listen(server_socket, 16);

    while (1)
    {
        consocket = accept(server_socket,
                           (struct sockaddr *)&client_sa, &socksize);

        if (!consocket)
        {
            debug_s( 5, "accept", "Couldn't accept connection!");
            continue;
        }

        debug_s( 3, "incoming connection", inet_ntoa(client_sa.sin_addr));

        if (fork() == 0)
        {
            /* this is the child process */
            close(server_socket); /* child doesn't need the listener */
            /* receive command - 1 char */
            /* process commands until disconnect */
            while ((len = recvall(consocket, buffer, 1)) != 0)
            {
                debug_c( 3, "received command", buffer[0]);
                process_command(buffer[0]);
            }
            close(consocket);
            exit(0);
        }

        close(consocket); /* parent doesn't need this */
    }
}

void exit_cleanup(void)
{
    if (map) free(map);
    close(consocket);
    close(server_socket);
}

void sigint_handler(int signum)
{
    debug_s( 3, "sigint", "Received SIGINT, cleaning up...");

    exit(EXIT_SUCCESS);
}

void sigchld_handler(int signum)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

void process_command(Command cmd)
{
    char reply[MAXRCVLEN + 1];

    switch (cmd)
    {
    case GET_MAP:
        debug_s( 0, "send map", "Received GET_MAP. Sending map...");
        /* TODO check sent length */
        sendall(consocket, &map_info, sizeof(map_info));

        map = generate_map(map_info);

        break;
    default:
        debug_c( 5, "unrecognized command", cmd);
    }
}
