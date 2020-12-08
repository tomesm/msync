#ifndef _NOTIFY_H_
#define _NOTIFY_H_

#include <stdint.h>
#include <sys/inotify.h>

void notify_watch(int dir_count, char **paths,int client_socket);


#endif
