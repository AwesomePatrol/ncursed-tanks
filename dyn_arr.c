#include "dyn_arr.h"

void *dyn_arr_append(struct dyn_arr *arr, void *el)
{
    size_t old_count = arr->count;
    size_t el_size = arr->el_size;

    arr->data = realloc(arr->data, ++arr->count * el_size);
    return memcpy(arr->data + (old_count * el_size),
                  el, el_size);
}
