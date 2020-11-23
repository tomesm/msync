#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <bits/stdint-uintn.h>
#include <stdint.h>
#include "darray.h"

#define DEFAULT_BUCKETS_NUM 100

typedef int (*HashmapCompare)(void *a, void *b);
typedef uint32_t (*HashmapHash)(void *key);

typedef struct Hashmap {
    DynamicArray *buckets; // Will hold array of dict nodes
    HashmapCompare compare;
    HashmapHash hash;
}Hashmap;

typedef struct HashmapNode {
    void *key;
    void *value;
    uint32_t hash;
}HashmapNode;

typedef int (*HashmapTraverseCb)(HashmapNode *node);

Hashmap *hashmapCreate(HashmapCompare compare, HashmapHash hash);

void hashmapDestroy(Hashmap *map);

int hashmapSet(Hashmap *map, void *key, void *value);

void *hashmapGet(Hashmap *map, void *key);

int hashmapTraverse(Hashmap *map, HashmapTraverseCb traverse_cb);

void *hashmapDelete(Hashmap *map, void *key);

#endif // _HASHMAP_H_
