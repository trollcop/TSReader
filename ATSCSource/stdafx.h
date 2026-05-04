/*
Copyright (c) David R. Cattley (dcattley@msn.com). All rights reserved.

Module Name:

    stdafx.h

Abstract:

	ATSC BDA Source for TSReader (www.coolstf.com)

Author:

    David R. Cattley (dcattley@msn.com)

Revision History:

	01-Feb-2005 - Created
*/
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

// Windows Header Files:
#include <atlbase.h>
#include <commctrl.h>

#define	STRSAFE_NO_DEPRECATE
#include <strsafe.h>

// DirectShow Header Files:
#include <dshow.h>
#include <uuids.h>
#include <tuner.h>
#include <qedit.h>

// Kernel Streaming & BDA Header Files:
#include <ks.h>
#include <ksmedia.h>
#include <bdatypes.h>
#include <bdamedia.h>
#include <bdaiface.h>


// TODO: reference additional headers your program requires here

// TSReader Header Files:
#include <sources.h>

#ifndef	ASSERT
#define	ASSERT	ATLASSERT
#endif

