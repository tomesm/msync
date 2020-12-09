#ifndef __FS_H__
#define __FS_H__

#include "net.h"
#include "bstrlib.h"

// void fs_file_create(unsigned char *path);

// void fs_file_delete(unsigned char *path);

// void fs_dir_create(unsigned char *path);

// int fs_dir_delete(unsigned char *path);

void fs_make_changes(Message *message, bstring root);
unsigned char *fs_read_file (const char * file_name);

#endif