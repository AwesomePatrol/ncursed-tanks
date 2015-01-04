#include "client.h"

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
    for (int i=0; i<players_size; i++)
        if (player_id == players[i].id)
            return i;
    return -1; /* returns -1 case not found */
}

/* fetch changes and apply them */
void fetch_changes()
{
    send_int8(sock, C_GET_CHANGES);
    struct update *UpdateNet;
    while (UpdateNet = recv_update(sock)) {
        if (UpdateNet->type) {
            switch (UpdateNet->type) {
                case U_MAP:
                    debug_s(1, "Update", "map");
                    struct map_position map_u = (struct map_position) {
                        UpdateNet->x, UpdateNet->new_height};
                    dyn_arr_append(&MapUpdates, &map_u);
                    break;
                case U_ADD_PLAYER:
                    debug_s(1, "AddPlayer", UpdateNet->player.nickname);
                    int play_i = find_player(UpdateNet->player.id);
                    if (play_i >= 0)
                            players[play_i] = UpdateNet->player;
                    else {
                        players[players_size] = UpdateNet->player;
                        players_size++;
                    }
                    ScreenUpdate add_player = SCR_LOBBY;
                    dyn_arr_append(&ScrUpdates, &add_player);
                    break;
                case U_PLAYER:
                    debug_s(1, "UpdatePlayer", UpdateNet->player.nickname);
                    int play_u_i = find_player(UpdateNet->player.id);
                    if (play_u_i >= 0) {
                        /* free player-to-delete's nickname */
                        clear_player(&players[play_u_i]);
                        players[play_u_i] = UpdateNet->player;
                        ScreenUpdate u_player;
                        switch (players[play_u_i].state)
                        {
                            case PS_DEAD:
                                u_player = SCR_TANKS;
                                dyn_arr_append(&ScrUpdates, &u_player);
                                break;
                            case PS_READY:
                                u_player = SCR_LOBBY;
                                dyn_arr_append(&ScrUpdates, &u_player);
                                break;
                        }
                    } else
                        debug_s(5, "UpdatePlayer", "wrong id");
                    break;
                case U_DEL_PLAYER:
                    debug_s(1, "DeletePlayer", UpdateNet->player.nickname);
                    int play_d_i = find_player(UpdateNet->player.id);
                    if (play_d_i > 0) {
                        players_size--;
                        /* free player's nickname */
                        clear_player(&players[play_d_i]);
                        players[play_d_i] = players[players_size];
                    }
                    else
                        debug_s(5, "DeletePlayer", "wrong id");
                    /* free player's nickname from update */
                    clear_player(&UpdateNet->player);
                    ScreenUpdate del_player = SCR_TANKS;
                    dyn_arr_append(&ScrUpdates, &del_player);
                    break;
                case U_SHOT:
                    debug_d(1, "ShootingPlayerID", UpdateNet->player_id);
                    s_update = *UpdateNet;
                    /* add SCR_SHOOT to screen update queue so that
                     * the client won't hang at this stage */
                    ScreenUpdate u_shot = SCR_SHOOT;
                    dyn_arr_append(&ScrUpdates, &u_shot);
                    break;
                case U_CONFIG:
                    debug_s(1, "ValueName", UpdateNet->opt_name);
                    debug_d(1, "Value", UpdateNet->opt_value);
                    config_set(UpdateNet->opt_name, UpdateNet->opt_value);
                    /* free name of the value */
                    free(UpdateNet->opt_name);
                    break;
                default:
                    debug_d(5, "UnknownGetChangesType", UpdateNet->type);
            }
            free(UpdateNet);
        }
        else {
            free(UpdateNet);
            break;
        }
    }
}

/* join the game. fetch map and changes if successful */
int join_game(char *nickname)
{
    send_int8(sock, C_JOIN);
    send_string(sock, nickname);
    u_int8_t j_net;
    recv_int8(sock, &j_net);
    JoinReply jr = j_net;
    switch (jr) {
        case JR_OK:
            debug_s(1, "ClientName", nickname);
            recv_int16(sock, &players[0].id);
            players_size++;/*hopefully will receive itself in next update */
            fetch_map(sock);
            fetch_changes(sock);
            break;
        case JR_GAME_IN_PROGRESS:
            if (DEBUG <= 5) puts("Game in progress, cannot join!");
            return -1;
            break;
        case JR_NICKNAME_TAKEN:
            if (DEBUG <= 5) puts("Nickname's already taken!");
            return -1;
            break;
        case JR_FORBIDDEN:
            if (DEBUG <= 5) puts("Join forbidden!");
            return -1;
            break;
        default:
            debug_d(3, "JoinReplyNet", j_net);
            debug_d(3, "JoinReply", jr);
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
