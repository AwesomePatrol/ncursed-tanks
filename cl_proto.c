#include "client.h"

/* fetch map from server and generate it */
void fetch_map(int sock)
{
    send_int8(sock, GET_MAP);
    map_data = recv_map_info(sock);
    g_map = generate_map(map_data);
}

/* find the address of player with a given id */
int find_player(u_int16_t player_id)
{
    for (int i=0; i<players_size; i++)
        if (player_id == players[i].id)
            return i;
    return 0; /* returns local player in case not found */
}

/* fetch changes and apply them */
void fetch_changes(int sock)
{
    send_int8(sock, GET_CHANGES);
    struct update *UpdateNet;
    while ((UpdateNet = recv_update(sock)) &&
            UpdateNet->type) {
        switch (UpdateNet->type) {
            case U_MAP:
                debug_s(1, "Update", "map");
                g_map[UpdateNet->x] = UpdateNet->new_height;
                break;
            case U_ADD_PLAYER:
                debug_s(1, "AddPlayer", UpdateNet->player.nickname);
                players[players_size] = UpdateNet->player;
                players_size++;
                break;
            case U_PLAYER:
                debug_s(1, "UpdatePlayer", UpdateNet->player.nickname);
                players[find_player(UpdateNet->player.id)] = UpdateNet->player;
                break;
            default:
                debug_d(3, "GetChangesType", UpdateNet->type);
        }
        free(UpdateNet);
    }
}

/* join the game and fetch map if successful */
int join_game(int sock, char *nickname)
{
    send_int8(sock, JOIN);
    send_string(sock, nickname);
    u_int8_t j_net;
    recv_int8(sock, &j_net);
    JoinReply jr = j_net;
    switch (jr) {
        case JR_OK:
            debug_s(1, "ClientName", nickname);
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
