#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>

#define HAWKNEST_VERSION_STRING "0.1"

#define INFO_PRINT(fmt, args...) printf(fmt, ##args)
#define DEBUG_PRINT(fmt, args...) printf("DEBUG: " fmt, ##args)
#define ERROR_PRINT(fmt, args...) printf("ERROR: " fmt, ##args)


#if DEBUG_ENABLE != 1
#undef DEBUG_PRINT
#define DEBUG_PRINT(fmt, args...)
#endif

#endif /* !__BASE_H__! */
