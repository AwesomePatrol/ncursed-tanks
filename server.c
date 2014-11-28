#include "server.h"

struct thread_data
{
    pthread_t thread;
    int socket;

    struct player player;
    struct updates_queue updates;
};

/* is this initialization going to work? */
struct thread_data globals[MAX_THREADS] = {0};
pthread_key_t thread_i;
pthread_mutex_t players_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void *connection_thread(void *thr_i);

void process_command(int socket, Command cmd);
void all_uq_append(struct update p);

struct player new_player(char *nickname);
int new_player_x();

void exit_cleanup(void);
void terminate_handler(int signum);

int main(int argc, char *argv[])
{
    debug_open("server.debug");

    init_signals();

    init_game();

    pthread_key_create(&thread_i, NULL);

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

    /* long for passing to thread */
    for (long i = 0; i < MAX_THREADS; i++)
    {
        globals[i].socket = accept(server_socket,
                                   (struct sockaddr *)&client_sa,
                                   &socksize);

        if (!globals[i].socket)
        {
            debug_s( 5, "accept", "Couldn't accept connection!");
            continue;
        }

        debug_s( 3, "incoming connection", inet_ntoa(client_sa.sin_addr));

        /* TODO check for errors */
        pthread_create(&globals[i].thread, NULL,
                       connection_thread, (void *)i);
    }

    debug_s( 5, "listen",
"Maximum thread number exceeded! No new connections will be accepted");
}

void *connection_thread(void *thr_i_)
{
    pthread_setspecific(thread_i, thr_i_);
    long thr_i = (long)pthread_getspecific(thread_i);
    int consocket = globals[thr_i].socket;

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
    pthread_mutex_destroy(&players_mutex);
    /* for every thread / player */
    for (int i = 0; i < MAX_THREADS; i++)
    {
        close(globals[i].socket);
        if (uq_is_nonempty(&globals[i].updates))
            uq_clear(&globals[i].updates);
        if (globals[i].player.state)
            free(globals[i].player.nickname);
    }
    close(server_socket);
}

void terminate_handler(int signum)
{
    debug_s( 3, "terminate", "Received SIGINT or SIGTERM, cleaning up...");

    exit(EXIT_SUCCESS);
}

void process_command(int socket, Command cmd)
{
    long thr_i = (long)pthread_getspecific(thread_i);
    char reply[MAXRCVLEN];

    switch (cmd)
    {
    case JOIN:
        globals[thr_i].player = new_player(recv_string(socket));

        debug_s( 3, "new player", globals[thr_i].player.nickname);

        reply[0] = JR_OK;
        sendall(socket, &reply, 1);

        /* Notify all other clients of the new player */
        all_uq_append(
            (struct update) {
                .type = U_ADD_PLAYER,
                .data = (update_data_t *)&globals[thr_i].player
            });
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

struct player new_player(char *nickname)
{
    int player_x = new_player_x();

    return (struct player) {
        .nickname = nickname,
        .state = PS_WAIT,
        .hitpoints = INITIAL_HP,
        .pos_x = player_x,
        .pos_y = map[player_x] - 1,
    };
}

int new_player_x()
{
    return MAP_NOTANK_MARGIN
        + rand() % (map_info.length - 2 * MAP_NOTANK_MARGIN);
}

void all_uq_append(struct update p)
{
    pthread_mutex_lock(&players_mutex);
    
    for (int i = 0; i < MAX_THREADS; i++)
        if (globals[i].thread)
            uq_append(&globals[i].updates, p);

    pthread_mutex_unlock(&players_mutex);
}
