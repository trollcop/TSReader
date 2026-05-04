/*! Time-stamp: <@(#)SysFunc.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : SysFunc.cpp
 *
 *  Project : ATTestApp. Testappliction for Alitronika devices.
 *			  Supports Linux and windows operating System.
 *
 *  Package : 
 *
 *  Company : Engineering Spirit
 *
 *  Author  : P.Hoogervorst                              Date: 07/05/2007
 *
 *  Purpose : Implementation of methods for class 
 *
 *********************************************************************
 * Version History:
 *
 * V 0.10  07/05/2007  BN : First Revision
 *
 *********************************************************************
 */

/**********************************************************************************************************/
// includes
#include "SysFunc.h"

// include the operating system dependent file

#ifdef LINUX
#include "SysFuncLinux.cpp"
#else
#ifdef WIN32
#include "SysFuncWindows.cpp"
#endif
#endif
