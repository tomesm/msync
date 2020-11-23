#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/inotify.h>
#include <signal.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include "darray.h"
#include "hashmap.h"
#include "debug.h"
#include "bstrlib.h"
#include "hashmap.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define DIR_DEEP_LEN 100


int keep_running;
int watched_items;

void signal_handler(int signum)
{
    keep_running = 0;
}

int watch_dir(int fd, const char *dirname, unsigned long mask)
{
    int wd;
    wd = inotify_add_watch(fd, dirname, mask);
    if (wd < 0) {
        printf("ERROR::Can't add watch for \"%s\" with event mask %1X", dirname,
               mask);
        fflush(stdout);
        perror(" ");
    } else {
        watched_items++;
        printf("Watching %s WD=%d\n", dirname, wd);
        printf("Watching = %d items\n", watched_items);
    }
    return wd;
}

int read_events(int fd)
{
    return 0;
}

void clean(Hashmap *map, int fd) {
    int i, j = 0;

    for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
        DynamicArray *bucket = dynamicArrayGet(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                HashmapNode *node = dynamicArrayGet(bucket, j);
                inotify_rm_watch(fd, (int)node->key);
                hashmapDelete(map, node->key);
            }
        }
    }
    close(fd);
}

char *concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

void handle_event(Hashmap *map, struct inotify_event *event, int fd)
{
error:
    exit(1);
}

void read_buffer(Hashmap *map, char *buffer, int len, int fd)
{
    // Read through events one by one and process it accordingly
}


void start_watching_loop(int argc, char **argv)
{
    int fd, wd, i, j, len;

    keep_running = 1;
    Hashmap *map = hashmapCreate(NULL, NULL);
    char buffer[EVENT_BUF_LEN];
    struct sigaction int_handler = {
        .sa_handler=signal_handler
    };
    sigaction(SIGINT,&int_handler, 0);

    //fcntl(fd, F_SETFL, O_NONBLOCK);

    printf("Inotify init\n");
    fd = inotify_init();
    if (fd < 0) perror("ERROR::inotify init");

    for (i = 1; (i < argc) && (wd >= 0); i++) {
        wd = watch_dir(fd, argv[i], IN_CREATE | IN_DELETE);
        int rc = hashmapSet(map, &wd, argv[i]);
        check(rc == 0, "Falied to set watch on: %s", argv[i]);
    }

    while (keep_running) {
        len = read(fd, buffer, EVENT_BUF_LEN);
        if (len < 0) {
            if (errno == EINTR) { // close running by pressing ctrl-c
                clean(map, fd);
                debug("SIGINT captured. Closing the watcher");
            } else {
                log_err("Error in read events.");
            }
        }
        int j = 0;
        while (j < len) {
            struct inotify_event *event = (struct inotify_event *)&buffer[j];

            if (event->len) {
                char *parentdir = hashmapGet(map, &event->wd);
                if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        printf("New directory %s created in: %s.\n",
                                event->name, parentdir);
                        // If directory created start watching it
                        char *new_dir_path = concat(3, parentdir, "/", event->name);
                        int wd = watch_dir(fd, new_dir_path, IN_CREATE | IN_DELETE);
                        int rc = hashmapSet(map, &wd, new_dir_path);
                        check(rc == 0, "Falied to set watch on: %s", new_dir_path);
                    } else {
                        printf("New file %s created in %s.\n", event->name,
                                parentdir);
                    }
                } else if (event->mask & IN_DELETE) {
                    if (event->mask & IN_ISDIR) {
                        printf("Directory %s deleted.\n", event->name);
                    } else {
                        printf("File %s deleted.\n", event->name);
                    }
                }

            }
            j += EVENT_SIZE + event->len;
        }
    }
    clean(map, fd);

error:
    clean(map, fd);
    exit(1);
}

