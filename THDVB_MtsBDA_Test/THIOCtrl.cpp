
#include "stdafx.h"
#include "graph.h"

extern CBDAFilterGraph* g_pfg;


BOOL CBDAFilterGraph::BDAIOControl( DWORD  dwIoControlCode,
									LPVOID lpInBuffer,
									DWORD  nInBufferSize,
									LPVOID lpOutBuffer,
									DWORD  nOutBufferSize,
									LPDWORD lpBytesReturned)
{
    if (!m_KsTunerPropSet)
        return FALSE;

    KSPROPERTY instance_data;

    ULONG    ulOutBuf = 0;
    ULONG    ulReturnBuf = 0;
    THBDACMD THBDACmd;

    THBDACmd.CmdGUID = GUID_THBDA_CMD;
    THBDACmd.dwIoControlCode = dwIoControlCode;
    THBDACmd.lpInBuffer = lpInBuffer;
    THBDACmd.nInBufferSize = nInBufferSize;
    THBDACmd.lpOutBuffer = lpOutBuffer;
    THBDACmd.nOutBufferSize = nOutBufferSize;
    THBDACmd.lpBytesReturned = lpBytesReturned;

    HRESULT hr = m_KsTunerPropSet->Set(GUID_THBDA_TUNER, 
                              NULL, 
	  						  &instance_data, sizeof(instance_data),
                              &THBDACmd, sizeof(THBDACmd));

    if (FAILED(hr))
        return FALSE;
    else
        return TRUE;
}


//***************************************************************//
//************** Basic IOCTL sets  (must support) ***************//
//***************************************************************//

BOOL CBDAFilterGraph::THBDA_IOCTL_CHECK_INTERFACE_Fun(void)
{
    BOOL bResult = FALSE;
    DWORD nBytes = 0;

    bResult = BDAIOControl((DWORD) THBDA_IOCTL_CHECK_INTERFACE,
									NULL,
									0,
									NULL,
									0,
									(LPDWORD)&nBytes);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CHECK_INTERFACE_Fun failed! \n"));
	}
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_SET_REG_PARAMS_DATA_Fun(THBDAREGPARAMS *pTHBDAREGPARAMS)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_SET_REG_PARAMS,
							(LPVOID)pTHBDAREGPARAMS, 
							sizeof(THBDAREGPARAMS),     
							NULL, 
							0,                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_SET_REG_PARAMS_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_REG_PARAMS_Fun(THBDAREGPARAMS *pTHBDAREGPARAMS)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_REG_PARAMS,							     
							NULL, 
							0,
							(LPVOID)pTHBDAREGPARAMS, 
							sizeof(THBDAREGPARAMS),                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_REG_PARAMS failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_DEVICE_INFO_Fun(DEVICE_INFO *pDEVICE_INFO)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_DEVICE_INFO,							     
							NULL, 
							0, 
							(LPVOID)pDEVICE_INFO, 
							sizeof(DEVICE_INFO),                   
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_DEVICE_INFO_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_DRIVER_INFO_Fun(DriverInfo *pDriverInfo)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_DRIVER_INFO,							     
							NULL, 
							0, 
							(LPVOID)pDriverInfo, 
							sizeof(DriverInfo),                   
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_DRIVER_INFO_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_SET_TUNER_POWER_Fun(BYTE TunerPower)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_SET_TUNER_POWER,
							(LPVOID)&TunerPower, 
							sizeof(BYTE),     
							NULL, 
							0,                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_SET_TUNER_POWER_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_TUNER_POWER_Fun(BYTE *pTunerPower)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_TUNER_POWER,							    
							NULL, 
							0, 
							(LPVOID)pTunerPower, 
							sizeof(BYTE),                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_TUNER_POWER_Fun failed! \n"));
	}

	return bResult;
}


//***************************************************************//
//****************** Remote Controller (optional) ***************//
//***************************************************************//

BOOL CBDAFilterGraph::THBDA_IOCTL_HID_RC_ENABLE_Fun(BYTE RCEnable)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_HID_RC_ENABLE,
							(LPVOID)&RCEnable, 
							sizeof(BYTE),     
							NULL, 
							0,                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_HID_RC_ENABLE_Fun failed! \n"));
	}

	return bResult;
}


//***************************************************************//
//********************* DVB-S (must support)*********************//
//***************************************************************//

BOOL CBDAFilterGraph::THBDA_IOCTL_SET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_SET_LNB_DATA,
							(LPVOID)pLNB_DATA, 
							sizeof(LNB_DATA),     
							NULL, 
							0,                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_SET_LNB_DATA_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_LNB_DATA_Fun(LNB_DATA *pLNB_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_LNB_DATA,							  
							NULL, 
							0,
							(LPVOID)pLNB_DATA, 
							sizeof(LNB_DATA),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_LNB_DATA_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_SET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_SET_DiSEqC,
							(LPVOID)pDiSEqC_DATA, 
							sizeof(DiSEqC_DATA),     
							NULL, 
							0,                    
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_SET_DiSEqC_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_GET_DiSEqC_Fun(DiSEqC_DATA *pDiSEqC_DATA)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_GET_DiSEqC,							  
							NULL, 
							0,
							(LPVOID)pDiSEqC_DATA, 
							sizeof(DiSEqC_DATA),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_GET_DiSEqC_Fun failed! \n"));
	}

	return bResult;
}

//***************************************************************//
//****************** CI & MMI (must support for CI)**************//
//***************************************************************//

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_STATE_Fun(THCIState *pTHCIState)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_GET_STATE,							  
							NULL, 
							0,
							(LPVOID)pTHCIState, 
							sizeof(THCIState),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_STATE_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_STATE_Fun(THCIStateOld *pTHCIStateOld)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_GET_STATE,							  
							NULL, 
							0,
							(LPVOID)pTHCIStateOld, 
							sizeof(THCIStateOld),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_STATE_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_APP_INFO_Fun(THAppInfo *pTHAppInfo)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_GET_APP_INFO,							  
							NULL, 
							0,
							(LPVOID)pTHAppInfo, 
							sizeof(THAppInfo),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_APP_INFO_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_INIT_MMI_Fun(void)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_INIT_MMI,							  
							NULL, 
							0,
							NULL, 
							0,                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_INIT_MMI_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_MMI_Fun(THMMIInfo *pTHMMIInfo)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_GET_MMI,							  
							NULL, 
							0,
							(LPVOID)pTHMMIInfo, 
							sizeof(THMMIInfo),                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_MMI_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_ANSWER_Fun(THMMIInfo *pTHMMIInfo)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_ANSWER,	
							NULL, 
							0,				  
							(LPVOID)pTHMMIInfo, 
							sizeof(THMMIInfo),							                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_ANSWER_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_CLOSE_MMI_Fun(void)
{	
	BOOLEAN bResult	= FALSE;
	DWORD   nBytes  = 0;	

	bResult = BDAIOControl(	(DWORD)THBDA_IOCTL_CI_CLOSE_MMI,							  
							NULL, 
							0,
							NULL, 
							0,                       
							(LPDWORD)&nBytes);
	if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_CLOSE_MMI_Fun failed! \n"));
	}

	return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_SEND_PMT_Fun(PBYTE pBuff, DWORD dwBuffSize)
{
    BOOL bResult = FALSE;
    DWORD nBytes = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_SEND_PMT,
							(LPVOID)pBuff,
							dwBuffSize,
							NULL,
							0,
							(LPDWORD)&nBytes);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_SEND_PMT_Fun failed! \n"));
	}
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_PMT_REPLY_Fun(PBYTE pBuff, DWORD dwBuffSize)
{
    BOOL bResult = FALSE;
    DWORD dwBytesReturned = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_GET_PMT_REPLY,
							NULL,
							0,
							(LPVOID)pBuff,
							dwBuffSize,
							(LPDWORD)&dwBytesReturned);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_PMT_REPLY_Fun failed! \n"));
	}
    
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_EVENT_CREATE_Fun(HANDLE hCIEvent)
{
    BOOL bResult = FALSE;
    DWORD dwBytesReturned = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_EVENT_CREATE,
							(LPVOID)&hCIEvent,
							sizeof(HANDLE),
							NULL,
							0,
							(LPDWORD)&dwBytesReturned);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_EVENT_CREATE_Fun failed! \n"));
	}
    
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_EVENT_CLOSE_Fun(HANDLE hCIEvent)
{
    BOOL bResult = FALSE;
    DWORD dwBytesReturned = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_EVENT_CLOSE,
							(LPVOID)&hCIEvent,
							sizeof(HANDLE),
							NULL,
							0,
							(LPDWORD)&dwBytesReturned);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_EVENT_CLOSE_Fun failed! \n"));
	}
    
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_SEND_RAW_CMD_Fun(PBYTE pBuff, DWORD dwBuffSize)
{
    BOOL bResult = FALSE;
    DWORD nBytes = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_SEND_RAW_CMD,
							(LPVOID)pBuff,
							dwBuffSize,
							NULL,
							0,
							(LPDWORD)&nBytes);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_SEND_RAW_CMD_Fun failed! \n"));
	}
    return bResult;
}

BOOL CBDAFilterGraph::THBDA_IOCTL_CI_GET_RAW_CMD_DATA_Fun(PBYTE pBuff, DWORD dwBuffSize)
{
    BOOL bResult = FALSE;
    DWORD dwBytesReturned = 0;

    bResult = BDAIOControl( THBDA_IOCTL_CI_GET_RAW_CMD_DATA,
							NULL,
							0,
							(LPVOID)pBuff,
							dwBuffSize,
							(LPDWORD)&dwBytesReturned);
    if (bResult==FALSE)	{
		OutputDebugString(TEXT("IOCTL Error: THBDA_IOCTL_CI_GET_RAW_CMD_DATA_Fun failed! \n"));
	}
    
    return bResult;
}

//***************************************************************//
//****************** PID filters (for 7021, 7041) ***************//
//***************************************************************//


//***************************************************************//
//******************* Miscellaneous (Optional) ******************//
//***************************************************************//


//***************************************************************//
//********************* DVB-T ***********************************//
//***************************************************************//


//***************************************************************//
//********************* DVB-C ***********************************//
//***************************************************************//



//***************************************************************//
//***** Stream Capture (optional, used in 7045 BDA Agent) *******//
//***************************************************************//


//***************************************************************//
//**** DVB-S with virtual DVB-T interface for MCE (Optional) ****//
//***************************************************************//


//***************************************************************//
//******* Tuner firmware download (optional, used in 704C) ******//
//***************************************************************//