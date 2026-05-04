//*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#* DtRecord.cpp *#*#*#*#*#*#*#*# (C) 2000-2003 DEKTEC
//
// DtRecord is an elementary command-line program to record an MPEG-2 Transport Stream
// into a file.
//
// PCI cards supported:	DTA-120   DVB/ASI input
//                      DTA-122   DVB/SPI input
//                      DTA-140   DVB/ASI input + output
//
// For questions or problems with respect to this application, contact DEKTEC using the
// email address below. For new versions please check the DEKTEC website.
// 
// email  :  support@dektec.com
// website:  www.dektec.com

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Change History -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//
//  SD  2003.01.20  V2.0.1  - Read FIFO load before reading data
//							- Set Rx control to IDLE when done, to avoid FIFO overflow
//	MG	2002.11.26	V2.0.0	Added support for LINUX
//  SD  2002.11.14  V1.1.0  Support for DTA-140	
//  SD  2002.03.22  V1.0.1  Recognise single DTA-120 without /t option. Fix a few typo's.	
//  MG  2002.02.20  V1.0.0  Initial version	


//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtRecord Version -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define DTRECORD_VERSION_MAJOR		2
#define DTRECORD_VERSION_MINOR		0
#define DTRECORD_VERSION_BUGFIX		1


//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Includes -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

#include <stdio.h>
#include "DTAPI.h"

#ifdef WIN32
	#include <conio.h>
#else
	#include <stdlib.h>
	#include <string.h>
#endif

//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Type definitions and constants -.-.-.-.-.-.-.-.-.-.-.-.-.-.
// Parser states
enum PARSE_STATE {
	eParseArg = 0,	// Parse the next commandline argument
	eWhichOpt,		// Which option was found
	eVerifyOpt,		// Verify that a valid option + parameter pair has been found
};

// Parser option states
#define OPTION_UNKNOWN		0x0000
#define OPTION_NONE			0x0001
#define OPTION_RECFILE		0x0002
#define OPTION_RXMODE		0x0004
#define OPTION_MAXSIZE		0x0008
#define OPTION_BOARDTYPE	0x0010
#define OPTION_BOARDNUM		0x0020
#define OPTION_SILENT		0x0040
#define OPTION_HELP			0x0080

const unsigned int  c_nReceiveBufferSize	= 65536;		// Receive buffer size (64kb)

//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Internal function declaration -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
int		ParseCommandline(int argc, char* argv[]);
bool	ParseCmdLineArg(char* szCmdLineOption, int& rSizeOption, char szOption[],
						int& rSizeOptPar, char szOptPar[]);
void	DisplayHelp();
void	DisplayRecInfo();
bool	AttachToBoard(PciCard& rCard, TsInpChannel& rInputCh);
void	LogMessage(char* szMessage);
void	LogMessageAlways(char* szMessage);
void	SetLastError(char* szError);
FILE*	OpenRecFile();
bool	IsNumber(char* szNumber);

//-.-.-.-.-.-.-.-.-.-.-.-.-.- Data declaration and definition -.-.-.-.-.-.-.-.-.-.-.-.-.-.
char*	g_szRecFile = NULL;					// Name of the record file
int		g_nMaxSize(0);						// Maximum file size
int		g_nRxMode(DTAPI_RXMODE_ST188);		// Receive mode	
int		g_nBoardType(0);					// Type of board to connect to
int		g_nBoardNum(1);						// Board number to connect to
bool	g_bRunSilent(false);				// Run in silent mode
bool	g_bDisplayHelp(false);				// Display help

int		g_nBoardBus(0);						// Bus number where the board was found
int		g_nBoardSlot(0);					// Slot number where the board was found
char	g_szLastError[256];					// Last error message

// Characters that may be used to prefix command-line options
// On UNIX systems we cannot use the forward slash (used for paths)
#ifdef WIN32
	const char c_szCmdLinePrefix[]	= "-/";
#else
	const char c_szCmdLinePrefix[]	= "-";	
#endif

const char c_szRxModeCmdOptId[]		= "m";	// Receive mode option identifier
const char c_szMaxSizeCmdOptId[]	= "x";	// Maximum file size option identifier
const char c_szBoardTypeCmdOptId[]	= "t";	// Board type option identifier
const char c_szBoardNumCmdOptId[]	= "n";	// Board number option identifier
const char c_szSilentCmdOptId[]	    = "s";	// Silent option identifier
const char c_szHelpCmdOptId[]	    = "?";	// Help option identifier

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Error messages -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define CLE_MISSING_INVALID_OPTIONS		"Missing or invalid command line option(s)"
#define CLE_MISSING_RECORDFILE_OPTION	"No record file specified"
#define ERR_SET_RCV_MODE				"Can't set Receive mode"
#define ERR_SET_RCV_CTRL_TO_IDLE		"Can't set Receive-control state to IDLE"
#define ERR_SET_RCV_CTRL_TO_RCV			"Can't set Receive-control state to RECEIVE"
#define ERR_READUSINGDMA				"ReadUsingDma returns 0x%04x"
#define ERR_WRITE_TO_FILE				"Write error"
#define ERR_FIFO_OVERFLOW				"FIFO overflow. Can't write data fast enough"
#define ERR_DTAPISCAN_FAIL				"DtapiPciScan failed"
#define ERR_NO_INPUT_BOARD				"No input board in the system"
#define ERR_NO_DTA1XX					"No DTA-%d in the system"
#define ERR_COULD_NOT_FIND_BOARD_X		"Could not find input board #%d in the system"
#define ERR_COULD_NOT_FIND_DTA_X		"Could not find DTA-%d #%d in the system"
#define ERR_COULD_NOT_ATTACH_TO_BOARD	"Failed to attach to the DTA-%d on Bus: %d and Slot: %d"
#define ERR_COULD_NOT_ATTACH_TO_CHANNEL	"Can't attach to the channel"
#define ERR_COULD_NOT_OPEN_FILE			"Can't open '%s' for writing"


//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ main +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
int main(int argc, char* argv[])
{
	int iRetValue(0);
	FILE* pRecFile = NULL;

	try
	{
		int r;
		char szTemp[256];

		//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Parse the command line -.-.-.-.-.-.-.-.-.-.-.-.-.-.

		r = ParseCommandline(argc, argv);

		if ( g_bDisplayHelp || r != 0 ) {

			// Error on the command line???
			if ( 0 == r ) {	
				DisplayHelp();
				return 0;
			} else { 
				if ( 2 == r ) {
					// Error on the command line
					LogMessageAlways(NULL);
					LogMessageAlways(CLE_MISSING_INVALID_OPTIONS);
				} else if ( 1 == r ) {
					// Missing the record file option
					LogMessageAlways(NULL);
					LogMessageAlways(CLE_MISSING_RECORDFILE_OPTION);
				}
				
				DisplayHelp();
				return 1; 
			}
		}

		//.-.-.-.-.-.-.-.-.-.-.-.-.- Attach to the target card -.-.-.-.-.-.-.-.-.-.-.-.-.-
		
		PciCard theCard;
		TsInpChannel theInput;
		if ( !AttachToBoard(theCard, theInput) ) { throw iRetValue; }

		//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Open the record file -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
		if ( NULL == (pRecFile = OpenRecFile()) ) { throw iRetValue; }
		
		//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Final initialisation -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
		
		// Set Receive mode
		if ( DTAPI_OK != theInput.SetRxMode(g_nRxMode) ) {
			SetLastError(ERR_SET_RCV_MODE);
			throw iRetValue;
		}

		// Do not start recording yet, Start after printing final messages
		// So for now set receive control to idle.
		if ( DTAPI_OK != theInput.SetRxControl(DTAPI_RXCTRL_IDLE) ) {
			SetLastError(ERR_SET_RCV_CTRL_TO_IDLE);
			throw iRetValue;
		}

		// Create the write buffer
		char pBuffer[c_nReceiveBufferSize];

		//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Display record info -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
		DisplayRecInfo();

			
		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Reception loop -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
		int nFifoLoad(0), nStoredBytes(0);
		DTAPI_RESULT dr;

		// Start reception now
		dr = theInput.SetRxControl(DTAPI_RXCTRL_RCV);
		if ( DTAPI_OK != dr ) {
			SetLastError(ERR_SET_RCV_CTRL_TO_RCV);
			throw iRetValue;
		}
		
#ifdef WIN32
		while ( !_kbhit() ) {
#else
		while ( 1 ) {
#endif
			// Get FIFO load. Continue if too few bytes available in receive FIFO
			theInput.GetFifoLoad(nFifoLoad);
			if (nFifoLoad < c_nReceiveBufferSize+512)
				continue;

			// Fill the reception buffer
			dr = theInput.Read(pBuffer, c_nReceiveBufferSize);
			if ( DTAPI_OK != dr ) {
				sprintf(szTemp, ERR_READUSINGDMA, dr);
				SetLastError(szTemp);
				throw iRetValue;
			
			} else {
				unsigned int nAllowedtoStore, nStore;

				if ( 0 == g_nMaxSize ) {
					// No maximum size required i.e. store all
					nStore = c_nReceiveBufferSize;
				} else {
					// Maximum size specified i.e. calc how many bytes should be stored

					if ( nStoredBytes >= (g_nMaxSize*1024*1024)) {
						// Reached maximum ==> Stop recording 
						break;
					}

					// Check how many bytes are left to store
					nAllowedtoStore = (g_nMaxSize*1024*1024) - nStoredBytes;
					if (nAllowedtoStore>=c_nReceiveBufferSize) {
						nStore = c_nReceiveBufferSize;
					} else {
						nStore = c_nReceiveBufferSize-nAllowedtoStore;
					}
				}
				
				// Write the received data to file
				unsigned long  ulNumberOfBytesWritten;
				ulNumberOfBytesWritten = fwrite(pBuffer, sizeof(char), nStore, pRecFile);
				if ( ulNumberOfBytesWritten != nStore) {
					SetLastError(ERR_WRITE_TO_FILE);
					throw iRetValue;
				} else {
					// update number of bytes stored
					nStoredBytes += ulNumberOfBytesWritten;
				}
			}

			int nStatus(0), nLatched(0);
			// Get fifo status flag
			theInput.GetFlags(nStatus, nLatched);
			if ( 0 != (DTAPI_RX_FIFO_OVF & nLatched) ) {
				SetLastError(ERR_FIFO_OVERFLOW);
				throw iRetValue;
			}
		}

		//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Detach in a clean way -.-.-.-.-.-.-.-.-.-.-.-.-.-.-

		// Stop reception
		dr = theInput.SetRxControl(DTAPI_RXCTRL_IDLE);

		// Detach from input channel and then from PCI card
		theInput.Detach(DTAPI_INSTANT_DETACH);
		theCard.Detach();

	} catch (...) {

		// Fatal internal error encountered
		LogMessageAlways(g_szLastError);
		LogMessageAlways("Use DtRecord -? for help");

		iRetValue = 1;
	}

	//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Final Cleanup -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

	if ( NULL != pRecFile ) {
		fclose(pRecFile);
	}
	delete g_szRecFile;

	return iRetValue;
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ ParseCommandline +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
int ParseCommandline(int argc, char* argv[])
{
	bool bFoundRecFileOpt(false), bFoundMaxSizeOpt(false), bFoundRxModeOpt(false),
		 bFoundBoardTypeOpt(false), bFoundBoardNumOpt(false), bFoundSilentOpt(false),
		 bFoundHelpOpt(false);

	int i(1), nOptIdSize, nParSize;
	char szOptionId[3], szParameter[256]; 
	int nCurrentOption(OPTION_NONE);
	PARSE_STATE ParseState(eParseArg);
	bool bError(false), bParsedAll(false), bNewOptionIdFound(false),
		 bFirstOption(true);


	while (!bError && !bParsedAll) {

		switch (ParseState) {

		//-.-.-.-.-.-.-.-.-.-.-.-.-.- Parse the next argument -.-.-.-.-.-.-.-.-.-.-.-.-.-.
		case eParseArg: 

			nOptIdSize=3; nParSize=256;
		
			if ( i>=argc && (OPTION_NONE == nCurrentOption) ) {
				// Parsed all argement and no options left to verify
				bParsedAll = true;

			} else if ( i>=argc && (OPTION_NONE != nCurrentOption) ) {
				// Still one option left to verify
				ParseState = eVerifyOpt;
				// No argument left to parsed, so no option or parameter found
				bNewOptionIdFound = false; nOptIdSize=0; nParSize=0;

			} else 	if ( ParseCmdLineArg(argv[i], nOptIdSize, szOptionId, nParSize, szParameter) ) {

				if ( nOptIdSize > 0 ) { bNewOptionIdFound=true; }
				else { bNewOptionIdFound = false; }

				i++;
				
				if ( OPTION_NONE == nCurrentOption ) {
					ParseState = eWhichOpt;
				} else {
					ParseState = eVerifyOpt;
				}

			} else {
				bError = true;

			}

			break;

		//.-.-.-.-.-.-.-.-.-.-.-.-.- which option have we found -.-.-.-.-.-.-.-.-.-.-.-.-.
		case eWhichOpt: 
		
			if ( bNewOptionIdFound ) {
			
				if ( 0 == strcmp(c_szMaxSizeCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_MAXSIZE;
				} else if ( 0 == strcmp(c_szRxModeCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_RXMODE;
				} else if ( 0 == strcmp(c_szBoardTypeCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_BOARDTYPE;
				} else if ( 0 == strcmp(c_szBoardNumCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_BOARDNUM;
				} else if ( 0 == strcmp(c_szSilentCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_SILENT;
				} else if ( 0 == strcmp(c_szHelpCmdOptId, szOptionId) ) {
					nCurrentOption = OPTION_HELP;
				} else {
					nCurrentOption = OPTION_UNKNOWN;
				}

				if ( nParSize > 0 ) {
					ParseState = eVerifyOpt;
				} else {
					ParseState = eParseArg;
				}

			} else if ( nParSize > 0 ) {
				// Only option not having a identifier is the recordfile parameter
				ParseState = eVerifyOpt;

				nCurrentOption = OPTION_RECFILE;

			} else {
				// No option and no parameter. Should never happen!!
				bError = true;
			}

			// Clear new option found flag
			bNewOptionIdFound = false;

			break;

		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- verify the option -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
		case eVerifyOpt: 
			if ( nCurrentOption == OPTION_RECFILE ) {
				if ( !bFirstOption ) {
					// Record file must be the first option encountered
					bError = true;
				} else if ( 0 == nParSize ) {
					// Must specify a parameter for this option
					bError = true;
				} else if (bFoundRecFileOpt) {
					// Already found this option
					bError = true;
				} else {
					g_szRecFile = new char[256];
					strcpy(g_szRecFile, szParameter);
					bFoundRecFileOpt = true;
				}

			} else if ( nCurrentOption == OPTION_HELP ) { 
				if ( bFoundHelpOpt ) {
					// Already found this option
					bError = true;
				} else if ( !bNewOptionIdFound && (0 != nParSize) ) {
					// Found a parameter. This option does not support a parameter
					bError = true;
				} else {
					g_bDisplayHelp = true;
					
					bFoundHelpOpt = true;

					// Do not have to evaluate any other command options
					bParsedAll = true;
				}
			} else if ( nCurrentOption == OPTION_MAXSIZE ) {
				if ( bFirstOption ) {
					// Not allowed as first option
					bError = true;
				} else if ( bNewOptionIdFound ) {
					// Did not expect a new option as parameter
					bError = true;
				} else if ( 0 == nParSize ) {
					// Must specify a parameter for this option
					bError = true;
				} else if (bFoundMaxSizeOpt ) {
					// Already found this option
					bError = true;
				} else {
					// Must be a number 
					if ( !IsNumber( szParameter) ) {
						bError = true;
					} else {
						
						g_nMaxSize = atoi(szParameter);
						if ( g_nMaxSize < 0 ) {
							// illegal ts rate
							bError = true;
						}
					}

					bFoundMaxSizeOpt = true;
				}

			} else if ( nCurrentOption == OPTION_RXMODE ) { 
				if ( bFirstOption ) {
					// Not allowed as first option
					bError = true;
				} else if ( bFoundRxModeOpt ) {
					// Already found this option
					bError = true;
				} else if ( bNewOptionIdFound || (0 == nParSize) ) {
					// Found an option as parameter or no parameter found.
					// Must specify valid parameter if option is found
					bError = true;
				} else {
					if ( 0 == strcmp(szParameter, "ST188")  ) {
						g_nRxMode = DTAPI_RXMODE_ST188;
					} else if ( 0 == strcmp(szParameter, "ST204")  ) {
						g_nRxMode = DTAPI_RXMODE_ST204;
					} else if ( 0 == strcmp(szParameter, "STRAW")  ) {
						g_nRxMode = DTAPI_RXMODE_STRAW;
					} else {
						// Unknown mode
						bError = true;
					}

					bFoundRxModeOpt = true;
				}

			} else if ( nCurrentOption == OPTION_BOARDTYPE ) { 
				if ( bFirstOption ) {
					// Not allowed as first option
					bError = true;
				} else if ( bFoundBoardTypeOpt ) {
					// Already found this option
					bError = true;
				} else if ( bNewOptionIdFound || (0 == nParSize) ) {
					// Found an option as parameter or no parameter found.
					// Must specify valid parameter if option is found
					bError = true;
				} else {
					g_nBoardType = atoi(szParameter);
					
					if (g_nBoardType!=120 && g_nBoardType!=122 && g_nBoardType!=140) {
						// Unknown board type
						bError = true;
					}
					
					bFoundBoardTypeOpt = true;
				}

			} else if ( nCurrentOption == OPTION_BOARDNUM ) { 
				if ( bFirstOption ) {
					// Not allowed as first option
					bError = true;
				} else if ( bFoundBoardNumOpt ) {
					// Already found this option
					bError = true;
				} else if ( bNewOptionIdFound || (0 == nParSize) ) {
					// Found an option as parameter or no parameter found.
					// Must specify valid parameter if option is found
					bError = true;
				} else {
					g_nBoardNum = atoi(szParameter);
					
					if ( (g_nBoardNum < 1) ) {
						// Illegal value
						bError = true;
					}
					
					bFoundBoardNumOpt = true;
				}

			} else if ( nCurrentOption == OPTION_SILENT ) { 
				if ( bFirstOption ) {
					// Not allowed as first option
					bError = true;
				} else if ( bFoundSilentOpt ) {
					// Already found this option
					bError = true;
				} else if ( !bNewOptionIdFound && (0 != nParSize) ) {
					// Found a parameter. This option does not support a parameter
					bError = true;
				} else {
					g_bRunSilent = true;

					bFoundSilentOpt = true;
				}

			} else  { 
				// Unknown option
				bError = true;
			}

			// Reset first option field
			bFirstOption = false;

			// Verified current option ==> reset current option state
			nCurrentOption = OPTION_NONE;

			// Had already found the next option??
			if ( bNewOptionIdFound ) { ParseState = eWhichOpt; }
			else { ParseState = eParseArg; }

			break;

		default:
			break;
		}
	}

	if ( g_bDisplayHelp ) {
		return 0;
	} else if ( bError ) {
		// Error while parsing the command line options
		return 2;
	} else if ( !bFoundRecFileOpt ) {
		// Missing the record file option
		return 1;	
	} else {
		return 0;
	}
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ ParseCmdLineArg +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
//
bool ParseCmdLineArg(char* szCmdLineOption,
					 int& rSizeOption,
					 char szOption[],
					 int& rSizeOptPar,
					 char szOptPar[])
{
	bool bError(false);

	char szTemp[128];
	szTemp[0] = szCmdLineOption[0];
	szTemp[1] = '\0';

	// Check whether first character is the command line identifier prefix
	if ( 0 == strspn(szTemp, c_szCmdLinePrefix) ) {
		// No command line option identifier specified ==> only parameter

		// Make sure the parameter fits
		if ( (rSizeOptPar-1) >= (int)strlen(szCmdLineOption) ) {
			strcpy(szOptPar, szCmdLineOption);

			rSizeOptPar = strlen(szOptPar);
		} else {
			rSizeOptPar = 0;

			bError = true;
		}

		rSizeOption = 0;
	} else {
		// Make sure the option fits
		if (rSizeOption < 2 ) {
			rSizeOption = 0;

			bError = true; 
		} else {
			// Copy the second character (i.e. the command line option identifier)
			szOption[0] = szCmdLineOption[1];
			szOption[1] = '\0';

			rSizeOption = 1;
		}

		// Parameter specified directly after the commandline option???
		if ( 0 != strlen( (char*)(&szCmdLineOption[2]) ) ) {

			// Make sure the parameter fits
			if ( (rSizeOptPar-1) >= (int)strlen((char*)(&szCmdLineOption[2])) ) {
				strcpy(szOptPar, (char*)(&szCmdLineOption[2]));

				rSizeOptPar = strlen( szOptPar );
		
			} else {
				rSizeOptPar = 0;

				bError = true;
			}
		} else {
			rSizeOptPar = 0;
		}
	}

	return !bError;
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DisplayHelp +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
//
void DisplayHelp()
{
	char szTemp[256];

	LogMessageAlways(NULL);
	LogMessageAlways("Syntax:");
	sprintf(szTemp, "  DtRecord recfile [-%s SizeMax] [-%s Mode] [-%s Type]"
					" [-%s Number] [-%s] [-%s]",
					c_szMaxSizeCmdOptId, c_szRxModeCmdOptId, c_szBoardTypeCmdOptId,
					c_szBoardNumCmdOptId, c_szSilentCmdOptId, c_szHelpCmdOptId);
	LogMessageAlways(szTemp);
	LogMessageAlways(NULL);
	LogMessageAlways("Where:");
	LogMessageAlways("  recfile is the name of the record file");
	LogMessageAlways(NULL);
	LogMessageAlways("Options:");
	sprintf(szTemp, "  -%s  Display help", c_szHelpCmdOptId);
	LogMessageAlways(szTemp);
	sprintf(szTemp, "  -%s  Maximum file size in MB (default: no maximum)", c_szMaxSizeCmdOptId);
	LogMessageAlways(szTemp);
	sprintf(szTemp, "  -%s  Receive Mode (default: ST188)", c_szRxModeCmdOptId);
	LogMessageAlways(szTemp);
	LogMessageAlways("      Use: ST188 store packets as 188-byte packets");
	LogMessageAlways("           ST204 store packets as 204-byte packets");
	LogMessageAlways("           STRAW No notion of packets. Store all valid bytes");
	sprintf(szTemp, "  -%s  Board Type to use (default: any input board)",
		            c_szBoardTypeCmdOptId);
	LogMessageAlways(szTemp);
	LogMessageAlways("      Use: 120 (for DTA-120)");
	LogMessageAlways("           122 (for DTA-122)");
	LogMessageAlways("           140 (for DTA-140)");
	sprintf(szTemp, "  -%s  Board Number to use (default: 1) ", c_szBoardNumCmdOptId);
	LogMessageAlways(szTemp);
	LogMessageAlways("      Note: 1 indicates the first board");
	sprintf(szTemp, "  -%s  Silent mode. No messages printed", c_szSilentCmdOptId);
	LogMessageAlways(szTemp);
	LogMessageAlways(NULL);
	LogMessageAlways("NOTE: The first option can only be the recfile or the help option");
	LogMessageAlways(NULL);
	LogMessageAlways("Examples:");
	sprintf(szTemp, "  DtRecord myfile.ts -%s 5", c_szMaxSizeCmdOptId);
	LogMessageAlways(szTemp);
#ifdef WIN32
	sprintf(szTemp, "  DtRecord \"c:\\Media Files\\myfile.ts\" -%s 10 -%s STRAW -%s 122",
					c_szMaxSizeCmdOptId, c_szRxModeCmdOptId, c_szBoardTypeCmdOptId);
	LogMessageAlways(szTemp);
#endif
	LogMessageAlways(NULL);
	sprintf(szTemp, "DtRecord version:  %d.%d.%d", DTRECORD_VERSION_MAJOR,
		    DTRECORD_VERSION_MINOR, DTRECORD_VERSION_BUGFIX);
	LogMessageAlways(szTemp);
}

//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ AttachToBoard +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
bool AttachToBoard(PciCard& rCard, TsInpChannel& rInputCh)
{
	bool bResult(true);

	try
	{
		int i(0);
		char szTemp[256];

		//.-.-.-.-.-.-.-.-.-.-.-.-.- Determine number of cards -.-.-.-.-.-.-.-.-.-.-.-.-.-
		DtapiHwFunc HwFuncs[10];
		int iNumOfFuncs(10);

		if ( DTAPI_OK != DtapiPciScan(iNumOfFuncs, iNumOfFuncs, HwFuncs) ) {
			strcpy(szTemp, ERR_DTAPISCAN_FAIL);
			SetLastError(szTemp);
			throw bResult;
		}
		
		//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Find the Nth DTA-XXX -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
		int nBoard(0);

		for (i=0; i<iNumOfFuncs; i++) {
			
			if (g_nBoardType==0 && (  HwFuncs[i].m_nTypeNumber == 120
								   || HwFuncs[i].m_nTypeNumber == 122
								   || HwFuncs[i].m_nTypeNumber == 140)) {
				// No board type specified so any input board is OK
				nBoard++;
			} else if ( g_nBoardType == HwFuncs[i].m_nTypeNumber ) {
				nBoard++;
			}

			if (g_nBoardNum==nBoard) {
				// Found the target board
				
				if ( 0 == g_nBoardType ) {
					// No specific type was specified, so store the type found
					g_nBoardType = HwFuncs[i].m_nTypeNumber;

					// Store position info
					g_nBoardBus  = HwFuncs[i].m_nPciBusNumber;
					g_nBoardSlot = HwFuncs[i].m_nSlotNumber;
				}
				break;
			}
		}

		if ( 0 == nBoard ) {
			// No board found at all
			
			if ( 0 == g_nBoardType ) {
				// No specific type specified
				sprintf(szTemp, ERR_NO_INPUT_BOARD);
			} else {
				// Specific type specified
				sprintf(szTemp, ERR_NO_DTA1XX, g_nBoardType);
			}

			SetLastError(szTemp);
			throw bResult;

		} else if ( g_nBoardNum != nBoard ) { 
			// Could not find specified board

			if ( 0 == g_nBoardType ) {
				// No specific type specified
				sprintf(szTemp, ERR_COULD_NOT_FIND_BOARD_X, g_nBoardNum);
			} else {
				// Specific type specified
				sprintf(szTemp, ERR_COULD_NOT_FIND_DTA_X, g_nBoardType, g_nBoardNum);
			}
			
			SetLastError(szTemp);
			throw bResult;

		}

		//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Attach to Card -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

		DTAPI_RESULT  dr;
		dr = rCard.AttachToSlot(HwFuncs[i].m_nPciBusNumber, HwFuncs[i].m_nSlotNumber);
		if (dr != DTAPI_OK) {
			if (dr == DTAPI_E_DRIVER_INCOMP) {
				int nDriverVersionMajor(-1), nDriverVersionMinor(-1),
					nDriverVersionBugFix(-1), nDriverVersionBuild(-1);
				::DtapiGetDeviceDriverVersion(nDriverVersionMajor, nDriverVersionMinor,
											  nDriverVersionBugFix, nDriverVersionBuild);
				sprintf(szTemp, "The version of the Dta1xx device driver (V%d.%d.%d %d) "
								"is not compatible\nwith this version of DtRecord.\n"
								"Please install the latest version of the Dta1xx driver.", 
								nDriverVersionMajor, nDriverVersionMinor,
								nDriverVersionBugFix, nDriverVersionBuild);

			} else {
				sprintf(szTemp, ERR_COULD_NOT_ATTACH_TO_BOARD,  g_nBoardType,
						HwFuncs[i].m_nPciBusNumber, HwFuncs[i].m_nSlotNumber);
			}
			SetLastError(szTemp);
			throw bResult;
		}

		if (rInputCh.Attach(&rCard) != DTAPI_OK) {
			SetLastError(ERR_COULD_NOT_ATTACH_TO_CHANNEL);
			throw bResult;
		}

	} catch (...) {
		bResult = false;
	}

	return bResult;
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ OpenRecFile +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
//
FILE* OpenRecFile()
{
	char szTemp[256];

	// Open the file for writing
	FILE* pFile = fopen(g_szRecFile, "wb");

	if ( NULL == pFile) {
		sprintf(szTemp, ERR_COULD_NOT_OPEN_FILE, g_szRecFile);
		SetLastError(szTemp);
		return NULL;
	} else {
		return pFile;
	}
}

//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DisplayRecInfo +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
//
void DisplayRecInfo()
{
	char szRxMode[8];

	// Convert to 'readable' string
	if ( DTAPI_RXMODE_ST188 == g_nRxMode )  {
		strcpy(szRxMode, "ST188");
	} else if ( DTAPI_RXMODE_ST204 == g_nRxMode )  {
		strcpy(szRxMode, "ST204");
	} else {//if ( DTAPI_RXMODE_STRAW == g_nRxMode )  {
		strcpy(szRxMode, "STRAW");
	}

	char szTemp[256];

	LogMessage("Start Recording:");
	sprintf(szTemp, "- Record file name  : %s", g_szRecFile);
	LogMessage(szTemp);
	if (g_nMaxSize == 0)
		sprintf(szTemp, "- Maximum file size : no maximum");
	else
		sprintf(szTemp, "- Maximum file size : %d MB", g_nMaxSize);
	LogMessage(szTemp);
	sprintf(szTemp, "- Receive Mode      : %s", szRxMode);
	LogMessage(szTemp);
	sprintf(szTemp, "- Input board       : DTA-%d (#%d). Located on bus %d in slot %d",
		    g_nBoardType, g_nBoardNum, g_nBoardBus, g_nBoardSlot);
	LogMessage(szTemp);
	LogMessage(NULL);
	LogMessage(NULL);
#ifdef WIN32
	LogMessage("Press any key to stop recording");
	LogMessage(NULL);
#endif

}

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Helper functions -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ IsNumber +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
bool IsNumber(char* szNumber)
{
	const char c_szDigits[] = "1234567890";

	int nLength = strlen(szNumber);

	// Does string consist out of digits only??
	if ( nLength != (int)strspn(szNumber, c_szDigits)  ) {
		return false;
	} else {
		return true;
	}
}

//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ LogMessage +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=
//
void LogMessage(char* szMessage)
{
	char szTemp[256];

	// Running in silent mode ??
	if ( !g_bRunSilent ) {

		if ( NULL == szMessage ) { strcpy(szTemp, "");	}
		else { strcpy(szTemp, szMessage); }

		strcat(szTemp, "\n");
		printf(szTemp);
	}
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ LogMessageAlways +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// Logs message without regard for the run silent parameter
// 
void LogMessageAlways(char* szMessage)
{
	char szTemp[256];

	if ( NULL == szMessage ) { strcpy(szTemp, "");	}
	else { strcpy(szTemp, szMessage); }

	strcat(szTemp, "\n");
	printf(szTemp);
}

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ SetLastError +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
void SetLastError(char* szError)
{
	strcpy(g_szLastError, "\nError: ");
	strcat(g_szLastError, szError);
}
