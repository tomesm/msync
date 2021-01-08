#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <CoreServices/CoreServices.h>

#define EV_MODIFY kFSEventStreamEventFlagItemModified | \
                    kFSEventStreamEventFlagItemFinderInfoMod | \
                    kFSEventStreamEventFlagItemInodeMetaMod

#define EV_CREATE  kFSEventStreamEventFlagItemCreated

#define EV_DELETE  kFSEventStreamEventFlagItemRemoved

#define EV_ISDIR    kFSEventStreamEventFlagItemIsDir

#define EV_RENAME  kFSEventStreamEventFlagItemRenamed


#endif