/* Simple minimalistic unit tests */

#undef NDEBUG
#ifndef _munit_h
#define _munit_h

#include <stdio.h>
#include <stdlib.h>
#include "../src/debug.h"

#define mu_suite_start() char *message = NULL

#define mu_assert(test, message) if (!(test)) { \
        log_err(message); return message; }

#define mu_run_test(test) debug("\n-----%s", " " #test); \
    message = test(); tests_run++; if (message) return message;

#define RUN_TESTS(name) int main(int argc, char *argv[]) { \
    debug("----- RUNNING: %s -- args: %d", argv[0], argc); \
    printf("-----\nRUNNING: %s\n", argv[0]); \
    char *result = name(); \
    if (result != 0) { \
        printf("FAILED: %s\n", result); \
    } else { \
        printf("ALL TESTS PASSED\n"); \
    }\
    printf("Tests run: %d\n", tests_run); \
    exit(result != 0); \
}

int tests_run;

#endif

