#ifndef SERVER_UPDATES_QUEUE_H
#define SERVER_UPDATES_QUEUE_H

#include "server.h"

struct updates_queue_elt
{
    struct update value;

    struct updates_queue_elt *next;
};

struct updates_queue
{
    struct updates_queue_elt *first;
    struct updates_queue_elt *last;
};

typedef struct updates_queue_elt uq_elt_t;

struct updates_queue *new_uq();
void uq_append(struct updates_queue *q, struct update what);
int uq_is_nonempty(struct updates_queue *q);
void uq_clear(struct updates_queue *q);

int send_uq(int socket, struct updates_queue *q);

#endif /* SERVER_UPDATES_QUEUE_H */
