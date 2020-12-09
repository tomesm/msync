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
#include "bstrlib.h"
#include "fs.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define EVENT_MASK IN_MODIFY | IN_CREATE | IN_DELETE

#define MAXDATASIZE 255

int keep_running;
int watched_items;
int watch_lock;
bstring root_dir;

void signal_handler(int signum)
{
    printf("SIG num: %d\n", signum);
    keep_running = 0;
}

int add_watch_directory(HashMap *map, int fd, bstring dirname)
{
    int wd = 0;
    // const char str[dirname->slen];
    // bassigncstr(dirname, str);

    wd = inotify_add_watch(fd, (const char *) dirname->data, EVENT_MASK);
    if (wd < 0) {
        printf("ERROR::Can't add watch for \"%s\" with event mask %1X", dirname->data, EVENT_MASK);
        fflush(stdout);
        perror(" ");
    } else {
        watched_items++;
        printf("Watching %s WD=%d\n", dirname->data, wd);
        printf("Watching = %d items\n", watched_items);
    }
    return collections_hashmap_set(map, wd, dirname);
}

void notif_clean(HashMap *map, int fd) {
    int j = 0;
    int i = 0;

    if (map) {
        if (map->buckets) {
            for (i = 0; i < DYNAMIC_ARRAY_COUNT(map->buckets); i++) {
                DArray *bucket = collections_darray_get(map->buckets, i);
                if (bucket) {
                    for (j = 0; j < DYNAMIC_ARRAY_COUNT(bucket); j++) {
                        HashMapNode * node = collections_darray_get(bucket, j);
                        inotify_rm_watch(fd, node->key);
                        free(node);
                    }
                    collections_darray_destroy(bucket);
                }
            }
            collections_darray_destroy(map->buckets);
        }
        free(map);
    }
    close(fd);
}

void send_message(int sock, uint16_t action, uint16_t isdir, bstring path)
{
    unsigned char *data = "";
    if (action == 3 && isdir == 0) {
        bstring file_path = bstrcpy(root_dir);
        int r =bcatcstr(file_path, "/");
        r = bconcat(file_path, path);
        // check(r == BSTR_OK, "ERROR :: concatenating bstrings");
        data = fs_read_file((const char *) file_path->data);
    } 
    Message *message = net_message_request_create(action, isdir, path->data, data);
    net_message_send(sock, message);    
}

bstring create_path(bstring dir, char *name, int is_full, int root_len)
{
    bstring path; //must be full for intotify add watch, relative to root dir for sending a message
    int r = 0;

    if (is_full) {
        path = bstrcpy(dir);
        r = bcatcstr(path, "/");
        check(r == BSTR_OK, "ERROR :: concatenating bstrings");

        r = bcatcstr(path, name);
        check(r == BSTR_OK, "ERROR :: concatenating bstrings");

        return path;
    } else {
        path = bstrcpy(dir);
  
        int r = bdelete(path, 0, root_len + 1);
        check(r == BSTR_OK, "ERROR :: delete root string");

        if (path->slen == 0) {
            // We creating something in a root dir
            r = bcatcstr(path, name);
            check(r == BSTR_OK, "ERROR :: concatenating bstrings");
        } else {
            // We are in a subdir, need to append a backslash
            r = bcatcstr(path, "/");
             check(r == BSTR_OK, "ERROR :: concatenating bstrings");

            r = bcatcstr(path, name);
            check(r == BSTR_OK, "ERROR :: concatenating bstrings");
        }
        return path;
    }
error:
    return NULL;
}

int handle_action(HashMap *map, struct inotify_event *event, bstring dir, int action, int fd, int sock, int root_len)
{
    int r = 0;
    bstring path; //must be full for intotify add watch, 
    bstring relative_path = create_path(dir, event->name, 0, root_len); // we will always use relative path
    check_mem(relative_path);
 
    if (event->mask & IN_ISDIR) { 
        path = create_path(dir, event->name, 1, root_len);
        check_mem(path);

        r = add_watch_directory(map, fd, path);
        check(r == 0, "ERROR :: Falied to set watch on: %s", path->data);

        send_message(sock, (uint16_t) action, (uint16_t) 1, relative_path);
        printf("Action %d for a directory %s sent.\n", action, relative_path->data);

        return 1;
    } else {                        
        send_message(sock, (uint16_t) action, (uint16_t) 0, relative_path);
        printf("Action %d for a file %s sent.\n", action, relative_path->data);
    }

    return 1;
error:
    return -1;
}

int process_event(HashMap *map, char *buffer, int fd, int sock, int len)
{
    int j = 0;
    while (j < len) {
        struct inotify_event *event = (struct inotify_event *)&buffer[j];
        if (event->len) {

            printf("Event action: %d\n", event->mask);
            bstring curr_dir = (bstring) collections_hashmap_get(map, event->wd);
            int rc = 0;
            if (event->mask & IN_CREATE) {
                rc = handle_action(map, event, curr_dir, 1, fd, sock, root_dir->slen);
                check(rc == 1, "ERROR :: failed to handle %d event", 1);

                return 1;
            } else if (event->mask & IN_DELETE) {
                rc = handle_action(map, event, curr_dir, 2, fd, sock, root_dir->slen);
                check(rc == 1, "ERROR :: failed to handle %d event", 2);

                return 1;
            } else if (event->mask & IN_MODIFY) {
                // For directories we support only rename operation
                rc = handle_action(map, event, curr_dir, 3, fd, sock, root_dir->slen);
                check(rc == 1, "ERROR :: failed to handle %d event", 2);
                // printf("File %s was modified.\n", event->name);

                return 1;
            } else if (event->mask & IN_MOVE) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was moved.\n", event->name);
                } else {
                    printf("File %s was moved.\n", event->name);
                }
            }
            else if (event->mask & IN_MOVED_TO) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was moved TO.\n", event->name);
                } else {
                    printf("File %s was moved TO.\n", event->name);
                }
            } else if (event->mask & IN_MOVED_FROM) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was moved FROM.\n", event->name);
                } else {
                    printf("File %s was moved FROM.\n", event->name);
                }
            } else if (event->mask & IN_CLOSE) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was CLOSED.\n", event->name);
                } else {
                    printf("File %s was CLOSED.\n", event->name);
                }
            } else if (event->mask & IN_MOVE_SELF) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was moved SELF.\n", event->name);
                } else {
                    printf("File %s was moved SELF.\n", event->name);
                }
            } else if (event->mask & IN_ALL_EVENTS) {
                if (event->mask & IN_ISDIR) { 
                    printf("Directory %s was ALL.\n", event->name);
                } else {
                    printf("File %s was ALL.\n", event->name);
                }
            }
        }
        j += EVENT_SIZE + event->len;
    } 
error:
    return -1;
}

void notify_watch(int dir_count, char **paths, int client_socket)
{
    int fd = 0;
    int read_len = 0;
    

    keep_running = 1;
    // Map keeps track of watch dirs as keys and its names as values
    HashMap *map = collections_hashmap_create(NULL, NULL);
    char event_buffer[EVENT_BUF_LEN];

    unsigned char data_buffer[2048];

    struct sigaction int_handler = {
        .sa_handler=signal_handler
    };
    sigaction(SIGINT,&int_handler, 0);

    fd = inotify_init();
    check(fd > 0, "ERROR :: inotify init");

    // char *root_dir = paths[1];
    // TODO: handle backslash presence in rootdir
    root_dir = bfromcstr(paths[1]);

    int rc = add_watch_directory(map, fd, root_dir);
    check(rc == 0, "ERROR :: Falied to set watch on root directory: %s", root_dir->data);

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    FD_ZERO(&read_fds);
    FD_ZERO(&master);    // clear the master and temp sets
    FD_SET(fd, &master);
    FD_SET(client_socket, &master);

    // Set timeout to 1.0 seconds
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    watch_lock = 0;

    fdmax = fd;
    while (keep_running) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        

        int i = 0;
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got something
                if (i == fd) { // got inotify event

                    // Do not process event if we just made a change to fs
                    if (watch_lock) {
                        watch_lock = 0;
                        // FD_CLR(fd, &master);
                        // clear the FD by consumating the event
                        read_len = read(fd, event_buffer, EVENT_BUF_LEN);
                        
                    } else {
                        read_len = read(fd, event_buffer, EVENT_BUF_LEN);
                        if (read_len < 0) {
                            if (errno == EINTR) { // close running by pressing ctrl-c
                            // notif_clean(map, fd);
                            debug("SIGINT captured. Closing the watcher");
                            goto error;
                            } else {
                                log_err("ERROR :: read events.");
                            }
                        }

                        int ret = process_event(map, event_buffer, fd, client_socket, read_len);
                        check(ret == 1, "ERROR :: Failed to process intotify event");
                    }
                    // FD_SET(fd, &read_fds);
                    // FD_SET(client_socket, &read_fds);
                } else {
                    // got a message from server
                    int numbytes = 0;
                    // check(i == client_socket, "ERROR :: not a client socket");
                    if ((numbytes = recv(client_socket, data_buffer, MAXDATASIZE - 1, 0)) == -1) {
                        log_err("ERROR::ACK not received properly");
                        exit(1);
                    }

                    /* we got some data from a client */
                    uint16_t path_len = (uint16_t) data_buffer[MESSAGE_PATH_LEN_OFFSET];
                    char *path_len_str = net_get_message_str_length(path_len);

                    uint16_t data_len = (uint16_t) data_buffer[MESSAGE_PATH_LEN_OFFSET + path_len + 2];
                    char *data_len_str = net_get_message_str_length(data_len);
                        
                    Message *message = malloc(sizeof(Message));
                    assert(message != NULL);

                    message->path = malloc(path_len * sizeof(unsigned char) + 1); // 1 + for ending nul
                    message->data = malloc(data_len * sizeof(char) + 1); // 1 + for ending nul

                    char * format= strdup(net_concat_strings(5, "hhh", path_len_str, "s", data_len_str, "s"));

                    int16_t message_size;

                    printf("RECEIVED: size of path: %d\n", path_len);
                    printf("RECEIVED: size of data: %d\n", data_len);

                    net_unpack(data_buffer, format, &message_size, &message->action, &message->isdir, 
                                message->path, message->data);

                    printf(" %" PRId16" %" PRId16 "% " PRId16 ": %s \n data: %s \n", 
                                message_size, message->action, message->isdir, message->path, 
                                message->data);
                    
                    watch_lock = 1;
                    fs_make_changes(message, root_dir);
                    // FD_SET(fd, &read_fds);
                    // FD_SET(client_socket, &read_fds);
                }
                
            } 
        }    
    }
    printf("Out of while loop\n");

error:
    printf("Exiting!\n");
    notif_clean(map, fd);
    exit(1);
}

