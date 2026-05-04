/*! Time-stamp: <@(#)ATDemoTunDVBCAnnexA.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoTunDVBCAnnexA.cpp
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

// Set the DVB-C AnnexA tuner
BOOL SetTunDVBCAnnexA(CATBoardTunDVBCAnnexA *pATBoardCtrl, STunDVBCAnnexAOpt Options)
{
	char TmpStr[MAX_TMP_STR_LEN];
	char InvertStr[MAX_TMP_STR_LEN];
	char ModulationStr[MAX_TMP_STR_LEN];

	Message("*****************************************************************");
	Message("Initializing Tuner....");
	// initialize the tuner
	if (!pATBoardCtrl->Init())
		return FALSE;
	Message("Initializing Tuner Done");

	Message("Set tuner....");
	// set the tuner
	if (!pATBoardCtrl->Set(Options.m_nFrequency, Options.m_nSymbolRate, Options.m_Invert, Options.m_Modulation))
		return FALSE;

	// give the tuner some time to lock.
	SysSleep(1000);
	Message("Set tuner Done");

	// print settings
	switch(Options.m_Invert)
	{
	default:
	case INVERSION_OFF:
		sprintf(InvertStr, "OFF");
		break;
	case INVERSION_ON:
		sprintf(InvertStr, "ON");
		break;
	case INVERSION_AUTO:
		sprintf(InvertStr, "AUTO");
		break;
	}

	switch(Options.m_Modulation)
	{
	case QAM_16:
		sprintf(ModulationStr, "QAM16");
		break;
	case QAM_32:
		sprintf(ModulationStr, "QAM32");
		break;
	case QAM_64:
		sprintf(ModulationStr, "QAM64");
		break;
	case QAM_128:
		sprintf(ModulationStr, "QAM128");
		break;
	case QAM_256:
		sprintf(ModulationStr, "QAM256");
		break;
	case QAM_AUTO:
		sprintf(ModulationStr, "AUTO");
		break;
	default:
		sprintf(ModulationStr, "NOT SUPPORTED");
		break;
	}

	Message("Settings:");
	sprintf(TmpStr, "Frequency: %ld Hz", Options.m_nFrequency);
	Message(TmpStr);
	sprintf(TmpStr, "SymbolRate: %ld Symbols/s", Options.m_nSymbolRate);
	Message(TmpStr);
	sprintf(TmpStr, "Inversion: %s", InvertStr);
	Message(TmpStr);
	sprintf(TmpStr, "Modulation: %s", ModulationStr);
	Message(TmpStr);

	BOOL bInSync;
	BOOL bCarrierDetect;
	u16 nSignalStrength;
	u32 nBer;
	src_spectral_inversion_t Invert;
	src_modulation_t Modulation;
	
	// get status
	if (!pATBoardCtrl->Get(bInSync, bCarrierDetect, nSignalStrength, nBer, Invert, Modulation))
		return FALSE;

	// print status
	Message("Status:");
	sprintf(TmpStr, "In sync: %s", bInSync ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Carrier detect: %s",  bCarrierDetect ? "YES" : "NO");
	Message(TmpStr);
	sprintf(TmpStr, "Signal Strength: %ld", nSignalStrength);
	Message(TmpStr);
	sprintf(TmpStr, "Bit error rate: %ld Bit/s", nBer);
	Message(TmpStr);

	return TRUE;
}
