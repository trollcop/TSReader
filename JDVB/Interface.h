#ifndef INTERFACE_H___
#define INTERFACE_H___

typedef int BOOL;
#define		LNBTYPE_NORMAL			0
#define		LNBTYPE_UNIVERSAL		1
#define		LNBTYPE_CUSTOM			2
#define		SETPID_BOTH_STREAM		0
#define		SETPID_SIGNAL_STREAM	1
#define		SETPID_PREVIEW			2
#define		SETPID_TS_STREAM		3



//-----------------------------------------------------------------
//Constants used in strutures
#define		CA_INFO_MAX_COUNT		8
//-----------------------------------------------------------------

typedef void (__stdcall *PidFilterFunc) (unsigned char *data);

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


typedef enum GraphState
{
	GS_Play = 0,
	GS_Pause = 1,
	GS_Stop = 2
}GraphState;

#include <streams.h>

//-----------------------------------------------------------------

DECLARE_INTERFACE_(IDVBSource, IUnknown) {

    // Compare these with the functions in class CAsyncFilter in asyncflt.h


	STDMETHOD(set_LNBType)
		( THIS_
		  int	LNBType,	//[in] LNB Type, such as Normal, Universal, Custom
		  int	LNB1,		//[in] if LNB Type is Custom, fill this var
		  int	LNB2		//[in] if LNB Type is Custom, fill this var
		) PURE;

	STDMETHOD(get_LNBType)
		( THIS_
		  int	*LNBType,	//[out] LNB Type, such as Normal, Universal, Custom
		  int	*LNB1,		//[out] it is valid only when LNB Type is Custom
		  int	*LNB2		//[out] it is valid only when LNB Type is Custom
		) PURE;

	STDMETHOD(LockChannel)
		( THIS_
		  long	Frequency,	//[in]  frequency, MHz
		  long	SymbolRate,	//[in]  symbolRate, Ksps
		  int	HV,			//[in]  1 for Horizotal, 0 for Vertical
		  int	Tone,		//[in]  22KHz Tone is enable or disable
		  int	DiSEqC		//[in]  DiSEqC switch
		) PURE;

	STDMETHOD(SetPid)
		( THIS_
		  int	SetType,	//[in]  type for control stream

		  long	VideoPid1,	//[in]  video stream pid
		  long	AudioPid1,	//[in]  audio stream pid
		  bool	Scrambled,	//[in]  the program is scrambled or not
		  long  PMTPid,		//[in]  if Scrambled is valid, it must be filled
		  long  ProgramNumber,
							//[in]  if Scrambled is valid, it must be filled
		  long  VideoPid2,
		  long	AudioPid2
		) PURE;

	STDMETHOD(ScanChannel) () PURE;
		  
	STDMETHOD(QueryScanStatus)
		( THIS_
		  int	*Status,	//[out] return the status of scanning
							//		0: scanning
							//		1: scanning complete
							//		2: occur error and scanning exist
							//		others: not valid value
		  int	*ProgramNum //[out] when Status return 1, this var is valid
		) PURE;

	STDMETHOD(ReadChannel)
		( THIS_
		  ProgramValue	*pProgramValue,
							//[out] the size according to ProgramNum
		  ProgramValueSenior
						*pProgramValueSenior
							//[out] the size according to ProgramNum
		) PURE;

	STDMETHOD(GetSignalState)
		( THIS_
		  bool	*LockFlag,	//[out] return lock or unlock flag
		  int	*Strength,	//[out] return the signal's strength, 0 - 100
		  int	*Quality	//[out] return the signal's quality, 0 - 100
		) PURE;

	STDMETHOD(StartRecording)
		( THIS_
		  char	*FileName,	//[in]  if the FileName is NULL, "temp.mpg" is the default setting
		  long	VPid,		//[in]	video pid for the program
		  long	APid		//[in]	audio pid for the program
		) PURE;

	STDMETHOD(StopRecording) () PURE;
	  
	STDMETHOD(GetEPGCount) 
		( THIS_
		  int	*EPGNum		//[out]  current EPG items found 
		) PURE;

	STDMETHOD(GetEPGList)
		( THIS_
		  EPGValue *pEPGValue
							//[out]  the size according to the EPGNum
		) PURE;

	STDMETHOD(ResetEPG) () PURE;
		  
	STDMETHOD(InitTeletextDataBuffer)
		( THIS_
		  unsigned char *pBuf,
							//[in]   teletext data buffer(188*n)
		  long	BufSize,	//[in]	 buffer size
		  unsigned long *WritePtr	
							//[in]	 write pointer(0 to BufSize)
		) PURE;

	STDMETHOD(SetTeletextDataTempFilePath) 
		( THIS_
		  char *FilePath	//[in]	 save the teletext seperate file(include unham)
		) PURE;

	STDMETHOD(InitSectionDataBuffer)
		( THIS_
		  unsigned char *pBuf,
							//[in]   data buffer
		  long	BufSize,	//[in]	 buffer size
		  unsigned long *WritePtr,	
							//[in]	 write pointer(0 to BufSize)
		  long Pid			//[in]	 pid for the payload you want
		) PURE;

	STDMETHOD(SetSectionPid)
		( THIS_
		  long	Pid			//[in]   the new pid for the payload you want
		) PURE;

	STDMETHOD(SetTSFile)
		( THIS_
		  char	*FileName	//[in]	 if the FileName is NULL, "temp.dat" is the default setting
		) PURE;

	STDMETHOD(TSResume) () PURE;

	STDMETHOD(AddPidFilter)
		( THIS_
		  PidFilterFunc PidFilter,
							//[in]	 the function pointer for filter
		  long	Pid			//[in]	 Pid for the filter
		) PURE;

	STDMETHOD(DelPidFilter)
		( THIS_
		  PidFilterFunc PidFilter,
							//[in]	 the function pointer for filter
		  long	Pid			//[in]	 Pid for the filter
		) PURE;  

	STDMETHOD(ClearAllPidFilter) () PURE;
};

//
// IDVBDishDrv
//
DECLARE_INTERFACE_(IDVBDishDrv, IUnknown) {
	STDMETHOD(DriveEast)
		( THIS_
	  	  int Step			//[in]	 Step for drive
		) PURE;  

	STDMETHOD(DriveWest)
		( THIS_
		  int Step			//[in]	 Step for drive
		) PURE;  

	STDMETHOD(StopMotor) () PURE;

	STDMETHOD(SetEastLimit) () PURE;

	STDMETHOD(SetWestLimit) () PURE;

	STDMETHOD(CancelLimit) () PURE;

	STDMETHOD(EnableLimit) () PURE;

	STDMETHOD(GotoZero) () PURE;

	STDMETHOD(GotoSat)
		( THIS_
		  double Degree,	//[in]	 Degree of the satellite(0 ~ 360)
		  double Longitude,	//[in]	 Longitude of User located
		  double Latitude,	//[in]	 Latitude of User located
		  double *Degrees,	//[out]	 Degrees dish running indeed
		  double *Elevation	//[out]	 Elevation of the dish
		) PURE;

	STDMETHOD(StoreSatPosition)
		( THIS_
		  int n				//[in]	 Position serial number 0 ~ 255
		) PURE;

	STDMETHOD(GotoSatPosition)
		( THIS_
		  int n				//[in]	 Position serial number 0 ~ 255
		) PURE;
};

//
// IDVBCIMenu
//
DECLARE_INTERFACE_(IDVBCIMenu, IUnknown) {
	STDMETHOD(GetCAMState)
		( THIS_
		  int	*CAM_Exist_Flag,	
							//[out]	 Flag for CAM exist or not
		  int	*MMI_Info_Flag
							//[out]	 Flag for MMI Info need to respond
		) PURE;

	STDMETHOD(GetAppInfo)
		( THIS_
		  App_Info *pAppInfo
							//[out]  Return App Info
		) PURE;

	STDMETHOD(InitMMI) () PURE;

	STDMETHOD(GetMMI) 
		( THIS_
		  MMI_Info *pMMIInfo,
		  int *pType
		) PURE;

	STDMETHOD(Answer)
		( THIS_
		  MMI_Info *pMMIInfo,
		  int Type
		 ) PURE;

	STDMETHOD(CloseMMI) () PURE;
};

//
//IAnalogVideoSet
//
DECLARE_INTERFACE_(IAnalogVideoSet, IUnknown) {
	STDMETHOD(GetVideoFormat)
		( THIS_
		  int *Type			//[out] return Video Format
		  					//1 NTSC_M 
							//2 NTSC_JAPAN
							//3 PAL_BDGHI
							//4 PAL_M
							//5 PAL_N
							//6 SECAM
		) PURE;

	STDMETHOD(SetVideoFormat)
		( THIS_
		  int Type			//[in]  Video Format
		  					//1 NTSC_M 
							//2 NTSC_JAPAN
							//3 PAL_BDGHI
							//4 PAL_M
							//5 PAL_N
							//6 SECAM
		) PURE;

	STDMETHOD(GetColorBitCount)
		( THIS_
		  int *BitCount		//[out] return ColorBitCount, 
							//such as 8,16,32bits
		) PURE;

	STDMETHOD(SetColorBitCount)
		( THIS_
		  int BitCount		//[in]  ColorBitCount, 
							//such as 8,16,32bits
		) PURE;

	STDMETHOD(GetColorControl)
		( THIS_
		  long *Brightness,	//[out] return Brightness value 0 ~ 255
		  long *Contrast,	//[out] return Contrast value -255 ~ 255
		  long *HUE,		//[out] return HUE value -255 ~ 255	
		  long *Saturation	//[out] return Saturation value 0 ~ 255
		) PURE;

	STDMETHOD(SetBrightness)
		( THIS_
		  long Brightness	//[in]  Brightness value 0 ~ 255
		) PURE;

	STDMETHOD(SetContrast)
		( THIS_
		  long Contrast		//[in]  Contrast value -255 ~ 255
		) PURE;

	STDMETHOD(SetHUE)
		( THIS_
		  long HUE			//[in]  HUE value -255 ~ 255
		) PURE;

	STDMETHOD(SetSaturation)
		( THIS_
		  long Saturation	//[in]  Saturation value 0 ~ 255
		) PURE;
};


#endif