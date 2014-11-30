#include "client.h"

/* fetch map from server and generate it */
void fetch_map(int sock)
{
    send_int8(sock, GET_MAP);
    map_data = recv_map_info(sock);
    g_map = generate_map(map_data);
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
        case JR_FORBIDDEN:
            if (DEBUG <= 5) puts("Join forbidden!");
            return -1;
            break;
        default:
            debug_d(1, "JoinReplyNet", j_net);
            debug_d(1, "JoinReply", jr);
    }
    return 0;
}
