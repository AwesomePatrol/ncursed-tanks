#include "server_updates_queue.h"

struct updates_queue *new_uq()
{
    struct updates_queue *result = malloc(sizeof(*result));

    *result = (struct updates_queue) {0};

    return result;
}

void uq_append(struct updates_queue *q, struct update what)
{
    uq_elt_t *elt = malloc(sizeof(elt));
    *elt = (uq_elt_t) { .value = what };

    debug_x( 0, "uq_append: elt", (long)elt);

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
        /* Temporary workaround in order not to crash */
        //debug_x( 0, "uq_clear: freeing cur", (long)cur);
        //free(cur);
    }

    q->first = q->last = NULL;
}
