/** @file	: common.h
  *	purpose	: Providing includes, macros, and
  *			  types included in all environments
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
#ifndef COMMON_H
#define COMMON_H 1

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef char			s8;
typedef short			s16;
typedef long			s32;

#ifdef TUNER_EMBEDDED
#  include "embedded.h"
#else
#ifdef WIN32
#  include "win32.h"
#else
#ifdef LINUX
#  include "linux.h"
#endif /* LINUX */
#endif /* WIN32 */
#endif /* TUNER_EMBEDDED */


#endif /* COMMON_H */

