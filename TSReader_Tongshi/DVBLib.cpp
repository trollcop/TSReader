// DVBLib.cpp : implementation file
//

#include <windows.h>
#include "DVBLib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define MYTEST
/////////////////////////////////////////////////////////////////////////////
// CDVBLib

CDVBLib::CDVBLib()
{	
	m_hDVBLib = LoadLibrary("dvbnet.dll");
	if(m_hDVBLib)
	{		
//////////////////////////////////////////////////////////////////////////
		m_pGetDevCount = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"GetDevCount");
		m_pQueryDevice = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"QueryDevice");
		m_pOpenDevice = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"OpenDevice");
		m_pSendRawDiSEqCMsg = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"SendRawDiSEqCMsg");
		m_pLockTransponder = (DWORDFUNC4)GetProcAddress(m_hDVBLib,"LockTransponder");
		m_pGetSignal = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"GetSignal");
		m_pReadBuffer = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"ReadBuffer");
		m_pGetPIDList = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"GetPIDList");
		m_pPassFullStream = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"PassFullStream");
		m_pAddPID = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"AddPID");
		m_pDeletePID = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"DeletePID");
		m_pDeleteAllPIDs = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"DeleteAllPIDs");
//////////////////////////////////////////////////////////////////////////
		m_pGetAndyDebugInfo = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"GetAndyDebugInfoOut");
//////////////////////////////////////////////////////////////////////////

		m_pExitDrv = (VOIDFUNC1)GetProcAddress(m_hDVBLib,"ExitDrv");
		m_pClearDrvBuffer = (VOIDFUNC1)GetProcAddress(m_hDVBLib,"ClearDrvBuffer");
	
		m_pGetDrvVer = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"GetDrvVer");
		m_pGetDrvBuildNo = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"GetDrvBuildNo");
		m_pGetDllVer = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"GetDllVer");
		m_pGetDllBuildNo = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"GetDllBuildNo");
		m_pCanModifyTuner = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"CanModifyTuner");
		m_pTunerInputStatus = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"TunerInputStatus");
		m_pTunerSNStatus = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"TunerSNStatus");

		m_pInitDrv = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"InitDrv");
		m_pTunerDBS = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"TunerDBS");

		m_pQueryRecPID = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"QueryRecPID");
		m_pReadDataStream = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"ReadDataStream");
		m_pRecPID = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"RecPID");
		m_pStopPID = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"StopPID");

		if(GetDrvBuildNo() >= 110)
		{			
			m_pTestTunerLock = (DWORDFUNC0)GetProcAddress(m_hDVBLib,"TestTunerLock");
			m_pQueryDrv = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"QueryDrv");
			m_pTunerQAM = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"TunerQAM");
			m_pSetParamInfo = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"SetParamInfo");

			m_pQuerySatellite = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"QuerySatellite");
			m_pQueryStation = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"QueryStation");
			m_pImportParam = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"ImportParam");
			m_pExportParam = (DWORDFUNC2)GetProcAddress(m_hDVBLib,"ExportParam");
			
			m_pQueryParamQAM = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"QueryParamQAM");
			m_pQueryChannel = (DWORDFUNC3)GetProcAddress(m_hDVBLib,"QueryChannel");

			m_pQueryParamDBS = (DWORDFUNC4)GetProcAddress(m_hDVBLib,"QueryParamDBS");
		    
			m_pPIDAlwaysDBS = (VOIDFUNC4)GetProcAddress(m_hDVBLib,"PIDAlwaysDBS");
			m_pPIDAlwaysQAM = (VOIDFUNC3)GetProcAddress(m_hDVBLib,"PIDAlwaysQAM");
			m_pSetCanTunerInDll = (VOIDFUNC1)GetProcAddress(m_hDVBLib,"SetCanTunerInDll");
			//1.20
			m_pAutoSearch = (DWORDFUNC1)GetProcAddress(m_hDVBLib,"AutoSearch");
		}
		else
		{
			m_pTestTunerLock = NULL;
			m_pQueryDrv = NULL;
			m_pTunerQAM = NULL;
			m_pSetParamInfo = NULL;

			m_pQuerySatellite = NULL;
			m_pQueryChannel = NULL;
			m_pQueryStation = NULL;
			m_pImportParam = NULL;
			m_pExportParam = NULL;

			m_pQueryParamQAM = NULL;
			m_pQueryParamDBS = NULL;
			m_pPIDAlwaysDBS = NULL;
			m_pPIDAlwaysQAM = NULL;
			m_pAutoSearch = NULL;
		}
	}
}

CDVBLib::~CDVBLib()
{
	if(m_hDVBLib)
	{
		ExitDrv(NULL);
		FreeLibrary(m_hDVBLib);
		m_hDVBLib = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////
// CDVBLib message handlers

bool CDVBLib::IsLibLoadOK()
{
	return (m_hDVBLib != NULL);
}
	
///////////////////////////////////////////////////
//Version Infomation
DWORD CDVBLib::GetDrvVer()
{
	if(m_hDVBLib && m_pGetDrvVer)
		return (*m_pGetDrvVer)();
	else
		return 0xffffffff;
}

DWORD CDVBLib::GetDrvBuildNo()
{
	if(m_hDVBLib && m_pGetDrvBuildNo)
		return (*m_pGetDrvBuildNo)();
	else
		return 0xffffffff;
}

DWORD CDVBLib::GetDllVer()
{
	if(m_hDVBLib && m_pGetDllVer)
		return (*m_pGetDllVer)();
	else
		return 0xffffffff;
}

DWORD CDVBLib::GetDllBuildNo()
{
	if(m_hDVBLib && m_pGetDllBuildNo)
		return (*m_pGetDllBuildNo)();
	else
		return 0xffffffff;
}

///////////////////////////////////////////////////
//Driver Function
BOOL CDVBLib::InitDrv(HWND hWnd)
{
	if(m_hDVBLib && m_pInitDrv)
		return (*m_pInitDrv)((DWORD)hWnd);
	else
		return FALSE;
}

VOID CDVBLib::ExitDrv(HWND hWnd)
{
	if(m_hDVBLib && m_pExitDrv)
		(*m_pExitDrv)((DWORD)hWnd);
}

///////////////////////////////////////////////////
//Tuner Function
BOOL CDVBLib::CanModifyTuner()
{
	if(m_hDVBLib && m_pCanModifyTuner)
		return (*m_pCanModifyTuner)();
	else
		return FALSE;
}

BOOL CDVBLib::TunerDBS(LPVOID lParam)				//定义一个结构
{
	if(m_hDVBLib && m_pTunerDBS)
		return (*m_pTunerDBS)((DWORD)lParam);
	else
		return FALSE;
}

DWORD CDVBLib::TunerInputStatus()
{
	if(m_hDVBLib && m_pTunerInputStatus)
		return (*m_pTunerInputStatus)();
	else
		return 0xffffffff;
}

DWORD CDVBLib::TunerSNStatus()
{
	if(m_hDVBLib && m_pTunerSNStatus)
		return (*m_pTunerSNStatus)();
	else
		return 0xffffffff;
}

///////////////////////////////////////////////////
//Receive Function
DWORD CDVBLib::RecPID(HWND hWnd, int nCount, PPIDSet psuArrPID)
{
#ifdef MYTEST
	char szDebug[200];
	wsprintf(szDebug,"REC: Count = %d,Type = %d,PID = %d",\
		nCount,psuArrPID->wRecType,psuArrPID->wRecPID);
	OutputDebugString(szDebug);
	OutputDebugString("\r\n");
#endif
	if(m_hDVBLib && m_pRecPID)
		return (*m_pRecPID)((DWORD)hWnd,
				(DWORD)nCount,
				(DWORD)psuArrPID);
	else
		return 0xffffffff;
}

DWORD CDVBLib::StopPID(HWND hWnd, int nCount, PPIDSet psuArrPID)
{
#ifdef MYTEST
	char szDebug[200];
	wsprintf(szDebug,"STOP: Count = %d,Type = %d,PID = %d",\
		nCount,psuArrPID->wRecType,psuArrPID->wRecPID);
	OutputDebugString(szDebug);
	OutputDebugString("\r\n");
#endif
	if(m_hDVBLib && m_pStopPID)
		return (*m_pStopPID)((DWORD)hWnd,
				(DWORD)nCount,
				(DWORD)psuArrPID);
	else
		return 0xffffffff;
}

DWORD CDVBLib::QueryRecPID(HWND hWnd, PPIDSet psuArrPID)
{
	if(m_hDVBLib && m_pQueryRecPID)
		return (*m_pQueryRecPID)((DWORD)hWnd,(DWORD)psuArrPID);
	else
		return 0xffffffff;
}

DWORD CDVBLib::ReadDataStream(HWND hWnd, DWORD dwBufLen, PBYTE pDataBuf)
{
	if(m_hDVBLib && m_pReadDataStream)
		return (*m_pReadDataStream)((DWORD)hWnd,dwBufLen,(DWORD)pDataBuf);
	else
		return 0xffffffff;		
}

VOID CDVBLib::ClearDrvBuffer(int nType)
{
	if(m_hDVBLib && m_pClearDrvBuffer) (*m_pClearDrvBuffer)(nType);
}
/////////////////////////////////////////////////
//Build 1.10
BOOL CDVBLib::QueryDrv(PInitParam lParam)
{
	if(m_hDVBLib && m_pQueryDrv)
		return (*m_pQueryDrv)((DWORD)lParam);
	else
		return FALSE;		
}

DWORD CDVBLib::TestTunerLock()
{
//0     	调谐正常
//1         驱动没有加载
//2			I2C总线忙
//3         没有锁定信号
	if(m_hDVBLib && m_pTestTunerLock)
		return (*m_pTestTunerLock)();
	else
		return 1;
}

BOOL CDVBLib::TunerQAM(PQAMParam lParam)
{
	if(m_hDVBLib && m_pTunerQAM)
		return (*m_pTunerQAM)((DWORD)lParam);
	else
		return FALSE;		
}

DWORD CDVBLib::SetParamInfo(BOOL bType)				//激活一个对话框
{
	if(m_hDVBLib && m_pSetParamInfo)
		return (*m_pSetParamInfo)((DWORD)bType);
	else
		return 0xffffffff;		
}

DWORD CDVBLib::QuerySatellite(int nCount, char ** pStrArray)
{
	if(m_hDVBLib && m_pQuerySatellite)
		return (*m_pQuerySatellite)((DWORD)nCount,
						(DWORD)pStrArray);
	else
		return 0xffffffff;		
}

DWORD CDVBLib::QueryChannel(int nCount, char *pSatellite, char ** pStrArray)
{
	if(m_hDVBLib && m_pQueryChannel)
		return (*m_pQueryChannel)((DWORD)nCount,
						(DWORD)pSatellite,
						(DWORD)pStrArray);
	else
		return 0xffffffff;		
}

DWORD CDVBLib::QueryStation(int nCount, char ** pStrArray)
{
	if(m_hDVBLib && m_pQueryStation)
		return (*m_pQueryStation)((DWORD)nCount,
						(DWORD)pStrArray);
	else
		return 0xffffffff;		
}

BOOL CDVBLib::QueryParamDBS(PTunerParam lParam, char * pSatellite, \
					  char * pChannel, BOOL bSearchLock)
{
	if(m_hDVBLib && m_pQueryParamDBS)
		return (BOOL)(*m_pQueryParamDBS)((DWORD)lParam,
						(DWORD)pSatellite,
						(DWORD)pChannel,
						(DWORD)bSearchLock);
	else
		return FALSE;		
}

BOOL CDVBLib::QueryParamQAM(PQAMParam lParam, char * pStation, BOOL bSearchLock)
{
	if(m_hDVBLib && m_pQueryParamQAM)
		return (BOOL)(*m_pQueryParamQAM)((DWORD)lParam,
						(DWORD)pStation,
						(DWORD)bSearchLock);
	else
		return FALSE;		
} 




VOID CDVBLib::PIDAlwaysDBS(BOOL bMode,PUINT puValue,char * pSatellite,char * pChannel)
{
	if(m_hDVBLib && m_pPIDAlwaysDBS)
		(*m_pPIDAlwaysDBS)((DWORD)bMode,
						   (DWORD)puValue,
						   (DWORD)pSatellite,
						   (DWORD)pChannel);
	else
		*puValue = 0xffff;
}		

VOID CDVBLib::PIDAlwaysQAM(BOOL bMode,PUINT puValue,char * pStation)
{
	if(m_hDVBLib && m_pPIDAlwaysQAM)
		(*m_pPIDAlwaysQAM)((DWORD)bMode,
						   (DWORD)puValue,
						   (DWORD)pStation);
	else
		*puValue = 0xffff;
}

VOID CDVBLib::SetCanTunerInDll(BOOL bValue)
{
	if(m_hDVBLib && m_pSetCanTunerInDll)
		(*m_pSetCanTunerInDll)(bValue);
}
///////////////////////////////////////////////////
//Build 1.20
BOOL CDVBLib::ImportParam(int nType, char * szIniFile)
{
	if(m_hDVBLib && m_pImportParam)
		return (BOOL)(*m_pImportParam)((DWORD)nType,
						(DWORD)szIniFile);
	else
		return FALSE;		
}

BOOL CDVBLib::ExportParam(int nType, char * szIniFile)
{
	if(m_hDVBLib && m_pExportParam)
		return (BOOL)(*m_pExportParam)((DWORD)nType,
						(DWORD)szIniFile);
	else
		return FALSE;		
}

DWORD CDVBLib::AutoSearch(BOOL bType)				//激活一个对话框
{
	if(m_hDVBLib && m_pAutoSearch)
		return (BOOL)(*m_pAutoSearch)((DWORD)bType);
	else
		return FALSE;
}


//////////////////////////////////////////////////////////////////////////
int CDVBLib::TSDVB0_GetDevCount()
{
	if(m_hDVBLib && m_pGetDevCount)
		return (int)(*m_pGetDevCount)();
	else
		return 0;
}

BOOL CDVBLib::TSDVB0_QueryDevice(int nIndex,LPVOID Info,HWND hWnd)
{
	if(m_hDVBLib && m_pQueryDevice)
		return (BOOL)(*m_pQueryDevice)((DWORD)nIndex,(DWORD)Info,(DWORD)hWnd);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_OpenDevice(int Num)
{
	if(m_hDVBLib && m_pOpenDevice)
		return (BOOL)(*m_pOpenDevice)((DWORD)Num);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_SendRawDiSEqCMsg(BYTE *Data, int Len)
{
	if(m_hDVBLib && m_pSendRawDiSEqCMsg)
		return (BOOL)(*m_pSendRawDiSEqCMsg)((DWORD)Data,(DWORD)Len);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_LockTransponder(DWORD dwFreq, DWORD dwSymbRate, DWORD Polarisation, BOOL bF22KHz)
{
	if(m_hDVBLib && m_pLockTransponder)
		return (BOOL)(*m_pLockTransponder)((DWORD)dwFreq,(DWORD)dwSymbRate,(DWORD)Polarisation,(DWORD)bF22KHz);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_GetSignal(BOOL *Locked, DWORD *Quality, DWORD *Strength)
{
	if(m_hDVBLib && m_pGetSignal)
		return (BOOL)(*m_pGetSignal)((DWORD)Locked,(DWORD)Quality,(DWORD)Strength);
	else
		return FALSE;
}

int CDVBLib::TSDVB0_ReadBuffer(BYTE *Data, int Len)
{
	if(m_hDVBLib && m_pReadBuffer)
		return (BOOL)(*m_pReadBuffer)((DWORD)Data,(DWORD)Len);
	else
		return 0;
}


BOOL CDVBLib::TSDVB0_AddPID(WORD PID)
{
	if(m_hDVBLib && m_pAddPID)
		return (BOOL)(*m_pAddPID)((DWORD)PID);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_DeletePID(WORD PID)
{
	if(m_hDVBLib && m_pDeletePID)
		return (BOOL)(*m_pDeletePID)((DWORD)PID);
	else
		return FALSE;
}

BOOL CDVBLib::TSDVB0_DeleteAllPIDs()
{
	if(m_hDVBLib && m_pDeleteAllPIDs)
		return (BOOL)(*m_pDeleteAllPIDs)();
	else
		return FALSE;
}

int CDVBLib::TSDVB0_GetPIDList(WORD *List, int Len)
{
	if(m_hDVBLib && m_pGetPIDList)
		return (BOOL)(*m_pGetPIDList)((DWORD)List,(DWORD)Len);
	else
		return 0;
}

BOOL CDVBLib::TSDVB0_PassFullStream(BOOL Full)
{
	if(m_hDVBLib && m_pPassFullStream)
		return (BOOL)(*m_pPassFullStream)((DWORD)Full);
	else
		return FALSE;
}
//////////////////////////////////////////////////////////////////////////
int CDVBLib::GetAndyDebugInfo(PBYTE pBuff, int nBufLen)	//DEBUG Func ,Not Call!!!
{
//	if(m_hDVBLib && m_pPassFullStream)
//		return (int)(*m_pGetAndyDebugInfo)((DWORD)pBuff,(DWORD)nBufLen);
//	else
		return 0;
}
