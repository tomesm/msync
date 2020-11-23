#ifndef DARRAY_H_
#define DARRAY_H_

#include <stdlib.h>
#include <assert.h>
#include "debug.h"

typedef struct dynamic_array {
    int end;
    int max;
    size_t element_size;
    size_t expand_rate;
    void **contents;
}DynamicArray;

DynamicArray *dynamicArrayCreate(size_t element_size, size_t initial_max);

void dynamicArrayDestroy(DynamicArray *array);

void dynamicArrayClear(DynamicArray *array);

int dynamicArrayExpand(DynamicArray *array);

int dynamicArrayContract(DynamicArray *array);

int dynamicArrayPush(DynamicArray *array, void *element);

void *dynamicArrayPop(DynamicArray *array);

void dynamicArrayClearDestroy(DynamicArray *array);

#define DYNAMIC_ARRAY_LAST(A) ((A)->contents[(A)->end - 1])
#define DYNAMIC_ARRAY_FIRST(A) ((A)->contents[0])
#define DYNAMIC_ARRAY_END(A) ((A)->end)
#define DYNAMIC_ARRAY_COUNT(A) DYNAMIC_ARRAY_END(A)
#define DYNAMIC_ARRAY_MAX(A) ((A)->max)
#define DYNAMIC_ARRAY_FREE(E) free((E))

#define DEFAULT_EXPAND_RATE 300

static inline void dynamicArraySet(DynamicArray *array, int index, void *element)
{
    check(index < array->max, "Dynamic array to set past max");
    if (index > array->end) array->end = index;
    array->contents[index] = element;

error:
    return;
}

static inline void *dynamicArrayGet(DynamicArray *array, int index)
{
    check(index < array->max, "Dynamic array to get past max");
    return array->contents[index];

error:
    return NULL;
}

static inline void *dynamicArrayRemove(DynamicArray *array, int index)
{
    void *element = array->contents[index];
    array->contents[index] = NULL;
    return element;
}

static inline void *dynamicArrayNew(DynamicArray *array)
{
    check(array->element_size > 0, "Can't use darray_new on 0 size darrays");
    return calloc(1, array->element_size);

error:
    return NULL;
}

#endif // DARRAY_H_
