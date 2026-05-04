#ifndef __SEPROT_H__
#define __SEPROT_H__

/* $Header: $ */

// *********************************************************************************
// *                                                                               *
// * Module Name: SEProt.h                                                         *
// *                                                                               *
// * Description: Prototypes for functions implemented in the SE control DLL.      *
// *                                                                               *
// *                                                                               *
// *                                                                               *
// * Author     : George Kostas Grous                                              *
// *                                                                               *
// * Date       : 02/18/98                                                         *
// *                                                                               *
// * Notes      :                                                                  *
// *                                                                               *
// * Date       : Comment                                                          *
// * -----------  ---------------------------------------------------------------- *
// * 00/00/98                                                                      *
// *                                                                               *
// *                                                                               *
// *                                                                               *
// *********************************************************************************
// *                                                                               *
// * Copyright 1999 - 2000 BroadLogic, Inc.,  All Rights Reserved                  *
// *                                                                               *
// * This software contains the valuable trade secrets of BroadLogic.  The         *
// * software is protected under copyright laws as an unpublished work of          *
// * BroadLogic.  Notice is for informational purposes only and does not imply     *
// * publication.  The user of this software may make copies of the software       *
// * for use with parts manufactured by BroadLogic or under license from BroadLogic*
// * and for no other use.                                                         *
// *                                                                               *
/* *********************************************************************************
 *
 * $Revision: $
 * $Date: $
 * $Author: $
 * $History: $
 * 
 * $NoKeywords: $
 *
 **********************************************************************************/



// include files
#include "SEConst.h"



// definitions
#define EXPORTED_FUNCTION __declspec(dllexport)

#ifdef __cplusplus

   #define EXTERN_DEF	extern

#else

   #define EXTERN_DEF	extern

#endif



#ifdef __cplusplus
extern "C"

{

#endif

/**************************************************************************

	Section 2 - System Configuration and Capabilities.

***************************************************************************/

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryInterfaceVersion(PADPTSE_VERSION pADPTSEVersion);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryNumBoards(PSE_INT piNumBoards);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl  ADPTSE_QueryBoardInformation(SE_INT iBoardNumber, \
                                                PBOARD_INFO pBoardInfo);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl  ADPTSE_QueryBoardInformationEx(SE_INT iBoardNumber, \
                                                  PBOARD_INFO_EX pBoardInfoEx);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_RegisterApp(SE_INT iBoardNumber, \
                                     PADPTSE_APP_CALLBACK pAppCallbackFunction, \
                                     SE_BOOL bRequestMaster);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_SetEventNotificationCallback(IN SE_INT iBoardNumber, \
                                                      EVENT_NOTIFICATION_CLBK *pEventCallabck);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_UnRegisterApp(SE_INT iBoardNumber);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_ResetBoard(SE_INT iBoardNumber);

/**************************************************************************

	Section 3 - Tuner and Front End.

***************************************************************************/

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryTunerCapabilities(SE_INT iBoardNumber, \
                                                PTUNER_CAPABILITIES pTunerCapabilities);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_SetTunerLBand(SE_INT iBoardNumber, \
                                       PLB_TUNER_SETTINGS pLBTunerSettings);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryLNBFrequency(SE_INT iBoardNumber, \
                                           PSE_ULONG pulLNBFrequencyLow, \
                                           PSE_ULONG pulLNBFrequencyHigh);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_SetLNBFrequency(SE_INT iBoardNumber, \
                                         SE_ULONG ulLNBFrequencyLow, \
                                         SE_ULONG ulLNBFrequencyHigh);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_SetTunerWithLNB(SE_INT iBoardNumber, \
                                         PSAT_TUNER_SETTINGS pSatTunerSettings);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_TuneToTransportStream(SE_INT iBoardNumber, \
                                               SE_USHORT usTransportStreamID, \
                                               SE_USHORT usOriginalNetworkID);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_TuneToChannel(IN SE_INT iBoardNumber, \
                                       IN SE_ULONG ulChannel);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryChannelSettings(IN SE_INT iBoardNumber, \
                                              IN PSE_ULONG pulChannel);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryTunerLBandSettings(SE_INT iBoardNumber, \
                                                 PLB_TUNER_SETTINGS pLBTunerSettings);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryFELockStatus(SE_INT iBoardNumber, \
                                           PFE_LOCK_STATUS pFELockStatus);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryFEErrorStatus(SE_INT iBoardNumber, \
                                            PFE_ERROR_STATUS pFEErrorStatus);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_ClearErrorCount(IN SE_INT iBoardNumber);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryViterbiBitErrorRate(IN SE_INT iBoardNumber, \
                                                  OUT PVITERBI_BER pViterbiBER);

EXPORTED_FUNCTION    \
SE_STATUS __cdecl ADPTSE_QueryTunerSignalStrength(SE_INT iBoardNumber, \
                                                  PSIGNAL_STRENGTH pSignalStrength);


EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryPIDList(SE_INT iBoardNumber, \
                                      PPID_LIST pPIDList, \
                                      SE_ULONG  ulPIDListLength, \
                                      PSE_ULONG pulPIDListLength);

/**************************************************************************

	Section 4 - SI / PSI / Private Tables

***************************************************************************/

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryTableCapability(SE_INT iBoardNumber, \
                                              PTABLE_CAPABILITY pTableCapability);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryAvailableFilters(SE_INT iBoardNumber, \
                                               PFILTER_AVAILABLE pFilterAvailable);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_CaptureTableSection(SE_INT iBoardNumber, \
                                             PADPTSE_TABLE_SECTION_SETUP pTableSettingSetup, \
                                             PSE_HANDLE pseHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_ChangeCaptureTableSection(SE_INT iBoardNumber, \
                                                   SE_HANDLE seHandle, \
                                                   PTABLE_SETTING pTableSetting);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_StopCaptureTableSection(SE_INT iBoardNumber, \
                                                 SE_HANDLE seHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_GetDateTime(SE_INT iBoardNumber, \
                                     LPSYSTEMTIME pSystemTime);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryTimeDateTable(SE_INT board_number, \
                                            PTDT_TABLE pTDTTable);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryProgramAssociationTable(SE_INT iBoardNumber, \
                                                      PPAT_TABLE pPATTable);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryNetworkInformationTable(SE_INT iBoardNumber, \
                                                      PNIT_TABLE pNITTable);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryProgramMapTable(SE_INT iBoardNumber, \
                                              SE_USHORT usProgramNumber, \
                                              PPMT_TABLE pPMTTable);
EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryServiceDescriptorTable(SE_INT iBoardNumber, \
                                                     SE_USHORT usTransportStreamID, \
                                                     PSDT_TABLE pSDTTable);

/**************************************************************************

	Section 5 - Streaming Interface

***************************************************************************/

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryStreamCapability(SE_INT iBoardNumber, \
                                               PSTREAM_CAPABILITY pStreamCapability);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_OpenStream(SE_INT iBoardNumber, \
                                    PADPTSE_STREAM_SETUP pStreamSetup,
                                    PSE_HANDLE pseHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_CloseStream(SE_INT iBoardNumber, \
                                     SE_HANDLE seHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_StartCapture(SE_INT iBoardNumber, \
                                      SE_HANDLE seHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_StopCapture(SE_INT iBoardNumber, \
                                     SE_HANDLE seHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_FreeBuffer(SE_INT iBoardNumber, \
                                    SE_HANDLE seHandle, \
                                    PSE_VOID pStreamBuffer);

/**************************************************************************

	Section 6 - MPE Data Interface

***************************************************************************/

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_CaptureMPEStream(SE_INT iBoardNumber, \
                                          SE_USHORT usPID, \
                                          PSE_HANDLE pseHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_StopCaptureMPEStream(SE_INT iBoardNumber, \
                                              SE_HANDLE seHandle);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryMulticastAddressList(SE_INT iBoardNumber, \
                                                   PMULTICAST_ENTRY pMulticastEntryList, \
                                                   SE_ULONG ulMulticastEntryListLength, \
                                                   SE_ULONG* pulMulticastEntryListLength);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_AssignMACAddress(SE_INT iBoardNumber, \
                                          ETHERNET_ADDRESS* peaEthernetAddress);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_SetDefaultMulticastAction(SE_INT iBoardNumber, \
                                                   SE_BOOL bEnable);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_EnableMulticastAddress(SE_INT iBoardNumber, \
                                                SE_ULONG ulEthernetAddress, \
                                                SE_BOOL bEnable);

/**************************************************************************

	Section 7 - Control Access Communication

***************************************************************************/

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_SendRemoteCAMessage(SE_INT iBoardNumber, \
                                             PVOID pvCAMessage, \
                                             SE_INT iMessageLength);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_SetFixedKeyCAS(SE_INT iBoardNumber, \
                                        PFIXED_KEY_CAS_INFO pFixedKeyCAS);

/**************************************************************************

	Section 9 - Diagnostic and Privileged Operations

***************************************************************************/

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_QueryNdisStatistics(IN SE_INT iBoardNumber, 
												 OUT PNDIS_STATISTICS_INFO pNdisStatisticsInfo);
 
EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_Test(SE_INT iBoardNumber, \
                              SE_UINT iTestNumber, \
                              PVOID inputBuffer, \
                              SE_INT inputLength, \
                              PVOID outputBuffer, \
                              SE_INT outputLength);
EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_BoardIoControl(IN SE_INT iBoardNumber,
	 	 								  IN SE_ULONG  ulIoControlCode,
										  IN PSE_VOID  pvInBuffer,
										  IN SE_ULONG  ulInBufferSize,
										  IN PSE_VOID  pvOutBuffer,
										  IN SE_ULONG  ulOutBufferSize,
										  IN PSE_ULONG pulBytesReturned,
										  IN SE_INT	   iTimeOut);

EXPORTED_FUNCTION		\
SE_STATUS __cdecl ADPTSE_DownloadSoftware(SE_INT iBoardNumber, \
                                          LPCSTR pstrFirmwareCodeImageFile, \
                                          DOWNLOAD_SOFTWARE_CLBK* pfADPTSE_DownloadSoftwareCallback);


/**************************************************************************

	STB MPEG Decoder

***************************************************************************/

EXPORTED_FUNCTION
SE_STATUS __cdecl ADPTSE_SetupSTB(SE_INT iBoardNumber, \
                                  void* ptr, \
                                  SE_INT size);

#ifdef __cplusplus

}

#endif



#endif // __SEPROT_H__
