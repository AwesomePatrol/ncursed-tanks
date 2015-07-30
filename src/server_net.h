#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "server_lib_includes.h"
#include "server_data.h"
#include "server_game.h"

#define PORTNUM 7979

extern int server_socket; /* socket used to listen for incoming connections */

void init_server(void);
void server_listen(void);

void *connection_thread(void *thr_data);

void process_command(Command cmd);
void process_join_command(struct thread_data *data, int socket);
void process_set_ability_command(struct thread_data *data, int socket);
void process_ready_command(struct thread_data *data);
void process_get_impact_command(struct thread_data *data, int socket);
void process_shoot_command(struct thread_data *data, int socket);
void process_ability_command(struct thread_data *data, int socket);
void process_get_changes_command(struct thread_data *data, int socket);
void process_get_map_command(struct thread_data *data, int socket);

void disconnect_cur_client(void);
void delete_client(struct client *client);

#endif /* SERVER_NET_H */
