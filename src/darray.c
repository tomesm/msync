#include <assert.h>
#include <stdlib.h>

#include "darray.h"
#include "debug.h"

DynamicArray *dynamicArrayCreate(size_t element_size, size_t initial_max)
{
    DynamicArray *array = malloc(sizeof(DynamicArray));
    check_mem(array);
    array->max = initial_max;
    check(array->max > 0, "You must set an initial max > 0");

    array->contents = calloc(initial_max, sizeof(void *));
    check_mem(array->contents);

    array->end = 0;
    array->element_size = element_size;
    array->expand_rate = DEFAULT_EXPAND_RATE;

    return array;

error:
    if (array) free(array);

    return NULL;
}

void dynamicArrayClear(DynamicArray *array)
{
    int i = 0;
    if (array->element_size > 0) {
        for (i = 0; i < array->max; i++) {
            if (array->contents != NULL) free(array->contents[i]);
        }
    }
}

static inline int dynamicArrayResize(DynamicArray *array, size_t new_size)
{
    array->max = new_size;
    check(array->max > 0, "The newsize must be > 0");

    void *contents = realloc(array->contents, array->max * sizeof(void *));
    check_mem(contents);

    array->contents = contents;

    return 0;
error:
    return -1;
}

int dynamicArrayExpand(DynamicArray *array)
{
    size_t old_max = array->max;
    check(dynamicArrayResize(array, array->max + array->expand_rate) == 0,
          "Failed to expand array to new size: %d",
          array->max + (int)array->expand_rate);

    memset(array->contents + old_max, 0, array->expand_rate + 1);

    return 0;
error:
    return -1;
}

int dynamicArrayContract(DynamicArray *array)
{
    int new_size = array->end < (int)array->expand_rate
                       ? (int)array->expand_rate
                       : array->end;

    return dynamicArrayResize(array, new_size + 1);
}

void dynamicArrayDestroy(DynamicArray *array)
{
    if (array) {
        if (array->contents) free(array->contents);
        free(array);
    }
}

void dynamicArrayClearDestroy(DynamicArray *array)
{
    dynamicArrayClear(array);
    dynamicArrayDestroy(array);
}

int dynamicArrayPush(DynamicArray *array, void *element)
{
    array->contents[array->end] = element;
    array->end++;

    if (DYNAMIC_ARRAY_END(array) >= DYNAMIC_ARRAY_MAX(array)) {
        return dynamicArrayExpand(array);
    } else {
        return 0;
    }
}

void *dynamicArrayPop(DynamicArray *array)
{
    check(array->end - 1 >= 0, "Attempt to pop from empty array.");

    void *element = dynamicArrayRemove(array, array->end - 1);
    array->end--;

    if (DYNAMIC_ARRAY_END(array) > (int)array->expand_rate
        && DYNAMIC_ARRAY_END(array) % array->expand_rate) {
        dynamicArrayContract(array);
    }
    return element;
error:
    return NULL;
}
