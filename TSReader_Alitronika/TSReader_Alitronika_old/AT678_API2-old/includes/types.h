#ifndef _AT_TYPES_H_
#define _AT_TYPES_H_


typedef unsigned long u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef signed long s32;
typedef signed short s16;
typedef signed char s8;

#ifndef TUNER_EMBEDED
/** TSysMutex
 *	A wrapper type for a system mutex handle.
 */
typedef HANDLE	TSysMutex;		

/** TDeviceClass
 *	A wrapper type to define a device class (win32: const GUID)
 */
typedef const GUID	TDeviceClass;
#else
#include <windef.h>

#endif

#endif	// _AT_TYPES_H