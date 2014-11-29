#include "server_updates_queue.h"

void uq_append(struct updates_queue *q, struct update what)
{
    uq_elt_t *elt = malloc(sizeof(elt));
    *elt = (uq_elt_t) { .cur = what };

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
        free(cur);
    }

    q->first = q->last = NULL;
}
