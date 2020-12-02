#include <assert.h>
#include "munit.h"
#include "../src/collections.h"
#include <stdio.h> 
#include <stdlib.h>

Hashmap *map = NULL;
static int traverse_called = 0;
// struct tagbstring test1 = bsStatic("test data 1");
int test1 = 1;


// struct tagbstring test2 = bsStatic("test data 2");
// struct tagbstring test3 = bsStatic("xest data 3");
int test2 = 2;
int test3 = 3;
char *expect1 = "THE VALUE 1";
char *expect2 = "THE VALUE 2";
char *expect3 = "THE VALUE 3";

static int traverse_good_cb(HashmapNode *node)
{
    debug("KEY: %d", *node->key);
    traverse_called++;
    return 0;
}

static int traverse_fail_cb(HashmapNode *node)
{
    debug("KEY: %d", *node->key);
    traverse_called++;

    if (traverse_called == 2) {
        return 1;
    } else {
        return 0;
    }
}

char *test_create()
{
    map = collections_hashmap_create(NULL, NULL);
    mu_assert(map != NULL, "Failed to create dict.");

    return NULL;
}

char *test_destroy()
{
    collections_hashmap_destroy(map);

    return NULL;
}

char *test_get_set()
{
    int rc = collections_hashmap_set(map, &test1, &expect1);
    mu_assert(rc == 0, "Failed to set &test1");
    char *result = (char *)collections_hashmap_get(map, &test1);
    mu_assert((strcmp(result, expect2)), "Wrong value for test1.");

    rc = collections_hashmap_set(map, &test2, &expect2);
    mu_assert(rc == 0, "Failed to set test2");
    result = (char *)collections_hashmap_get(map, &test2);
    mu_assert(strcmp(result, expect2), "Wrong value for test2.");

    rc = collections_hashmap_set(map, &test3, &expect3);
    mu_assert(rc == 0, "Failed to set test3");
    result = (char *)collections_hashmap_get(map, &test3);
    mu_assert(strcmp(result, expect3), "Wrong value for test3.");

    return NULL;
}

char *test_traverse()
{
    int rc = collections_hashmap_traverse(map, traverse_good_cb);
    mu_assert(rc == 0, "Failed to traverse.");
    mu_assert(traverse_called == 3, "Wrong count traverse.");

    traverse_called = 0;
    rc = collections_hashmap_traverse(map, traverse_fail_cb);
    mu_assert(rc == 1, "Failed to traverse.");
    mu_assert(traverse_called == 2, "Wrong count traverse for fail.");

    return NULL;
}

char *test_delete()
{
    char *deleted = (char *)collections_hashmap_delete(map, &test1);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(strcmp(deleted, expect1), "Should get test1");
    char *result = collections_hashmap_get(map, &test1);
    mu_assert(result == NULL, "Should delete.");

    deleted = (char *)collections_hashmap_delete(map, &test2);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(strcmp(deleted, expect2), "Should get test2");
    result = collections_hashmap_get(map, &test2);
    mu_assert(result == NULL, "Should delete.");

    deleted = (char *)collections_hashmap_delete(map, &test3);
    mu_assert(deleted != NULL, "Got NULL on delete.");
    mu_assert(strcmp(deleted, expect3), "Should get test3");
    result = collections_hashmap_get(map, &test3);
    mu_assert(result == NULL, "Should delete.");

    return NULL;
}

char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_get_set);
    mu_run_test(test_traverse);
    mu_run_test(test_delete);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);

