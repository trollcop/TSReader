// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ATDV_API_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ATDV_API_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifndef _ATDV_APIDEF_H
#define _ATDV_APIDEF_H

#ifdef ATDV_API_EXPORTS
#ifdef _WINDOWS
#define ATDV_API_API __declspec(dllexport)
#else
#ifdef _LINUX
#define ATDV_API_API
#endif	// _LINUX
#endif	// _WINDOWS

#else	// !ATDV_API_EXPORTS
#ifdef WIN32
#define ATDV_API_API __declspec(dllimport)
#else
#ifdef LINUX
#define ATDV_API_API
#endif	// _LINUX
#endif	// _WINDOWS
#endif	// ATDV_API_EXPORTS

#endif	// _ATDV_APIDEF_H
