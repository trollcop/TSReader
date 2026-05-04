/* @file	: win32.h
  *	purpose	: Providing windows-specific includes,
  *			  macros, and types
  *
  * (c) Copyright Engineering Spirit NL 2005.
  * All rights reserved.
  *
  *	By		: Henk van de Berg
  *
  *	Reviion list:
  *	Date		Changes\n
  *	2006-12-12	Initial release
  */
#ifndef WIN32_H
#define WIN32_H

#ifdef WIN32

#include "common.h"
#include "ATcommands.h"
#include "ddkinc/DEVIOCTL.h"	// Standard file from the ddkinc

/**	The version of the ERROR and SLEEP macros.
  *	These macros are available in every
  *	environment.
  */
#define ERROR_MSG(msg) \
    ::MessageBox(NULL, msg, "Error", MB_OK|MB_ICONERROR);

/** TSlong
  *	Thread-Safe long, a class for portable, thread-safe long
  *	4-byte values.
  */
class TSLONG
{
	public:
		explicit TSLONG(void)
		{
			value_ = 0;
		}
		
		long operator++()
		{
			return InterlockedIncrement(&value_);
		}

		long operator--()
		{
			return InterlockedDecrement(&value_);
		}

		operator long() const
		{
			return value_;
		}

	private:

		long value_;
};

/** TSysMutex
 *  A wrapper type for a system mutex handle.
 */
typedef HANDLE	TSysMutex;

/** TDeviceClass
 *  A wrapper type to define a device class (win32: const GUID)
 */
typedef const GUID	TDeviceClass;

#endif // WIN32

#endif	// WIN32_H

