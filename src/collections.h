#ifndef _COLLECTIONS_H
#define _COLLECTIONS_H

#include <stdlib.h>
#include "stdint.h"
#include <assert.h>
#include "debug.h"
// #include "bstrlib.h"

// Dynamic Array
typedef struct DArray {
    int end;
    int max;
    size_t element_size;
    size_t expand_rate;
    void **contents;
}DArray;

DArray *collections_darray_create(size_t element_size, size_t initial_max);

void collections_darray_set(DArray *array, int index, void *element);

void *collections_darray_get(DArray *array, int index);

void *collections_darray_remove(DArray *array, int index);

void *collections_darray_new(DArray *array);

void collections_darray_destroy(DArray *array);

void collections_darray_clear(DArray *array);

int collections_darray_expand(DArray *array);

int collections_darray_contract(DArray *array);

int collections_darray_push(DArray *array, void *element);

void *collections_darray_pop(DArray *array);

void collections_darray_clear_destroy(DArray *array);

#define DYNAMIC_ARRAY_LAST(A) ((A)->contents[(A)->end - 1])
#define DYNAMIC_ARRAY_FIRST(A) ((A)->contents[0])
#define DYNAMIC_ARRAY_END(A) ((A)->end)
#define DYNAMIC_ARRAY_COUNT(A) DYNAMIC_ARRAY_END(A)
#define DYNAMIC_ARRAY_MAX(A) ((A)->max)
#define DYNAMIC_ARRAY_FREE(E) free((E))

#define DEFAULT_EXPAND_RATE 300

// Hash Map

#define DEFAULT_BUCKETS_NUM 100

typedef int (*HashMapCompare)(int *a, int *b);
typedef uint32_t (*HashMapHash)(void *key);

typedef struct HashMap {
    DArray *buckets; // Will hold array of dict nodes
    HashMapCompare compare;
    HashMapHash hash;
}HashMap;

typedef struct HashMapNode {
    int *key;
    void *value;
    uint32_t hash;
}HashMapNode;

typedef int (*HashMapTraverseCb)(HashMapNode *node);

HashMap *collections_hashmap_create(HashMapCompare compare, HashMapHash hash);

void collections_hashmap_destroy(HashMap *map);

int collections_hashmap_set(HashMap *map, int *key, void *value);

void *collections_hashmap_get(HashMap *map, int *key);

int collections_hashmap_traverse(HashMap *map, HashMapTraverseCb traverse_cb);

void *collections_hashmap_delete(HashMap *map, int *key);

#endif // DARRAY_H_
