#ifndef NOTIF_H
#define NOTIF_H

#include <stdint.h>
#include <sys/inotify.h>

void notif_watching_loop_start(int argc, char **argv);


#endif
