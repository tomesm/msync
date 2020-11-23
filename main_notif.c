#include <stdio.h>
#include <stdlib.h>
#include "src/debug.h"
#include "src/notif.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        log_err("Must provide at least one directory name");
        exit(1);
    }

    start_watching_loop(argc, argv);

}
