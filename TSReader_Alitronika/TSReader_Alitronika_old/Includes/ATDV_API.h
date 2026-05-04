#ifndef _ATDV_API_H
#define _ATDV_API_H

#include "AT_APIDefs.h"
#include "ATBoard.h"
#include "AtBoardManager.h"

#ifdef WIN32
extern HINSTANCE ApiModule;
#endif

#ifdef _DEBUG
#define DEBUGSTRING(s) OutputDebugString(s)
#else
#define DEBUGSTRING(s) // s
#endif

#endif	// _ATDV_API_H
