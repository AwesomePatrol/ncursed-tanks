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

    switch (cmd)
    {
    case C_JOIN:
        process_join_command(data, socket);
        break;
    case C_READY:
        process_ready_command(data);
        break;
    case C_GET_CHANGES:
        process_get_changes_command(data, socket);
        break;
    case C_GET_MAP:
        process_get_map_command(data, socket);
        break;
    default:
        debug_c( 5, "unrecognized command", cmd);
    }
}

void process_join_command(struct thread_data *data, int socket)
{
    char *nickname = recv_string(socket);

    /* Check if the nickname already used */
    lock_clients();                                              /* {{{ */
    if (find_client_by_nickname(nickname))
    {
        unlock_clients();                                        /* }}} 1 */
        debug_s( 3, "nickname already taken", nickname);
        send_int8(socket, JR_NICKNAME_TAKEN);
        return;
    }

    debug_s( 3, "new player", nickname);
    struct client *cl = new_client(nickname);
    data->client_id = cl->id;

    send_int8(socket, JR_OK);
    send_int16(socket, cl->id);

    /* Notify all other clients of the new player
     * and then add the new client to the array */
    /* In that way only clients that have already joined
     * are going to receive the notification */
    all_add_update(new_player_update(U_ADD_PLAYER, cl->player));

    add_client(cl);

    /* Add all existing players to updates queue */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *other_cl = dyn_arr_get(&clients, i);
        add_update(cl, new_player_update(U_ADD_PLAYER, other_cl->player));
    }

    unlock_clients();                                            /* }}} 2 */

    free(cl);
}

void process_ready_command(struct thread_data *data)
{
    lock_clients();                                              /* {{{ */
    struct client *cl = find_client(data->client_id);

    cl->player->state = PS_READY;

    /* Notify all players that the player has become ready */
    all_add_update(new_player_update(U_PLAYER, cl->player));

    /* Check if all the players are ready */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);

        if (cl->player->state != PS_READY)
            goto end;
    }
    debug_s( 3, "starting game", "All players ready");
    start_game();

end:
    unlock_clients();                                            /* }}} */
}

void process_get_changes_command(struct thread_data *data, int socket)
{
    lock_clients();                                              /* {{{ */
    struct client *cl = find_client(data->client_id);

    debug_d( 3, "sending changes to client #", cl->id);

    /* Send updates queue */
    send_uq(socket, cl->updates);

    uq_clear(cl->updates);
    unlock_clients();                                            /* }}} */
}

void process_get_map_command(struct thread_data *data, int socket)
{
    debug_s( 3, "send map", "Received C_GET_MAP. Sending map...");
    /* TODO check if sent */
    send_map_info(socket, &map_info);
}

void delete_cur_client(void)
{
    struct thread_data *data = pthread_getspecific(thread_data);

    lock_clients();                                              /* {{{ */


    struct client *cl = find_client(data->client_id);
    if (cl)
    {
        /* Notify clients of the player being deleted */
        debug_s( 3, "removing player", cl->player->nickname);
        all_add_update(new_player_update(U_DEL_PLAYER, cl->player));
        clear_client(cl);
        dyn_arr_delete(&clients, cl);
    }

    unlock_clients();                                            /* }}} */
}

void start_game(void)
{
    lock_clients();                                              /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = dyn_arr_get(&clients, i);

        cl->player->state = PS_WAITING;
        all_add_update(new_player_update(U_PLAYER, cl->player));
    }

    /* Give turn to the first player */
    /* Assume the first player is the first client.
     * May become not true in the future?
     */
    struct client *cl = dyn_arr_get(&clients, 0);
    cl->player->state = PS_ACTIVE;
    all_add_update(new_player_update(U_PLAYER, cl->player));

    unlock_clients();                                            /* }}} */
}
