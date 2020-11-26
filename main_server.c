#include "src/connection.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    connection_server_run();
    //
    /*unsigned char buf[1024];*/
    /*int8_t magic;*/
    /*int16_t monkeycount, isdir;*/
    /*int32_t altitude;*/
    /*float32_t absurdityfactor;*/
    /*char *s = "Great unmitigated Zot! You've found the Runestaff!";*/
    /*char s2[96];*/
    /*int16_t packetsize, ps2;*/
    /*char *path = "path/to/test/dir";*/
    /*char path2[17];*/


    /*ProtocolMessage *message = malloc(sizeof(ProtocolMessage));*/

    /*ProtocolMessage *message_to_send = malloc(sizeof(ProtocolMessage));*/
    /*message_to_send->action = 1;*/
    /*message_to_send->isdir = 1;*/

    /*packetsize = networkSerialize(buf, "hh", message_to_send->action, message_to_send->isdir);*/
    /*packi16(buf, packetsize); // store packet size in packet for kicks*/

    /*printf("packet is %" PRId32 " bytes\n", packetsize);*/
    /*printf("Content of buffer is: %c\n", buf[0]);*/


    /*networkDeserialize(buf, "hh", &message->action, &message->isdir);*/

    /*printf(" %" PRId16" %" PRId16, message->action, message->isdir);*/



    return 0;
}
