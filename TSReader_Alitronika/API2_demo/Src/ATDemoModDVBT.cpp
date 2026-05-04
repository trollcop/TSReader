/*! Time-stamp: <@(#)ATDemoModDVBT.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoModDVBT.cpp
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
	"QPSK   ",
	"QAM_16 ",
	"INVALID",
	"QAM_64 ",
	"INVALID",
	"INVALID",
	"INVALID",
	"INVALID",
	"INVALID"
};

static const char *strModTransmitMode[] =
{
	"2K",
	"4K",
	"8K",
	"INVALID"
};

static const char *strModGuard[] =
{
	"1/32",
	"1/16",
	"1/8",
	"1/4",
	"INVALID"
};

static const char *strModFecCodeRate[] =
{
	"INVALID",
	"1/2",
	"2/3",
	"3/4",
	"INVALID",
	"5/6",
	"INVALID",
	"7/8",
	"INVALID",
	"INVALID"
};

static const char *strModBandwidth[] =
{
	"8MHz",
	"7MHz",
	"6MHz",
	"5MHz"
};

static const char *strModInversion[] =
{
	"OFF",
	"ON"
};

/**********************************************************************************************************/
// functions

// Set the DVB-T modulator
BOOL SetModDVBT(CATBoardModDVBT *pATBoardCtrl, SModDVBTOpt Options)
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
								Options.m_TranMode,
								Options.m_GuardInt,
								Options.m_FECCodeRate,
								Options.m_Bandwidth,
								Options.m_Inversion,
								Options.m_bDvbHEn,
								Options.m_bDvbHTimeSlice,
								Options.m_bDvbH_MPE_FEC,
								Options.m_bDvbHIndepthInt,
								Options.m_nDvbHCellId))
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
	sprintf(TmpStr, "Transmission mode: %s", strModTransmitMode[Options.m_TranMode]);
	Message(TmpStr);
	sprintf(TmpStr, "Guard interval: %s", strModGuard[Options.m_GuardInt]);
	Message(TmpStr);
	sprintf(TmpStr, "FEC code rate: %s", strModFecCodeRate[Options.m_FECCodeRate]);
	Message(TmpStr);
	sprintf(TmpStr, "Bandwidth: %s", strModBandwidth[Options.m_Bandwidth]);
	Message(TmpStr);
	sprintf(TmpStr, "Inversion: %s", strModInversion[Options.m_Inversion]);
	Message(TmpStr);
	sprintf(TmpStr, "DVB-H extensions enabled: %s", Options.m_bDvbHEn ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "DVB-H Timeslicing enabled: %s", Options.m_bDvbHTimeSlice ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "DVB-H MPE FEC enabled: %s", Options.m_bDvbH_MPE_FEC ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "DVB-H Indepth interleaver enabled: %s", Options.m_bDvbHIndepthInt ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "DVB-H Cell ID: %d", Options.m_nDvbHCellId);
	Message(TmpStr);
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
	BOOL bModUFlow;
	BOOL bRfIfLocked;

	// get the modulation status
	if (!pATBoardCtrl->Get(bModInSync, bMod204PacketSize, bModOFlow, bModUFlow))
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
	sprintf(TmpStr, "Modulator underflow status: %s", bModUFlow ? "YES ERROR" : "OK");
	Message(TmpStr);
	sprintf(TmpStr, "Modulator RF is locked: %s", Options.m_bRfIfSel ? (bRfIfLocked ? "YES" : "NO") : "ONLY IF");
	Message(TmpStr);

	return TRUE;
}
