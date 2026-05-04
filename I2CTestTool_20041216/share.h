#ifndef __SHARE__
#define __SHARE__

#define		CARDTYPE_DST_FTA_CARD	0
#define		CARDTYPE_DST_CI_CARD	1
#define		CARDTYPE_DCT_FTA_CARD	2
#define		CARDTYPE_DCT_CI_CARD	3

#define		LNBTYPE_NORMAL			0
#define		LNBTYPE_UNIVERSAL		1
#define		LNBTYPE_CUSTOM			2

#define		STATUS_SCANNING			0
#define		STATUS_SCAN_COMPLETE	1
#define		STATUS_SCAN_ERROR		2

#define		SETPID_BOTH_STREAM		0
#define		SETPID_SIGNAL_STREAM	1
#define		SETPID_PREVIEW			2
#define		SETPID_TS_STREAM		3

#define		CA_INFO_MAX_COUNT		8

#define		VIDEOFORMAT_NTSC_M		1
#define		VIDEOFORMAT_NTSC_JAPAN	2
#define		VIDEOFORMAT_PAL_BDGHI	3
#define		VIDEOFORMAT_PAL_M		4
#define		VIDEOFORMAT_PAL_N		5
#define		VIDEOFORMAT_SECAM		6

#define		HW_MAX_COUNT			3

typedef enum _HW_TYPE {
    HW_ERROR,
	HW_DST_FTA_CARD,
    HW_DST_CI_CARD,
    HW_DCT_CI_CARD,
	HW_DTT_FTA_CARD,
	HW_DTT_AD_CARD,
} HW_TYPE;

typedef struct PCIConfigStruct {
    unsigned short	VendorID;
    unsigned short  DeviceID;
    unsigned short  Command;
    unsigned short  Status;
    unsigned short  RevisionID;
    unsigned short  SubVendorID;
    unsigned short  SubSystemID;
}PCIConfig;

typedef struct PCIConfig2Struct {
    USHORT  VendorID;
    USHORT  DeviceID;
    USHORT  Command;
    USHORT  Status;
    USHORT  RevisionID;
    USHORT  SubVendorID;
    USHORT  SubSystemID;
	USHORT	MaxCardNum;
}PCIConfig2;

typedef struct ProgramValueStruct
{
	long Frequency;
	long SymbolRate;
	int  HV;
	long Video_Pid;
	long Audio_Pid[5];
	char Language[5][8];
	long PMT_PID;
	long Program_Number;
	bool Scrambled;
	char Program_Name[256];
	char Service_Provider_Name[256];
	struct ProgramValueStruct *Next;
}ProgramValue;

typedef struct EPGValueStruct
{
	char start_time[32];
	char duration[32];
	char event_name_char[256];
	char text_char[256];
	struct EPGValueStruct *Next;
}EPGValue;

typedef struct CAInfoStruct
{
	long CA_system_ID;
	long ECM;
	long EMM;
}CAInfo;

typedef struct ProgramValueSeniorStruct
{
	char	Program_Name[30];
	char	Service_Provider_Name[30];
	long	Service_ID;
	long	Service_Type;
	long	Transponder_Stream_ID;
	long	PMT_Pid;
	long	PCR_Pid;
	long	Video_Pid;
	long	Audio_Pid;
	bool	Scrambled;
	long	Teletext_Pid;
	long	CA_system_ID;
	long	CA_Pid;
	int		CA_Info_Count;
	CAInfo	CA_Info[CA_INFO_MAX_COUNT];
	struct ProgramValueSeniorStruct *Next;
}ProgramValueSenior;

typedef struct MMIInfoStruct
{
	char Header[256];
	char SubHeader[256];
	char ButtomLine[256];
	char MenuItem[9][42];
	int  ItemCount;

	BOOL EnqFlag;

	BOOL Blind_Answer;
	int  Answer_Text_Length;
	char Prompt[256];

	int  Answer;
	char AnswerStr[256];
}MMI_Info;

typedef struct AppInfoStruct
{
	unsigned int app_type;
	unsigned int application_manufacture;
	unsigned int manufacture_code;
	char application_info[64];
}App_Info;

// Tuner data define
typedef struct TunerDataStruct
{
	unsigned char	address;			// 0
	unsigned char	frequencyMSB; 		// 1
	unsigned char	frequencyLSB;		// 2
	unsigned char	tunerStep;			// 3
	unsigned char	symbolRateHSB;		// 4
	unsigned char	symbolRateMSB;		// 5
	unsigned char	symbolRateLSB;		// 6
	unsigned char	flag;				// 7
	unsigned char	checkSum;			// 8
}TunerData;

// using it for DST_GET_EEPROM_INFO
typedef struct _EEprom_Info_ {
	unsigned char address;
	unsigned char data;
	unsigned char flag;
} EEprom_Info, *P_EEprom_Info;


#endif
