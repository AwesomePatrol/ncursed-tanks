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
