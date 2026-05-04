
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ATDV_API_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ATDV_API_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifndef _ATDV_API_H
#define _ATDV_API_H

#ifdef ATDV_API_EXPORTS
#define ATDV_API_API __declspec(dllexport)
#else
#define ATDV_API_API __declspec(dllimport)
#endif

#include "types.h"
//#include "SysFunc.h"
#include "ATBoard.h"
#include "AtBoardManager.h"

#ifdef _DEBUG
#define DEBUGSTRING(s) OutputDebugString(s)
#else
#define DEBUGSTRING(s)
#endif

#endif	// _ATDV_API_H