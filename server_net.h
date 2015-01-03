#ifndef SERVER_NET_H
#define SERVER_NET_H

#include "server_lib_includes.h"
#include "server_data.h"

extern int server_socket; /* socket used to listen for incoming connections */

void init_server(void);
void server_listen(void);

void *connection_thread(void *thr_data);

void process_command(Command cmd);
void process_join_command(struct thread_data *data, int socket);
void process_ready_command(struct thread_data *data);
void process_shoot_command(struct thread_data *data, int socket);
void process_get_changes_command(struct thread_data *data, int socket);
void process_get_map_command(struct thread_data *data, int socket);

void start_game(void);
void next_turn(void);
void shot_update_map(struct map_position impact_pos);
void shot_deal_damage(struct map_position impact_pos);
void delete_cur_client(void);

#endif /* SERVER_NET_H */
