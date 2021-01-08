#ifndef __NET_H__
#define __NET_H__

#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h> // defines uintN_t types
#include <inttypes.h> // defines PRIx macros
#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

// various bits for floating point types--
// varies for different architectures
typedef float float32_t;
typedef double float64_t;

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

#define MAXDATASIZE 255
#define MESSAGE_PATH_LEN_OFFSET 7

typedef struct Message {
    uint16_t action;
    uint16_t isdir;
    unsigned char *path;
    char *data;
}Message;

// uint64_t pack754(long double f, unsigned bits, unsigned expbits);

// long double unpack754(uint64_t i, unsigned bits, unsigned expbits);

// /**
//  * @brief store a 16-bit int into a char buffer (like htons())
//  *
//  * @param unsigned char * buf a buffer
//  * @param unsigned int i
//  */
// void packi16(unsigned char * buf, unsigned int i);


// /**
//  * @brief store a 32-bit int into a char buffer (like htonl())
//  */
// void packi32(unsigned char * buf, unsigned long i);

// /**
//  * @brief unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
// */
// unsigned int unpacki16(unsigned char * buf);


// /*
// ** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
// */
// unsigned long unpacki32(unsigned char * buf);


/*
** connection_pack() -- store data dictated by the format string in the buffer
**
**  h - 16-bit              l - 32-bit
**  c - 8-bit char          f - float, 32-bit
**  s - string (16-bit length is automatically prepended)
*/
int32_t net_pack(unsigned char * buf, const char * format, ...);

/*
** net_unpack() -- unpack data dictated by the format string into the buffer
*/
void net_unpack(unsigned char * buf, const char * format, ...);

// void *net_get_in_addr(struct sockaddr *sa);

void net_message_send(int sockfd, Message *message);

Message *net_message_request_create(uint16_t action, uint16_t isdir, unsigned char *path, unsigned char *data);

char *net_concat_strings(int count, ...);

int net_client_get_socket();

void net_server_run();

char *net_get_message_str_length(uint16_t len);

#endif
