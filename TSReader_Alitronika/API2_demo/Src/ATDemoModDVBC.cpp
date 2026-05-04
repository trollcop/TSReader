/*! Time-stamp: <@(#)ATDemoModDVBC.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoModDVBC.cpp
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
// static strings
static const char *strModulation[] =
{
	"INVALID",
	"QAM_16",
	"QAM_32",
	"QAM_64",
	"QAM_128",
	"QAM_256",
	"INVALID",
	"INVALID",
	"INVALID"
};

static const char *strFltRollOff[] =
{
	"12%",
	"13%",
	"15%",
	"18%"
};

static const char *strInterleaver[] =
{
	"I128/J1",
	"I128/J2",
	"I128/J3",
	"I128/J4",
	"I128/J5",
	"I128/J6",
	"I128/J7",
	"I128/J8",
	"I64/J2",	
	"I32/J4",	
	"I16/J8",	
	"I8/J16",	
	"I12/J17"	
};

static const char *strModInversion[] =
{
	"OFF",
	"ON"
};

/**********************************************************************************************************/
// functions

// Set the DVB-C Modulator
BOOL SetModDVBC(CATBoardModDVBC *pATBoardCtrl, SModDVBCOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];

	Message("*****************************************************************");
	Message("Initializing Modulator....");
	// initialize the modulation
	if (!pATBoardCtrl->Init())
		return FALSE;
	SysSleep(100);
	// initialize the RF and IF
	if (!pATBoardCtrl->InitRfIf())
		return FALSE;
	SysSleep(100);
	Message("Initializing Modulator Done");

	Message("Set Modulator....");
	// set the modulation parameters
	if (!pATBoardCtrl->Set(		Options.m_Modulation,
								Options.m_FltRollOff,
								Options.m_Interleaver,
								Options.m_Inversion,
								Options.m_nSymbolRate,
								Options.m_bAnnexBEn))
		return FALSE;
	
	SysSleep(100);
	// set the RF and IF parameters
	if (!pATBoardCtrl->SetRfIf(	Options.m_bRfIfSel,
								Options.m_iIfFrequency,
								Options.m_nRfFrequency,
								Options.m_iRfoutputLevel))
		return FALSE;

	// give the modulator some time to lock.
	SysSleep(1000);
	Message("Set modulator Done");

	// print settings
	Message("Settings:");
	sprintf(TmpStr, "Modulation: %s", strModulation[Options.m_Modulation]);
	Message(TmpStr);
	sprintf(TmpStr, "Interleaver: %s", strInterleaver[Options.m_Interleaver]);
	Message(TmpStr);
	sprintf(TmpStr, "Inversion: %s", strModInversion[Options.m_Inversion]);
	Message(TmpStr);
	sprintf(TmpStr, "AnnexB enabled: %s", Options.m_bAnnexBEn ? "YES" : "NO");
	Message(TmpStr);
	if (Options.m_bAnnexBEn)
	{
		sprintf(TmpStr, "Interleaver: %s", strInterleaver[Options.m_Interleaver]);
		Message(TmpStr);
	}
	else
	{
		sprintf(TmpStr, "Filter roll off: %s", strFltRollOff[Options.m_FltRollOff]);
		Message(TmpStr);
		sprintf(TmpStr, "SymbolRate: %ld", Options.m_nSymbolRate);
		Message(TmpStr);
	}
	sprintf(TmpStr, "RF output enabled: %s", Options.m_bRfIfSel ? "YES" : "IF ONLY");
	Message(TmpStr);
	if (Options.m_bRfIfSel)
	{
		sprintf(TmpStr, "RF output frequency: %ld Hz", Options.m_nRfFrequency);
		Message(TmpStr);
		sprintf(TmpStr, "RF output level: %d dBm", Options.m_iRfoutputLevel/10);
		Message(TmpStr);
	}
	else
	{
		sprintf(TmpStr, "IF output frequency: %ld Hz", Options.m_iIfFrequency);
		Message(TmpStr);
	}
	
	BOOL bModInSync;
	BOOL bMod204PacketSize;
	BOOL bModOFlow;
	BOOL bRfIfLocked;

	// get the modulation status
	if (!pATBoardCtrl->Get(bModInSync, bMod204PacketSize, bModOFlow))
		return FALSE;
	// get the RF and IF status
	if (!pATBoardCtrl->GetRfIf(bRfIfLocked))
		return FALSE;

	// print status
	Message("Status:");
	sprintf(TmpStr, "Modulator is in sync: %s", bModInSync ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Modulator packet size: %s", bMod204PacketSize ? "204" : "188");
	Message(TmpStr);
	sprintf(TmpStr, "Modulator overflow status : %s", bModOFlow ? "YES ERROR" : "OK");
	Message(TmpStr);
	sprintf(TmpStr, "Modulator RF is locked: %s", Options.m_bRfIfSel ? (bRfIfLocked ? "YES" : "NO") : "ONLY IF");
	Message(TmpStr);

	if(bModInSync == FALSE)
		return FALSE;

	return TRUE;
}
