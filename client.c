#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "src/notify.h"
#include "src/net.h"
#include "src/debug.h"
#include "src/bstrlib.h"
#include "src/events.c"

static inline void watch_cb(ConstFSEventStreamRef stream_ref, void *client_cb_info, size_t num_events,
                  void *event_paths, const FSEventStreamEventFlags event_flags[],
                  const FSEventStreamEventId event_ids[])
{
    int i = 0;
    char **paths = event_paths;
    
    for (int i = 0; i < num_events; i++ ) {
        printf( "Changed %s\n", paths[i] );
        printf( "  Modified : %d\n", !!(event_flags[i] & EV_MODIFY));
        printf( "  Renamed  : %d\n", !!( event_flags[i] & EV_RENAME));
        printf( "  Created  : %d\n", !!(event_flags[i] & EV_CREATE));
        printf( "  Deleted  : %d\n", !!( event_flags[i] & EV_DELETE));
        printf( "  Is dir   : %d\n", !!( event_flags[i] & EV_ISDIR));
    }

}

int main(int argc, char **argv)
{
    if (argc < 2) {
		log_err("usage: client dirname\n");
		exit(1);
	}
    
    
    int client_socket = net_client_get_socket();
    events_start_watching(argv[1], watch_cb);

    // int i = 0;

    // bstring root = bfromcstr("dirtest");
    // bstring full_path = bfromcstr("dirtest");

    // bstring relative_path = bstrcpy(full_path);
    
    // int r = bdelete(relative_path, 0, root->slen + 1);

    // printf("relative path: %s\n", relative_path->data);

    return 0;
}
