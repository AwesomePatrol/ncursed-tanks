#include "client.h"

/* fetch map from server and generate it */
void fetch_map(int sock)
{
    sendall(sock, "M", 1);
    struct map_info map_info_net;
    recvall(sock, &map_info_net, sizeof(map_info_net));
    map_data = map_info_from_net(&map_info_net);
    g_map = generate_map(map_data);
}

int join_game(int sock, char *nickname)
{
    sendall(sock, "J", 1);
    send_string(sock, nickname);
    u_int8_t j_net;
    recvall(sock, &j_net, 1);
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
