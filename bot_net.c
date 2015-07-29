#include "bot.h"

/* fetch map from server and generate it */
void fetch_map()
{
    send_int8(sock, C_GET_MAP);
    map_data = recv_map_info(sock);
    g_map = generate_map(map_data);
}

/* find the address of player with a given id */
int find_player(u_int16_t player_id)
{
    for (int i=0; i<Players.count; i++) {
        struct player *pl = dyn_arr_get(&Players, i);
        if (player_id == pl->id)
            return i;
    }
    return -1; /* returns -1 case not found */
}

void update_loc_player()
{
    loc_player = dyn_arr_get(&Players, find_player(loc_player_id));
}

void process_update(struct update *UpdateNet)
{
    switch (UpdateNet->type) {
        case U_MAP:
            debug_s(1, "Update", "map");
            g_map[UpdateNet->x]=UpdateNet->new_height;
            break;
        case U_ADD_PLAYER:
            debug_s(1, "AddPlayer", UpdateNet->player.nickname);
            if (loc_player_id == UpdateNet->player.id) {
                /* wow! we get ourself */
                debug_s(1, "AddPlayer", "recv loc_player");
                loc_player = dyn_arr_append(&Players, &UpdateNet->player);
            } else {
                /* yay! yet another player in game */
                int play_i = find_player(UpdateNet->player.id);
                if (play_i >= 0) {
                    /* woops! It seems we already have this player */
                    debug_s(1, "AddPlayer", "->UpdatePlayer");
                    clear_player(dyn_arr_get(&Players, play_i));
                    struct player *u_player = dyn_arr_get(&Players, play_i);
                    *u_player = UpdateNet->player;
                } else {
                    debug_s(1, "AddPlayer", "Append");
                    dyn_arr_append(&Players, &UpdateNet->player);
                    update_loc_player();
                }
            }
            break;
        case U_PLAYER:
            debug_s(1, "UpdatePlayer", UpdateNet->player.nickname);
            int play_u_i = find_player(UpdateNet->player.id);
            if (play_u_i >= 0) {
                /* free player-to-delete's nickname */
                struct player *u_player = dyn_arr_get(&Players, play_u_i);
                clear_player(u_player);
                *u_player = UpdateNet->player;
                switch (u_player->state) {
                    case PS_WINNER:
                    case PS_LOSER:
                        if (check_end_game_state()) {
                            debug_s(1, "EndGame", "Quit?");
                            games_num--;
                        }
                        break;
                    case PS_DEAD:
                        break;
                    case PS_READY:
                    case PS_JOINED:
                        break;
                }
            } else
                debug_s(5, "UpdatePlayer", "wrong id");
            break;
        case U_DEL_PLAYER:
            debug_s(1, "DeletePlayer", UpdateNet->player.nickname);
            int play_d_i = find_player(UpdateNet->player.id);
            if (play_d_i >= 0) {
                /* free player's nickname */
                struct player *d_player = dyn_arr_get(&Players, play_d_i);
                clear_player(d_player);
                dyn_arr_delete(&Players, d_player);
                update_loc_player();
            } else
                debug_s(5, "DeletePlayer", "wrong id");
            /* free player's nickname from update */
            clear_player(&UpdateNet->player);
            break;
        case U_SHOT:
            debug_d(1, "ShootingPlayerID", UpdateNet->player_id);
            s_update = *UpdateNet;
            break;
        case U_SHOT_IMPACT:
            debug_d(1, "ImpactTime*Precision", UpdateNet->impact_t);
            g_impact_t= (float) UpdateNet->impact_t / IMPACT_T_NET_PRECISION;
            debug_f(1, "impact_t (converted)", g_impact_t);
            break;
        case U_CONFIG:
            debug_s(1, "ValueName", UpdateNet->opt_name);
            debug_d(1, "Value", UpdateNet->opt_value);
            config_set(UpdateNet->opt_name, UpdateNet->opt_value);
            /* free name of the value */
            free(UpdateNet->opt_name);
            break;
        case U_ADD_ABILITY:
            debug_s(1, "Add new ability", UpdateNet->ability.name);
            dyn_arr_append(&Abilities, &UpdateNet->ability);
            break;
        default:
            debug_d(5, "UnknownGetChangesType", UpdateNet->type);
    }
}

/* fetch changes and process */
void fetch_changes()
{
    if (send_int8(sock, C_GET_CHANGES) == -1) {
        /* try to reconnect */
        if (!client_connect(g_servername))
            return ;
    }
    struct update *UpdateNet;
    while ((UpdateNet = recv_update(sock))->type) {
            /* process update and free memory */
            debug_s(1, "NetUpdates", "process");
            process_update(UpdateNet);
            free(UpdateNet);
    }
    free(UpdateNet);
}

/* join the game. fetch map and changes if successful */
int join_game(char *nickname)
{
    send_int8(sock, C_JOIN);
    send_string(sock, nickname);
    int8_t j_net;
    recv_int8(sock, &j_net);
    JoinReply jr = j_net;
    switch (jr) {
        case JR_OK:
            debug_s(1, "ClientName", nickname);
            recv_int16(sock, &loc_player_id);
            fetch_map(sock);
            fetch_changes(sock);
            break;
        /* all other values of jr are considered error
         * and quits the client */
        case JR_GAME_IN_PROGRESS:
            if (DEBUG <= 5) puts("Game in progress, cannot join!");
            return -1;
            break;
        case JR_NICKNAME_TAKEN:
            if (DEBUG <= 5) puts("Nickname's already taken!");
            return -1;
            break;
        default:
            debug_d(3, "JoinReplyNet", j_net);
            debug_d(3, "JoinReply", jr);
            return -1;
    }
    return 0;
}

void send_shoot()
{
    debug_d(1, "ShootAngle", angle);
    debug_d(1, "ShootPower", power);
    struct shot shot = { angle, power };
    send_int8(sock, C_SHOOT);
    send_shot(sock, &shot);
}

void send_ready()
{
    debug_s(1, "Send", "Ready");
    send_int8(sock, C_SET_ABILITY);
    send_int16(sock, loc_player->ability_id);
    send_int8(sock, C_READY);
    fetch_changes();
}
