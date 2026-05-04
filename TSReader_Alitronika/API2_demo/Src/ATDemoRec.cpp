/*! Time-stamp: <@(#)ATDemoRec.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoRec.cpp
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

// record a file
BOOL RecordFile(CATBoardPlayRec *pATBoardCtrl, char *pFilename, SRecOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];

	Message("*****************************************************************");
	Message ("Initializing Record....");
	if (!pATBoardCtrl->InitRec())
	{
		Message ("Initializing Record failed");
		return FALSE;
	}
	SysSleep(100);

	Message ("Initializing record Done");

	ETSPSize TsPSize;
	BOOL bInSync;
	BOOL bCarrierDetect;
	BOOL bLocked;
	u32 nBitrate;
	pATBoardCtrl->GetRecStatus(TsPSize, bInSync, bCarrierDetect, bLocked, nBitrate);

	sprintf(TmpStr, "Packet size: %s", TsPSize==TSPSIZE_188 ? "188" : "204");
	Message(TmpStr);
	sprintf(TmpStr, "InSync: %s", bInSync ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Carrier Detect: %s", bCarrierDetect ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Locked: %s", bLocked ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Bitrate: %ld Bit/s", nBitrate);
	Message(TmpStr);

	if (bInSync && bCarrierDetect && bLocked)
	{
		Message ("Recording file.....");
		if (!pATBoardCtrl->RecFile(pFilename, Options.m_nFileSize))
		{
			Message ("Recording file failed");
			return FALSE;
		}
	}
	else
	{
		Message("No valid input transport stream detected: can't start recording");
		return FALSE;
	}

	Message ("Recording file Done");

	return TRUE;
}
