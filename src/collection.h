#ifndef COLLECTION_H
#define COLLECTION_H

#include <stdlib.h>
#include "stdint.h"
#include <assert.h>
#include "debug.h"

// Dynamic Array
typedef struct DArray {
    int end;
    int max;
    size_t element_size;
    size_t expand_rate;
    void **contents;
}DArray;

DArray *collection_darray_create(size_t element_size, size_t initial_max);

void collection_darray_set(DArray *array, int index, void *element);

void *collection_darray_get(DArray *array, int index);

void *collection_darray_remove(DArray *array, int index);

void *collection_darray_new(DArray *array);

void collection_darray_destroy(DArray *array);

void collection_darray_clear(DArray *array);

int collection_darray_expand(DArray *array);

int collection_darray_contract(DArray *array);

int collection_darray_push(DArray *array, void *element);

void *collection_darray_pop(DArray *array);

void collection_darray_clear_destroy(DArray *array);

#define DYNAMIC_ARRAY_LAST(A) ((A)->contents[(A)->end - 1])
#define DYNAMIC_ARRAY_FIRST(A) ((A)->contents[0])
#define DYNAMIC_ARRAY_END(A) ((A)->end)
#define DYNAMIC_ARRAY_COUNT(A) DYNAMIC_ARRAY_END(A)
#define DYNAMIC_ARRAY_MAX(A) ((A)->max)
#define DYNAMIC_ARRAY_FREE(E) free((E))

#define DEFAULT_EXPAND_RATE 300

// Hash Map

#define DEFAULT_BUCKETS_NUM 100

typedef int (*HashmapCompare)(void *a, void *b);
typedef uint32_t (*HashmapHash)(void *key);

typedef struct Hashmap {
    DArray *buckets; // Will hold array of dict nodes
    HashmapCompare compare;
    HashmapHash hash;
}Hashmap;

typedef struct HashmapNode {
    void *key;
    void *value;
    uint32_t hash;
}HashmapNode;

typedef int (*HashmapTraverseCb)(HashmapNode *node);

Hashmap *collection_hashmap_create(HashmapCompare compare, HashmapHash hash);

void collection_hashmap_destroy(Hashmap *map);

int collection_hashmap_set(Hashmap *map, void *key, void *value);

void *collection_hashmap_get(Hashmap *map, void *key);

int collection_hashmap_traverse(Hashmap *map, HashmapTraverseCb traverse_cb);

void *collection_hashmap_delete(Hashmap *map, void *key);

#endif // DARRAY_H_
