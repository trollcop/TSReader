#ifndef __SECONST_H__
#define __SECONST_H__

/* $Header: $ */

// *********************************************************************************
// *                                                                               *
// * Module Name: SEConst.h                                                        *
// *                                                                               *
// * Description: Typedefs and constants used by the SE control DLL.               *
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



/* 
 * set all global typedefs add the prefix SE in order not to
 * clash wih other typedefs 
 */

#ifdef	_MSC_VER
#pragma	pack(push, 4)
#endif	_MSC_VER


/* global constants */

#ifndef OUT

   #define OUT

#endif // OUT

#ifndef IN

   #define IN

#endif // IN



// interface version information
#define INTERFACE_MAJOR_VERSION     2
#define INTERFACE_MINOR_VERSION     0



/* 
 * Basic Data Types
 */

typedef unsigned long    SE_STATUS,
                         SE_ULONG,
                         *PSE_STATUS,
                         *PSE_ULONG;

typedef double           SE_DOUBLE,
                         *PSE_DOUBLE;

typedef unsigned int     SE_HANDLE,
                         SE_UINT,
                         *PSE_HANDLE,
                         *PSE_UINT;  

typedef unsigned short   SE_USHORT,
                         SE_UINT16,
                         *PSE_USHORT,
                         *PSE_UINT16;

typedef int              SE_INT,
                         *PSE_INT;

typedef unsigned char    SE_UCHAR,
                         SE_BYTE,
                         SE_BOOL,
                         *PSE_UCHAR,
                         *PSE_BYTE,
                         *PSE_BOOL;

typedef char             SE_CHAR,
                         *PSE_CHAR;

typedef void             SE_VOID,
                         *PSE_VOID;

typedef unsigned __int64  SE_ULONGLONG;

/* 
 * Constants
 */

//const int iETHERNET_ADDRESS_SIZE = 6;
#define iETHERNET_ADDRESS_SIZE 6
typedef   SE_UCHAR ETHERNET_ADDRESS[iETHERNET_ADDRESS_SIZE];

const int iSERIAL_NUMBER_LENGTH   = 6;
const int iCA_VERSION_LENGTH      = 6;
const int iCODED_DATE_TIME_LENGTH = 5;
const int iADAPTER_NAME_LENGTH    = 32;
const int iADAPTER_NAME_LENGTH_EX = 128;
const int iCA_ID_LENGTH           = 4;
const int iPAT_MAX_ELEMENT        = 256;
const int iNIT_MAX_ELEMENT        = 128;
const int iPMT_MAX_ELEMENT        = 20;
const int iSDT_MAX_ELEMENT        = 128;
const int iFIXED_KEY_ARRAY_SIZE   = 8;


// packet status for data capture operation
const int iPACKET_CONTINUED       = 0; // normal continuation of a packet
const int iPACKET_DISCONTINUITY   = 1; // discontinuity
const int iPACKET_START           = 2; // start of a packet
const int iPACKET_ERROR           = 4; // packet error, ie. checksum error
const int iPACKET_BUFFER_TOO_SMALL= 8; // packet buffer is too small to fit one section  

/* 
 * Section 2 system Configuration  and Capabilities
 */

typedef struct _ADPTSE_VERSION
{

   SE_BYTE byMajorVersion;
   SE_BYTE byMinorVersion;

} ADPTSE_VERSION, *PADPTSE_VERSION;


typedef struct _BOARD_INFO
{

   SE_USHORT        usMFGID;
   SE_USHORT        usModelID;
   SE_CHAR          ucSerialNumber[iSERIAL_NUMBER_LENGTH];
   SE_USHORT        usHardwareVersion;
   SE_UCHAR         ucFirmwareVersionMajor;
   SE_UCHAR         ucFirmwareVersionMinor;
   SE_ULONG         ulFirmwareCheckSum;
   SE_UCHAR         ucCAVersion[iCA_VERSION_LENGTH];
   SE_USHORT        usCustomerID;
   SE_USHORT        usProductNumber;
   SE_UINT          uiNumStreamFilters;
   SE_UINT          uiNumSectionFilters;
   SE_UINT          uiNumChannels;
   SE_UCHAR         ucBootBlockVersionMajor;
   SE_UCHAR         ucBootBlockVersionMinor;
   SE_CHAR          cAdapterName[iADAPTER_NAME_LENGTH];
   ETHERNET_ADDRESS eaPermanentMACAddress;
   ETHERNET_ADDRESS eaAssignedMACAddress;

} BOARD_INFO, *PBOARD_INFO;


/*
 * Extended board info structure
 */

typedef struct _BOARD_INFO_EX
{
   // PCI-related information
	SE_USHORT	      usVendorID;
	SE_USHORT	      usDeviceID;
	SE_USHORT	      usSubSystemVendorID;
	SE_USHORT	      usSubSystemID;
	SE_UCHAR	         ucPCIRevisionID;

   // Hardware-related information
	SE_USHORT         usMFGID;
	SE_USHORT         usModelID;
	SE_USHORT         usHardwareVersion;
	SE_CHAR  	      cHardwareRevision[2];
	SE_USHORT	      usProductNumber;
	SE_USHORT         usCustomerID;
	ETHERNET_ADDRESS  eaPermanentMACAddress;
	SE_UCHAR          ucCAId[iCA_ID_LENGTH];
   SE_CHAR           cProductName[8]; 
   SE_CHAR           cPartNumber[12];

   // BootBlock-related information
	SE_ULONG	         ulBootBlockChecksum;
	SE_ULONG	         ulBootBlockLength;
	SE_UCHAR	         ucBootBlockVersionMajor;
	SE_UCHAR	         ucBootBlockVersionMinor;
	SE_UCHAR	         ucBootBlockVersionExtension;
   
   // Firmware-related information
	SE_ULONG	         ulFirmwareChecksum;
	SE_ULONG	         ulFirmwareLength;
	SE_UCHAR	         ucFirmwareVersionMajor;
	SE_UCHAR	         ucFirmwareVersionMinor;
	SE_UCHAR	         ucFirmwareVersionExtension;

   // Driver-related information   
	ETHERNET_ADDRESS	eaAssignedMACAddress;
	SE_ULONG	         ulNumChannels;
   SE_ULONG	         ulNumStreamFilters;
   SE_ULONG	         ulNumSectionFilters;
   SE_CHAR           ucSerialNumber[12];

   // Ndvbs-related information
   SE_CHAR           cAdapterName[iADAPTER_NAME_LENGTH_EX];

   // Misc information
	SE_UCHAR	         ucCAVersion[iCA_VERSION_LENGTH];
   SE_UCHAR          ucReserved[1024];

}BOARD_INFO_EX, *PBOARD_INFO_EX;

/*
 * CallBack Functions
 */

typedef void    __cdecl MASTER_STATUS_CLBK(SE_UINT iBoardNumber);

typedef void    __cdecl STREAMCALLBACK(SE_UINT   iBoardNumber,
                                       SE_HANDLE seHandle,
                                       PSE_VOID  pvUserData,
                                       PSE_VOID  pvData,
                                       SE_UINT   uiDataBlockLength,
                                       SE_INT    iPacketStatus);

typedef void    __cdecl CA_MESSAGE_CLBK(SE_UINT  iBoardNumber,
                                        PSE_VOID pvMessage,
                                        SE_INT   iMessageLength);

typedef void    __cdecl ADD_MULTICAST_ADDRESS_CLBK(SE_UINT  iBoardNumber,
                                                   SE_ULONG ulEthernetAddress);

typedef void    __cdecl REMOVE_MULTICAST_ADDRESS_CLBK(SE_UINT  iBoardNumber,
                                                      SE_ULONG ulEthernetAddress);

typedef SE_INT  __cdecl DOWNLOAD_SOFTWARE_CLBK(SE_INT  iBoardNumber,
                                               SE_INT  iPercentComplete,
                                               SE_BOOL bCanCancel);

typedef void    __cdecl TABLE_SECTION_CLBK(SE_UINT iBoardNumber, SE_HANDLE seHandle, PSE_VOID pvTableSection, SE_UINT uiDataBlockLength);
typedef void    __cdecl STREAM_DATA_CLBK(SE_UINT iBoardNumber, SE_HANDLE seHandle, PSE_VOID pvStreamData, SE_UINT uiDataBlockLength);


typedef struct _ADPTSE_APP_CALLBACK
{

   MASTER_STATUS_CLBK*              pfADPTSE_MasterStatusGranted;
   TABLE_SECTION_CLBK*              pfADPTSE_TableSectionArrived;
   STREAM_DATA_CLBK*                pfADPTSE_StreamDataArrived;
   CA_MESSAGE_CLBK*                 pfADPTSE_CAMessageReceived;
   ADD_MULTICAST_ADDRESS_CLBK*      pfADPTSE_AddMulticastAddress;
   REMOVE_MULTICAST_ADDRESS_CLBK*   pfADPTSE_RemoveMulticastAddress;

} ADPTSE_APP_CALLBACK, *PADPTSE_APP_CALLBACK;

// generic message types and callback
typedef enum _ADPTSE_EVENT_ID
{
   ADPTSE_EVENT_TUNER = 0,
   ADPTSE_EVENT_RESET,
   ADPTSE_EVENT_DOWNLOAD,
   ADPTSE_EVENT_SMART_CARD_READER,

}ADPTSE_EVENT_ID;

typedef enum _TUNER_EVENT
{
   
   TUNER_LOST_SIGNAL = 1,
   TUNER_MASTER_RETUNE,
   TUNER_LOCKED_SIGNAL,

}TUNER_EVENT;

typedef enum _RESET_EVENT
{

   RESET_STARTED  = 1,
   RESET_COMPLETED,

}RESET_EVENT;

typedef enum _DOWNLOAD_EVENT
{

   DOWNLOAD_STARTED = 1,
   DOWNLOAD_COMPLETED, 

}DOWNLOAD_EVENT;

typedef enum _SMART_CARD_EVENT
{

	SMART_CARD_IN = 1,
	SMART_CARD_OUT,
	SMART_CARD_RESETED,

}SMART_CARD_EVENT;

typedef void    __cdecl EVENT_NOTIFICATION_CLBK(SE_UINT iBoardNumber, 
                                                ADPTSE_EVENT_ID seEventID, 
                                                PSE_VOID pvEventData, 
                                                SE_INT iEventDataLength);


// requested capture type
typedef enum _ADPTSE_CAPTURE_TYPE
{

   SE_PES_STREAM = 0,         // PES stream
   TRANSPORT_STREAM,       // transport stream

} ADPTSE_CAPTURE_TYPE;


// setup structure for PES and TS capture operations
typedef struct _ADPTSE_STREAM_SETUP
{

   ADPTSE_CAPTURE_TYPE  captureType;   // IN  - type of capture operation
   SE_UINT              uiPID;         // IN  - PID to capture
   SE_UINT              uiBuffers;     // IN  - The number of buffers to allocate
   SE_ULONG             ulBufferSize;  // IN  - The size of each buffer
   PSE_VOID             pvUserData;    // IN  - User Data
   STREAMCALLBACK*      pfCallback;    // IN  - Destination callback

} ADPTSE_STREAM_SETUP, *PADPTSE_STREAM_SETUP;

/* 
 * Section 3 tuner and front end 
 */

typedef enum _POLARIZATION
{

   POLARIZATION_VERTICAL = 0,
   POLARIZATION_HORIZONTAL,

} POLARIZATION;


typedef struct _TUNER_CAPABILITIES
{

   SE_ULONG	ulTuningMin;
   SE_ULONG	ulTuningMax;
   SE_ULONG	ulTuningIncrement;

} TUNER_CAPABILITIES, *PTUNER_CAPABILITIES;


typedef struct _LB_TUNER_SETTINGS
{

   SE_ULONG     ulSatFrequency_Khz;    // transponder frequency in kilohertz
   SE_ULONG     ulTunerFrequency_Khz;  // tuner frequency in kilohertz
                                       // (950000 - 2150000)
   POLARIZATION polarization;          // POLARIZATION enum
   SE_ULONG     ulSymbolRate_KSym;     // thousands of symbols per second
   SE_ULONG     ul22KhzSwitch;         // 0 - Low LNB, 1 - High LNB (22Khz on)

} LB_TUNER_SETTINGS, *PLB_TUNER_SETTINGS;


typedef struct _SAT_TUNER_SETTINGS
{

   SE_ULONG     ulTunerFrequency_Khz;  // frequency in kilohertz
                                       // (950000 to 2150000 + LNB)
   POLARIZATION polarization;          // POLARIZATION enum
   SE_ULONG     ulSymbolRate_KSym;     // thousands of symbols per second

} SAT_TUNER_SETTINGS, *PSAT_TUNER_SETTINGS;


typedef enum _FEC_RATE
{

   FEC_RATE_1_2  = 1,
   FEC_RATE_2_3  = 2,
   FEC_RATE_3_4  = 3,
   FEC_RATE_5_6  = 4,
   FEC_RATE_7_8  = 5,
   FEC_RATE_NONE = 0x100

} FEC_RATE;


typedef enum _DEMOD_STATUS
{

   DEMOD_LOCKED   = 1,
   DEMOD_UNLOCKED = 2,
   DEMOD_UNKNOWN  = 3,

} DEMOD_STATUS;


typedef enum _VITERBI_STATUS
{

   VITERBI_LOCKED   = 1,
   VITERBI_UNLOCKED = 2,
   VITERBI_UNKNOWN  = 3,

} VITERBI_STATUS;


typedef enum _TUNER_STATUS
{

   TUNER_LOCKED   = 1,
   TUNER_UNLOCKED = 2,
   TUNER_UNKNOWN  = 3,

} TUNER_STATUS;


typedef struct _FE_LOCK_STATUS
{

   SE_BYTE   byLockStatusBits;   // Lock information: 1 = locked, 0 = not locked.
                                 // (bit 0 is the least significant bit)
                                 // Bit0: Viterbi Decoder Sync
                                 // Bit1: Deinterleaver/Reed Solomon Decoder Sync
                                 // Bit2: Descrambler Sync
                                 // Bit3: Clock Frequency Lock Flag
                                 // Bit4: Carrier Phase Lock Flag
   SE_BYTE   byViterbiRate;      // Rate Values
                                 // 0 = 1/2
                                 // 1 = 2/3
                                 // 2 = 3/4
                                 // 3 = 5/6
                                 // 4 = 6/7
                                 // 5 = 7/8
   SE_DOUBLE dLNBOffset;         // Carrier (LNB) offset in MHz
                                 // Floating point value with 2 significant decimal
                                 // places. AFC range is + or - 5.00 MHz

} FE_LOCK_STATUS, *PFE_LOCK_STATUS;


typedef struct _FE_ERROR_STATUS
{

   SE_DOUBLE dBitErrRate;        // Bit Error Rate (BER)
   SE_DOUBLE dEbNo;              // signal to noise ratio
   SE_UINT16 usRSCError;         // Reed Solomon Corrected Error
   SE_UINT16 usRSUCError;        // Reed Solomon UnCorrected Error

} FE_ERROR_STATUS, *PFE_ERROR_STATUS;


typedef struct _SIGNAL_STRENGTH
{

   SE_BYTE byPowerLevel;         // 0 - 255 decimal

} SIGNAL_STRENGTH, *PSIGNAL_STRENGTH;


typedef struct _VITERBI_BER
{

   SE_DOUBLE dViterbiBER;   

} VITERBI_BER, *PVITERBI_BER;


typedef enum _PSISTREAM_TYPE
{

   strmUnknown = -1,
   strmReserved,
   strm11172Video,
   strm13818Video,
   strm11172Audio,
   strm13818Audio,
   strmPrivateSection,
   strmPESWithPrivateData,
   strm13522MHEG,
   strmDSMCC,
   strmAuxiliary

} PSISTREAM_TYPE;


typedef struct _PID_LIST
{

   SE_ULONG       ulPID;      // PID number
   PSISTREAM_TYPE PIDType;    // PID type

} PID_LIST, *PPID_LIST;



/* 
 * Section 4 SI/PSI/private tables 
 */

typedef struct _TABLE_CAPABILITY
{

   SE_UINT uiNumFilters;
   SE_UINT uiFilterSize;

} TABLE_CAPABILITY, *PTABLE_CAPABILITY;


typedef struct _FILTER_AVAILABLE
{

   SE_UINT uiNumFiltersAvailable;

} FILTER_AVAILABLE, *PFILTER_AVAILABLE;


typedef struct _TDT_TABLE
{

   SE_UCHAR ucCodedDateTime[iCODED_DATE_TIME_LENGTH];

} TDT_TABLE, *PTDT_TABLE;


typedef struct _TABLE_SETTING
{

   SE_UINT   uiFilterSize;
   SE_BYTE*  pbyFilterBits;
   SE_BYTE*  pbyDontCareBits;
   SE_USHORT usVersionFlag;
   SE_USHORT usPIDNumber;

} TABLE_SETTING, *PTABLE_SETTING;


typedef struct _ADPTSE_TABLE_SECTION_SETUP
{

   PTABLE_SETTING    pTableSetting; // IN  - table setting data
   SE_UINT           uiBuffers;     // IN  - The number of buffers to allocate
   SE_ULONG          ulBufferSize;  // IN  - The size of each buffer
   PSE_VOID          pvUserData;    // IN  - User Data
   STREAMCALLBACK*   pfCallback;    // IN  - Destination callback

} ADPTSE_TABLE_SECTION_SETUP, *PADPTSE_TABLE_SECTION_SETUP;


typedef struct _PAT_TABLE
{

   SE_USHORT usTransportStreamID;
   SE_UCHAR  ucVersion;
   SE_USHORT usNetITPID;                        // NIT PID
   SE_USHORT usNumberOfElements;                // number of programs
   SE_USHORT usProgramNumber[iPAT_MAX_ELEMENT]; // array of programs.

} PAT_TABLE , *PPAT_TABLE;


typedef struct _NIT_ARRAY_ELEMENT
{

   SE_USHORT usTransportStreamID;
   SE_USHORT usOriginalNetworkID;
   SE_ULONG  ulFrequency;
   SE_ULONG  ulSymbolRate;
   SE_ULONG  ulPolarization;
   SE_ULONG  ulOrbitalPosition;

} NIT_ARRAY_ELEMENT, *PNIT_ARRAY_ELEMENT;


typedef struct _NIT_TABLE
{

   SE_USHORT         usNetworkID;
   SE_UCHAR          ucVersion;
   SE_CHAR           szNetworkName[129];
   SE_USHORT         usNumberOfElements;
   NIT_ARRAY_ELEMENT naeNITTableElement[iNIT_MAX_ELEMENT];

} NIT_TABLE, *PNIT_TABLE ;


typedef struct _PMT_ARRAY_ELEMENT
{

   SE_UCHAR  ucStreamType;    // stream type
   SE_USHORT usElementaryPID; // PID to tune
   SE_UCHAR	 ucStreamID;      // array index

} PMT_ARRAY_ELEMENT, *PPMT_ARRAY_ELEMENT;


typedef struct _PMT_TABLE
{

   SE_USHORT usProgramNumber; // program number (for each PAT element)
   SE_UCHAR  ucVersion;
   SE_USHORT usPcrPid;
   SE_USHORT usNumberOfElements;
   PMT_ARRAY_ELEMENT paePMTTableElements[iPMT_MAX_ELEMENT];

} PMT_TABLE , *PPMT_TABLE;


// service type definitions
typedef enum _SDT_SERVICE_TYPE
{

   stReserved = 0,
   stDigitalTelevisionService,
   stDigitalRadioSoundService,
   stTeletextService,
   stNVODRefernceService,        // NVOD - Near Video On Demand 
   stNVODTimeShiftedService, 
   stMosaicService,
   stPALCodedSignal,
   stSECAMCodedSignal,
   stD_D2_MAC,
   stFMRadio,
   stNTSCCodedSignal

} SDT_SERVICE_TYPE;


// service type definitions
typedef enum _SDT_RUNNING_STATUS
{

   statUndefined = 0,
   statNotRunning,
   statStartsInFewSeconds,
   statPause,
   statRunning

} SDT_RUNNING_STATUS;


typedef struct _SE_SDT_ARRAY_ELEMENT 
{

   SE_USHORT          usProgramNumber;
   SE_UCHAR           usEITFlags;
   SDT_RUNNING_STATUS sdtRunningStatus; 
   SE_BOOL            bEncrypted;

   SDT_SERVICE_TYPE   sdtServiceType;
   SE_CHAR            cServiceProviderName[128];
   SE_CHAR            cServiceName[128];

} SDT_ARRAY_ELEMENT;


typedef struct _SDT_TABLE
{

   SE_USHORT         usTransportStreamId; 
	SE_USHORT         usOriginalNetworkId;  
	SE_UCHAR          ucVersion;
	SE_USHORT         usNumberOfElements;
	SDT_ARRAY_ELEMENT saeSDTTableElements[iSDT_MAX_ELEMENT];  

} SDT_TABLE , *PSDT_TABLE;


/*
 * Section 5 Streaming Interface
 */

//
// NOTE: THIS SECTION IS CURRENTLY UNDER DEVELOPMENT.
//
typedef struct _STREAM_CAPABILITY
{

   SE_UINT uiNumAudio;
   SE_UINT uiNumVideo;
   SE_UINT uiNumData;
   SE_UINT uiNumAny;
   
} STREAM_CAPABILITY, *PSTREAM_CAPABILITY;


typedef struct _TRANSPORT_CAPTURE
{

   // members
   
} TRANSPORT_CAPTURE, *PTRANSPORT_CAPTURE;


typedef struct _PES_CAPTURE
{

   // members
   
} PES_CAPTURE, *PPES_CAPTURE;

//
// **************************************************
//



/* 
 * Section 6 MPE Data Interface
 */

typedef struct _MULTICAST_ENTRY
{

   ETHERNET_ADDRESS eaEthernetAddress;
   SE_BOOL          bEnabled;

} MULTICAST_ENTRY, *PMULTICAST_ENTRY;



/* 
 * Section 7 Conditonal Access Communication
 */

typedef enum _FIXED_KEY_CAS_TYPE
{

   CAS_TYPE_EVEN = 0,
   CAS_TYPE_ODD,

} FIXED_KEY_CAS_TYPE;


typedef struct _FIXED_KEY_CAS_INFO
{

   SE_USHORT          usPid;    // Pid
   SE_UCHAR           ucKeyValue[iFIXED_KEY_ARRAY_SIZE]; // Key Value
   FIXED_KEY_CAS_TYPE KeyType;  // EVEN, ODD

} FIXED_KEY_CAS_INFO, *PFIXED_KEY_CAS_INFO;



/* 
 * Section 8 Callback functions
 */
// Developer Implements Callback in their Application



/* 
 * Section 9 Diagnostics and privilaged operations.
 */

typedef struct _NDIS_STATISTICS_INFO
{
	SE_ULONG			ulNdisStatisticsGenXmitOk;							// Frames transmitted without errors.
	SE_ULONG			ulNdisStatisticsGenRcvOk;							// Frames received without errors.
	SE_ULONG			ulNdisStatisticsGenXmitError;						// Frames not transmitted or transmitted with errors
	SE_ULONG			ulNdisStatisticsGenRcvError;						// Frames received with errors
	SE_ULONG			ulNdisStatisticsGenRvcNoBuffer;					// Frame missed, no buffers
	SE_ULONGLONG	ullNdisStatisticsGenDirectedBytesXmit;			// Directed bytes transmitted without errors
	SE_ULONG			ulNdisStatisticsGenDirectedFramesXmit;			// Directed frames transmitted without errors
	SE_ULONGLONG	ullNdisStatisticsGenMulticastBytesXmit;		// Multicast bytes transmitted without errors
	SE_ULONG			ulNdisStatisticsGenMulticastFramesXmit;		// Multicast frames transmitted without errors
	SE_ULONGLONG	ullNdisStatisticsGenBroadcastBytesXmit;		// Broadcast bytes transmitted without errors
	SE_ULONG			ulNdisStatisticsGenBroadcastFramesXmit;		// Broadcast frames transmitted without errors
	SE_ULONGLONG	ullNdisStatisticsGenDirectedBytesRcv;			// Directed bytes received without errors
	SE_ULONG			ulNdisStatisticsGenDirectedFramesRcv;			// Directed frames received without errors
	SE_ULONGLONG	ullNdisStatisticsGenMulticastBytesRcv;			// Multicast bytes received without errors
	SE_ULONG			ulNdisStatisticsGenMulticastFramesRcv;			// Multicast frames received without errors
	SE_ULONGLONG	ullNdisStatisticsGenBroadcastBytesRcv;			// Broadcast bytes received without errors
	SE_ULONG			ulNdisStatisticsGenBroadcastFramesRcv;			// Broadcast frames received without errors
	SE_ULONG			ulNdisStatisticsGenRcvCRCError;					// Frames received with circular redundancy check (CRC) or frame check sequence (FCS) error
	SE_ULONG			ulNdisStatistics802_3RcvErrorAlignment;		// Frames received with alignment error
	SE_ULONG			ulNdisStatistics802_3XmitOneCollision;			// Frames transmitted with one collision
	SE_ULONG			ulNdisStatistics802_3XmitMoreCollisions;		// Frames transmitted with more than one collision
	SE_ULONG			ulNdisStatistics802_3XmitDeferred;				// Frames transmitted after deferral
	SE_ULONG			ulNdisStatistics802_3XmitMaxCollisions;		// Frames not transmitted due to collisions
	SE_ULONG			ulNdisStatistics802_3RcvOverrun;					// Frames not received due to overrun
	SE_ULONG			ulNdisStatistics802_3XmitUnderrun;				// Frames not transmitted due to underrun
	SE_ULONG			ulNdisStatistics802_3XmitHeartbeatFailure;	// Frames transmitted with heartbeat failure
	SE_ULONG			ulNdisStatistics802_3XmitTimesCRSLost;			// Times carrier sense signal lost during transmission
	SE_ULONG			ulNdisStatistics802_3XmitLateCollisions;		// Late collisions detected
	SE_ULONGLONG	ullNdisStatisticsReserved1;						// Reserved for future use
	SE_ULONG			ulNdisStatisticsReserved2;							// Reserved for future use
	SE_ULONGLONG	ullNdisStatisticsReserved3;						// Reserved for future use
	SE_ULONG			ulNdisStatisticsReserved4;							// Reserved for future use

} NDIS_STATISTICS_INFO, *PNDIS_STATISTICS_INFO;

/*
 * Defines for ADPTSE_BoardIoControl
 */

// Get all the suported CA.
#define BL_GET_CA_SUPPORTED					0x01000001

#define BL_CA_FIXED_KEY							0x00000001
#define BL_CA_NDS									0x00000002
#define BL_CA_NAGRA								0x00000004
#define BL_CA_IRDETO								0x00000008

/*
 * // Define the Codes based on following
 * #define BOARD_CTL_CODE( Module, SubModule, Function) (  \
 *   ((Module) << 24) | ((SubModule) << 16) | (Function) \
 * )
 *
 *	// One byte module
 * #define SE_FIRMWARE_MODULE		0x01
 * #define SE_MINIPORT_MODULE		0x02
 * #define SE_NDVBS_MODULE			0x04
 * #define SE_LIBRARY_MODULE		0x08

 * // One byte Submodules
 * #define SE_FIRMWARE_GENERIC_SUBMODULE	0x00
 * #define SE_FIRMWARE_CA_SUBMODULE			0x01
 * #define SE_FIRMWARE_TUNER_SUBMODULE		0x02
 * #define SE_MINIPORT_GENERIC_SUBMODULE	0x00
 * #define SE_NDVBS_GENERIC_SUBMODULE		0x00
 * #define SE_LIBRARY_GENERIC_SUBMODULE	0x00
 *
 *	// Two bytes of function code.
 * Use values greater that 0x1000.
 *
 * eg. 0x01011001	- BOARD_CTL_CODE(SE_FIRMWARE_MODULE, SE_FIRMWARE_CA_SUBMODULE, 0x1001)
 * Jose : 05/15/2000
 *
 */

// Irdeto Smart Card Interface defines.
#define BL_OPEN_SMART_CARD_READER			0x01011001 
#define BL_CLOSE_SMART_CARD_READER			0x01011002
#define BL_RESET_SMART_CARD_READER			0x01011003
#define BL_QUERY_SMART_CARD_STATUS			0x01011004
#define BL_WRITE_SMART_CARD_DATA				0x01011005
#define BL_READ_SMART_CARD_DATA				0x01011006

typedef enum _SE_SMART_CARD_STATUS
{
	SE_SMART_CARD_STATUS_OK	= 0,
	SE_SMART_CARD_STATUS_CARD_REMOVED,
	SE_SMART_CARD_STATUS_MUTE,
	SE_SMART_CARD_STATUS_TIMEOUT,
	SE_SMART_CARD_STATUS_ERROR

}SE_SMART_CARD_STATUS;

#define SE_MAX_ATR_LENGTH 33

// Irdeto Smart Card Interface structures
typedef struct _SE_SMART_CARD_RESET_RESPONSE
{
	SE_SMART_CARD_STATUS	eStatus;
	SE_UCHAR					ucATR[SE_MAX_ATR_LENGTH];

}SE_SMART_CARD_RESET_RESPONSE, *PSE_SMART_CARD_RESET_RESPONSE;

typedef struct _SE_SMART_CARD_READ_RESPONSE
{
	SE_SMART_CARD_STATUS	eStatus;	
	SE_UCHAR					ucBuffer[1];		// Read data more bytes

}SE_SMART_CARD_READ_RESPONSE, *PSE_SMART_CARD_READ_RESPONSE;

// NAGRA Conditional Access defines.
#define BL_SEND_RAW_PMT							0x01012001
#define BL_UPDATE_RAW_PMT						0x01012002
#define BL_REMOVE_RAW_PMT						0x01012003
#define BL_SEND_RAW_CAT							0x01012004

/*
 * NAGRA Conditional Access structures
 */

// BL_SEND_RAW_PMT & BL_UPDATE_RAW_PMT
typedef struct _SE_RAW_PMT_INFO
{
	
	SE_USHORT usPid;					//Pid Number
	SE_USHORT usLength;				//Length of the data follow
	SE_UCHAR	 ucRawPMT[1];			//Raw Data.

}SE_RAW_PMT_INFO, *PSE_RAW_PMT_INFO;

#ifdef	_MSC_VER
#pragma	pack(pop)
#endif	_MSC_VER

#endif // __SECONST_H__
