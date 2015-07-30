#include "dyn_arr.h"

/* da_* functions not exported - internal to this module */
void da_clear(void **data, size_t *count)
{
    *count = 0;
    free(*data);
    *data = NULL;
}

void dyn_arr_clear(struct dyn_arr *arr)
{
    da_clear(&arr->data, &arr->count);
}

/* Doesn't free all the pointers, just the array of them */
void p_dyn_arr_clear(struct p_dyn_arr *arr)
{
    da_clear((void **)&arr->data, &arr->count);
}

void *da_get(size_t i, void *data, size_t el_size)
{
    return data + i * el_size;
}

void *dyn_arr_get(struct dyn_arr *arr, size_t i)
{
    return da_get(i, arr->data, arr->el_size);
}

void **p_dyn_arr_get_location(struct p_dyn_arr *arr, size_t i)
{
    return da_get(i, arr->data, sizeof(*arr->data));
}

void *p_dyn_arr_get(struct p_dyn_arr *arr, size_t i)
{
    return *p_dyn_arr_get_location(arr, i);
}

void da_extend(void **data, size_t *count, size_t el_size)
{
    *data = realloc(*data, ++*count * el_size);
}

void *dyn_arr_append(struct dyn_arr *arr, void *el)
{
    size_t old_count = arr->count;
    size_t el_size = arr->el_size;

    da_extend(&arr->data, &arr->count, el_size);
    return memcpy(arr->data + (old_count * el_size),
                  el, el_size);
}

void *p_dyn_arr_append(struct p_dyn_arr *arr, void *el)
{
    size_t old_count = arr->count;

    da_extend((void **)&arr->data, &arr->count, sizeof(*arr->data));
    return arr->data[old_count] = el;
}

/* Delete element pointed to by el from *dyn_arr */
void da_delete(void *el, void **data, size_t *count, size_t el_size)
{
    size_t i = (el - *data) / el_size;

    /* shift remaining elements to the left */
    memmove(el, el + el_size, (--*count - i) * el_size);
    /* realloc data with new size */
    *data = realloc(*data, *count * el_size);
}

void dyn_arr_delete(struct dyn_arr *arr, void *el)
{
    da_delete(el, &arr->data, &arr->count, arr->el_size);
}

void p_dyn_arr_delete(struct p_dyn_arr *arr, void **location)
{
    da_delete(location, (void **)&arr->data, &arr->count, sizeof(*arr->data));
}
