#include <stdlib.h>
#include <CoreServices/CoreServices.h>

#include "events.h"




//int eventModified = kFSEventStreamEventFlagItemFinderInfoMod |
//                    kFSEventStreamEventFlagItemModified |
//                    kFSEventStreamEventFlagItemInodeMetaMod |
//                    kFSEventStreamEventFlagItemChangeOwner |
//                    kFSEventStreamEventFlagItemXattrMod;



//
//int eventRenamed = kFSEventStreamEventFlagItemCreated |
//                   kFSEventStreamEventFlagItemRemoved |
//                   kFSEventStreamEventFlagItemRenamed;
//
//int eventSystem = kFSEventStreamEventFlagUserDropped |
//                  kFSEventStreamEventFlagKernelDropped |
//                  kFSEventStreamEventFlagEventIdsWrapped |
//                  kFSEventStreamEventFlagHistoryDone |
//                  kFSEventStreamEventFlagMount |
//                  kFSEventStreamEventFlagUnmount |
//                  kFSEventStreamEventFlagRootChanged;



void events_start_watching(char* path, FSEventStreamCallback watch_cb)
{
    /* Define variables and create a CFArray object containing CFString objects containing paths to watch. */
    CFStringRef dirname = CFStringCreateWithCString( NULL, path, kCFStringEncodingUTF8);
    CFArrayRef dir_to_watch = CFArrayCreate( NULL, ( const void ** ) &dirname, 1, NULL );
    void *cb_info = NULL; // could put stream-specific data here. FSEventStreamRef stream;*/
    CFAbsoluteTime latency  = 1.0; /* Latency in seconds */
    FSEventStreamRef stream;

    /* Create the stream, passing in a callback */
    stream = FSEventStreamCreate(NULL, watch_cb, cb_info, dir_to_watch,
                                 kFSEventStreamEventIdSinceNow, latency,
                                 kFSEventStreamCreateFlagFileEvents
                                 );

    /* Create the stream before calling this. */
    FSEventStreamScheduleWithRunLoop(stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    FSEventStreamStart(stream);
    CFRunLoopRun();

}
