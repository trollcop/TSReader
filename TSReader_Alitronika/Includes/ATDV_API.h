#ifndef _ATDV_API_H
#define _ATDV_API_H

#include "AT_APIDefs.h"
#include "AtBoardManager.h"
#include "ATBoard.h"
//#include "AtDevices.h"

//#ifdef WIN32
#ifndef LINUX
extern HINSTANCE ApiModule;
#endif
//extern CATDevices g_StoreDeviceLst;
//#endif

#ifdef _DEBUG
#define DEBUGSTRING(s) OutputDebugString(s)
#else
#define DEBUGSTRING(s) void(0)// s
#endif

#endif	// _ATDV_API_H
