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
    debug_s(3, "listen", "Server started listening");

    /* listen until terminated */
    while (TRUE)
    {
        struct thread_data *thr_data = malloc(sizeof(*thr_data));
        *thr_data = (struct thread_data) {
            .socket = accept(server_socket,
                             (struct sockaddr *)&client_sa,
                             &socksize),
        };

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
        debug_c( 0, "received command", command);
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
    case C_SHOOT:
        process_shoot_command(data, socket);
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

    if (game_started)
    {
        debug_s( 3, "join: game already in progress, player tried to join",
                 nickname);
        send_int8(socket, JR_GAME_IN_PROGRESS);
        goto fail;
    }
    /* Check if the nickname is already used */
    /* Lock kept for so long so that
     * no different client with the same nickname can join */
    lock_clients_array();                                        /* {{{ */
    if (find_client_by_nickname(nickname))
    {
        unlock_clients_array();                                  /* }}} 1 */
        debug_s( 3, "join: nickname already taken", nickname);
        send_int8(socket, JR_NICKNAME_TAKEN);
        goto fail;
    }

    debug_s( 3, "join: new player", nickname);
    struct client *cl = new_client(nickname);
    data->client = cl;

    send_int8(socket, JR_OK);
    send_int16(socket, cl->id);

    /* Notify all other clients of the new player
     * and then add the new client to the array */
    /* In that way only clients that have already joined
     * are going to receive the notification */
    all_add_update(new_player_update(U_ADD_PLAYER, cl->player));

    add_client(cl);

    unlock_clients_array();                                      /* }}} 2 */

    /* Add all existing players to updates queue */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *other_cl = p_dyn_arr_get(&clients, i);
        add_update(cl, new_player_update(U_ADD_PLAYER, other_cl->player));
    }

    /* Add current config to updates queue */
    for (int i = 0; i < config_count; i++)
        add_update(cl, new_config_update(&config[i]));

    return;
fail:
    free(nickname);
}

void process_ready_command(struct thread_data *data)
{
    struct client *cl = data->client;

    lock_clients_array();                                        /* {{{ */
    /* Notify all players that the player has become ready */
    player_change_state(cl->player, PS_READY);

    /* Check if all the players are ready */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        if (cl->player->state != PS_READY)
            goto end;
    }
    debug_s( 3, "starting game", "All players ready");
    start_game();

end:
    unlock_clients_array();                                      /* }}} */
}

void process_shoot_command(struct thread_data *data, int socket)
{
    struct shot *shot = recv_shot(socket);

    /* TODO What if no client? */
    struct client *cl = data->client;

    debug_d( 3, "shot: client #", cl->id);
    debug_d( 0, "shot: angle", shot->angle);
    debug_d( 0, "shot: power", shot->power);

    lock_clients_array();                                        /* {{{ */
    all_add_update(new_shot_update(shot, cl->id));
    unlock_clients_array();                                      /* }}} */

    struct map_position impact_pos = get_impact_pos(cl->player, shot);
    debug_d(0, "shot: impact x", impact_pos.x);
    debug_d(0, "shot: impact y", impact_pos.y);

    shot_update_map(impact_pos);

    shot_do_damage(impact_pos);

    next_turn();

    free(shot);
}

void process_get_changes_command(struct thread_data *data, int socket)
{
    struct client *cl = data->client;

    //debug_d( 3, "sending changes to client #", cl->id);

    lock_updates(cl);                                            /* {{{ */
    /* Send updates queue */
    send_uq(socket, cl->updates);

    uq_clear(cl->updates);
    unlock_updates(cl);                                          /* }}} */
}

void process_get_map_command(struct thread_data *data, int socket)
{
    debug_s( 3, "send map", "Received C_GET_MAP. Sending map...");
    /* TODO check if sent */
    send_map_info(socket, &map_info);
}

/* Checks if the current thread has a client and removes it */
void delete_cur_client(void)
{
    struct thread_data *data = pthread_getspecific(thread_data);

    /* No client, nothing to do */
    if (!data->client)
        return;

    lock_clients_array();                                        /* {{{ */

    struct client **client_loc = find_client_loc(data->client->id);
    if (client_loc)
    {
        struct client *cl = *client_loc;
        /* Notify clients of the player being deleted */
        debug_s( 3, "removing player", cl->player->nickname);
        all_add_update(new_player_update(U_DEL_PLAYER, cl->player));
        free_client(cl);
        p_dyn_arr_delete(&clients, (void **)client_loc);
    }

    unlock_clients_array();                                      /* }}} */
}

/* Called by other functions, doesn't do locking */
void start_game(void)
{
    tanks_map = map_with_tanks();
    debug_s(0, "start game", "Copied map");

    for (int i = 1; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        player_change_state(cl->player, PS_WAITING);
    }

    /* Give turn to the first player */
    /* Assume the first player is the first client.
     * May become not true in the future?
     */
    struct client *cl = p_dyn_arr_get(&clients, 0);
    player_change_state(cl->player, PS_ACTIVE);

    game_started = TRUE;
}

/* Advances turn to the next player */
void next_turn(void)
{
    /* TODO Change / make the algorithm easier to read */
    /* Returns 1 if made active */
    int make_active_if_not(struct player *player)
    {
        if (player->state != PS_WAITING)
            return 0;
        player_change_state(player, PS_ACTIVE);
        return 1;
    }

    bool_t made_inactive = FALSE;

    lock_clients_array();                                        /* {{{ */

    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;

        if (!made_inactive)
        {
            if (player->state == PS_ACTIVE)
            {
                player_change_state(player, PS_WAITING);
                made_inactive = TRUE;
            }
        }
        else
        {
            if (make_active_if_not(player)) goto end;
        }
    }
    /* Player made inactive but the next player still not made active */
    /* Continue to look for the next inactive client from the beginning */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);

        if (make_active_if_not(cl->player)) goto end;
    }

 end:
    unlock_clients_array();                                      /* }}} */
}

void shot_update_map(struct map_position impact_pos)
{
    lock_clients_array();                                        /* {{{ */

    change_map(impact_pos.x, impact_pos.y + 1);

    unlock_clients_array();                                      /* }}} */
}

void shot_do_damage(struct map_position impact_pos)
{
    lock_clients_array();                                        /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *cl = p_dyn_arr_get(&clients, i);
        struct player *player = cl->player;

        if (player->pos.x == impact_pos.x)
        {
            /* Found the tank which must receive damage */
            player_do_damage(player, config_get("dmg_cap"));
            break;
        }
    }
    unlock_clients_array();                                      /* }}} */
}
