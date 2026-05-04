// sources.h
// Copyright (C) 2004 COOLSTF.com Inc.
//
// Include file for developing TSReader sources
//
#include <windows.h>

// Size of transport stream buffers - do NOT change
// 256 elements in the ring buffer
// up to 698 x 188 per buffer. We chose this as it's close to 128KB.
// So 256 x 698 x 188 = 33593344 or about 32MB for the TS buffers
#define MAX_TS_BUFFERS 256
#define TS_PACKETS_AT_A_TIME 698
#define TS_BUFFER_SIZE 188 * TS_PACKETS_AT_A_TIME

// Satellite modulation modes
#define ADV_MOD_DVB_QPSK 0						// DVB-S QPSK
#define ADV_MOD_TURBO_QPSK 1					// Turbo QPSK
#define ADV_MOD_TURBO_8PSK 2					// Turbo 8PSK (also used for Trellis 8PSK DVB-SNG)
#define ADV_MOD_TURBO_16QAM 3					// Turbo 16QAM (also used for Trellis 8PSK DVB-SNG)
#define ADV_MOD_DCII_C_QPSK 4					// Digicipher II Combo
#define ADV_MOD_DCII_I_QPSK 5					// Digicipher II I-stream
#define ADV_MOD_DCII_Q_QPSK 6					// Digicipher II Q-stream
#define ADV_MOD_DCII_C_OQPSK 7					// Digicipher II offset QPSK
#define ADV_MOD_DSS_QPSK 8						// DSS (DIRECTV) QPSK
#define ADV_MOD_DVB_BPSK 9						// DVB-S BPSK

// Source capabilities
#define CAPABILITIES_DISEQC 0x1					// source can send DiSEqC commands
#define CAPABILITIES_TONEBURST 0x2				// source can send toneburst switch commands
#define CAPABILITIES_POWER 0x4					// source provides power (satellite type)
#define CAPABILITIES_SERIAL_CONTROL 0x8			// source can use TSReader's serial control
#define CAPABILITIES_UNIPROCESSOR 0x10			// source not MP safe so is run with only 1 processor
#define CAPABILITIES_DISEQC_POSITIONER 0x20		// source can send DiSEqC positioner commands
#define CAPABILITIES_DISH_SWITCH 0x40			// source supports Dish Network switch commands
#define CAPABILITIES_ADV_SATELLITE 0x80			// source handles advanced modulation (DVB-S + DCII and Turbo modes)
#define CAPABILITIES_TIMESTAMP 0x100			// source can timestamp packets

typedef struct tag_TSBuffers
{
	int nSize;					// size of data in the buffer. Always < TS_BUFFER_SIZE. Filled by the source
	BYTE * pData;				// pointer to buffer of size TS_BUFFER_SIZE. TS data is put there by the source
	DWORD * pTimestamps;		// pointer to 4 x TS_PACKETS_AT_A_TIME for timestamps. Filled by the source.
} TSBUFFERS, *PTSBUFFERS;

typedef struct _tagSourceStruct
{
	// The following group need to be setup after a tune - TSReader saves these
	// in the registry for you
	BOOL fSpectrumInversion;	// spectral inversion status
	int nFrequency;				// frequency to tune (ATSC/QAM/Satellite in MHz, DVB-T and DVB-C in KHz)
	int nPolarity;				// polarity for satellite: 0 = Vertical/RHCP 1 = Horizontal/LHCP -1 = no power
	int nSymbolRate;			// symbol rate
	int nLNBFrequency;			// LNB frequency
	int n22KHz;					// 22 KHz tone status
	int nDiSEqCInput;			// Switch input 0 = none, 1-4 = DiSEqC, 5 = TBA, 6 = TBB 7-20 = Dish switch (see below)
	int nBandwidth;				// Signal bandwidth 0 = 6MHz, 1 = 7MHz, 2 = 8 MHz
	int nADVModulationMode;		// Satellite modulation mode (see above)
	int nCodeRate;				// Code rate index
	int nQAM;					// QAM modulation mode 0 = QAM-16, 1 = QAM-32, 2 = QAM-64, 3 = QAM-128, 4 = QAM-256
	int nUDPMulticastPort;		// UDP source only: multicast port
	int nSourceIndex;			// Source device offset for multiple board support
	char szUDPMulticastAddress[MAX_PATH];	// UDP source only: multicast address
	char szUDPMulticastInterface[MAX_PATH];	// UDP source only: interface address

	// These are used for data flow from the source into TSReader
	int nTSBuffersInUse;		// Current buffers in use - bump by 1 each time a buffer is added
	int nLastSecondByteCounter;	// bump as data is received
	int nInputThreadPriority;	// priority input thread should run at 0 = THREAD_PRIORITY_NORMAL, 1 = THREAD_PRIORITY_HIGHEST, 2 = THREAD_PRIORITY_IDLE
	CRITICAL_SECTION csTSBuffersInUse;	// critical section to protect nTSBuffersInUse
	CRITICAL_SECTION csPIDCounter; // critical section to protect nLastSecondByteCounter
	TSBUFFERS tsb[MAX_TS_BUFFERS]; // transport stream buffer structs

	// Misc. flags between TSReader and the source
	BOOL fDontTune;				// flag set by the registry to tell the source not to tune - debug use mostly
	BOOL fReadThreadTerminated;	// set TRUE when the read thread is done
	BOOL fTerminateReadThread;	// flag from TSReader to terminate the read thread
	BOOL fSerialReceiverControlEnabled;	// serial control is enabled - you should use the sourcehelper functions
	BOOL fQuietMode;			// quiet mode - don't output "can't tune" messages
	BOOL fIgnoreContinuity;		// tells TSReader to ignore continuity errors in the stream
	BOOL fTimestampPackets;		// set if TSReader wants timestamps sent back with packets. Source needs CAPABILITIES_TIMESTAMP
	int nPATCATProcessed;		// sets to 3 once the PAT and CAT have been parsed by TSReader
								// this is useful to feed small buffers to TSReader when it starts
								// running with a demux interface.
	HANDLE hReadDataThread;		// handle of the thread sending data to TSReader (set by source)
	HWND hWndTSReader;			// TSReader's main window - use as the parent window for you windows/messages
	HINSTANCE hTSReaderInst;	// TSReader's resource handle

	// These are used by the file source only
	BOOL fLastFileTS;			// flags if the last file as .ts or .mpg
	char szDropFilename[MAX_PATH];	// filenames dropped onto the TSReader main window
	char szTransportStreamInitialDir[MAX_PATH];	// initial directory for transport stream files

} SOURCESTRUCT, *PSOURCESTRUCT;

// Sources with CAPABILITIES_DISH_SWITCH support Dish Network legacy switches with the following values:
//  7 - SW21 Dish 1
//  8 - SW21 Dish 2
//  9 - SW42 Dish 1
// 10 - SW42 Dish 2
// 11 - SW44 Dish 2
// 12 - SW64 Dish 1A
// 13 - SW64 Dish 1B
// 14 - SW64 Dish 2A
// 15 - SW64 Dish 2B
// 16 - SW64 Dish 3A
// 17 - SW64 Dish 3B
// 18 - Twin LNB 1
// 19 - Twin LNB 2
// 20 - Quad LNB 2

// Routines exported from TSReader_SourceHelper.dll
#ifdef  __cplusplus
extern "C" {
#endif

// Sets priorities of the threads used by TSReader
void __cdecl SourceHelper_SetWorkerThreadPriorities(BOOL fStreamProcessingThread);

// Conversion utilities (QAM/ATSC for USA bandplan only)
void __cdecl SourceHelper_ConvertPolarity(char * szPolarity);
int __cdecl  SourceHelper_GetFrequencyFromATSCChannel(int nChannel);
int __cdecl  SourceHelper_GetFrequencyFromQAMChannel(int nChannel);
int __cdecl  SourceHelper_GetQAMChannelFromFrequency(int nFrequency);
int __cdecl  SourceHelper_GetATSCChannelFromFrequency(int nFrequency);

// Tuner dialogs
BOOL __cdecl SourceHelper_DVBSTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_DVBTTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_DVBCTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_DSSTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_ATSCTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_ADVTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_QAMTuneDialog(HWND hWnd);
BOOL __cdecl SourceHelper_UDPMulticastTuneDialog(HWND hWnd);

// Misc functions
BOOL __cdecl SourceHelper_TuneSerialControl(char * szTunerString);
BOOL __cdecl SourceHelper_myGetOpenFileName(LPOPENFILENAME lpofn);
BOOL __cdecl SourceHelper_GetTSReaderVersion(int * nMajor, int * nMinor, int * nBuild);

#ifdef  __cplusplus
}
#endif
