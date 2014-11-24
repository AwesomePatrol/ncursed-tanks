#include "common.h"

/* function pointer to send or recv */
typedef ssize_t (*socket_io_fun)(int, void *, size_t, int);

/* do_all: sends/receives the whole buffer.
 *
 * action:           function pointer to either send or recv
 * socket, msg, len: standard arguments to send/recv
 * check_for_0:      recv() returns 0 when the connection is closed,
 *                   we want to check for this when using recv as action
 *
 * Returns the number of bytes actually processed, or -1 on failure
 */
int do_all(socket_io_fun action, int socket, char *msg, int len,
           int check_for_0)
{
    int processed = 0;
    int bytesleft = len; // how many we have left to send/receive
    int n;

    while (processed < len)
    {
        n = action(socket, msg + processed, bytesleft, 0);
        if (n == -1 || check_for_0 && n == 0)
            break;
        processed += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : processed;
}

/* sendall: sends the whole buffer. See do_all */
int sendall(int socket, char *msg, int len)
{
    /* typecasting because of slightly mismatching argument types */
    return do_all((socket_io_fun)send, socket, msg, len, 0);
}

/* sendall: receives the whole buffer. See do_all */
int recvall(int socket, char *msg, int len)
{
    return do_all(recv, socket, msg, len, 1);
}
