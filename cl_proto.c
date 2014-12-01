#include "client.h"

/* fetch map from server and generate it */
void fetch_map(int sock)
{
    send_int8(sock, GET_MAP);
    map_data = recv_map_info(sock);
    g_map = generate_map(map_data);
}

void fetch_changes(int sock)
{
    send_int8(sock, GET_CHANGES);
    struct update UpdateNet;
/*    do {
        UpdateNet = *recv_update(sock);
        switch (UpdateNet->type) {
            case U_EMPTY:
                break;
            case U_MAP:
                g_map[UpdateNet->x] = UpdateNet->new_height;
                break;
            case U_ADD_PLAYER:
                break;
            case U_PLAYER:
                break;
            default:
                debug_d(3, "GetChangesType", UpdateNet.type);
        }

    } while (UpdateNet->type);*/
}

int join_game(int sock, char *nickname)
{
    send_int8(sock, JOIN);
    send_string(sock, nickname);
    u_int8_t j_net;
    recv_int8(sock, &j_net);
    JoinReply jr = j_net;
    switch (jr) {
        case JR_OK:
            fetch_map(sock);
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
