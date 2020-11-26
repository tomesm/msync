#include "munit.h"
#include "../src/collection.h"

static DArray *array = NULL;
static int *val1 = NULL;
static int *val2 = NULL;

char *test_create()
{
    array = collection_darray_create(sizeof(int), 100);

    mu_assert(array != NULL, "Dynamic array create failed");
    mu_assert(array->contents != NULL, "contents are wrong in dynamic_array");
    mu_assert(array->end == 0, "end is not at the right spot");
    mu_assert(array->element_size == sizeof(int), "wrong element size");
    mu_assert(array->max == 100, "wrong max length of initial size");

    return NULL;
}

char *test_destroy()
{
    collection_darray_destroy(array);

    return NULL;
}

char *test_new()
{
    val1 = collection_darray_new(array);
    mu_assert(val1 != NULL, "failed to make new a element");

    val2 = collection_darray_new(array);
    mu_assert(val2 != NULL, "failed to make a new element");

    return NULL;
}

char *test_set()
{
    collection_darray_set(array, 0, val1);
    collection_darray_set(array, 1, val2);

    return NULL;
}

char *test_get()
{
    mu_assert(collection_darray_get(array, 0) == val1, "Wrong first value");
    mu_assert(collection_darray_get(array, 1) == val2, "Wrong second value");

    return NULL;
}

char *test_remove()
{
    int *val_check = collection_darray_remove(array, 0);
    mu_assert(val_check != NULL, "Should not get NULL.");
    mu_assert(*val_check == *val1, "Should get the first value.");
    mu_assert(collection_darray_get(array, 0) == NULL, "Should be gone.")
    DYNAMIC_ARRAY_FREE(val_check);

    val_check = collection_darray_remove(array, 1);
    mu_assert(val_check != NULL, "Should not get NULL.");
    mu_assert(*val_check == *val2, "Should get the second value.");
    mu_assert(collection_darray_get(array, 1) == NULL, "Should be gone.")
    DYNAMIC_ARRAY_FREE(val_check);

    return NULL;
}

char *test_expand_contract()
{
    int old_max = array->max;
    collection_darray_expand(array);
    mu_assert((unsigned int) array->max == old_max + array->expand_rate,
            "Wrong size after expand.");

    collection_darray_contract(array);
    mu_assert((unsigned int) array->max == array->expand_rate + 1,
            "Should stay at the expand rate at least.")

    collection_darray_contract(array);
    mu_assert((unsigned int) array->max == array->expand_rate + 1,
            "Should stay at the expand_rate at least");

    return NULL;
}

char *test_push_pop()
{
    int i = 0;
    for (i = 0; i < 1000; i++) {
        int *val = collection_darray_new(array);
        *val = i * 333;
        collection_darray_push(array, val);
    }
    mu_assert(array->max == 1201, "Wrong max size.");

    for (i = 999; i >= 0; i--) {
        int *val = collection_darray_pop(array);
        mu_assert(val != NULL, "Should not get a NULL");
        mu_assert(*val == i * 333, "Wrong value.");
        DYNAMIC_ARRAY_FREE(val);
    }
    return NULL;
}

char *all_tests()
{
    mu_suite_start();

    mu_run_test(test_create);
    mu_run_test(test_new);
    mu_run_test(test_set);
    mu_run_test(test_get);
    mu_run_test(test_remove);
    mu_run_test(test_expand_contract);
    mu_run_test(test_push_pop);
    mu_run_test(test_destroy);

    return NULL;
}

RUN_TESTS(all_tests);
