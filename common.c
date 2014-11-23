#include "common.h"

/* Returns number of bytes actually sent, or -1 on failure */
int sendall(int socket, char *msg, int len)
{
    int sent = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while (sent < len)
    {
        n = send(socket, msg + sent, bytesleft, 0);
        if (n == -1)
            break;
        sent += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : sent;
}
