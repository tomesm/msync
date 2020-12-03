#include <stdint.h>
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
#include <inttypes.h>

#include "collections.h"
#include "debug.h"
#include "net.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define EVENT_MASK IN_MODIFY | IN_CREATE | IN_DELETE

int keep_running;
int watched_items;

void signal_handler(int signum)
{
    printf("SIG num: %d\n", signum);
    keep_running = 0;
}

int add_watch_directory(HashMap *map, int fd, char *dirname)
{
    int wd;
    wd = inotify_add_watch(fd, dirname, EVENT_MASK);
    if (wd < 0) {
        printf("ERROR::Can't add watch for \"%s\" with event mask %1X", dirname, EVENT_MASK);
        fflush(stdout);
        perror(" ");
    } else {
        watched_items++;
        printf("Watching %s WD=%d\n", dirname, wd);
        printf("Watching = %d items\n", watched_items);
    }
    int ret = collections_hashmap_set(map, &wd, dirname);

    return ret;
}

void notif_clean(HashMap *map, int fd) {
    int i, j = 0;

    for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
        DArray *bucket = (DArray *)collections_darray_get(map->buckets, i);
        if (bucket) {
            for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                HashMapNode *node = (HashMapNode *) collections_darray_get(bucket, j);
                inotify_rm_watch(fd, *node->key);
                collections_hashmap_delete(map, node->key);
            }
        }
    }
    close(fd);
}

char *concat_strings(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i = 0 ; i < count ; i++) {
        len += strlen(va_arg(ap, char*));
    }
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char), len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i = 0 ; i < count ; i++) {
        char *s = va_arg(ap, char*);
        strcpy(merged + null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

void notify_watch(int dir_count, char **paths)
{
    int fd, i, len;

    keep_running = 1;
    // Map keeps track of watch dirs as keys and its names as values
    HashMap *map = collections_hashmap_create(NULL, NULL);
    char buffer[EVENT_BUF_LEN];

    struct sigaction int_handler = {
        .sa_handler=signal_handler
    };
    sigaction(SIGINT,&int_handler, 0);

    fd = inotify_init();
    check(fd > 0, "ERROR::inotify init");

    for (i = 1; (i < dir_count); i++) {
        int rc = add_watch_directory(map, fd, paths[i]);
        check(rc == 0, "Falied to set watch on: %s", paths[i]);
    }

    int wd = 1;
    //char * pdir1 = collections_hashmap_get(map, &wd);
    //char * pdir2 = collections_hashmap_get(map, &wd);

    int client_fd = net_client_get_socket();

    while (keep_running) {
        len = read(fd, buffer, EVENT_BUF_LEN);
        if (len < 0) {
            if (errno == EINTR) { // close running by pressing ctrl-c
                notif_clean(map, fd);
                debug("SIGINT captured. Closing the watcher");
            } else {
                log_err("Error in read events.");
            }
        }
        int j = 0;
        while (j < len) {
            struct inotify_event *event = (struct inotify_event *)&buffer[j];
            if (event->len) {
                printf("Event wd: %d\n", event->wd);
                char *parentdir = (char *) collections_hashmap_get(map, &event->wd);
                printf("Parentdir: %s\n", parentdir);
                check_mem(parentdir != NULL);

                if (event->mask & IN_CREATE) {
                    if (event->mask & IN_ISDIR) {
                        printf("New directory %s created in: %s.\n", event->name, parentdir);

                        // If directory created than start watching it
                        char *new_dir_path = concat_strings(3, parentdir, "/", event->name);
                        int rc = add_watch_directory(map, fd, new_dir_path);
                        check(rc == 0, "Falied to set watch on: %s", new_dir_path);
                    } else {
                        printf("New file %s created in %s.\n", event->name, parentdir);

                        Message *message = malloc(sizeof(Message));
                        message->action = 1;
                        message->isdir = 0;
                        message->path = strdup(concat_strings(3, parentdir, "/", event->name));

                        net_send_msg(client_fd, message);

                    }
                } else if (event->mask & IN_DELETE) {
                    if (event->mask & IN_ISDIR) {
                        printf("Directory %s deleted.\n", event->name);
                    } else {
                        printf("File %s deleted.\n", event->name);
                    }
                } else if (event->mask & IN_MODIFY) {
                    if (event->mask & IN_ISDIR) {
                        printf("Directory %s was modified.\n", event->name);
                    } else {
                        printf("File %s was modified\n", event->name);
                    }
                }
            }
            j += EVENT_SIZE + event->len;
        }
    }
    notif_clean(map, fd);
    close(client_fd);

error:
    printf("Exiting!\n");
    notif_clean(map, fd);
    exit(1);
}

