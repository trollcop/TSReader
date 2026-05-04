#ifndef _GP_DEBUG_H_
#define _GP_DEBUG_H_

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

void __cdecl odprintf(const char *format, ...)
{
char	buf[4096], *p = buf;
va_list	args;

        va_start(args, format);
        p += _vsnprintf(p, sizeof buf - 1, format, args);
        va_end(args);

        while ( p > buf  &&  isspace(p[-1]) )
                *--p = '\0';

        *p++ = '\r';
        *p++ = '\n';
        *p   = '\0';

        OutputDebugString(buf);
}
#if !defined(TRACE)
#define TRACE OutputDebugStringA
#endif

#if !defined(TRACE_F)
#define TRACE_F odprintf
#endif

#if !defined(ERR)
#define ERR odprintf
#endif

#define CYPRESS_INTERFACE_NUM 0
#define CYPRESS_ALTERNATE_SETTING 0


#define ATTACH_EVENT_TIMEOUT 30 // in seconds
#define TRANSFER_TIMEOUT 30000 // in msecs

#endif /* _GP_DEBUG_H_ */