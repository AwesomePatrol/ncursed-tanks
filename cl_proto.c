#include "client.h"

void fetch_map(int sock)
{
    debug_d( 3, "bytes sent",
            send( sock, "M", 1, 0));
    debug_d( 3, "bytes recived",
            recv( sock, &map_seed, sizeof(int), 0));
    debug_d( 3, "bytes recived",
            recv( sock, &map_length, sizeof(int), 0));
    debug_d( 3, "bytes recived",
            recv( sock, &map_height, sizeof(int), 0));
}
