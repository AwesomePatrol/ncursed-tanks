#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORTNUM 7979 /* Port Number */
#define MAXRCVLEN 128 /* Maximal Length of Received Value */

#define MAX_PLAYERS 16

#define INITIAL_HP 1000

/*
 * command        args                reply
 *
 * JOIN           string nickname     JoinReply
 * GET_MAP        none                struct map_info
 */
typedef enum Command
{
    JOIN = 'J', GET_CHANGES = 'C', GET_MAP = 'M', SHOOT = 'F', ERROR = 'E'
} Command;

/* JR_GAME_IN_PROGRESS - not allowed to join because game already started */
typedef enum JoinReply
{
    JR_OK, JR_GAME_IN_PROGRESS, JR_FORBIDDEN
} JoinReply;

typedef enum PlayerState
{
    PS_NO_PLAYER = 0, PS_WAIT, PS_SHOOT, PS_DEAD
} PlayerState;

struct player
{
    char *nickname;
    PlayerState state;
    int16_t hitpoints;
};

struct player_net
{
    u_int16_t state;
    int16_t   hitpoints;
};

struct map_info
{
    u_int32_t seed;
    u_int16_t length;
    u_int16_t height;
};

/**** Game updates sent by server ****/

/* map_updates -- list of map_update */
/* map_update -- (index, new_height) */

/* add_player_update -- struct player */
/* player_update -- struct player */

typedef enum UpdateType
{
    U_MAP, U_ADD_PLAYER, U_PLAYER,
} UpdateType;

typedef int           map_updates_t; /* placeholder */
typedef struct player add_player_update_t;
typedef struct player player_update_t;

typedef union update_data_t
{
    map_updates_t map_updates_data;
    add_player_update_t add_player_update_data;
    player_update_t player_update_data;
} update_data_t;

struct update
{
    UpdateType type;
    update_data_t *data;
};


int sendall(int socket, void *data, int len);
int recvall(int socket, void *data, int len);

int send_string(int socket, char *str);
char *recv_string(int socket);


struct map_info map_info_to_net(struct map_info *i);
struct map_info map_info_from_net(struct map_info *i);

struct player_net player_to_net(struct player *p);
struct player     player_from_net(struct player_net *p);

#endif /* COMMON_H */
