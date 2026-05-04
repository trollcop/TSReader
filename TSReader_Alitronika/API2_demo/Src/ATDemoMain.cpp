/*! Time-stamp: <@(#)ATDemoMain.cpp   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemoMain.cpp
 *
 *  Project : ATDemoApp. Testappliction for Alitronika devices.
 *			  Supports Linux and windows operating System.
 *			  
 *			: Devices supported (USB and PCI)
 *				-ATDEMO
 *				-AT20, AT200
 *				-AT30, AT30R1, AT300
 *				-AT4, AT40, AT40X, AT40R1, AT400
 *				-AT70, AT72, AT73, AT700, AT720, AT730
 *				-AT8, AT80, AT82, AT800, AT820, AT1800
 *				-AT2700, AT2800, AT3800, AT2780
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
#include "Support/getopt.h"
#include "Support/strfunc.h"

/** Action enumeration */
typedef enum {
	None,
	Play,
	Record,
} EAction;

/**********************************************************************************************************/
// static functions 

/*!
 * Print some device information
 *
 * @param *pATBoardPlayRec : Pointer to the class that controls the ATBoard
 */
static void PrintDeviceInfo(CATBoardPlayRec *pATBoardPlayRec);
void PrintHelp();
void PrintModHelp();
EAction ProcessCommandline(int argc, char **argv);

char * s_pFilename;

/**********************************************************************************************************/
// Option that are modified by command line parameters

SPlayOpt POptions = 
{
	TRUE,					///< enable bit rate re multiplexing
	0,						///< set file bit rate to 0. This means tsrate determents the bit rate 
	40000000UL				///< set output bit rate to 40000000 bit/s
};

SRecOpt ROptions =
{
	100UL					///< record a file of 100MBytes
};

STunDVBTOpt TunDVBTOptions =
{
	522000000UL,			///< set tuning frequency to 522MHz
	BANDWIDTH_8_7_MHZ,		///< set bandwidth to 8MHz
	FALSE					///< set bandwidth group selection to 0
};

STunDVBCAnnexAOpt TunDVBCAnnexAOptions =
{
	658000000UL,			///< set tuning frequency to 658MHz
	6900000UL,				///< set symbol rate to 6.9MSymbols/s
	INVERSION_AUTO,			///< set inversion selection to AUTO
	QAM_AUTO				///< set QAM mode selection to AUTO
};

STunDVBCAnnexBOpt TunDVBCAnnexBOptions =
{
	70000000UL,				///< set tuning frequency to 70MHz
	QAM_64					///< set QAM mode selection to QAM64
};

SModDVBTOpt ModDVBTOptions =
{
	TRUE,						///< select RF output
	0UL,						///< if RF is selected, IF output frequency can be 0
	500000000UL,				///< RF output frequency
	-350,						///< -35dBm 
	QAM_64,						///< set qam mode to QAM_64
	TRANSMISSION_MODE_8K,		///< set transmission mode to 8K
	GUARD_INTERVAL_1_32,		///< set guard interval to 1/32
	FEC_7_8,					///< set FEC code rate to 7/8
	BANDWIDTH_8_7_MHZ,			///< set bandwidth to 8MHz
	INVERSION_OFF,				///< set spectral inversion of
	FALSE,						///< disable DVB-H mode
	FALSE,						///< disable DVB-H time slicing
	FALSE,						///< disable DVB-H MPE FEC
	FALSE,						///< disable DVB-H Indepth interleaver.
	0							///< cell ID is zero
};

SModDVBCOpt ModDVBCOptions =
{
	TRUE,						///< select RF output
	0UL,						///< if RF is selected, IF output frequency can be 0
	500000000UL,				///< RF output frequency
	-350,						///< -35dBm 
	QAM_64,						///< set qam mode to QAM_16
	FLT_ALPHA_15,				///< set filter roll off to 15%
	INTLEAV_I12_J17,			///< set interleaver to I12/J17
	INVERSION_OFF,				///< set inversion off
	5500000UL,					///< set symbol rate to 5MSymbols/s
	FALSE						///< disable annex B mode
};



/**********************************************************************************************************/
// functions

// main entrance function
int main (int argc, char **argv)
{
	SetProgramName("ATDemoApp");
	Message("*****************************************************************");

#if 0	// For command line parameter testing only.
	memset(&POptions, 0xAA, sizeof(POptions));
	memset(&ROptions, 0xAA, sizeof(ROptions));
	memset(&TunDVBTOptions, 0xAA, sizeof(TunDVBTOptions));
	memset(&TunDVBCAnnexAOptions, 0xAA, sizeof(TunDVBCAnnexAOptions));
	memset(&TunDVBCAnnexBOptions, 0xAA, sizeof(TunDVBCAnnexBOptions));
	memset(&ModDVBTOptions, 0xAA, sizeof(ModDVBTOptions));
	memset(&ModDVBCOptions, 0xAA, sizeof(ModDVBCOptions));
#endif

	// decode command line options and do the action
	EAction Action = ProcessCommandline(argc, argv);
	if (Action == None)
	{
		PrintHelp();
		return 1;
	}

	// Instantiate the board manager
	CAtBoardManager *BoardManager;
	BoardManager = CAtBoardManager::Instance ();

	// Get a device list
	CAtDeviceList DeviceList;
	BoardManager->GetDeviceList(DeviceList);

	SBoardOpenParams BoardOpenParams;
		
	// Use the first board found. If none are found, use the demo board.
	if(DeviceList.GetFreeSize() == 0)
	{
		Message ("No (free) device(s) found. Using demo board.");

		// set demo board open parameters
		BoardOpenParams.m_BoardPos = -1;
		BoardOpenParams.m_pBoardName = NULL;
	}
	else
	{
		if (DeviceList.GetFirstFreeBoard(&BoardOpenParams))
		{
			Message ("Connecting to first free board found.");
		}
		else
		{
			Message ("No (free) device(s) found. Using demo board.");

			// set demo board open parameters
			BoardOpenParams.m_BoardPos = -1;
			BoardOpenParams.m_pBoardName = NULL;
		}
	}

	const ATDEVICEINFO *pDevInfo = DeviceList.GetDeviceInfo(&BoardOpenParams);

	// Create all board types
	CATBoardPlayRec			ATBoardPlayRec;
	CATBoardTunDVBT			ATBoardTunDVBT;
	CATBoardTunDVBCAnnexA	ATBoardTunDVBCAnnexA;
	CATBoardTunDVBCAnnexB	ATBoardTunDVBCAnnexB;
	CATBoardModDVBT			ATBoardModDVBT;
	CATBoardModDVBC			ATBoardModDVBC;
	
	CATBoardPlayRec *pATBoardCtrl = NULL;

	Message ("Open and initializing board....");

	switch (Action)
	{
	case Play:
		// Initialize optional front-ends
		if (pDevInfo)
		{
			if (pDevInfo->DeviceHasDVB_C_Mod)
			{
				// set DVB-C modulator
				pATBoardCtrl = &ATBoardModDVBC;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetModDVBC((CATBoardModDVBC *)pATBoardCtrl, ModDVBCOptions))
					goto _CloseBoard;
			}
			else
			if (pDevInfo->DeviceHasDVB_T_Mod)
			{
				// set DVB-T modulator
				pATBoardCtrl = &ATBoardModDVBT;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetModDVBT((CATBoardModDVBT *)pATBoardCtrl, ModDVBTOptions))
					goto _CloseBoard;
			}
			else
			{
				pATBoardCtrl = &ATBoardPlayRec;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
			}
		}
		else
		{
			pATBoardCtrl = &ATBoardPlayRec;
			pATBoardCtrl->BoardOpen(BoardOpenParams);
			Message ("Open and initializing board done");
			PrintDeviceInfo(pATBoardCtrl);
		}

		// Perform the actual play action
		if (pATBoardCtrl && !PlayFile(pATBoardCtrl, s_pFilename, POptions))
			Message("Play terminated with an error");
		break;

	case Record:

		if (pDevInfo)
		{
			if (pDevInfo->DeviceHasDVB_C == TYPE_DVB_C_ANNEXA || pDevInfo->DeviceHasDVB_C == TYPE_DVB_C_ANNEXC)
			{
				// set DVB-C tuner annexA,C
				pATBoardCtrl = &ATBoardTunDVBCAnnexA;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetTunDVBCAnnexA((CATBoardTunDVBCAnnexA *)pATBoardCtrl, TunDVBCAnnexAOptions))
					goto _CloseBoard;
			}
			else
			if (pDevInfo->DeviceHasDVB_C == TYPE_DVB_C_ANNEXB)
			{
				// set DVB-C tuner annexB
				pATBoardCtrl = &ATBoardTunDVBCAnnexB;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetTunDVBCAnnexB((CATBoardTunDVBCAnnexB *)pATBoardCtrl, TunDVBCAnnexBOptions))
					goto _CloseBoard;
			}
			else
			if (pDevInfo->DeviceHasDVB_T)
			{
				// set DVB-T tuner
				pATBoardCtrl = &ATBoardTunDVBT;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetTunDVBT((CATBoardTunDVBT *)pATBoardCtrl, TunDVBTOptions))
					goto _CloseBoard;
			}
			else
			if (pDevInfo->DeviceHasDVB_C_Mod)
			{
				// set DVB-T tuner
				pATBoardCtrl = &ATBoardModDVBC;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetModDVBC((CATBoardModDVBC *)pATBoardCtrl, ModDVBCOptions))
					goto _CloseBoard;
			}
			else
			if (pDevInfo->DeviceHasDVB_T_Mod)
			{
				// set DVB-T modulator
				pATBoardCtrl = &ATBoardModDVBT;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
				if (!SetModDVBT((CATBoardModDVBT *)pATBoardCtrl, ModDVBTOptions))
					goto _CloseBoard;
			}
			else
			{
				pATBoardCtrl = &ATBoardPlayRec;
				pATBoardCtrl->BoardOpen(BoardOpenParams);
				Message ("Open and initializing board done");
				PrintDeviceInfo(pATBoardCtrl);
			}
		}
		else
		{
			pATBoardCtrl = &ATBoardPlayRec;
			pATBoardCtrl->BoardOpen(BoardOpenParams);
			Message ("Open and initializing board done");
			PrintDeviceInfo(pATBoardCtrl);
		}

		// Perform the actual record action
		if (pATBoardCtrl && !RecordFile(pATBoardCtrl, s_pFilename, ROptions))
			Message("Record terminated with an error");
		break;
	}

_CloseBoard:
	// close the board
	pATBoardCtrl->BoardClose();

	return 0;
}



// Print some device information 
static void PrintDeviceInfo(CATBoardPlayRec *pATBoardPlayRec)
{
	CATBoard *pTheBoard = pATBoardPlayRec->GetCurBoard();
	
	char TmpStr[MAX_TMP_STR_LEN];

	// print friendly name
	u32 nLength = ATDEV_INFO_MAX_STR_LEN;
	char * pFriendlyName = new char[nLength];
	pTheBoard->GetFriendlyName(pFriendlyName, nLength);

	sprintf(TmpStr, "Board found: %s", pFriendlyName);
	Message(TmpStr);

	if (pFriendlyName)
		delete pFriendlyName;

	// print API version information
	ATVERSIONINFO * pAPIInfo = NULL;
	pAPIInfo = pTheBoard->GetAPIVersion();
	sprintf(TmpStr, "API version %d.%d", pAPIInfo->Mayor, pAPIInfo->Minor);
	Message(TmpStr);

	// print driver version information
	ATVERSIONINFO * pDriverInfo = NULL;
	pDriverInfo = pTheBoard->GetDriverVersion();
	
	if(pDriverInfo  != NULL)
	{
		sprintf(TmpStr, "Driver version %d.%d", pDriverInfo->Mayor, pDriverInfo->Minor);
		Message(TmpStr);
	}
}


struct CItemValue
{
	const char *Name;
	int			Value;
};

int GetItemValue(const CItemValue *pItem, const char *pstrArg)
{
	while (pItem->Name != NULL)
	{
		if (__stricmp(pItem->Name, pstrArg) == 0)
		{
			return pItem->Value;
		}
		pItem++;
	}
	return -1;
}

const CItemValue FecItems[] =
{
	{	"FEC_1_2", FEC_1_2	},
	{	"FEC_2_3", FEC_2_3	},
	{	"FEC_3_4", FEC_3_4	},
	{	"FEC_5_6", FEC_5_6	},
	{	"FEC_7_8", FEC_7_8	},
	{	NULL, -1			}
};

const CItemValue BandwidthItems[] =
{
	{	"8_MHZ", BANDWIDTH_8_7_MHZ	},
	{	"7_MHZ", BANDWIDTH_7_6_MHZ	},
	{	"6_MHZ", BANDWIDTH_6_MHZ  	},
	{	"5_MHZ", BANDWIDTH_5_MHZ	},
	{	NULL, -1					}
};

const CItemValue ModulationItems[] =
{
	{	"QPSK", QPSK		},
	{	"QAM_16", QAM_16	},
	{	"QAM_64", QAM_64	},
	{	"QAM_32", QAM_32	},
	{	"QAM_128", QAM_128	},
	{	"QAM_256", QAM_256	},
	{	NULL, -1			}
};	

const CItemValue TransmissionModeItems[] =
{
	{	"2K", TRANSMISSION_MODE_2K	},
	{	"4K", TRANSMISSION_MODE_4K	},
	{	"8K", TRANSMISSION_MODE_8K	},
	{	NULL, -1					}
};

const CItemValue GuardItervalItems[] =
{
	{	"4", GUARD_INTERVAL_1_4		},
	{	"8", GUARD_INTERVAL_1_8		},
	{	"16", GUARD_INTERVAL_1_16	},
	{	"32", GUARD_INTERVAL_1_32	},
	{	NULL, -1					}
};

const CItemValue InversionItems[] =
{
	{	"ON", INVERSION_ON		},
	{	"OFF", INVERSION_OFF	},
	{	NULL, -1				}
};

const CItemValue InterleaverItems[] =
{
	{	"I128_J1", INTLEAV_I128_J1	},
	{	"I128_J2", INTLEAV_I128_J2	},
	{	"I128_J3", INTLEAV_I128_J3	},
	{	"I128_J4", INTLEAV_I128_J4	},
	{	"I128_J5", INTLEAV_I128_J5	},
	{	"I128_J6", INTLEAV_I128_J6	},
	{	"I128_J7", INTLEAV_I128_J7	},
	{	"I128_J8", INTLEAV_I128_J8	},
	{	"I64_J2", INTLEAV_I64_J2	},
	{	"I32_J4", INTLEAV_I32_J4	},
	{	"I16_J8", INTLEAV_I16_J8	},
	{	"I8_J16", INTLEAV_I8_J16	},
	{	"I12_J17", INTLEAV_I12_J17	},
	{	NULL, -1							}
};

enum EOptCodes
{
	EOPT_BITRATE, EOPT_RECSIZE,
	EOPT_FREQ, EOPT_SYMBRATE, EOPT_SPECTINVERT, EOPT_MODULATION,
	EOPT_BANDWIDTH, EOPT_IFOUT, EOPT_FEC, EOPT_TMODE, EOPT_GUARD,
	EOPT_DVBH, EOPT_CELLID, EOPT_RFLEVEL, EOPT_INTERLEAVE, EOPT_ANNEXB
};


// Return 'true' is no errors occurred
EAction ProcessCommandline(int argc, char **argv)
{
	// Define the command line options. The string, declared
	// first, contains a list of the short options. The struct
	// contains the long options.
	char * ShortOptions = "h?p:r:l:";
	static const struct CmdLineOption LongOptions[] =
	{
		// General
		{"help", no_argument, NULL, 'h'},
		// Commands
		{"play", required_argument, NULL, 'p'},
		{"record", required_argument, NULL, 'r'},
		// Play - Record
		{"bitrate", required_argument, NULL, EOPT_BITRATE},
		{"length", required_argument, NULL, 'l'},

		// Other
		{"frequency", required_argument, NULL, EOPT_FREQ},
		{"symbolrate", required_argument, NULL, EOPT_SYMBRATE},
		{"fec", required_argument, NULL, EOPT_FEC},
		{"bandwidth", required_argument, NULL, EOPT_BANDWIDTH},
		{"modulation", required_argument, NULL, EOPT_MODULATION},
		{"tmode", required_argument, NULL, EOPT_TMODE},
		{"guard", required_argument, NULL, EOPT_GUARD},
		{"inversion", required_argument, NULL, EOPT_SPECTINVERT},
		{"DVBH", no_argument, NULL, EOPT_DVBH},
		{"ANNEXB", no_argument, NULL, EOPT_ANNEXB},
		{"CellId", required_argument, NULL, EOPT_CELLID},
		{"IF", no_argument, NULL, EOPT_IFOUT},
		{"RFLevel", required_argument, NULL, EOPT_RFLEVEL},
		{"interleaver", required_argument, NULL, EOPT_INTERLEAVE},
		{NULL, no_argument, NULL, 0}
	};

	EAction Action = None;
	char * TunerType = NULL;

	// Parse the arguments passed to the program.
	int NextOption = 0;

	do
	{
		NextOption = getopt_long(argc, argv, ShortOptions, LongOptions, NULL);
		switch(NextOption)
		{
		case 'h':								// Print Help
//			PrintHelp();
			return None;

		case 'p':								// Play mode
			if(Action == None)
			{
				Action = Play;

				if(optarg==NULL)
				{
					Error("No filename specified!");
				}
				s_pFilename = optarg;

				break;
			}
			Error("Play & Record simultaneously not supported!");
			break;

		case 'r':								// Record mode
			if(Action == None)
			{
				Action = Record;

				if(optarg==NULL)
				{
					Error("No filename specified!");
				}
				s_pFilename = optarg;

				break;
			}
			Error("Play & Record simultaneously not supported!");
			break;

		case 'l':
			if(optarg==NULL)
			{
				Error("recording length (in MByte) not specified!");
			}
			ROptions.m_nFileSize = atoi(optarg); 
			break;

		case EOPT_FREQ:									// Set frequency
			if(optarg==NULL)
			{
				Error("No frequency specified!");
			}
			TunDVBTOptions.m_nFrequency = 
			TunDVBCAnnexAOptions.m_nFrequency =
			TunDVBCAnnexBOptions.m_nFrequency =
			ModDVBTOptions.m_nRfFrequency =
			ModDVBCOptions.m_nRfFrequency = (u32)(1000000 * atof(optarg));
			break;

		case EOPT_BITRATE:						// Set bitrate
			if(optarg == NULL)
			{
				Error("No bitrate specified");
			}
			//printf("bitRate %s \n", optarg);
			POptions.m_dOutputBitrate = atol(optarg);
			//printf("bitRate %f \n",Options.Bitrate);

			break;

		case EOPT_SYMBRATE:						// Set Symbol rate
			if(optarg==NULL)
			{
				Error("No Symbol rate specified!");
			}
			TunDVBCAnnexAOptions.m_nSymbolRate = 
			ModDVBCOptions.m_nSymbolRate = atol(optarg);
			break;

		case EOPT_FEC:							// Set FEC rate
			if ((ModDVBTOptions.m_FECCodeRate = (src_code_rate_t)GetItemValue(FecItems, optarg)) < 0)
				Error("No FEC-rate specified!");
			break;

		case EOPT_BANDWIDTH:			//Set bandwidth
			if ((TunDVBTOptions.m_BandWidth = ModDVBTOptions.m_Bandwidth = (src_bandwidth_t)GetItemValue(BandwidthItems, optarg)) < 0)
				Error("No Bandwidth specified!");
			break;

		case EOPT_MODULATION:			//Set modulation
			if ((TunDVBCAnnexAOptions.m_Modulation = TunDVBCAnnexBOptions.m_Modulation = ModDVBTOptions.m_Modulation = ModDVBCOptions.m_Modulation = (src_modulation_t)GetItemValue(ModulationItems, optarg)) < 0)
				Error("No modulation specified!");
			break;

		case EOPT_TMODE:				//Set TRANSMISSIONMODE
			if ((ModDVBTOptions.m_TranMode = (src_transmit_mode_t)GetItemValue(TransmissionModeItems, optarg)) < 0)
				Error("No transmission mode specified!");
			break;

		case EOPT_GUARD:				//Set guard interval
			if ((ModDVBTOptions.m_GuardInt = (src_guard_interval_t)GetItemValue(GuardItervalItems, optarg)) < 0)
				Error("No guard interval specified!");
			break;

		case EOPT_SPECTINVERT:			//Set inversion
			if ((ModDVBTOptions.m_Inversion = ModDVBCOptions.m_Inversion = TunDVBCAnnexAOptions.m_Invert = (src_spectral_inversion_t)GetItemValue(InversionItems, optarg)) < 0)
			{
				Error("No inversion specified!");
			}
			break;

		case EOPT_DVBH:					//Set DVB-H
			{
				ModDVBTOptions.m_bDvbHEn = TRUE;
			}
			break;

		case EOPT_ANNEXB:
			{
				ModDVBCOptions.m_bAnnexBEn = TRUE;
			}
			break;

		case EOPT_CELLID:				//Set DVB-H Cellid
			if(optarg==NULL)
			{
				Error("No dvb_h_cell_id specified!");
			}
			ModDVBTOptions.m_nDvbHCellId = atoi(optarg); 
			break;

		case EOPT_IFOUT:				//Set to IF output
			{
				ModDVBTOptions.m_bRfIfSel = ModDVBCOptions.m_bRfIfSel = 0;
			}
			break;

		case EOPT_RFLEVEL:				//Set RF output level
			if(optarg==NULL)
			{
				Error("No RF output specified!");
			}
			ModDVBTOptions.m_iRfoutputLevel = ModDVBCOptions.m_iRfoutputLevel = (s32)(10 * atof(optarg));
			break;

		case EOPT_INTERLEAVE:				//Set interleaver
			if ((ModDVBCOptions.m_Interleaver = (src_interleaver_t)GetItemValue(InterleaverItems, optarg)) < 0)
			{
				Error("No interleaver specified!");
			}
			break;

		case -1:								// End of list
			break;

		case '?':								// Unknown option
		default:								// Unexpected error
			Message("  @@@ Incorrect command line option(s) @@@\n");
//			PrintHelp();
			return None;
		}
	}
	while(NextOption != -1);

	return Action;
}

//
//	test command lines:
//	-p d:\ts\TT1.ts --bitrate 35000000 --frequency 500 --symbolrate 30000000 --RFLevel -5.5 --bandwidth 7_MHz --modulation QAM_16 --tmode 4K --fec FEC_2_3 --guard 4 --inversion ON --interleaver I128_J3 --IF --DVBH --CellId 2048 -l 33
//	-r  d:\ts\rec_test.ts -l 36
//

void PrintHelp()
{
	const char *ProgName = GetProgramName();

	printf("%s is a program for performing basic operations on the Alitronika\n", ProgName);
	printf("ASI input/output, DVB Receivers and Modulator devices.\n");
	printf("Features: Play from a transport stream file\n");
	printf("          Record to a file.\n\n");

	printf("Help on commandline options:\n");
	printf("\t1: Play\n");
	printf("\t2: Record\n");
	printf("\t3: Modulator settings\n");
	printf("\t4: Receiver settings\n");
	printf("\t9: exit\n");

	bool bRepeat = true;

	while (bRepeat)
	{
		printf("\r==> ");
		int Ch = getchar();
		const char *pText = NULL;

		switch(Ch)
		{
		case '1':	// Play
			{
				pText =
					"\n"
					"  -p, --play\t<filename>\tPlay a file to ASI output\n"
					"  --bitrate\t<bitrate>\tOutput Bitrate in Bits/sec (ex. 39000000)\n"
					"\n"
					"Example:\n %s --play teststream.ts --bitrate 39000000\n"
					;
			}
			break;

		case '2':	// Record
			{
				pText =
					"\n"
					"  -r, --record\t<filename>\tRecord ASI input to a file\n"
					"  -l, --length\t<nr of MB>\tRecord file length\n"
					"\n"
					"Example:\n %s --record teststream.ts --length 50\n"
					;
			}
			break;

		case '3':	// Modulate
			{
				pText =
					"\n"
					"  -p, --play\t<filename>\tPlay a file to the Modulator\n"
					"  --frequency\t<Freq>\t\tRF Frequency in MHz (ex. 107.45)\n"
					"  --symbolrate\t<MSym/sec>\tNr of Symbols/sec (ex. 5457800)\n"
					"  --RFLevel\t<Level in dB>\tRF level -35.0 ~ 2.0 in 0.5dB steps (ex. -9.5)\n"
					"  --IF\t\tEnable IF output and disable RF output\n"
					"  --ANNEXB\tEnable DVBC modulator AnnexB mode\n"
					"  --bandwidth\t<BW>\t\tbandwidth in Megahertz:\n"
					"  \t\t8_MHz, 7_MHz, 6_MHz, 5_MHz\n"
					"  --modulation\t<mod type>\tModulation type:\n"
					"  \t\tQPSK, QAM_16, QAM_32, QAM_64, QAM_128, QAM_256\n"
					"  --tmode\t<mode>\t\tTransmission mode: 2K, 4K, 8K\n"
					"  --fec\t\t<fec rate>\tForward error correction:\n"
					"  \t\tFEC_1_2, FEC_2_3, FEC_3_4, FEC_5_6, FEC_7_8\n"
					"  --guard\t<1/interval>\t1/n Interval: 4, 8, 16, 32\n"
					"  --inversion\t<ON/OFF>\tUse spectral inversion: ON, OFF\n"
					"  --interleaver\t<interleaving>\tInterleaver setting:\n"
					"  \t\tI128_J1, I128_J2, I128_J3, I128_J4, I128_J5, I128_J6,\n"
					"  \t\tI128_J7, I128_J8, I64_J2, I32_J4, I16_J8, I8_J16, I12_J17\n"
					"  --DVBH\t\t\tEnable DVB-H mode\n"
					"  --CellId\t<cell ID>\tDVB-H Cell ID (ex. 16462)\n"
					"\n"
					"Example DVB-T:\n %s --play teststream.ts --frequency 500.0 --RFLevel -20 --modulation QAM_64 --bandwidth 8_MHz  --guard 32 --tmode 8K --fec FEC_7_8\n"
					"Example DVB-C annexA:\n %s --play teststream.ts --frequency 500 --RFLevel -20 --modulation QAM_64 --symbolrate 6900000 --inversion OFF\n"
					"Example DVB-C annexB:\n %s --play teststream.ts --frequency 500 --RFLevel -20 --modulation QAM_64 --inversion OFF --interleaver I128_J1 --ANNEXB\n"
					;
			}
		    break;

		case '4':	// Receive
			{
				pText =
					"\n"
					"  -r, --record\t<filename>\tRecord receiver input to a file\n"
					"  -l, --length\t<nr of MB>\tRecord file length\n"
					"  --frequency\t<Freq>\t\tRF Frequency in MHz (ex. 107.45)\n"
					"  --symbolrate\t<MSym/sec>\tNr of Symbols/sec (ex. 5457800)\n"
					"  --bandwidth\t<BW>\t\tbandwidth in Megahertz:\n"
					"  \t\t8_MHz, 7_MHz, 6_MHz, 5_MHz\n"
					"  --inversion\t<ON/OFF>\tUse spectral inversion: ON, OFF\n"
					"  --modulation\t<mod type>\tModulation type:\n"
					"  \t\tQPSK, QAM_16, QAM_32, QAM_64, QAM_128, QAM_256\n"
					"\n"
					"Example DVB-T:\n \t%s --record teststream.ts --length 50 --frequency 522.0 --bandwidth 8_MHz\n"
					"Example DVB-C annexA:\n \t%s --record teststream.ts --length 50 --frequency 658.0 --symbolrate 6900000\n"
					"Example DVB-C annexB:\n \t%s --record teststream.ts --length 50 --frequency 658.0 --modulation QAM_64\n"
					;
			}
		    break;

		case '\n':
		case '\r':
			break;

		default:
			bRepeat = false;
		    break;
		}
		if (pText)
			printf(pText, ProgName, ProgName, ProgName);
	}
}
