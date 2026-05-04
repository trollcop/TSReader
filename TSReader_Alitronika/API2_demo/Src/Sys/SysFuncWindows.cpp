/*! Time-stamp: <@(#)SysFuncWindows.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : SysFuncWindows.cpp
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
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "SysFunc.h"

/**********************************************************************************************************/
// functions

// sleep 
void SysSleep(u32 nSleepMs)
{
	Sleep(nSleepMs);
}

// open a file for read 
s32 SysFileOpenRead(char * pFileName)
{
	return (s32)_open(pFileName, _O_RDONLY| _O_BINARY);
}

// read the data from the file
s32 SysFileRead(s32 iFileDescrValue, char * pBuffer, u32 nBufSize)
{
	return (s32)_read(iFileDescrValue, pBuffer, nBufSize);
}

// open a file for write
s32 SysFileOpenWrite(char * pFileName)
{
	return (s32)_open(pFileName, _O_CREAT | _O_TRUNC | _O_BINARY | _O_RDWR, _S_IWRITE);
}

// write data to the file
s32 SysFileWrite(s32 iFileDescrValue, char * pBuffer, u32 nBufSize)
{
	return (s32)_write(iFileDescrValue, pBuffer, nBufSize);
}

// close a file
s32 SysFileClose(s32 iFileDescrValue)
{
	return (s32)_close(iFileDescrValue);
}
