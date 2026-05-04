/*! Time-stamp: <@(#)ATDemoTunDVBCAnnexB.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoTunDVBCAnnexB.cpp
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

// Set the DVB-C AnnexB tuner
BOOL SetTunDVBCAnnexB(CATBoardTunDVBCAnnexB *pATBoardCtrl, STunDVBCAnnexBOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];
	char ModulationStr[MAX_TMP_STR_LEN];

	Message("*****************************************************************");
	Message("Initializing Tuner.... (can take up to two minutes)");
	// initialize the tuner
	if (!pATBoardCtrl->Init())
		return FALSE;
	SysSleep(100);
	Message("Initializing Tuner Done");

	Message("Set tuner....");
	// set the tuner
	if (!pATBoardCtrl->Set(Options.m_nFrequency, Options.m_Modulation))
		return FALSE;

	// give the tuner some time to lock.
	SysSleep(1000);
	Message("Set tuner Done");

	switch(Options.m_Modulation)
	{
	case QAM_64:
		sprintf(ModulationStr, "QAM64");
		break;
	case QAM_256:
		sprintf(ModulationStr, "QAM256");
		break;
	case VSB_8:
		sprintf(ModulationStr, "VSB-8");
		break;
	default:
		sprintf(ModulationStr, "NOT SUPPORTED");
		break;
	}

	Message("Settings:");
	sprintf(TmpStr, "Frequency: %ld Hz", Options.m_nFrequency);
	Message(TmpStr);
	sprintf(TmpStr, "Modulation: %s", ModulationStr);
	Message(TmpStr);

	BOOL bInSync;
	BOOL bCarrierDetect;
	BOOL bTsLock;
	u16 nSnr;
	u32 nCorBer;
	u32 nUnCorBer;
	
	// get status
	if (!pATBoardCtrl->Get(bInSync, bCarrierDetect, bTsLock, nSnr, nCorBer, nUnCorBer))
		return FALSE;

	// Signal to noise ration should be divided by 32.
	double dSnr = nSnr / 32.0;

	// print status
	Message("Status:");
	sprintf(TmpStr, "In sync: %s", bInSync ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Carrier detect: %s",  bCarrierDetect ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Transports stream lock: %s",  bTsLock ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Signal to noise ratio: %.1f dB", dSnr);
	Message(TmpStr);
	sprintf(TmpStr, "Correctable bit error rate: %ld Bit/s", nCorBer);
	Message(TmpStr);
	sprintf(TmpStr, "UnCorrectable bit error rate: %ld Bit/s", nUnCorBer);
	Message(TmpStr);

	return TRUE;
}
