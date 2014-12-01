#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "server.h"

extern int server_socket; /* socket used to listen for incoming connections */

void init_server(void);
void server_listen(void);

void *connection_thread(void *thr_data);

void process_command(Command cmd);
void delete_cur_client();

#endif /* SERVER_NET_H */
