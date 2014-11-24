#include "client.h"

void fetch_map(int sock)
{
    sendall(sock, "M", 1);
    recvall(sock, &map_data, sizeof(map_data));
    g_map = generate_map(map_data);
}
