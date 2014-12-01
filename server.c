#include "server.h"

struct thread_data
{
    pthread_t thread;
    int socket;

    int client_id;
};

struct client
{
    int id;
    struct player *player;
    struct updates_queue *updates;
};

pthread_key_t thread_data;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

struct dyn_arr clients = { sizeof(struct client) };
int player_id_counter = 0;

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

void *connection_thread(void *thr_data);

void process_command(Command cmd);
void all_uq_append(struct update p);
void add_client(struct client *cl);
struct client *find_client(int id);

struct client *new_client(char *nickname);
int new_client_id(void);
void free_client(struct client *cl);
struct player *new_player(char *nickname);
int new_player_x(void);

void exit_cleanup(void);
void terminate_handler(int signum);

int main(int argc, char *argv[])
{
    debug_open("server.debug");

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

void init_game(void)
{
    unsigned int random_seed;

    /* Initialize random number renerator with current time */
    random_seed = time(NULL);
    debug_d( 0, "initial random seed", (signed long long)random_seed);
    srand(random_seed);

    map_info.seed = rand();
    debug_d( 0, "map seed", map_info.seed);

    map = generate_map(&map_info);
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

    /* Make socket reuse address => get rid of "address already in use" */
    int yes = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))
        == -1)
    {
        debug_errno("setsockopt");
    }
            
    /* bind serv information to server_socket */
    int res = bind(server_socket,
                   (struct sockaddr *)&serv, sizeof(struct sockaddr));
    if (res == -1)
    {
        debug_errno("bind");
        exit(EXIT_FAILURE);
    }
}

void server_listen(void)
{
    /* start listening, allowing a queue of up to 16 pending connection */
    listen(server_socket, 16);

    /* listen until terminated */
    while (1)
    {
        struct thread_data *thr_data = malloc(sizeof(*thr_data));
        thr_data->socket = accept(server_socket,
                                  (struct sockaddr *)&client_sa,
                                  &socksize);

        if (!thr_data->socket)
        {
            debug_errno("accept");
            free(thr_data);
            continue;
        }

        debug_s( 3, "incoming connection", inet_ntoa(client_sa.sin_addr));

        /* TODO check for errors */
        pthread_create(&thr_data->thread, NULL,
                       connection_thread, thr_data);
    }
}

void *connection_thread(void *thr_data)
{
    pthread_setspecific(thread_data, thr_data);
    struct thread_data *data = pthread_getspecific(thread_data);
    int socket = data->socket;

    char command;
    int len;

    /* receive command - 1 char */
    /* process commands until disconnect */
    while ((len = recv_int8(socket, &command)) != 0)
    {
        debug_c( 3, "received command", command);
        process_command(command);
    }
    close(socket);

    /* TODO print (stored) client IP */
    debug_s( 3, "client closed connection", "");

    free(data);
}

void exit_cleanup(void)
{
    free(map);
    pthread_mutex_destroy(&clients_mutex);
    /* TODO close sockets from threads */
    /* for every player */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);

        free_client(cl);
    }
    dyn_arr_clear(&clients);

    close(server_socket);
}

void terminate_handler(int signum)
{
    debug_s( 3, "terminate", "Received SIGINT or SIGTERM, cleaning up...");

    exit(EXIT_SUCCESS);
}

void process_command(Command cmd)
{
    struct thread_data *data = pthread_getspecific(thread_data);
    int socket = data->socket;
    struct client *cl;

    switch (cmd)
    {
    case JOIN:
        ; /* empty statement so that variable can be defined after label */
        char *nickname = recv_string(socket);

        /* Check if the nickname already used */
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < clients.count; i++)
        {
            struct client *old_cl = dyn_arr_get(&clients, i);

            if (old_cl->player->state &&
                strcmp(old_cl->player->nickname, nickname) == 0)
            {
                debug_s( 3, "nickname taken", nickname);
                send_int8(socket, JR_NICKNAME_TAKEN);
                pthread_mutex_unlock(&clients_mutex);
                return;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        send_int8(socket, JR_OK);
        debug_s( 3, "new player", nickname);
        cl = new_client(nickname);
        data->client_id = cl->id;
        add_client(cl);

        /* Notify all other clients of the new player */
        all_uq_append(
            (struct update) {
                .type = U_ADD_PLAYER,
                .player = *cl->player,
            });

        free(cl);
        break;
    case GET_CHANGES:
        debug_s( 0, "send changes", "Sending changes to client...");

        cl = find_client(data->client_id);

        /* Send updates queue */
        send_uq(socket, cl->updates);

        uq_clear(cl->updates);

        break;
    case GET_MAP:
        debug_s( 0, "send map", "Received GET_MAP. Sending map...");
        /* TODO check if sent */
        send_map_info(socket, &map_info);

        break;
    default:
        debug_c( 5, "unrecognized command", cmd);
    }
}

struct client *new_client(char *nickname)
{
    struct client *result = malloc(sizeof(*result));

    *result = (struct client) {
        .id = new_client_id(),
        .player = new_player(nickname),
        .updates = new_uq(),
    };

    return result;
}

int new_client_id(void)
{
    int result = player_id_counter++;
    if (player_id_counter == 0)
        debug_s( 5, "player id",
"Player ID counter overflowed to 0! Hope there won't be collisions \
when the next ID is needed.");

    return result;
}

struct player *new_player(char *nickname)
{
    int player_x = new_player_x();
    struct player *result = malloc(sizeof(*result));

    *result = (struct player) {
        .nickname = nickname,
        .state = PS_WAIT,
        .hitpoints = INITIAL_HP,
        .pos_x = player_x,
        .pos_y = map[player_x] - 1,
    };

    return result;
}

int new_player_x(void)
{
    return MAP_NOTANK_MARGIN
        + rand() % (map_info.length - 2 * MAP_NOTANK_MARGIN);
}

void free_client(struct client *cl)
{
    if (uq_is_nonempty(cl->updates))
        uq_clear(cl->updates);
    if (cl->player)
    {
        free(cl->player->nickname);
        free(cl->player);
    }
}

void all_uq_append(struct update upd)
{
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);
        uq_append(cl->updates, upd);
    }

    pthread_mutex_unlock(&clients_mutex);
}

void add_client(struct client *cl)
{
    pthread_mutex_lock(&clients_mutex);

    dyn_arr_append(&clients, cl);

    pthread_mutex_unlock(&clients_mutex);
}

struct client *find_client(int id)
{
    struct client *result = NULL;

    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);
        if (cl->id == id)
        {
            result = cl;
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    return result;
}
