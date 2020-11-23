#ifndef notif_h
#define notif_h

#include <stdint.h>
#include <sys/inotify.h>

int add_watchdir(int *fd, const char *dirname, uint32_t events);
void signal_handler(int signum);
int read_events(int fd);
void start_watching_loop(int argc, char **argv);


#endif
