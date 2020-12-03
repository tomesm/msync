#include <assert.h>
#include <stdlib.h>

#include "collections.h"
#include "debug.h"

void collections_darray_set(DArray *array, int index, void *element)
{
    check(index < array->max, "Dynamic array to set past max");
    if (index > array->end) array->end = index;
    array->contents[index] = element;

error:
    return;
}

void *collections_darray_get(DArray *array, int index)
{
    check(index < array->max, "Dynamic array to get past max");
    return array->contents[index];

error:
    return NULL;
}

void *collections_darray_remove(DArray *array, int index)
{
    void *element = array->contents[index];
    array->contents[index] = NULL;
    return element;
}

void *collections_darray_new(DArray *array)
{
    check(array->element_size > 0, "Can't use darray_new on 0 size darrays");
    return calloc(1, array->element_size);

error:
    return NULL;
}

DArray *collections_darray_create(size_t element_size, size_t initial_max)
{
    DArray *array = malloc(sizeof(DArray));
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

void collections_darray_clear(DArray *array)
{
    int i = 0;
    if (array->element_size > 0) {
        for (i = 0; i < array->max; i++) {
            if (array->contents != NULL) free(array->contents[i]);
        }
    }
}

static inline int collections_darray_resize(DArray *array, size_t new_size)
{
    array->max = new_size;
    check(array->max > 0, "The newsize must be > 0");

    void *contents = realloc(array->contents, array->max * sizeof(void *));
    check_mem(contents);

    array->contents = (void **) contents;

    return 0;
error:
    return -1;
}

int collections_darray_expand(DArray *array)
{
    size_t old_max = array->max;
    check(collections_darray_resize(array, array->max + array->expand_rate) == 0,
          "Failed to expand array to new size: %d",
          array->max + (int)array->expand_rate);

    memset(array->contents + old_max, 0, array->expand_rate + 1);

    return 0;
error:
    return -1;
}

int collections_darray_contract(DArray *array)
{
    int new_size = array->end < (int)array->expand_rate ? (int)array->expand_rate : array->end;

    return collections_darray_resize(array, new_size + 1);
}

void collections_darray_destroy(DArray *array)
{
    if (array) {
        if (array->contents) free(array->contents);
        free(array);
    }
}

void collections_darray_clear_destroy(DArray *array)
{
    collections_darray_clear(array);
    collections_darray_destroy(array);
}

int collections_darray_push(DArray *array, void *element)
{
    array->contents[array->end] = element;
    array->end++;

    if (DYNAMIC_ARRAY_END(array) >= DYNAMIC_ARRAY_MAX(array)) {
        return collections_darray_expand(array);
    } else {
        return 0;
    }
}

void *collections_darray_pop(DArray *array)
{
    check(array->end - 1 >= 0, "Attempt to pop from empty array.");

    void *element = collections_darray_remove(array, array->end - 1);
    array->end--;

    if (DYNAMIC_ARRAY_END(array) > (int)array->expand_rate
        && DYNAMIC_ARRAY_END(array) % array->expand_rate) {
        collections_darray_contract(array);
    }
    return element;
error:
    return NULL;
}

static int default_compare(int *a, int *b)
{
    // return strcmp((const char *)a, (const char *)b);
    // return bstrcmp((bstring) a, (bstring) b);

    if (a < b) {
        return -1;
    } else if (a == b) {
        return 0;
    } else {
        return 1;
    }

}

// Jenkins hash
static uint32_t default_hash(void *a)
{
    // size_t len = blength((bstring) a);
    // char *key = bdata((bstring) a);
    char *key;
    uint32_t hash = 0;
    uint32_t i = 0;

    key = (char *)a;
    size_t len = strlen(key);

    for (hash = i = 0; i < len; i++) {
        hash += key[i];

        hash += (hash << 10);
        hash = hash ^ (hash >> 6);
    }

    hash += (hash << 3);
    hash = hash ^ (hash >> 11);
    hash = hash + (hash << 15);

    return hash;
}

HashMap *collections_hashmap_create(HashMapCompare compare, HashMapHash hash)
{
    HashMap *map = calloc(1, sizeof(struct HashMap));
    check_mem(map);

    map->compare = compare == NULL ? default_compare : compare;
    map->hash = hash == NULL ? default_hash : hash;
    map->buckets = collections_darray_create(sizeof(DArray *), DEFAULT_BUCKETS_NUM);
    map->buckets->end = map->buckets->max;
    check_mem(map->buckets);

    return map;

error:
    if (map) collections_hashmap_destroy(map);
    return NULL;
}

void collections_hashmap_destroy(HashMap *map)
{
    int j = 0;
    int i = 0;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
                DArray *bucket = (DArray *) collections_darray_get(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                        free(collections_darray_get(bucket, j));
                    }
                    collections_darray_destroy(bucket);
                }
            }
            collections_darray_destroy(map->buckets);
        }
        free(map);
    }
}

static inline HashMapNode *create_node(int hash, int *key, void *value)
{
    HashMapNode *node = calloc(1, sizeof(HashMapNode));
    check_mem(node);

    node->key = key;
    node->value = value;
    node->hash = hash;

    return node;
error:
    return NULL;
}

static inline DArray *find_bucket(HashMap *map, int *key, int create, uint32_t *hash_out)
{
    uint32_t hash = map->hash(key);
    int bucket_n = hash % DEFAULT_BUCKETS_NUM;
    check(bucket_n >= 0, "Invalid bucket found: %d", bucket_n);

    *hash_out = hash;

    DArray *bucket = (DArray *) collections_darray_get(map->buckets, bucket_n);
    if (!bucket && create) {
        // new bucket, set it up
        bucket = collections_darray_create(sizeof(void *), DEFAULT_BUCKETS_NUM);
        check_mem(bucket);
        collections_darray_set(map->buckets, bucket_n, bucket);
    }
    return bucket;
error:
    return NULL;
}

int collections_hashmap_set(HashMap *map, int *key, void *value)
{
    uint32_t hash = 0;
    DArray *bucket = find_bucket(map, key, 1, &hash);
    check(bucket, "Error can't create bucket.");

    HashMapNode *node = create_node(hash, key, value);

    check_mem(node);

    collections_darray_push(bucket, node);
    return 0;
error:
    return -1;
}

static inline int get_node(HashMap *map, uint32_t hash, DArray *bucket, int *key)
{
    int i = 0;

    for (i = 0; i < DYNAMIC_ARRAY_END(bucket); i++) {
        debug("TRY: %d", i);
        HashMapNode *node = collections_darray_get(bucket, i);
        if (node->hash == hash && map->compare(node->key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void *collections_hashmap_get(HashMap *map, int *key)
{
    uint32_t hash = 0;
    int i = 0;
    DArray *bucket = find_bucket(map, key, 0, &hash);
    if (!bucket) return NULL;

    i = get_node(map, hash, bucket, key);
    if (i == -1) return NULL;

    HashMapNode *node = collections_darray_get(bucket, i);
    check(node != NULL, "Failed to get node from bucket when it should exist.");

    return node->value;
error:
    return NULL;
}

int collections_hashmap_traverse(HashMap *map, HashMapTraverseCb traverse_cb)
{
    int i, j, rc = 0;

    for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
        DArray *bucket = collections_darray_get(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                HashMapNode *node = collections_darray_get(bucket, j);
                rc = traverse_cb(node);
                if (rc != 0) return rc;
            }
        }
    }
    return 0;
}

void *collections_hashmap_delete(HashMap *map, int *key)
{
    uint32_t hash = 0;
    DArray *bucket = find_bucket(map, key, 0, &hash);
    if (!bucket) return NULL;

    int i = get_node(map, hash, bucket, key);
    if (i == -1) return NULL;

    HashMapNode *node = collections_darray_get(bucket, i);
    void *value = node->value;
    free(node);

    HashMapNode *ending = collections_darray_pop(bucket);

    if (ending != node) {
        // looks like it's not the last one, swap it
        collections_darray_set(bucket, i, ending);
    }
    return value;
}
