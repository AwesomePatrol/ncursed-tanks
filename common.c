#include "common.h"
/* Assume debug_open() is already called */
#include "debug.h"

void clear_player(struct player *p)
{
    free(p->nickname);
}

void clear_ability(struct ability *a)
{
    free(a->name);
    free(a->params);
}

/* function pointer to send or recv */
typedef ssize_t (*socket_io_fun)(int, void *, size_t, int);

/* do_all: sends/receives the whole buffer.
 *         Do not use outside this file.
 *
 * action:            function pointer to either send or recv
 * socket, data, len: standard arguments to send/recv
 * check_for_0:       recv() returns 0 when the connection is closed,
 *                    we want to check for this when using recv as action
 *
 * Returns the number of bytes actually processed, or -1 on failure
 */
int do_all(socket_io_fun action, int socket, void *data, int len,
           int check_for_0)
{
    int processed = 0;
    int bytesleft = len; // how many we have left to send/receive
    int n = -1;

    while (processed < len)
    {
        n = action(socket, data + processed, bytesleft, 0);
        if (n == -1 || (check_for_0 && n == 0))
            break;
        processed += n;
        bytesleft -= n;
    }

    return n == -1 ? -1 : processed;
}

/* sendall: sends the whole buffer. See do_all */
int sendall(int socket, void *data, int len)
{
    /* typecasting because of slightly mismatching argument types */
    return do_all((socket_io_fun)send, socket, data, len, 0);
}

/* sendall: receives the whole buffer. See do_all */
int recvall(int socket, void *data, int len)
{
    return do_all(recv, socket, data, len, 1);
}

int send_int8(int socket, int8_t i)
{
    return sendall(socket, &i, sizeof(i));
}

int send_int16(int socket, int16_t i)
{
    i = htons(i);

    return sendall(socket, &i, sizeof(i));
}

int send_int32(int socket, int32_t i)
{
    i = htonl(i);

    return sendall(socket, &i, sizeof(i));
}

int recv_int8(int socket, int8_t *i)
{
    return recvall(socket, i, sizeof(*i));
}

int recv_int16(int socket, int16_t *i)
{
    int test = recvall(socket, i, sizeof(*i));

    if (test > 0)
        *i = ntohs(*i);
    return test;
}

int recv_int32(int socket, int32_t *i)
{
    int test = recvall(socket, i, sizeof(*i));

    if (test > 0)
        *i = ntohl(*i);
    return test;
}

int send_bool(int socket, bool b)
{
    return send_int8(socket, b);
}

int recv_bool(int socket, bool *b)
{
    u_int8_t b_net;
    int test = recv_int8(socket, (int8_t *)&b_net);

    if (test > 0)
        *b = b_net;
    return test;
}

/* returns 0 on success, -1 on failure */
int send_string(int socket, char *str)
{
    int16_t size = strlen(str) + 1;

    if (send_int16(socket, size) == -1   ||
        sendall(socket, str, size) == -1)
        return -1;
    return 0;
}

/* receive a string sent with send_string */
char *recv_string(int socket)
{
    int16_t size;
    char *str;

    if (recv_int16(socket, &size) <= 0)
        return NULL;
    str = malloc(size);
    if (recvall(socket, str, size) <= 0)
    {
        free(str);
        return NULL;
    }

    return str;
}


int send_map_info(int socket, struct map_info *i)
{
    if (send_int32(socket, i->seed) == -1   ||
        send_int16(socket, i->length) == -1 ||
        send_int16(socket, i->height) == -1)
        return -1;
    return 0;
}

struct map_info *recv_map_info(int socket)
{
    struct map_info *result = malloc(sizeof(*result));

    if (recv_int32(socket, (int32_t *)&result->seed) <= 0   ||
        recv_int16(socket, (int16_t *)&result->length) <= 0 ||
        recv_int16(socket, (int16_t *)&result->height) <= 0)
    {
        free(result);
        return NULL;
    }
    return result;
}

int send_player(int socket, struct player *p)
{
    if (send_int8(socket, p->state) == -1             ||
        send_bool(socket, p->is_connected) == -1      ||
        send_int16(socket, p->id) == -1               ||
        send_string(socket, p->nickname) == -1        ||
        send_int16(socket, p->hitpoints) == -1        ||
        send_map_position(socket, &p->pos) == -1      ||
        send_int16(socket, p->ability_id) == -1       ||
        send_int16(socket, p->ability_cooldown) == -1)
        return -1;
    return 0;
}

struct player *recv_player(int socket)
{
    struct player *result = malloc(sizeof(*result));
    u_int8_t state_net;
    struct map_position *pos;

    if (recv_int8(socket, (int8_t *)&state_net) <= 0       ||
        recv_bool(socket, &result->is_connected) <= 0      ||
        recv_int16(socket, &result->id) <= 0               ||
        (result->nickname = recv_string(socket)) == NULL   ||
        recv_int16(socket, &result->hitpoints) <= 0        ||
        (pos = recv_map_position(socket)) == NULL          ||
        recv_int16(socket, &result->ability_id) <= 0       ||
        recv_int16(socket, &result->ability_cooldown) <= 0)
    {
        free(result);
        return NULL;
    }
    else
    {
        result->state = state_net;
        result->pos = *pos;
        free(pos);
        return result;
    }
}

int send_ability(int socket, struct ability *a)
{
    if (send_int16(socket, a->id) == -1          ||
        send_string(socket, a->name) == -1       ||
        send_int8(socket, a->type) == -1         ||
        send_int16(socket, a->cooldown) == -1    ||
        send_int8(socket, a->params_count) == -1)
        return -1;
    for (int i = 0; i < a->params_count; i++)
        if (send_int32(socket, a->params[i]) == -1)
            return -1;
    return 0;
}

struct ability *recv_ability(int socket)
{
    struct ability *result = malloc(sizeof(*result));
    u_int8_t type_net;
    if (recv_int16(socket, &result->id) <= 0          ||
        (result->name = recv_string(socket)) == NULL  ||
        recv_int8(socket, (int8_t *)&type_net) <= 0   ||
        recv_int16(socket, &result->cooldown) <= 0    ||
        recv_int8(socket, &result->params_count) <= 0)
    {
        goto fail;
    }
    else
    {
        result->type = type_net;
        /* Receive all the params */
        result->params = malloc(result->params_count * sizeof(*result->params));
        for (int i = 0; i < result->params_count; i++)
            if (recv_int32(socket, &result->params[i]) <= 0)
                goto fail;
        return result;
    }
fail:
    free(result);
    return NULL;
}

int send_shot(int socket, struct shot *s)
{
    if (send_int16(socket, s->angle) == -1 ||
        send_int16(socket, s->power) == -1)
        return -1;
    return 0;
}

struct shot *recv_shot(int socket)
{
    struct shot *result = malloc(sizeof(*result));

    if (recv_int16(socket, &result->angle) <= 0 ||
        recv_int16(socket, &result->power) <= 0)
    {
        free(result);
        return NULL;
    }
    return result;
}

/* *_map_position almost the same as *_shot */
int send_map_position(int socket, struct map_position *p)
{
    if (send_int16(socket, p->x) == -1 ||
        send_int16(socket, p->y) == -1)
        return -1;
    return 0;
}

struct map_position *recv_map_position(int socket)
{
    struct map_position *result = malloc(sizeof(*result));

    if (recv_int16(socket, &result->x) <= 0 ||
        recv_int16(socket, &result->y) <= 0)
    {
        free(result);
        return NULL;
    }
    return result;
}

int send_update(int socket, struct update *u)
{
    if (send_int8(socket, u->type) == -1)
        return -1;
    switch(u->type)
    {
    case U_EMPTY:
        break;
    case U_CONFIG:
        if (send_string(socket, u->opt_name) == -1 ||
            send_int32(socket, u->opt_value) == -1)
            return -1;

        break;
    case U_ADD_ABILITY:
        if (send_ability(socket, &u->ability) == -1)
            return -1;

        break;
    /* ---  careful, code duplication */
    case U_SHOT:
        if (send_int16(socket, u->player_id) == -1 ||
            send_shot(socket, &u->shot) == -1)
            return -1;

        break;
    case U_ABILITY_USE:
        if (send_int16(socket, u->player_id) == -1  ||
            send_int16(socket, u->ability_id) == -1)
            return -1;

        break;
    /* ---  end. */
    case U_SHOT_IMPACT:
        if (send_int16(socket, u->impact_t) == -1)
            return -1;

        break;
    case U_MAP:
        if (send_int16(socket, u->x) == -1          ||
            send_int16(socket, u->new_height) == -1)
            return -1;

        break;
    case U_PLAYER: case U_ADD_PLAYER: case U_DEL_PLAYER:
        if (send_player(socket, &u->player) == -1)
            return -1;

        break;
    }
    return 0;
}

struct update *recv_update(int socket)
{
    struct update *result = malloc(sizeof(*result));
    int8_t type_net;

    struct ability *ability;
    struct shot *shot;
    struct player *player;

    if (recv_int8(socket, &type_net) <= 0)
        goto fail;
    result->type = type_net;

    switch(result->type)
    {
    case U_EMPTY:
        break;
    case U_CONFIG:
        if ((result->opt_name = recv_string(socket)) == NULL ||
            recv_int32(socket, &result->opt_value) <= 0)
            goto fail;

        break;
    case U_ADD_ABILITY:
        if ((ability = recv_ability(socket)) == NULL)
            goto fail;

        result->ability = *ability;
        free(ability);

        break;
    case U_SHOT:
        if (recv_int16(socket, &result->player_id) <= 0 ||
            (shot = recv_shot(socket)) == NULL)
            goto fail;

        result->shot = *shot;
        free(shot);

        break;
    case U_ABILITY_USE:
        if (recv_int16(socket, &result->player_id) <= 0  ||
            recv_int16(socket, &result->ability_id) <= 0)
            goto fail;

        break;
    case U_SHOT_IMPACT:
        if(recv_int16(socket, &result->impact_t) <= 0)
            goto fail;

        break;
    case U_MAP:
        if (recv_int16(socket, &result->x) <= 0          ||
            recv_int16(socket, &result->new_height) <= 0)
            goto fail;

        break;
    case U_PLAYER: case U_ADD_PLAYER: case U_DEL_PLAYER:
        if ((player = recv_player(socket)) == NULL)
            goto fail;

        result->player = *player;
        free(player);

        break;
    }

    return result;
fail:
    free(result);
    return NULL;
}
