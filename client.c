#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "src/notify.h"
#include "src/net.h"
#include "src/debug.h"
#include "src/bstrlib.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
		log_err("usage: client dirname\n");
		exit(1);
	}
    
    
    int client_socket = net_client_get_socket();
    notify_watch(argc, argv, client_socket);

    // int i = 0;

    // bstring root = bfromcstr("dirtest");
    // bstring full_path = bfromcstr("dirtest");

    // bstring relative_path = bstrcpy(full_path);
    
    // int r = bdelete(relative_path, 0, root->slen + 1);

    // printf("relative path: %s\n", relative_path->data);

    return 0;
}
