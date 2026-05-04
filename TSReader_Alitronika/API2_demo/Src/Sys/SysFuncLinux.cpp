/*! Time-stamp: <@(#)SysFuncLinux.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : SysFuncLinux.cpp
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
#include <sys/fcntl.h>			// for open
#include <sys/unistd.h>			// for close, read, write
#include <sys/stat.h>
#include <stdio.h>

#include "SysFunc.h"


/**********************************************************************************************************/
// functions

// sleep  //Already defined in the lib 
//void SysSleep(u32 nSleepMs)
//{
	//nSleepMs *= 1000;
	//usleep(nSleepMs );
//}


// open a file for read 
s32 SysFileOpenRead(char * pFileName)
{
	return (s32)open(pFileName, O_RDONLY| O_LARGEFILE );
}

// read the data from the file
s32 SysFileRead(s32 iFileDescrValue, char * pBuffer, u32 nBufSize)
{
	return (s32)read(iFileDescrValue, pBuffer, nBufSize);
}

// open a file for write
s32 SysFileOpenWrite(char * pFileName)
{
	return (s32)open(pFileName, O_WRONLY | O_CREAT | O_LARGEFILE);
}

// write data to the file
s32 SysFileWrite(s32 iFileDescrValue, char * pBuffer, u32 nBufSize)
{
	return (s32)write (iFileDescrValue, pBuffer, nBufSize);
}

// close a file
s32 SysFileClose(s32 iFileDescrValue)
{
	return (s32)close(iFileDescrValue);
}
