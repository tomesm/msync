#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#include "fs.h"

#define DATA_SIZE 2048

static inline int unlink_dir_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv = remove(fpath);

    if (rv)
        perror(fpath);

    return rv;
}

static inline int dir_delete(const char *path)
{
    return nftw(path, unlink_dir_cb, 64, FTW_DEPTH | FTW_PHYS);
}


static inline void dir_create(const char *path) 
{
    struct stat st = {0};

    if (stat(path, &st) == -1) {
        mkdir(path, 0700);
        printf("Directory created successfully.\n");
    }
}

static inline void file_create(const char *path)
{    
    FILE *file;
    file = fopen(path, "wr");
    if(file == NULL) {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    // /* Write data to file */
    // fputs(data, fPtr);

    fclose(file);
    printf("File created successfully.\n");
}

static inline void file_delete(const char *path)
{
    if (remove(path) == 0) 
      printf("File %s deleted successfully\n", path); 
   else
    //   printf("Unable to delete the file %s\n", path);
    perror( "Error deleting file" );
}

void fs_make_changes(Message *message, bstring root)
{
    bstring path = bstrcpy(root);
    int r = bcatcstr(path, "/");
    //check(r == BSTR_OK, "ERROR :: concatenating bstrings");

    r = bcatcstr(path, (const char*) message->path);
    //check(r == BSTR_OK, "ERROR :: concatenating bstrings");

    if (message->action == 1) {
        if (message->isdir) {
            dir_create((const char*) path->data);
        } else {
            file_create( (const char*) path->data);
        }
    } else if (message->action == 2) {
        if (message->isdir) {
            dir_delete((const char*) path->data);
        } else {
            file_delete((const char*) path->data);
        }
    }
}

// void content_create(char *path, int isdir)
// {
//     /* Variable to store user content */
//     // TODO: implement situation for creating files with data
//     // char data[DATA_SIZE];

//     if (isdir) {
//         dir_create(path);
//     } else {
//         file_create(path);
//     }
// }





// char * search_file(int number)  
// {   
//     int i;      
//     for (i = 0; i < max_watchers; i++) {
//         if (i == number) {        
//             //if (strlen(files[i]) > 4)
//             return files[i];
//         }
//     }
    
//     return NULL;
// }

// static unsigned get_file_size (const char * file_name)
// {
//     struct stat sb;
//     if (stat (file_name, & sb) != 0) {
//         fprintf (stderr, "'stat' failed for '%s': %s.\n",
//                  file_name, strerror (errno));
//         exit (EXIT_FAILURE);
//     }
//     return sb.st_size;
// }

// static unsigned char * read_whole_file (const char * file_name)
// {
//     unsigned s;
//     unsigned char * contents;
//     FILE * f;
//     size_t bytes_read;
//     int status;

//     s = get_file_size (file_name);
//     contents = malloc (s + 1);
//     if (! contents) {
//         fprintf (stderr, "Not enough memory.\n");
//         exit (EXIT_FAILURE);
//     }

//     f = fopen (file_name, "rb");
//     if (!f) {
//         fprintf (stderr, "Could not open '%s': %s.\n", file_name,
//         strerror (errno));
//         exit (EXIT_FAILURE);
//     }

//     bytes_read = fread (contents, sizeof (unsigned char), s, f);
//     if (bytes_read != s) {
//         fprintf (stderr, "Short read of '%s': expected %d bytes "
//                  "but got %d: %s.\n", file_name, s, (int) bytes_read,
//                  strerror (errno));
//         exit (EXIT_FAILURE);
//     }

//     status = fclose (f);
//     if (status != 0) {
//         fprintf (stderr, "Error closing '%s': %s.\n", file_name,
//                  strerror (errno));
//         exit (EXIT_FAILURE);
//     }
//     return contents;
// }