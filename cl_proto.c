#include "client.h"

struct map_info net_to_map_info(struct map_info *i)
{
    return
        (struct map_info) {ntohl(i->seed), ntohs(i->length), ntohs(i->height)};
}

void fetch_map(int sock)
{
    sendall(sock, "M", 1);
    struct map_info map_info_net;
    recvall(sock, &map_info_net, sizeof(map_info_net));
    map_data = net_to_map_info(&map_info_net);
    g_map = generate_map(map_data);
}
