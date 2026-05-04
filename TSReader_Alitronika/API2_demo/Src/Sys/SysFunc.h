/*! Time-stamp: <@(#)SysFunc.h   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : SysFunc.h
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

#ifndef __SYSFUNC_H__
#define __SYSFUNC_H__

/**********************************************************************************************************/
// includes
#include <common.h>


/**********************************************************************************************************/
// functions

/*!
 * Go to sleep
 *
 * @param nSleepMs : Time in milliseconds
 *
 */
void SysSleep(u32 nSleepMs);

/*!
 * Open a file for read
 *
 * @param pFileName : File name to open
 *
 * @return s32  : -1 on error else a positive value
 */
s32 SysFileOpenRead(char * pFileName);

/*!
 * Read data from a file
 *
 * @param iFileDescrValue : File descriptor referring to the open file
 * @param pBuffer : Pointer to buffer to store the read data to
 * @param nBufSize : Requested number of bytes to read
 *
 * @return s32  : -1 on error, else the number of bytes read (can be smaller then nBufSize when there is less data available)
 */
s32 SysFileRead(s32 iFileDescrValue, char * pBuffer, u32 nBufSize);

/*!
 * Open file for write
 *
 * @param pFileName : File name to open
 *
 * @return s32  : -1 on error else a positive value
 */
s32 SysFileOpenWrite(char * pFileName);

/*!
 * Write data to a file
 *
 * @param iFileDescrValue : File descriptor referring to the open file
 * @param pBuffer : pointer to the buffer that contains the data to be written
 * @param nBufSize : Number of bytes to write
 *
 * @return s32  : -1 on error, else the actual number of bytes written.
 */
s32 SysFileWrite(s32 iFileDescrValue, char * pBuffer, u32 nBufSize);

/*!
 * Close a file
 *
 * @param iFileDescrValue : File descriptor referring to the open file
 *
 * @return s32  : -1 on error, else 0.
 */
s32 SysFileClose(s32 iFileDescrValue);


#endif // __SYSFUNC_H__
