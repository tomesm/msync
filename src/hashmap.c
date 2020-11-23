#undef NDEBUG
#include "darray.h"
#include <stdlib.h>
#include <stdint.h>
#include "hashmap.h"
#include "debug.h"
#include "bstrlib.h"

static int defaultCompare(void *a, void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

// Jenkins hash
static uint32_t defaultHash(void *a)
{
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

Hashmap *hashmapCreate(HashmapCompare compare, HashmapHash hash)
{
    Hashmap *map = calloc(1, sizeof(struct Hashmap));
    check_mem(map);

    map->compare = compare == NULL ? defaultCompare : compare;
    map->hash = hash == NULL ? defaultHash : hash;
    map->buckets = dynamicArrayCreate(sizeof(DynamicArray *), DEFAULT_BUCKETS_NUM);
    map->buckets->end = map->buckets->max;
    check_mem(map->buckets);

    return map;

error:
    if (map) hashmapDestroy(map);
    return NULL;
}

void hashmapDestroy(Hashmap *map)
{
    int j = 0;
    int i = 0;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
                struct dynamic_array *bucket = dynamicArrayGet(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                        free(dynamicArrayGet(bucket, j));
                    }
                    dynamicArrayDestroy(bucket);
                }
            }
            dynamicArrayDestroy(map->buckets);
        }
        free(map);
    }
}

static inline HashmapNode *createNode(int hash, void *key, void *value)
{
    HashmapNode *node = calloc(1, sizeof(HashmapNode));
    check_mem(node);

    node->key = key;
    node->value = value;
    node->hash = hash;

    return node;
error:
    return NULL;
}

static inline DynamicArray *findBucket(Hashmap *map, void *key,
        int create,
        uint32_t *hash_out)
{
    uint32_t hash = map->hash(key);
    int bucket_n = hash % DEFAULT_BUCKETS_NUM;
    check(bucket_n >= 0, "Invalid bucket found: %d", bucket_n);

    *hash_out = hash;

    DynamicArray *bucket = dynamicArrayGet(map->buckets, bucket_n);
    if (!bucket && create) {
        // new bucket, set it up
        bucket = dynamicArrayCreate(sizeof(void *), DEFAULT_BUCKETS_NUM);
        check_mem(bucket);
        dynamicArraySet(map->buckets, bucket_n, bucket);
    }
    return bucket;
error:
    return NULL;
}

int hashmapSet(Hashmap *map, void *key, void *value)
{
    uint32_t hash = 0;
    struct dynamic_array *bucket = findBucket(map, key, 1, &hash);
    check(bucket, "Error can't create bucket.");

    HashmapNode *node = createNode(hash, key, value);

    check_mem(node);

    dynamicArrayPush(bucket, node);
    return 0;
error:
    return -1;
}

static inline int getNode(Hashmap *map, uint32_t hash, DynamicArray *bucket,
        void *key)
{
    int i = 0;

    for (i = 0; i < DYNAMIC_ARRAY_END(bucket); i++) {
        debug("TRY: %d", i);
        HashmapNode *node = dynamicArrayGet(bucket, i);
        if (node->hash == hash && map->compare(node->key, key) == 0) {
            return i;
        }
    }
    return -1;
}

void *hashmapGet(Hashmap *map, void *key)
{
    uint32_t hash = 0;
    DynamicArray *bucket = findBucket(map, key, 0, &hash);
    if (!bucket) return NULL;

    int i = getNode(map, hash, bucket, key);
    if (i == -1) return NULL;

    HashmapNode *node = dynamicArrayGet(bucket, i);
    check(node != NULL, "Failed to get node from bucket when it should exist.");

    return node->value;
error:
    return NULL;
}

int hashmapTraverse(Hashmap *map, HashmapTraverseCb traverse_cb)
{
    int i = 0;
    int j = 0;
    int rc = 0;

    for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
        DynamicArray *bucket = dynamicArrayGet(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                HashmapNode *node = dynamicArrayGet(bucket, j);
                rc = traverse_cb(node);
                if (rc != 0) return rc;
            }
        }
    }
    return 0;
}

void *hashmapDelete(Hashmap *map, void *key)
{
    uint32_t hash = 0;
    struct dynamic_array *bucket = findBucket(map, key, 0, &hash);
    if (!bucket) return NULL;

    int i = getNode(map, hash, bucket, key);
    if (i == -1) return NULL;

    HashmapNode *node = dynamicArrayGet(bucket, i);
    void *value = node->value;
    free(node);

    HashmapNode *ending = dynamicArrayPop(bucket);

    if (ending != node) {
        // looks like it's not the last one, swap it
        dynamicArraySet(bucket, i, ending);
    }
    return value;
}

