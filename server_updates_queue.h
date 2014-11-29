#ifndef SERVER_UPDATES_QUEUE_H
#define SERVER_UPDATES_QUEUE_H

#include "server.h"

struct updates_queue_elt
{
    struct update cur;

    struct updates_queue_elt *next;
};

struct updates_queue
{
    struct updates_queue_elt *first;
    struct updates_queue_elt *last;
};

typedef struct updates_queue_elt uq_elt_t;


void uq_append(struct updates_queue *q, struct update what);
int uq_is_nonempty(struct updates_queue *q);
void uq_clear(struct updates_queue *q);

#endif /* SERVER_UPDATES_QUEUE_H */
