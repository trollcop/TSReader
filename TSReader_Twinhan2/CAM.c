#define CAM_MESSAGE_LEN 256

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>

#include "..\sources.h"
#include "TSReader_Twinhan2.h"
#include "..\CI-CAM.h"

#ifdef CI_SUPPORT

extern PSOURCESTRUCT ss;
extern HANDLE hDevice2;
extern CRITICAL_SECTION csHardware;

unsigned char MakeCheckSum(TunerData *pTunerData);
int nCAMType = -1;

unsigned char MakeCheckSum2(BYTE *pBuf)
{
	int length = pBuf[1];
	int checksum = 0;
	int i;
	
	if (length == 0)
		length = 7; 
	for (i=1;i<length+1;i++)
		checksum += pBuf[i];
	checksum = ~checksum + 1;
	pBuf[length+1] = checksum;
	
	return checksum;
}

BOOL getCAMState(int *CAM_Flag, int *MMI_Flag)
{
    BOOL   StatusSuccess = TRUE;
    LPVOID inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength;
    DWORD  outBufLength;
    ULONG  bytesReturned;
    BYTE   bBuf[CAM_MESSAGE_LEN];
    TunerData tunerData;
	BYTE  *pbuf;
	DWORD Temp = 0;

    if (hDevice2 == NULL)
        return FALSE;
    
    memset(&tunerData, 0, sizeof(TunerData));
    
    tunerData.address = 0xaa;
    tunerData.frequencyMSB = 0;
    tunerData.frequencyLSB = 0x05;
    
    MakeCheckSum(&tunerData);
    
    inBuf = &tunerData;
    inBufLength	= sizeof(tunerData);		
    outBuf = (LPVOID)bBuf;
    outBufLength = inBufLength;

    EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_SET_INFO, 
                                    inBuf, 
                                    inBufLength + 1, 
                                    outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess) 
        return FALSE;
    
    pbuf = (BYTE *)outBuf;
    Temp = 0;
    
    Temp = pbuf[5];
    if (Temp == 0xff)
	{
        *CAM_Flag = NON_CI_INFO;
        *MMI_Flag = NON_CI_INFO;
        return TRUE;
    }
    
    if (Temp & 0x80)
        *CAM_Flag = ME0;
    else if (Temp & 0x40)
        *CAM_Flag = ME1;
    else
        *CAM_Flag = NON_CI_INFO;
    
    if (Temp & 0x20)
        *MMI_Flag = MMI0;
    else if (Temp & 0x10)
        *MMI_Flag = MMI1;
    else if (Temp & 0x08)
        *MMI_Flag = MMI0_ClOSE;
    else if (Temp & 0x04)
        *MMI_Flag = MMI1_CLOSE;
    else
        *MMI_Flag = NON_CI_INFO;
    
    return TRUE;
}

BOOL TSReader_GetAppInfo(App_Info *pAppInfo, int * CAM_Type)
{
    BOOL   StatusSuccess = TRUE;
    //LPVOID			inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength = 9;
    ULONG  bytesReturned;
    BYTE   bBuf[CAM_MESSAGE_LEN];
    DWORD  outBufLength = sizeof(bBuf);
    char   *Str = NULL;
    BYTE Command[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_APPLICATION_INFO, 0x00, 0x00, 0xff};
    
    if (hDevice2 == NULL)
        return FALSE;
        
    outBuf = (PVOID)bBuf;
    ZeroMemory((PVOID)bBuf, sizeof(bBuf));
    MakeCheckSum2(Command);

    EnterCriticalSection(&csHardware);    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)Command, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess)
	{
	    LeaveCriticalSection(&csHardware);
        return FALSE;
	}
    	
    Sleep(100);   
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_READ, 
                                    NULL,
                                    0,
                                    (PVOID)outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess)
        return FALSE;
    
    pAppInfo->app_type = bBuf[6];
    pAppInfo->application_manufacture = (bBuf[7] << 8) | bBuf[8];
    pAppInfo->manufacture_code = (bBuf[9] << 8) | bBuf[10];
    strcpy((char *)pAppInfo->application_info, (char *)(&(bBuf[11])));
    
    Str = strlwr(strdup(pAppInfo->application_info));

	if (NULL != strstr(Str,"conax"))
	{
		*CAM_Type = CAM_CONAX;
	}
	else if (NULL != strstr(Str,"cryptoworks"))
	{
		*CAM_Type = CAM_CRYPTOWORKS;
	}
	else if (NULL != strstr(Str,"aston"))
	{
		*CAM_Type = CAM_ASTON;
	}
    
    return TRUE;
}

BOOL TSReader_InitMMI()
{
    BOOL  StatusSuccess = TRUE;
    //LPVOID			inBuf;
    DWORD inBufLength = 9;
    ULONG bytesReturned;
    BYTE EnterMenu[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_ENTER_MENU, 0x00, 0x00, 0xff};
    
    if (hDevice2 == NULL)
        return FALSE;
        
    MakeCheckSum2(EnterMenu);    
    EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)EnterMenu, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess)
        return FALSE;
    
    return TRUE;
}

BOOL TSReader_CloseMMI()
{
    BOOL  StatusSuccess = TRUE;
    //LPVOID inBuf;
    DWORD inBufLength = 9;
    ULONG bytesReturned;

    BYTE CloseMenu[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_CLOSE_MMI, 0x00, 0x00, 0xff};
    MakeCheckSum2(CloseMenu);
    
    EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)CloseMenu, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess)
        return FALSE;
    
    return TRUE;
}

BOOL TSReader_GetMMI(MMI_Info *pMMIInfo, int *pType)
{
    BOOL   StatusSuccess = TRUE;
    //LPVOID inBuf;
    LPVOID outBuf; 
    DWORD  inBufLength = 9;
    ULONG  bytesReturned;
    BYTE   bBuf[CAM_MESSAGE_LEN];
    DWORD  outBufLength = sizeof(bBuf);
    BYTE GetMMI[] = {0xaa, 0x07, 0x40, 0x00, 0x00, PCMSG_GET_MMI, 0x00, 0x00, 0xff};
    int i = 0;
    BOOL noItemFlag = FALSE;
    BYTE *pBuf = NULL;
    
    if (hDevice2 == NULL)
		return FALSE;
    
    outBuf = (PVOID)bBuf;
    ZeroMemory((PVOID)bBuf, sizeof(bBuf));
    MakeCheckSum2(GetMMI);
    
    EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)GetMMI, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    if (!StatusSuccess)
	{
	    LeaveCriticalSection(&csHardware);
        return FALSE;
	}
    
    Sleep(250);    
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_READ, 
                                    NULL,
                                    0,
                                    (PVOID)outBuf, 
                                    outBufLength, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess)
        return FALSE;
    
    pBuf = bBuf + 6;
    switch (bBuf[3])
	{
        case PCMSG_MENU:
            *pType = 1;
            
            pMMIInfo->ItemCount = *pBuf++;
            wsprintf((char *)pMMIInfo->Header,"%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            wsprintf((char *)pMMIInfo->SubHeader,"%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            wsprintf((char *)pMMIInfo->ButtomLine,"%s", (char *)pBuf);
            
            if (pMMIInfo->ItemCount == 0xff)
                pMMIInfo->ItemCount = 0;
            
            if (pMMIInfo->ItemCount > 9)
                pMMIInfo->ItemCount = 9;
            
            for (i=0;i<pMMIInfo->ItemCount;i++)
			{
                pBuf += strlen((char *)pBuf) + 1;
                wsprintf((char *)pMMIInfo->MenuItem[i], "%s", (char *)pBuf);
            }
            break;
        
        case PCMSG_LIST:
            *pType = 2;
            
            pMMIInfo->ItemCount = *pBuf++;
            wsprintf((char *)pMMIInfo->Header, "%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            wsprintf((char *)pMMIInfo->SubHeader, "%s", (char *)pBuf);
            pBuf += strlen((char *)pBuf) + 1;
            wsprintf((char *)pMMIInfo->ButtomLine, "%s", (char *)pBuf);
            
            if ((pMMIInfo->ItemCount == 0xff) || (pMMIInfo->ItemCount == 0))
			{
                noItemFlag = TRUE;
                pMMIInfo->ItemCount = 1;
            }
            
            if (pMMIInfo->ItemCount > 9)
                pMMIInfo->ItemCount = 9;
            
            if (noItemFlag)
			{
                wsprintf((char *)pMMIInfo->MenuItem[0], "%s", pMMIInfo->ButtomLine);
            }
            else
			{
                for (i=0;i<pMMIInfo->ItemCount;i++)
				{
                    pBuf += strlen((char *)pBuf) + 1;
                    wsprintf((char *)pMMIInfo->MenuItem[i], "%s", (char *)pBuf);
                }
            }
            break;
            
        case PCMSG_ENQ:
            *pType = 3;
            
            pMMIInfo->Blind_Answer = *pBuf++;
            pMMIInfo->Answer_Text_Length = *pBuf++;
            wsprintf((char *)pMMIInfo->Prompt, "%s", (char *)pBuf);
            pMMIInfo->EnqFlag = TRUE;
            break;
            
        default:
            *pType = 0;
            return FALSE;
    }
    
    return TRUE;
}

BOOL TSReader_AnswerMMI(MMI_Info *pMMIInfo, int Type)
{
    BOOL   StatusSuccess = TRUE;
    LPVOID inBuf;
    DWORD  inBufLength = 0;
    ULONG  bytesReturned;
    BYTE MenuAnswer[] = {0xaa, 0x08, 0x40, 0x00, 0x00, PCMSG_MENU_ANSWER, 0x01, 0x00, 0x01, 0xff};
    BYTE EnqAnswer[CAM_MESSAGE_LEN] = {0xaa,0x08,0x40,0x00,0x00, PCMSG_ANSWER, 0x01,0x00,0x01,0xff};

    if (hDevice2 == NULL)
	{
        return FALSE;
    }

    //Answer Cancel, 
    //When Type == 1, pMMIInfo->Answer = 0;
    //When Type == 2, Automatic;
    //When Type == 3, pMMIInfo->Answer_Text_Length = 0;
    switch (Type)
	{
        case 1:
            MenuAnswer[8] = pMMIInfo->Answer;
            inBufLength = 12;
            MakeCheckSum2(MenuAnswer);
            inBuf = (PVOID)MenuAnswer;
            break;
            
        case 2:
            MenuAnswer[8] = 0;
            inBufLength = 12;
            MakeCheckSum2(MenuAnswer);
            inBuf = (PVOID)MenuAnswer;
            break;
            
        case 3:
            inBufLength = 10;
            if (pMMIInfo->Answer_Text_Length)
			{
                CopyMemory((PVOID)(EnqAnswer + 9), (PVOID)(pMMIInfo->AnswerStr), pMMIInfo->Answer_Text_Length);
                EnqAnswer[1] += pMMIInfo->Answer_Text_Length;
                EnqAnswer[6] += pMMIInfo->Answer_Text_Length;
                inBufLength += pMMIInfo->Answer_Text_Length;
            }
            else
			{
                EnqAnswer[8] = 0;
            }
            MakeCheckSum2(EnqAnswer);
            inBuf = (PVOID)EnqAnswer;
            break;
            
        default:
            return FALSE;
    }
    
    EnterCriticalSection(&csHardware);
    StatusSuccess = DeviceIoControl(hDevice2, 
                                    (DWORD)DST_IOCTL_CA_WRITE, 
                                    (PVOID)inBuf, 
                                    inBufLength, 
                                    NULL, 
                                    0, 
                                    &bytesReturned, 
                                    NULL);
    LeaveCriticalSection(&csHardware);
    if (!StatusSuccess)
        return FALSE;
    
    return TRUE;
}

BOOL TSReader_SendCAPMT(BYTE *pBuf, int Size)
{
    BYTE *pBuffer = pBuf;
    int nSize = Size;
    int i = 0;
    DWORD bytesReturned = 0;

    if (nSize) 
	{
        // set CI Information
        for (i=nSize-1; i>=0; i--)
		{
            pBuf[i+8] = pBuf[i];
        }
        nSize += 9;
        pBuf[0] = 0xaa;
        pBuf[1] = nSize - 2;
        pBuf[2] = 0x40;
        pBuf[3] = 0x03;
        pBuf[4] = 0;
        pBuf[5] = 0x03;
        pBuf[6] = nSize - 9;
        pBuf[7] = 0;
        pBuf[nSize - 1] = 0;
        for (i=1; i<nSize-1; i++)
		{
            pBuf[nSize-1] += pBuf[i];
        }
        pBuf[nSize-1] = ~pBuf[nSize-1] + 1;
        
	    EnterCriticalSection(&csHardware);
        DeviceIoControl(hDevice2, 
                        (DWORD)DST_IOCTL_CA_WRITE, 
                        pBuf, 
                        nSize, 
                        NULL, 
                        0, 
                        &bytesReturned, 
                        NULL);
		LeaveCriticalSection(&csHardware);

    }

	return TRUE;
}

#else CI_SUPPORT

BOOL TSReader_InitMMI()
{
	return FALSE;
}

BOOL TSReader_CloseMMI()
{
	return FALSE;
}

BOOL TSReader_GetMMI(MMI_Info *pMMIInfo, int *pType)
{
	return FALSE;
}

BOOL TSReader_AnswerMMI(MMI_Info *pMMIInfo, int Type)
{
	return FALSE;
}

BOOL TSReader_GetAppInfo(App_Info *pAppInfo, int * CAM_Type)
{
	return FALSE;
}

BOOL TSReader_SendCAPMT(BYTE *pBuf, int Size)
{
	return FALSE;
}

#endif CI_SUPPORT
