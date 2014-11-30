#ifndef DYN_ARR_H
#define DYN_ARR_H

#include <stdlib.h>
#include <string.h>

struct dyn_arr
{
    size_t el_size;
    size_t count;
    /* use a flexible array member instead of pointer? */
    void *data;
};

size_t dyn_arr_clear(struct dyn_arr *arr);
void *dyn_arr_get(struct dyn_arr *arr, size_t i);
void *dyn_arr_append(struct dyn_arr *arr, void *el);

#endif /* DYN_ARR_H */
