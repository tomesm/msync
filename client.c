#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "src/notify.h"
#include "src/collections.h"

int main(int argc, char **argv)
{
    notify_watch(argc, argv);

    return 0;
}
