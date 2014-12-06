#include "server_updates_queue.h"

struct updates_queue *new_uq()
{
    struct updates_queue *result = malloc(sizeof(*result));

    *result = (struct updates_queue) {0};

    return result;
}

void uq_append(struct updates_queue *q, struct update *upd)
{
    uq_elt_t *elt = malloc(sizeof(*elt));
    *elt = (uq_elt_t) { .value = copy_update(upd) };

    if (!q->first)
    {
        q->first = q->last = elt;
    }
    else
    {
        q->last->next = elt;
        q->last = elt;
    }
}

int uq_is_nonempty(struct updates_queue *q)
{
    return q->first != NULL;
}

void uq_clear(struct updates_queue *q)
{
    if (!q || !uq_is_nonempty(q))
        return;

    uq_elt_t *next;
    for (uq_elt_t *cur = q->first; cur != NULL; cur = next)
    {
        next = cur->next;
        clear_update(&cur->value);
        free(cur);
    }

    q->first = q->last = NULL;
}

void free_uq(struct updates_queue *q)
{
    uq_clear(q);
    free(q);
}


int send_uq(int socket, struct updates_queue *q)
{
    struct update end = (struct update) {
        .type = U_EMPTY
    };
    /* Send all queue elements */
    for (uq_elt_t *i = q->first; i != NULL; i = i->next)
        if (send_update(socket, &i->value) == -1)
            return -1;

    /* Send an empty update to indicate end of list transmission */
    if (send_update(socket, &end) == -1)
        return -1;

    return 0;
}
