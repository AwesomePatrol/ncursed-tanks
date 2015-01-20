#ifndef COMMON_H
#define COMMON_H

/*
 * common.h:
 *
 * Datatypes common to client and server
 * (including functions for sending/receiving them over the network)
 *
 * Must be included before any other headers in a file
 */

/* needed for *addrinfo, sigaction, strdup
 * the value can be increased if required by another thing
 */
#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/*
 * command         args                reply
 *   \- requirements
 * C_JOIN          string nickname     JoinReply[, int16_t id]
 *   sends id only if JoinReply is JR_OK
 * C_SET_ABILITY   int16_t ability_id
 * C_READY
 * C_GET_MAP                           struct map_info
 * C_GET_CHANGES                       list(struct update)
 *   \- client joined
 * C_SHOOT         struct shot
 *   \- game started, state == PS_ACTIVE
 * C_USE_ABILITY   struct shot
 * 
 * list(X) means sending / receiving a series of X with an empty X in the end.
 */
typedef enum Command
{
    C_JOIN = 'J', C_READY = 'R',
    C_SET_ABILITY = 'a',
    C_GET_MAP = 'M', C_GET_CHANGES = 'C',
    C_SHOOT = 'F', C_USE_ABILITY = 'A',
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
 *
 * PS_WINNER    - winner of the game
 * PS_LOSER     - loser  of the game
 */
typedef enum PlayerState
{
    PS_NO_PLAYER = 0, PS_JOINED, PS_READY, PS_WAITING, PS_ACTIVE, PS_DEAD,
    PS_WINNER, PS_LOSER
} PlayerState;

typedef enum AbilityType
{
    A_NONE = 0, A_DOUBLE_SHOT, A_MOVE, A_SNIPE
} AbilityType;

struct map_position
{
    int16_t x;
    int16_t y;
};

struct f_pair
{
    float x;
    float y;
};

struct player
{
    PlayerState state;
    bool is_connected;
    /* A value of 0 is invalid */
    int16_t id;
    char *nickname;
    int16_t hitpoints;
    struct map_position pos;

    int16_t ability_id;
    int16_t ability_cooldown;
};

struct ability
{
    int16_t id;
    char *name;
    AbilityType type;
    int16_t cooldown;
    int8_t params_count;
    int32_t *params;
};

struct map_info
{
    u_int32_t seed;
    u_int16_t length;
    u_int16_t height;
};

struct shot
{
    int16_t angle; /* in degrees, can be 0..359 */
    int16_t power;
};

/**** Game updates sent by server ****/

/* Precision for sending/receiving impact time through the net.
 * Higher value -> higher precision. */
#define IMPACT_T_NET_PRECISION 100

/* U_EMPTY -- end of updates -- no pending updates left */
typedef enum UpdateType
{
    U_EMPTY = 0,
    U_CONFIG, U_ADD_ABILITY,
    U_SHOT, U_ABILITY_USE,
    U_SHOT_IMPACT, U_MAP,
    U_PLAYER, U_ADD_PLAYER, U_DEL_PLAYER,
} UpdateType;

struct update
{
    UpdateType type;
    union
    {
        /* for U_CONFIG */
        struct
        {
            char *opt_name;
            int32_t opt_value;
        };
        /* for U_ADD_ABILITY */
        struct ability ability;
        /* for U_{SHOT,ABILITY_USE} */
        struct
        {
            int16_t player_id;
            union
            {
                /* for U_SHOT */
                struct shot shot;
                /* for U_ABILITY_USE */
                int16_t ability_id;
            };
        };
        /* for U_SHOT_IMPACT */
        int16_t impact_t;
        /* for U_MAP */
        struct
        {
            int16_t x;
            int16_t new_height;
        };
        /* for U_*PLAYER */
        struct player player;
    };
};

void clear_player(struct player *p);
void clear_ability(struct ability *a);


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

int send_bool(int socket, bool b);
int recv_bool(int socket, bool *b);

int send_string(int socket, char *str);
char *recv_string(int socket);


int send_map_info(int socket, struct map_info *i);
struct map_info *recv_map_info(int socket);

int send_player(int socket, struct player *p);
struct player *recv_player(int socket);

int send_ability(int socket, struct ability *a);
struct ability *recv_ability(int socket);

int send_shot(int socket, struct shot *s);
struct shot *recv_shot(int socket);

int send_map_position(int socket, struct map_position *p);
struct map_position *recv_map_position(int socket);

int send_update(int socket, struct update *u);
struct update *recv_update(int socket);

#endif /* COMMON_H */
