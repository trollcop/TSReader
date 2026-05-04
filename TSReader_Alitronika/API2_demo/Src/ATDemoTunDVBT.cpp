/*! Time-stamp: <@(#)ATDemoTunDVBT.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoTunDVBT.cpp
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
	"INVALID",
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

static const char *strModHierarch[] =
{
	"NONE",
	"1",
	"2",
	"3",
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

/**********************************************************************************************************/
// functions

// Set the DVB-C AnnexA tuner
BOOL SetTunDVBT(CATBoardTunDVBT *pATBoardCtrl, STunDVBTOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];
	char BandWidthStr[MAX_TMP_STR_LEN];

	Message("*****************************************************************");
	Message("Initializing Tuner....");
	// initialize the tuner
	if (!pATBoardCtrl->Init())
		return FALSE;
	Message("Initializing Tuner Done");

	Message("Set tuner....");
	// set the tuner
	if (!pATBoardCtrl->Set(Options.m_nFrequency, Options.m_BandWidth, Options.m_bBandWidthGroup))
		return FALSE;

	// give the tuner some time to lock.
	SysSleep(1000);
	Message("Set tuner Done");

	// print settings
	switch(Options.m_BandWidth)
	{
	case BANDWIDTH_8_7_MHZ:
		if (Options.m_bBandWidthGroup)
			sprintf(BandWidthStr, "NOT SUPPORTED");
		else
			sprintf(BandWidthStr, "8MHz");
		break;
	case BANDWIDTH_7_6_MHZ:
		if (Options.m_bBandWidthGroup)
			sprintf(BandWidthStr, "6MHz");
		else
			sprintf(BandWidthStr, "7MHz");
		break;
	case BANDWIDTH_6_MHZ:
		if (Options.m_bBandWidthGroup)
			sprintf(BandWidthStr, "NOT SUPPORTED");
		else
			sprintf(BandWidthStr, "6MHz");
		break;
	default:
		sprintf(BandWidthStr, "NOT SUPPORTED");
		break;
	}

	Message("Settings:");
	sprintf(TmpStr, "Frequency: %ld Hz", Options.m_nFrequency);
	Message(TmpStr);
	sprintf(TmpStr, "BandWidth: %s", BandWidthStr);
	Message(TmpStr);
	sprintf(TmpStr, "BandWidth Group: %d", Options.m_bBandWidthGroup);
	Message(TmpStr);

	BOOL bInSync;
	BOOL bCarrierDet;
	BOOL bSigDet;
	BOOL bViterbiDet;
	u16 nSignalStrength;
	u16 nSnr;
	u32 nBer;
	src_modulation_t Modulation;
	src_transmit_mode_t ModTransmitMode;
	src_guard_interval_t ModGuard;
	src_hierarchy_t ModHierarch;
	src_code_rate_t ModHPFecCodeRate;
	src_code_rate_t ModLPFecCodeRate; 

	// get status
	if (!pATBoardCtrl->Get(	bInSync,
		bCarrierDet,
		bSigDet,
		bViterbiDet,
		nSignalStrength,
		nSnr,
		nBer,
		Modulation,
		ModTransmitMode,
		ModGuard,
		ModHierarch,
		ModHPFecCodeRate,
		ModLPFecCodeRate))
		return FALSE;

	// Signal to noise ration should be divided by 32.
	double dSnr = nSnr / 32.0;

	// print status
	Message("Status:");
	sprintf(TmpStr, "Signal detected: %s", bSigDet ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Carrier detected: %s", bCarrierDet ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "COFDM detected: %s", bViterbiDet ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "In sync: %s", bInSync ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Signal Strength: %ld", nSignalStrength);
	Message(TmpStr);
	sprintf(TmpStr, "Signal to noise ratio: %.1f dB", dSnr);
	Message(TmpStr);
	sprintf(TmpStr, "Bit error rate: %ld bit/s", nBer);
	Message(TmpStr);
	sprintf(TmpStr, "Modulation: %s", strModulation[Modulation]);
	Message(TmpStr);
	sprintf(TmpStr, "Transmission mode: %s", strModTransmitMode[ModTransmitMode]);
	Message(TmpStr);
	sprintf(TmpStr, "Guard interval: %s", strModGuard[ModGuard]);
	Message(TmpStr);
	sprintf(TmpStr, "Hierarchical mode: %s", strModHierarch[ModHierarch]);
	Message(TmpStr);
	sprintf(TmpStr, "High priority FEC code rate: %s", strModFecCodeRate[ModHPFecCodeRate]);
	Message(TmpStr);
	sprintf(TmpStr, "Low priority FEC code rate: %s", strModFecCodeRate[ModLPFecCodeRate]);
	Message(TmpStr);
	
	return TRUE;
}
