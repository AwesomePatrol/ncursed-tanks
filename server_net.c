#include "server_net.h"

int server_socket; /* socket used to listen for incoming connections */
/* socket info about the machine connecting to us */
struct sockaddr_in client_sa;
socklen_t socksize = sizeof(struct sockaddr_in);

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

    /* receive command - 1 char */
    /* process commands until disconnect */
    while (recv_int8(socket, &command) != 0)
    {
        debug_c( 3, "received command", command);
        process_command(command);
    }
    close(socket);

    /* TODO print (stored) client IP */
    debug_s( 3, "client closed connection", "");

    /* provided that game not started */
    delete_cur_client();

    free(data);
}

void process_command(Command cmd)
{
    struct thread_data *data = pthread_getspecific(thread_data);
    int socket = data->socket;
    struct client *cl;

    switch (cmd)
    {
    case JOIN:
        process_join_command(data, socket);
        break;
    case GET_CHANGES:
        debug_s( 0, "send changes", "Sending changes to client...");

        pthread_mutex_lock(&clients_mutex);                          /* {{{ */
        cl = find_client(data->client_id);

        /* Send updates queue */
        send_uq(socket, cl->updates);

        uq_clear(cl->updates);
        pthread_mutex_unlock(&clients_mutex);                        /* }}} */

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

void process_join_command(struct thread_data *data, int socket)
{
    char *nickname = recv_string(socket);

    /* Check if the nickname already used */
    pthread_mutex_lock(&clients_mutex);                          /* {{{ */
    if (find_client_by_nickname(nickname))
    {
        pthread_mutex_unlock(&clients_mutex);                    /* }}} 1 */
        debug_s( 3, "nickname taken", nickname);
        send_int8(socket, JR_NICKNAME_TAKEN);
        return;
    }

    send_int8(socket, JR_OK);
    debug_s( 3, "new player", nickname);
    struct client *cl = new_client(nickname);
    data->client_id = cl->id;

    add_client(cl);

    /* Notify all other clients of the new player */
    all_uq_append(new_player_update(U_ADD_PLAYER, cl->player));

    /* Add all existing players to updates queue */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);
        one_uq_append(cl->updates,
                      new_player_update(U_ADD_PLAYER, cl->player));
    }

    pthread_mutex_unlock(&clients_mutex);                        /* }}} 2 */

    free(cl);
}

void delete_cur_client()
{
    struct thread_data *data = pthread_getspecific(thread_data);

    pthread_mutex_lock(&clients_mutex);                          /* {{{ */


    struct client *cl = find_client(data->client_id);
    if (cl)
    {
        /* Notify clients of the player being deleted */
        debug_s( 3, "removing player", cl->player->nickname);
        all_uq_append(new_player_update(U_DEL_PLAYER, cl->player));
        clear_client(cl);
        dyn_arr_delete(&clients, cl);
    }

    pthread_mutex_unlock(&clients_mutex);                        /* }}} */
}
