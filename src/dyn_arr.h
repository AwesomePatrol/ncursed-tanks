#ifndef DYN_ARR_H
#define DYN_ARR_H

#include <stdlib.h>
#include <string.h>

/* Dynamic array of elements of size el_size
 *
 * To define a dyn_arr of type:
 *   struct dyn_arr name = { sizeof(type) };
 */
struct dyn_arr
{
    size_t el_size;
    size_t count;
    /* use a flexible array member instead of pointer? */
    void *data;
};

/* Dynamic array of pointers
 *
 * To define one:
 *   struct p_dyn_arr name = {0};
 */
struct p_dyn_arr
{
    size_t count;
    void **data;
};

void *dyn_arr_get(const struct dyn_arr *arr, size_t i);
void *dyn_arr_append(struct dyn_arr *arr, void *el);
void dyn_arr_clear(struct dyn_arr *arr);
void dyn_arr_delete(struct dyn_arr *arr, void *el);

void **p_dyn_arr_get_location(const struct p_dyn_arr *arr, size_t i);
void *p_dyn_arr_get(const struct p_dyn_arr *arr, size_t i);
void *p_dyn_arr_append(struct p_dyn_arr *arr, void *el);
void p_dyn_arr_clear(struct p_dyn_arr *arr);
void p_dyn_arr_delete(struct p_dyn_arr *arr, void **location);

#endif /* DYN_ARR_H */
