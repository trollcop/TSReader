/*! Time-stamp: <@(#)ATDemoPlay.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoPlay.cpp
 *
 *  Project : ATDemoApp. Testappliction for Alitronika devices.
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
// operating system specific operations
#ifndef LINUX
#pragma warning (disable : 4996)
#pragma warning (disable : 4005)
#endif

/**********************************************************************************************************/
// includes
#include <stdio.h>
#include "ATDemo.h"

/**********************************************************************************************************/
// functions

// play a file
BOOL PlayFile(CATBoardPlayRec *pATBoardCtrl, char *pFilename, SPlayOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];
	FILE * InFileStream;

	// open file
	InFileStream = fopen (pFilename, "rb");

	if (InFileStream == NULL)
	{
		Message ("Error opening input file.");
		return FALSE;
	}

	// get file bit rate by tsrate
	double dFileBitrate = tsrate(InFileStream);
	
	// close file
	fclose (InFileStream);

	// check file bit rate
	if(dFileBitrate == 0.0)		// use default bit rate is file bit rate is not found in file
		dFileBitrate = (double)DEFAULT_BITRATE;
	if(Options.m_dFileBitrate)	// override file bit rate by user bit rate
		dFileBitrate = Options.m_dFileBitrate;

	// check output bit rate
	BOOL bReMux = FALSE;
	double dOutputBitrate = MAX_ASIBITRATE;
	
	if(dFileBitrate < Options.m_dOutputBitrate)
	{
		bReMux = Options.m_bReMux;
		if (bReMux)
			dOutputBitrate = Options.m_dOutputBitrate;
	}

	Message("*****************************************************************");
	Message ("Initializing Play....");
	if (!pATBoardCtrl->InitPlay((u32)(dFileBitrate+0.5), bReMux,(u32)(dOutputBitrate+0.5)))
	{
		Message ("Initializing Play failed");
		return FALSE;
	}
	SysSleep(100);

	Message ("Initializing Play Done");
	
	sprintf(TmpStr, "File bitrate: %ld Bit/s", (u32)dFileBitrate);
	Message(TmpStr);
	sprintf(TmpStr, "Output bitrate: %ld Bit/s", bReMux ? (u32)dOutputBitrate : (u32)dFileBitrate);
	Message(TmpStr);
	sprintf(TmpStr, "Bitrate remultiplexing: %s", bReMux ? "On" : "Off");
	Message(TmpStr);

	Message ("Playing file.....");
	if (!pATBoardCtrl->PlayFile(pFilename))
	{
		Message ("Playing file failed");
		return FALSE;
	}

	Message ("Playing file Done");

	return TRUE;
}
