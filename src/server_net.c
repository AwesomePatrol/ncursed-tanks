#include "server_net.h"
#include "abilities_config.h"

int server_port; // initialized from commandline args
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
    serv.sin_port = htons(server_port);

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
    debug_s( 3, "listen", "Server started listening");
    debug_d( 3, "listen port", server_port);

    /* listen until terminated */
    while (true)
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
    while (recv_int8(socket, (int8_t *)&command) != 0)
    {
        //debug_c( 0, "received command", command);
        process_command(command);
    }
    close(socket);

    /* TODO print (stored) client IP */
    debug_s( 3, "client closed connection", "");

    disconnect_cur_client();

    free(data);

    return NULL;
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
    case C_SET_ABILITY:
        process_set_ability_command(data, socket);
        break;
    case C_READY:
        process_ready_command(data);
        break;
    case C_SHOOT:
        process_shoot_command(data, socket);
        break;
    case C_GET_IMPACT:
        process_get_impact_command(data, socket);
        break;
    /* case C_USE_ABILITY: */
    /*     process_ability_command(data, socket); */
    /*     break; */
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
    struct client *cl;

    /* Check if the client with this nickname is already present */
    /* Lock kept for so long so that
     * no different client with the same nickname can join */
    lock_clients_array();                                        /* {{{ */
    cl = find_client_by_nickname(nickname);
    /* Whether there is already a client with this nickname */
    bool nickname_found = (bool) cl;

    if (game_started)
    {
        debug_s( 3, "join", "Game already in progress");

        if (nickname_found)
            if (!cl->player->is_connected)
            {
                debug_s(3, "join: player rejoins", nickname);
                player_set_connected(cl->player, true);

                free(nickname);
                /* cl already set to the found client */
                goto join_ok;
            }
            else
            {
                goto nickname_taken;
            }
        else
            goto game_in_progress;
    }
    else
    {
        if (nickname_found)
        {
            goto nickname_taken;
        }
        else
        {
            debug_s( 3, "join: new player", nickname);
            cl = new_client(nickname);

            /* Notify all other clients of the new player
             * and then add the new client to the array */
            /* In that way only clients that have already joined
             * are going to receive the notification */
            all_add_update(new_player_update(U_ADD_PLAYER, cl->player));

            add_client(cl);

            goto join_ok;
        }
    }

game_in_progress:
    unlock_clients_array();                                  /* }}} 1 */
    debug_s( 3, "join: new player not allowed to join during game",
             nickname);
    send_int8(socket, JR_GAME_IN_PROGRESS);

    free(nickname);
    return;

nickname_taken:
    unlock_clients_array();                                  /* }}} 2 */
    debug_s( 3, "join: nickname already taken", nickname);
    send_int8(socket, JR_NICKNAME_TAKEN);

    free(nickname);
    return;

join_ok:
    unlock_clients_array();                                  /* }}} 3 */
    data->client = cl;

    send_int8(socket, JR_OK);
    send_int16(socket, cl->id);

    /* Add all existing players to updates queue */
    lock_clients_array();                                    /* {{{ */
    for (int i = 0; i < clients.count; i++)
    {
        struct client *other_cl = p_dyn_arr_get(&clients, i);
        add_update(cl, new_player_update(U_ADD_PLAYER, other_cl->player));
    }
    unlock_clients_array();                                  /* }}} */

    /* Add current config to updates queue */
    for (int i = 0; i < config_count; i++)
        add_update(cl, new_config_update(&config[i]));
    /* Add abilities to updates */
    for (int i = 0; i < abilities.count; i++)
        add_update(cl, new_add_ability_update(
                           dyn_arr_get(&abilities, i)));
}

void process_set_ability_command(struct thread_data *data, int socket)
{
    struct client *cl = data->client;
    struct player *player = cl->player;

    recv_int16(socket, &player->ability_id);

    /* Notify all the clients of the change of ability */
    all_add_update(new_player_update(U_PLAYER, player));

    cl->ability = find_ability(player->ability_id);

    debug_s(3, "set ability: player", player->nickname);
    if (cl->ability)
        debug_s(3, "set ability: ability", cl->ability->name);
    else
        debug_s(3, "set ability: no ability", "");
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

void process_get_impact_command(struct thread_data *data, int socket)
{
    struct shot *shot = recv_shot(socket);
    
    double impact_t;
    struct map_position impact_pos = get_impact_pos(data->client->player,
        shot, &impact_t);

    send_int16(socket, impact_pos.x);

    free(shot);
}

void process_shoot_command(struct thread_data *data, int socket)
{
    struct shot *shot = recv_shot(socket);

    /* TODO What if no client? */
    struct client *cl = data->client;

    debug_d( 3, "shot: client #", cl->id);
    debug_d( 0, "shot: angle", shot->angle);
    debug_d( 0, "shot: power", shot->power);

    all_add_update(new_shot_update(shot, cl->id));
 
    process_impact(shot_without_dmg(cl->player, shot));

    free(shot);
}


void process_ability_command(struct thread_data *data, int socket)
{
    struct shot *shot = recv_shot(socket);
    
    struct client *cl = data->client;

    debug_d( 3, "ability: client #", cl->id);

    if (cl->player->ability_cooldown == 0) {
        switch(cl->ability->type) {
            case A_NONE:
                /* player has no ability in slot and still want to use one
                 * prefer to do nothing
                 */
                break;
            case A_DOUBLE_SHOT:
                ability_double_shot(cl, shot, socket);
                break;
            case A_MOVE:
                ability_move(cl, shot, socket);
                break;
            case A_SNIPE:
                break;
            default:
                debug_d( 5, "UnknownAbility", cl->ability->type);
        }
        /* TODO: set cooldown to some configured value */
    } else {
        debug_s( 3, "ability:", "still on cooldown");
    }
    
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

/* Marks the current player as disconnected
 * or removes it if the game hasn't started yet */
void disconnect_cur_client(void)
{
    struct thread_data *data = pthread_getspecific(thread_data);
    struct client *cl = data->client;

    /* Thread has no client struct, nothing to do */
    if (!cl)
        return;

    if (!game_started) // still in lobby, can delete players
    {
        delete_client(cl);
    }
    else
    {
        /* Mark current player as disconnected and add an update about it */
        struct player *player = cl->player;

        debug_s( 3, "player disconnected", player->nickname);

        player_set_connected(player, false);
    }
}

/* Removes the given client and notifies clients about this */
void delete_client(struct client *client)
{
    lock_clients_array();                                        /* {{{ */

    struct client **client_loc = find_client_loc(client->id);
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
