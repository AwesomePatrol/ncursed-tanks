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
#define MAP_NOTANK_MARGIN 5

/*
 * command         args                reply
 *   \- requirements
 * C_JOIN          string nickname     JoinReply[, int16_t id]
 *   sends id only if JoinReply is JR_OK
 * C_GET_MAP                           struct map_info
 * C_SHOOT         (direction, force)  target_point
 *   \- game started, state == PS_ACTIVE
 * C_GET_CHANGES                       list(struct update)
 *   \- client joined
 * 
 * list(X) means sending / receiving a series of X with an empty X in the end.
 */
typedef enum Command
{
    C_JOIN = 'J', C_READY = 'R',
    C_GET_CHANGES = 'C', C_GET_MAP = 'M',
    C_SHOOT = 'F',
} Command;

/* JR_GAME_IN_PROGRESS - not allowed to join because game already started */
typedef enum JoinReply
{
    JR_OK, JR_GAME_IN_PROGRESS, JR_NICKNAME_TAKEN, JR_FORBIDDEN
} JoinReply;

/*
 * PS_NO_PLAYER - empty struct
 * PS_JOINED    - joined the game, now in the lobby
 * PS_READY     - ready to start the game
 * PS_WAITING   - waiting for its turn
 * PS_ACTIVE    - player's turn
 */
typedef enum PlayerState
{
    PS_NO_PLAYER = 0, PS_JOINED, PS_READY, PS_WAITING, PS_ACTIVE, PS_DEAD
} PlayerState;

struct player
{
    PlayerState state;
    /* A value of 0 is invalid */
    int16_t id;
    char *nickname;
    int16_t hitpoints;
    int16_t pos_x;
    int16_t pos_y;
};

struct map_info
{
    u_int32_t seed;
    u_int16_t length;
    u_int16_t height;
};

/**** Game updates sent by server ****/

/* U_EMPTY -- end of updates -- no pending updates left */
typedef enum UpdateType
{
    U_EMPTY = 0, U_MAP, U_PLAYER, U_ADD_PLAYER, U_DEL_PLAYER,
} UpdateType;

struct update
{
    UpdateType type;
    union
    {
        /* for U_*PLAYER */
        struct player player;
        /* for U_MAP */
        struct
        {
            int16_t x;
            int16_t new_height;
        };
    };
};

void clear_player(struct player *p);


int sendall(int socket, void *data, int len);
int recvall(int socket, void *data, int len);

/* the following send_* functions with return type int
 * return -1 on failure */

int send_int8(int socket, int8_t i);
int send_int16(int socket, int16_t i);
int send_int32(int socket, int32_t i);

int recv_int8(int socket, int8_t *i);
int recv_int16(int socket, int16_t *i);
int recv_int32(int socket, int32_t *i);

int send_string(int socket, char *str);
char *recv_string(int socket);


int send_map_info(int socket, struct map_info *i);
struct map_info *recv_map_info(int socket);

int send_player(int socket, struct player *p);
struct player *recv_player(int socket);

int send_update(int socket, struct update *u);
struct update *recv_update(int socket);

#endif /* COMMON_H */
