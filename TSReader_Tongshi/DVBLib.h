#if !defined(AFX_DVBLIB_H__4B4F0082_9A4D_43DC_9EBE_95705AC956C9__INCLUDED_)
#define AFX_DVBLIB_H__4B4F0082_9A4D_43DC_9EBE_95705AC956C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DVBLib.h : header file
//
#define POWER			0
#define FREQ			1
#define DUALBAND		2

#define PIDTYPE_VIDEO		0x2000
#define PIDTYPE_IPDATA		0x4000
#define PIDTYPE_RAWDATA		0x8000

//#define SIZE_DATABUFLEN		3948
#define SIZE_DATABUFLEN		20680
//#define SIZE_DATABUFLEN		39480
#define SIZE_FRAMEBLOCK		188
#define SIZE_NAME			16
/////////////////////////////////////////////////////////////////////////////
// CDVBLib window
typedef struct tagTunerParam{
	DWORD dwLowFreq;						//Freq
	DWORD dwSymbolBaud;						//Symbol Rate
	DWORD dwLNBHFreq;						//LNB H Freq (default)
	DWORD dwLNBVFreq;						//LNB V Freq
	DWORD BitVolt		:		1;			//same with SatelliteParem
	DWORD BitDualBand	:		1;
	DWORD BitF22		:		1;
	DWORD BitTone		:		1;
	DWORD BitDiseqc		:		1;
	DWORD BitUniversal	:		1;			
	DWORD BitNoVolt		:		1;
	DWORD BitReserve	:		1;
	DWORD ToneCh		:		1;
	DWORD DiseqcCh		:		7;
	DWORD flagPolar		:	    1;			//1 H         0 V
	DWORD flagChanged	:	    1;			//is Changed，used in auto Tuner
	DWORD flagShareFail	:		1;			//share Tuner false flag
	DWORD flagReserver	:       13;
}TunerParam, *PTunerParam;

typedef struct tagQAMParam{
	double dFrequence;						//Channel Freg
	double dSymbolBaud;						//Symbol Rate
	DWORD dwQAMType		:		3;			//0        QAM16;     1          QAM32;
											//2        QAM64;     3          QAM128;
											//5        QAM256;
	DWORD flagChanged	:		1;			//is changed useing auto tuner
	DWORD flagShareFail :		1;			//share tuner false
	DWORD flagReserver	:       27;
}QAMParam, *PQAMParam;

typedef struct InitParam{
	DWORD flagType		:		1;			//Receive Adaptor Type
	DWORD flagLock		:		1;			//Current Tuner Status
	DWORD flagSimulate  :		1;			//Current Receiving Status
	DWORD flagChanged	:		1;			//Channel Changed Flag
	DWORD flagFirstRun	:		1;			//First Run Flag
	DWORD flagUpdateCh	:		1;			//Operation Channel Flag
	DWORD flagUSB		:		1;			//is USB Adaptor
	DWORD flagDVBT		:		1;			//is DVB-T，Only effect when flagType = 0
	DWORD flagReserve	:		8;			//Reserved
	DWORD maskSatellite :		1;			//is szDBSName[] enable ? 
	DWORD maskReserve	:       15;			//Reserved
	char szChannelName[SIZE_NAME];			//Channel Name
	char szDBSName[SIZE_NAME];				//Satellite Name // when maskSatellite = 1
}InitParam, *PInitParam;

typedef struct tagPIDSet
{
	DWORD wRecPID			:	 16;
	DWORD wRecType			:	 16;
}PIDSet, * PPIDSet;

class CDVBLib
{
// Member Value
protected:
	HMODULE m_hDVBLib;

	typedef VOID (* VOIDFUNC1)(DWORD);
	typedef VOID (* VOIDFUNC3)(DWORD,DWORD,DWORD);
	typedef VOID (* VOIDFUNC4)(DWORD,DWORD,DWORD,DWORD);

	typedef DWORD (* DWORDFUNC0)();
	typedef DWORD (* DWORDFUNC1)(DWORD);
	typedef DWORD (* DWORDFUNC2)(DWORD,DWORD);
	typedef DWORD (* DWORDFUNC3)(DWORD,DWORD,DWORD);
	typedef DWORD (* DWORDFUNC4)(DWORD,DWORD,DWORD,DWORD);

//////MyTheatre Useing Functions//////////////////////////////////////////
	DWORDFUNC0 m_pGetDevCount;
	DWORDFUNC3 m_pQueryDevice;
	DWORDFUNC1 m_pOpenDevice;
	DWORDFUNC2 m_pSendRawDiSEqCMsg;
	DWORDFUNC4 m_pLockTransponder;
	DWORDFUNC3 m_pGetSignal;
	DWORDFUNC2 m_pReadBuffer;
	DWORDFUNC2 m_pGetPIDList;
	DWORDFUNC1 m_pPassFullStream;
	DWORDFUNC1 m_pAddPID;
	DWORDFUNC1 m_pDeletePID;
	DWORDFUNC0 m_pDeleteAllPIDs;
//////////////////////////////////////////////////////////////////////////
	DWORDFUNC2 m_pGetAndyDebugInfo;
//////////////////////////////////////////////////////////////////////////


	VOIDFUNC1 m_pExitDrv,
			  m_pClearDrvBuffer,
			  m_pSetCanTunerInDll;
			  
	VOIDFUNC3 m_pPIDAlwaysQAM;
	VOIDFUNC4 m_pPIDAlwaysDBS;
	
	DWORDFUNC0 m_pGetDrvVer,
		       m_pGetDrvBuildNo,
			   m_pGetDllVer,
			   m_pGetDllBuildNo,
			   m_pCanModifyTuner,
			   m_pTunerInputStatus,
			   m_pTunerSNStatus,
			   //Build 1.10	
			   m_pTestTunerLock;

	DWORDFUNC1 m_pInitDrv,
		       m_pTunerDBS,
			   //1.10
			   m_pQueryDrv,
			   m_pTunerQAM,
			   m_pSetParamInfo,
			   //1.20
			   m_pAutoSearch;

	DWORDFUNC2 m_pQueryRecPID,
   			   //1.10
			   m_pQuerySatellite,			   
			   m_pQueryStation,
			   m_pImportParam,
			   m_pExportParam;
			   	
	DWORDFUNC3 m_pQueryChannel,
			   m_pReadDataStream,
			   m_pRecPID,
		       m_pStopPID,
			   //1.10
			   m_pQueryParamQAM;

	DWORDFUNC4	m_pQueryParamDBS;

// Construct and Deconstruct
public:
	CDVBLib();
	virtual ~CDVBLib();
// Main Function
public:
	int GetAndyDebugInfo(PBYTE pBuff,int nBufLen);
	BOOL TSDVB0_PassFullStream(BOOL Full);
	int TSDVB0_GetPIDList(WORD* List, int Len);
	BOOL TSDVB0_DeleteAllPIDs(void);
	BOOL TSDVB0_DeletePID(WORD PID);
	BOOL TSDVB0_AddPID(WORD PID);
	int  TSDVB0_ReadBuffer(BYTE* Data, int Len);
	BOOL TSDVB0_GetSignal(BOOL* Locked, DWORD* Quality, DWORD* Strength);
	BOOL TSDVB0_LockTransponder(DWORD dwFreq, DWORD dwSymbRate, DWORD Polarisation, BOOL bF22KHz);
	BOOL TSDVB0_SendRawDiSEqCMsg(BYTE* Data, int Len);
	BOOL TSDVB0_OpenDevice(int Num);
	BOOL TSDVB0_QueryDevice(int nIndex,LPVOID Info,HWND hWnd);
	int  TSDVB0_GetDevCount(void);
	///////////////////////////////////////////////////
	//Lib Status Test
	bool IsLibLoadOK();
	
	///////////////////////////////////////////////////
	//Version Infomation
	DWORD GetDrvVer();
	DWORD GetDrvBuildNo();
	DWORD GetDllVer();
	DWORD GetDllBuildNo();

	///////////////////////////////////////////////////
	//Driver Function
	BOOL InitDrv(HWND hWnd);
	VOID ExitDrv(HWND hWnd);

	///////////////////////////////////////////////////
	//Tuner Function
	BOOL CanModifyTuner();
	BOOL TunerDBS(LPVOID lParam);				//定义一个结构
	DWORD TunerInputStatus();
	DWORD TunerSNStatus();

	///////////////////////////////////////////////////
	//Receive Function
	DWORD RecPID(HWND hWnd, int nCount, PPIDSet psuArrPID);
	DWORD StopPID(HWND hWnd, int nCount, PPIDSet psuArrPID);
	DWORD QueryRecPID(HWND hWnd, PPIDSet psuArrPID);
	DWORD ReadDataStream(HWND hWnd, DWORD dwBufLen, PBYTE pDataBuf);
	VOID ClearDrvBuffer(int nType);

	///////////////////////////////////////////////////
	//Build 1.10
	DWORD TestTunerLock();
	BOOL QueryDrv(PInitParam lParam);
	BOOL TunerQAM(PQAMParam lParam);	

	DWORD SetParamInfo(BOOL bType);				//active a dialog
	DWORD QuerySatellite(int nCount, char ** pStrArray);
	DWORD QueryChannel(int nCount, char * pSatellite, char ** pStrArray);
	DWORD QueryStation(int nCount, char ** pStrArray);
	BOOL QueryParamDBS(PTunerParam lParam, char * pSatellite, \
						  char * pChannel, BOOL bSearchLock);
	BOOL QueryParamQAM(PQAMParam lParam, char * pStation, BOOL bSearchLock);

	BOOL ImportParam(int nType, char * szIniFile);
	BOOL ExportParam(int nType, char * szIniFile);

	VOID PIDAlwaysDBS(BOOL bMode,PUINT puValue,char * pSatellite,char * pChannel);
	VOID PIDAlwaysQAM(BOOL bMode,PUINT puValue,char * pStation);
	VOID SetCanTunerInDll(BOOL bValue);
	///////////////////////////////////////////////////
	//Build 1.20
	DWORD AutoSearch(BOOL bType);				//auto search channel
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DVBLIB_H__4B4F0082_9A4D_43DC_9EBE_95705AC956C9__INCLUDED_)


//////////////////////////////////////////////////////////////////////////
/*
MyTheatre函数接口说明

int GetDevCount(void)
原文：
   How many your cards are in PC. If your
   driver doesn't support more than one card then this function can
   simple return 1 or 0.
注释：
    此函数最先被调用；获取在主机上所具有的DVB卡/盒的数目；通过查询注册表中记录的设备ID号来匹配实现，
    返回找到卡的个数。
    ps： 目前DVBNET中本身就有选择打开设备的选择，因此当USB和PCI同时插上时也返回找到设备个数为1。

BOOL QueryDevice(LPVOID Info);
原文：
   structure defined by yourself. This structure must include at
   least type of card (DVB-S/C/T). Other parameters of this structure
   not limited.
   important note: this function will be called BEFORE opening your
   card (OpenDevice).
注释：
    此函数在GetDevCount(void)之后调用；获取需要打开设备的类型，即DVB-S/C/T；
　　输入的参数LPVOID Info由DVBNet所决定，目前为PInitParam。


BOOL OpenDevice(int Num);
原文：   
   Num: number of your card. If only one card is supported then ignore
   this parameter.
注释：
   此函数在QueryDevice(LPVOID Info)之后调用；用来初始化接收卡，输入参数num是对应于GetDevCount的
需要打开的设备号。


BOOL SendRawDiSEqCMsg(BYTE* Data, int Len);
原文：for DVB-S models only.
注释：在DVB－S情况下发送Diseqc命令。
      BYTE *Data 是Diseqc命令字串的缓冲区指针，
      int LEN 是Diseqc命令字串的长度
   
BOOL LockTransponder(DWORD dwFreq, DWORD dwSymbRate, DWORD Polarisation, BOOL bF22KHz)
原文：
   for DVB-S:
   dwFreq is already tuner frequency (TranspFreq-LOF)
   all parameters used.
   notes: FEC assumed as AUTO.

   for DVB-T:
   used parameters: dwFreq, dwSymbRate as bandwidth(6,7,8)
   other parameters must be ignored

   for DVB-C:
   used parameters: dwFreq, dwSymbRate;
   if your card doesn't able autodetect QAM value then parameter
   Polarisation can be used as QAM parameter.
   other parameters must be ignored
注释：
   此函数为MyTheatre调台时第一个调用；对于DVB-S DVB-T DVB-C信号，调台参数的解释如下：
   DVB-S:
         dwFreq : Khz,中频频率
         dwSymbolRate : Khz,符号率
         Polarisation : 信号极化方向 0：垂直 ；非零：水平
         bF22Khz: 22Khz导频是否打开  true :打开; false:关闭
   DVB-C:
         dwFreq: Khz, 输入射频频率
         dwSymbolRate : Khz ,符号率
         Polarisation: 调制模式，ps：此参数对我们无效，我们可以支持QAMAuto
   DVB-T：
         dwFreq: Khz,输入射频频率
         dwSymbolRate: 6,7,8 整数                
         其他参数无效。  
   
   
BOOL GetSignal(BOOL* Locked, DWORD* Quality, DWORD* Strength)
原文：
   Locked: 1 - locked, 0 - not locked
   Quality: 0..100 quality of signal
   Strength: 0..100 strength of signal
注释：
    此函数为获取信号状态。
    
 
int ReadBuffer(BYTE* Data, int Len)
原文：
   return value is length of actually read data, less or equals to Len
   parameter.
注释：
   读取TS流数据的函数。
   Data为数据缓冲区指针。
   Len 为需要读取的数据长度
   返回值为实际读取到的数据长度


BOOL AddPID(WORD PID);
BOOL DeletePID(WORD PID);
BOOL DeleteAllPIDs(void);
上述三个函数为PID控制

int GetPIDList(WORD* List, int Len)

获取已经接收的TS流PID列表，List为列表指针，对Len的定义为不是很清楚，返回值为已经接收的PID个数


BOOL  PassFullStream(BOOL Full);
原文：    Full: true - switch PID filter off (full transport stream);
          false - switch PID filter on;
注释：
      打开或关闭所有的PID。
*/
//////////////////////////////////////////////////////////////////////////
