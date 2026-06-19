#include <windows.h>
#include <commctrl.h>
#include <limits.h>
#include "TSReader.h"
#include "CI-CAM.h"
#include "bcdmux.h"
#include "resource.h"

extern PVARIABLES v;
extern char gszAppName[];

typedef BOOL (* td_initMMI) ();
typedef BOOL (* td_getMMI) (MMI_Info *pMMIInfo, int *pType);
typedef BOOL (* td_answerMMI) (MMI_Info *pMMIInfo, int Type);
typedef BOOL (* td_getAppInfo) (App_Info *pAppInfo, int * CAM_Type);
typedef BOOL (* td_closeMMI) ();
BOOL (* initMMI) ();
BOOL (* getMMI) (MMI_Info *pMMIInfo, int *pType);
BOOL (* answerMMI) (MMI_Info *pMMIInfo, int Type);
BOOL (* getAppInfo) (App_Info *pAppInfo, int * CAM_Type);
BOOL (* closeMMI) ();

void CursorNormal();
void CursorWait(HWND hWnd);

int CAM_Type = CAM_DEFAULT;
int m_nAnswerMode;  					// 0: initiative answer mode; 1: passivity answer mode.
// If get the WM_MMI_INFO message, it is passivity answer 
// mode.
int m_nAnswerType = 0;
char m_strEnterCode[128] = {0};  
int m_pShowCtrl = 0;

WNDPROC wpOrigListProc;

int GenerateCAPMT(BYTE * pCAPMTBuffer, int nBufferLength, int nPMTIndex)
{
	int nLength = 0;
	int ca_pmt_list_management = 0x03;		// only item
	int program_number = v->pat.pmt[nPMTIndex].nProgramNumber;
	int program_info_length = 0;
	int nOffset;
	int nLoopCounter = 0;

	BYTE bDescriptorBuffer[1024];

	//do
	{
		int nESIndex;

		set_buf(BM_CI_INTERFACE, pCAPMTBuffer, nBufferLength, TRUE);
		set_bits(BM_CI_INTERFACE, ca_pmt_list_management, 8);
		set_bits(BM_CI_INTERFACE, program_number, 16);
		set_bits(BM_CI_INTERFACE, 3, 2);				// reserved
		set_bits(BM_CI_INTERFACE, 1, 5);				// version_number
		set_bits(BM_CI_INTERFACE, 1, 1);				// current_next_indicator
		set_bits(BM_CI_INTERFACE, 0xf, 4);			// reserved

		// Get the program level CA descriptors
		program_info_length = 0;
		for (nOffset = 0; nOffset < v->pat.pmt[nPMTIndex].nProgramInfoLength;)
		{
			int nDescriptor = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset];
			int nDescriptorLength = v->pat.pmt[nPMTIndex].pProgramInfo[nOffset + 1];

			if (nDescriptor == 0x09)
			{
				memcpy(&bDescriptorBuffer[program_info_length], &v->pat.pmt[nPMTIndex].pProgramInfo[nOffset], nDescriptorLength + 2);
				program_info_length += nDescriptorLength + 2;
			}
			nOffset += nDescriptorLength + 2;
		}

		// Build program level descriptors
		if (!program_info_length)
			set_bits(BM_CI_INTERFACE, program_info_length, 12);
		else
		{
			int ca_pmt_cmd_id = 0x01;	// ok_descrambling
			int i;

			set_bits(BM_CI_INTERFACE, program_info_length + 1, 12);
			set_bits(BM_CI_INTERFACE, ca_pmt_cmd_id, 8);
			for (i = 0; i < program_info_length; i++)
				set_bits(BM_CI_INTERFACE, bDescriptorBuffer[i], 8);
		}


		// Check for ES level descriptors		
		for (nESIndex = 0; nESIndex < MAX_ESLIST_ENTRIES; nESIndex++)
		{
			int ES_info_length = 0;

			if (v->pat.pmt[nPMTIndex].es[nESIndex].nESPID == 0)
				break;

			if (v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors)
			{
				for (nOffset = 0; nOffset < v->pat.pmt[nPMTIndex].es[nESIndex].nDescriptorsLength; )
				{
					int nDescriptor = v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[nOffset];
					int nDescriptorLength = v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[nOffset + 1];
					
					if (nDescriptorLength && nDescriptor == 0x09)
					{
						memcpy(&bDescriptorBuffer[ES_info_length], &v->pat.pmt[nPMTIndex].es[nESIndex].pDescriptors[nOffset], nDescriptorLength + 2);
						ES_info_length += nDescriptorLength + 2;
					}
					nOffset += nDescriptorLength + 2;
				}
				if (ES_info_length)
				{
					int ca_pmt_cmd_id = 01;	// ok_descrambling
					int stream_type = v->pat.pmt[nPMTIndex].es[nESIndex].nStreamType;
					int elementary_PID = v->pat.pmt[nPMTIndex].es[nESIndex].nESPID;
					int i;

					set_bits(BM_CI_INTERFACE, stream_type, 8);
					set_bits(BM_CI_INTERFACE, 7, 3);			// reserved
					set_bits(BM_CI_INTERFACE, elementary_PID, 13);
					set_bits(BM_CI_INTERFACE, 0xf, 4);		// reserved
					set_bits(BM_CI_INTERFACE, ES_info_length + 1, 12);
					set_bits(BM_CI_INTERFACE, ca_pmt_cmd_id, 8);
					for (i = 0; i < ES_info_length; i++)
						set_bits(BM_CI_INTERFACE, bDescriptorBuffer[i], 8);
				}
			}
		}
		nLength = get_byte_pos(BM_CI_INTERFACE);
	}// while (nLoopCounter++ < 2);
	
	return nLength;
}


/*
// Buffer is started in program_number of TS_program_map_section()
void Analyse_PMT(BYTE *pBuf, int BufSize, DWORD VPid, DWORD APid, BYTE *pStartPMTBuf, int *pStartPMTSize, BYTE *pStopPMTBuf, int *pStopPMTSize)
{
    BYTE  *pData = pBuf;
    int   DataSize = BufSize;
    int   ReadPosition = 0;
    ULONG program_info_length = 0;
    BYTE  stream_type = 0;
    ULONG elementary_PID = 0;
    ULONG ES_info_length = 0;
    BOOL  Match_Flag = FALSE;

    *pStartPMTSize = 0;
    *pStopPMTSize = 0;
    
    ReadPosition = 7;
    program_info_length = *(pData + ReadPosition++) & 0x0f;
    program_info_length = (program_info_length << 8) | *(pData + ReadPosition++);
    ReadPosition += program_info_length;
    
    while (ReadPosition < (DataSize - 4))
	{
        stream_type = *(pData + ReadPosition++);
        elementary_PID = *(pData + ReadPosition++) & 0x1f;
        elementary_PID = (elementary_PID << 8) | *(pData + ReadPosition++);
        ES_info_length = *(pData + ReadPosition++) & 0x0f;
        ES_info_length = (ES_info_length << 8) | *(pData + ReadPosition++);
        
        // stream type:
        // 0x01    MPEG1 Video;
        // 0x02    MPEG2 Video;
        // 0x03    MPEG1 Audio;
        // 0x04    MPEG2 Audio;
        
        if ((stream_type == 0x02) || (stream_type == 0x01))
		{
			if (VPid == elementary_PID)
			{
                Match_Flag = TRUE;
            }
        }
        else if ((stream_type == 0x04) || (stream_type == 0x03))
		{
            if (APid == elementary_PID)
			{
                Match_Flag = TRUE;
            }
        }
        
        ReadPosition += ES_info_length;
    }
    
    if (Match_Flag)
	{
        ReadPosition = 7;
        program_info_length = *(pData + ReadPosition++) & 0x0f;
        program_info_length = (program_info_length << 8) | *(pData + ReadPosition++);
        
        pStartPMTBuf[(*pStartPMTSize)++] = 0x03;
        pStopPMTBuf[(*pStopPMTSize)++] = 0x03;
        CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)pData, 3);
        (*pStartPMTSize) += 3;
        CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)), (PVOID)pData, 3);
        (*pStopPMTSize) += 3;
        
        if (program_info_length && (program_info_length < 256))
		{
            pStartPMTBuf[(*pStartPMTSize)++] = *(pData + 7) & 0x0f;
            pStartPMTBuf[(*pStartPMTSize)++] = *(pData + 8) + 1;
            pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
            CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition), program_info_length);
            (*pStartPMTSize) += program_info_length;
        }
        else
		{
            pStartPMTBuf[(*pStartPMTSize)++] = 0xf0;
            pStartPMTBuf[(*pStartPMTSize)++] = 0;
        }
        
        pStopPMTBuf[(*pStopPMTSize)++] = 0xf0;
        pStopPMTBuf[(*pStopPMTSize)++] = 0;
        
        ReadPosition += program_info_length;        
        while (ReadPosition < (DataSize - 4))
		{
            stream_type = *(pData + ReadPosition++);
            elementary_PID = *(pData + ReadPosition++) & 0x1f;
            elementary_PID = (elementary_PID << 8) | *(pData + ReadPosition++);
            ES_info_length = *(pData + ReadPosition++) & 0x0f;
            ES_info_length = (ES_info_length << 8) | *(pData + ReadPosition++);
            
            ReadPosition -= 5;
            
            if (CAM_Type == CAM_DEFAULT)
			{
                if ((VPid == elementary_PID) || (APid == elementary_PID))
				{
                    int nLen;
                    int descriptor_tag;
                    int descriptor_length;
                    int nESInfoLenPos;
                    int nESInfoLen;
                    int CADescriptorLen;

                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition), 3);
                    (*pStartPMTSize) += 3;                   
                    CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)), (PVOID)(pData + ReadPosition), 3);
                    (*pStopPMTSize) += 3;
                    pStopPMTBuf[(*pStopPMTSize)++] = 0;
                    pStopPMTBuf[(*pStopPMTSize)++] = 0;
                    
                    ReadPosition += 5;
                    
                    nLen = 0;
                    descriptor_tag = 0;
                    descriptor_length = 0;
                    nESInfoLenPos = *pStartPMTSize;
                    nESInfoLen = 0;
                    CADescriptorLen = 0;
                    
                    if (ES_info_length != 0)
					{
                        while (nLen < (int)ES_info_length - 1)
						{
                            descriptor_tag = *(pData + ReadPosition + nLen++);
                            descriptor_length = *(pData + ReadPosition + nLen++);
                            
                            //descriptor tag is 09, the other is not need.
                            if (descriptor_tag == 0x09)
							{
                                if (nESInfoLenPos == *pStartPMTSize)
								{
                                    //tag and length 2 bytes, ca_pmt_cmd_id 1 byte
                                    nESInfoLen = CADescriptorLen = descriptor_length + 2 + 1;
                                    pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
                                    pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
                                    pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
                                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition + nLen - 2), CADescriptorLen - 1);
                                    (*pStartPMTSize) += CADescriptorLen - 1;
                                }
                                else
								{
                                    CADescriptorLen = descriptor_length + 2;
                                    nESInfoLen += CADescriptorLen;
                                    pStartPMTBuf[nESInfoLenPos] = (BYTE)(nESInfoLen >> 8);
                                    pStartPMTBuf[nESInfoLenPos+1] = (BYTE)(nESInfoLen & 0xff);
                                    CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition + nLen - 2), CADescriptorLen);
                                    (*pStartPMTSize) += CADescriptorLen;
                                }
                            }
                            nLen += descriptor_length;
                        }
                        
                        //Haven't CA_descripor
                        if (nESInfoLenPos == *pStartPMTSize)
						{
                            pStartPMTBuf[(*pStartPMTSize)++] = 0;
                            pStartPMTBuf[(*pStartPMTSize)++] = 0;
                        }
                    }
                    else
					{
                        pStartPMTBuf[(*pStartPMTSize)++] = 0;
                        pStartPMTBuf[(*pStartPMTSize)++] = 0;
                    }
                    ReadPosition += ES_info_length;
                }
                else {
                    ReadPosition += 5 + ES_info_length;
                }
            }
            
            if (CAM_Type == CAM_ASTON)
			{
				if ((VPid == elementary_PID) ||	(APid == elementary_PID))
				{
					int nLen;
					int descriptor_tag;
					int descriptor_length;
					int nValidTagPos;
					int CADescriptorLen;

					CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition), 3);
					(*pStartPMTSize) += 3;
					CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)), (PVOID)(pData + ReadPosition), 3);
					(*pStopPMTSize) += 3;
					pStopPMTBuf[(*pStopPMTSize)++] = 0;
					pStopPMTBuf[(*pStopPMTSize)++] = 0;

					ReadPosition += 5;

					nLen = 0;
					descriptor_tag = 0;
					descriptor_length = 0;
					nValidTagPos = 0;
					CADescriptorLen = 0;

					if (ES_info_length != 0)
					{
						int CADescriptorLen;

						while (nLen < (int)ES_info_length - 1)
						{
							descriptor_tag = *(pData + ReadPosition + nLen++);
							descriptor_length = *(pData + ReadPosition + nLen++);
							if ((descriptor_tag == 0x09) && (descriptor_length > 0x04))
							{
								nValidTagPos = nLen - 2;
								break;
							}
							nLen += descriptor_length;
						}

						//descriptor tag is 09, the other is not need.
						CADescriptorLen = *(pData + ReadPosition + nValidTagPos + 1);
						CADescriptorLen += 2 + 1;
						pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
						pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
						pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
						CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition + nValidTagPos), CADescriptorLen - 1);
						(*pStartPMTSize) += CADescriptorLen - 1;
					}
					else
					{
						pStartPMTBuf[(*pStartPMTSize)++] = 0;
						pStartPMTBuf[(*pStartPMTSize)++] = 0;
					}
					ReadPosition += ES_info_length;
				}
				else
				{
					ReadPosition += 5 + ES_info_length;
				}
			}

			if ((CAM_Type == CAM_CONAX) || (CAM_Type == CAM_CRYPTOWORKS))
			{
				int nLen;
				int descriptor_tag;
				int descriptor_length;
				int nPos;
				int CADescriptorLen;

				CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition), 3);
				(*pStartPMTSize) += 3;
				CopyMemory((PVOID)(pStopPMTBuf + (*pStopPMTSize)), (PVOID)(pData + ReadPosition), 3);
				(*pStopPMTSize) += 3;
				pStopPMTBuf[(*pStopPMTSize)++] = 0;
				pStopPMTBuf[(*pStopPMTSize)++] = 0;

				ReadPosition += 5;

				nLen = 0;
				descriptor_tag = 0;
				descriptor_length = 0;
				nPos = *pStartPMTSize;
				CADescriptorLen = 0;

				if (ES_info_length != 0)
				{
					//descriptor tag is 0x09, the other is not need.
					while (nLen < (int)ES_info_length - 1)
					{
						descriptor_tag = *(pData + ReadPosition + nLen++);
						descriptor_length = *(pData + ReadPosition + nLen++);
						if (descriptor_tag == 0x09)
						{
							*pStartPMTSize = nPos;
							CADescriptorLen = descriptor_length + 2 + 1;
							pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen >> 8);
							pStartPMTBuf[(*pStartPMTSize)++] = (BYTE)(CADescriptorLen & 0xff);
							pStartPMTBuf[(*pStartPMTSize)++] = 0x01;
							CopyMemory((PVOID)(pStartPMTBuf + (*pStartPMTSize)), (PVOID)(pData + ReadPosition + nLen - 2), CADescriptorLen - 1);
							(*pStartPMTSize) += CADescriptorLen - 1;
						}
						nLen += descriptor_length;
					}
				}

				if ((nPos == *pStartPMTSize) || (ES_info_length == 0))
				{
					pStartPMTBuf[(*pStartPMTSize)++] = *(pData + ReadPosition - 2) & 0xf0;
					pStartPMTBuf[(*pStartPMTSize)++] = 0;
				}

				ReadPosition += ES_info_length;
			}
            
        }
    }
}
*/

// the m_strEnterCode is a parameter that password or list 
// menu sequence number get from
// pMMIInfo->MenuItem, sample, if select the 
// item 3, m_strEnterCode = "3".

int GetCIInfo()
{
	return 6;
}

void DeEscapeCIString(unsigned char * szString)
{
	int i;

	for (i = 0; i < lstrlen(szString); i++)
	{
		if ((szString[i] & 0x7f) < 32)
		{
			szString[i] &= 0x7f;
			if (szString[i] != '\n')
				szString[i] = ' ';
		}
	}
}

void UpdateCIMenu(HWND hDlg, MMI_Info *pCIMenuInfo, int nType)
{
	m_nAnswerType = nType;

	if(!pCIMenuInfo)
		return;

	DeEscapeCIString(pCIMenuInfo->Header);
	DeEscapeCIString(pCIMenuInfo->SubHeader);
	DeEscapeCIString(pCIMenuInfo->ButtomLine);

	SetDlgItemText(hDlg, IDC_CAM_HEADER, pCIMenuInfo->Header);
	SetDlgItemText(hDlg, IDC_CAM_SUB_HEADER, pCIMenuInfo->SubHeader);
	SetDlgItemText(hDlg, IDC_CAM_BOTTOM_LINE, pCIMenuInfo->ButtomLine);

	if(nType == 3)
	{
		char strMsg[128];

		// Get CI Menu prompt from pCIMenuInfo->Prompt.
		wsprintf(strMsg, "CAM: %s\n", pCIMenuInfo->Prompt);
		OutputDebugString(strMsg);
		if(pCIMenuInfo->Blind_Answer)
		{
		}
			//Limt only entry password
	}
	else
	{
		int i;
		char * m_strStatus = pCIMenuInfo->ButtomLine;

		while (SendDlgItemMessage(hDlg, IDC_CAM_LIST, LB_DELETESTRING, 0, 0) != LB_ERR);
		for(i = 0; i < 10; i++)
		{
			if(nType == 1 && lstrlen(pCIMenuInfo->MenuItem[i]))
			{
				char szTemp[128];
				wsprintf(szTemp, "%d.  %s", i + 1, pCIMenuInfo->MenuItem[i]);
				SendDlgItemMessage(hDlg, IDC_CAM_LIST, LB_ADDSTRING, 0, (LPARAM)szTemp);
			}
		}
	}
}

BOOL GetMMIAnswer(HWND hDlg, MMI_Info *pMMIInfo, int *pType)
{
	int nTimeOut = 0;

	while (nTimeOut++ < 10)
	{
		int i;
		BOOL fRetVal;
		MSG msg;

		for (i = 0; i < 100; i++)
		{
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			Sleep(10);
		}
		fRetVal = getMMI(pMMIInfo, pType);
		if (fRetVal == TRUE)
			break;
	}
	ShowWindow(GetDlgItem(hDlg, IDC_CAM_ERROR_INDICATOR), nTimeOut >= 10);
	return (nTimeOut < 10);
}

BOOL CIInit(HWND hDlg)
{
	int nCIFlag = GetCIInfo();
	int nCAMInfo = 1;	//?
	BOOL bCIFlag = (nCIFlag == 0 || nCIFlag == 1 || nCIFlag == 6);
	BOOL bCAMInfo = (nCAMInfo == 1 || nCAMInfo == 2);
	int nRetType = 0;
	int nTimeout;
	MMI_Info CIMenuInfo;
	App_Info AppInfo;
	char strType[128] = {0};

	if (!bCIFlag || ! bCAMInfo)
		return FALSE;

	CursorWait(hDlg);

	// App infomation
	nTimeout = 0;
	while (nTimeout++ < 10)
	{
		ZeroMemory(&AppInfo, sizeof(AppInfo));
		if (getAppInfo(&AppInfo, &CAM_Type) == TRUE)
			break;
		Sleep(100);
	}
	
	ZeroMemory(&CIMenuInfo, sizeof(CIMenuInfo));
	switch(m_nAnswerMode)
	{
	case 0:		// Initiative answer mode
		initMMI();
		GetMMIAnswer(hDlg, &CIMenuInfo, &nRetType);
		break;
	case 1: 		// Passivity answer mode
		answerMMI(&CIMenuInfo, nRetType);
		GetMMIAnswer(hDlg, &CIMenuInfo, &nRetType);
		break;
	}
    
	//After we call GeMMI(&CIMenuInfo, &nRetType), we can get the
    //menu item information from pCIMenuInfo->MenuItem[i].
	UpdateCIMenu(hDlg, &CIMenuInfo, nRetType);

	switch(AppInfo.app_type)
	{
	case 1:
		lstrcpy(strType, "CA");
		break;
	case 2:
		lstrcpy(strType, "EPG");
		break;
	}
	SetDlgItemText(hDlg, IDC_CAM_APPLICATION_INFO, AppInfo.application_info);
	SetDlgItemText(hDlg, IDC_CAM_APPLICATION_TYPE, strType);

	//if(m_eAnswerMode == CI_PASSIVITY_ANSWER &&  nRetType  ==  2)
	{
		// Disabled the CI function
	}

	CursorNormal();
	return TRUE;
}


// Enter next CI Menu
void EntryNextCI(HWND hDlg)
{
// the m_strEnterCode is a parameter that password or list menu sequence number
// get from pMMIInfo->MenuItem, sample, if select the item 3, 
// m_strEnterCode = "3".

	int nStrLen = lstrlen(m_strEnterCode);
	int nCommand = 0;
	int nRetType = 1;
	MMI_Info CIMenuInfo;

	if(nStrLen <= 0 && m_nAnswerType != 2)
		return;

	CursorWait(hDlg);
	ZeroMemory(&CIMenuInfo, sizeof(CIMenuInfo));
	if(m_nAnswerType == 2)
	{
		CIMenuInfo.Answer = 0;
	}
	else
	{
		if(nStrLen == 1 && (nCommand = atoi(m_strEnterCode)) > 0)
		{
			CIMenuInfo.Answer = nCommand;
		}
		else
		{
			CIMenuInfo.Answer_Text_Length = nStrLen;
			strcpy(CIMenuInfo.AnswerStr, m_strEnterCode);
		}
	}

	answerMMI(&CIMenuInfo, nRetType);
	GetMMIAnswer(hDlg, &CIMenuInfo, &nRetType);

	if(m_nAnswerMode == 1)
	{
		if (closeMMI())
		{
			CursorNormal();
			return;
		}
	}

	UpdateCIMenu(hDlg, &CIMenuInfo, nRetType);

	if(nRetType == 2)
	{
		// Disabled the CI function
	}

	CursorNormal();
}

// Return previous CI Menu
void ReturnPrevCI(HWND hDlg)
{
	int nRetAnswer = 0;
	int nRetType = 0;
	MMI_Info CIMenuInfo;

	ZeroMemory(&CIMenuInfo, sizeof(CIMenuInfo));
	if(m_nAnswerType == 1 || m_nAnswerType == 2)
	{
		CIMenuInfo.Answer = 0;
	}
	else if(m_nAnswerType == 3)
	{
		CIMenuInfo.Answer_Text_Length = 0;
	}

	if(m_pShowCtrl)
	{
		nRetAnswer = answerMMI(&CIMenuInfo, nRetType);
	}

	if(m_nAnswerMode == 1)
	{
		if(closeMMI())
			return;
	}

	UpdateCIMenu(hDlg, &CIMenuInfo, nRetType);

	if(nRetType == 2)
	{
		// Disabled the CI function
	}
}

LRESULT APIENTRY ListViewSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	switch(uMsg)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_DEADCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR:
		break;
	case WM_CHAR:
		m_strEnterCode[0] = (char)wParam;
		m_strEnterCode[1] = 0;
		m_nAnswerType = 1;
		EntryNextCI(GetParent(hWnd));
		break;
	default:
		return CallWindowProc(wpOrigListProc, hWnd, uMsg, wParam, lParam); 
	}

	return FALSE;
}

INT_PTR CALLBACK CAMMenuDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
            wpOrigListProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hDlg, IDC_CAM_LIST), GWLP_WNDPROC, (LONG_PTR)ListViewSubclassProc);
			if (initMMI == NULL)
			{
				initMMI = (td_initMMI)GetProcAddress(v->hSource, "TSReader_InitMMI");
				getMMI = (td_getMMI)GetProcAddress(v->hSource, "TSReader_GetMMI");
				answerMMI = (td_answerMMI)GetProcAddress(v->hSource, "TSReader_AnswerMMI");
				getAppInfo = (td_getAppInfo)GetProcAddress(v->hSource, "TSReader_GetAppInfo");
				closeMMI = (td_closeMMI)GetProcAddress(v->hSource, "TSReader_CloseMMI");
				if (initMMI == NULL || getMMI == NULL || answerMMI == NULL || getAppInfo == NULL || closeMMI == NULL)
				{
					MessageBox(hDlg, "Unable to locate source module CI-CAM function(s)", gszAppName, MB_ICONSTOP);
					EndDialog(hDlg, FALSE);
					break;
				}
			}

			SetDlgItemText(hDlg, IDC_CAM_APPLICATION_TYPE, "");
			SetDlgItemText(hDlg, IDC_CAM_APPLICATION_INFO, "");
			SetDlgItemText(hDlg, IDC_CAM_HEADER, "");
			SetDlgItemText(hDlg, IDC_CAM_SUB_HEADER, "");						
			SetDlgItemText(hDlg, IDC_CAM_BOTTOM_LINE, "");
							
			SetFocus(GetDlgItem(hDlg, IDC_CAM_LIST));
			SetTimer(hDlg, 1, 100, NULL);
		}
		break;
	case WM_TIMER:
		KillTimer(hDlg, 1);
		if (CIInit(hDlg) == FALSE)
		{
			MessageBox(hDlg, "Unable to connect with the CI-CAM", gszAppName, MB_ICONSTOP);
			EndDialog(hDlg, FALSE);			
			break;
		}
		break;
	case WM_DESTROY:
		if (closeMMI != NULL)
			closeMMI();
		SetWindowLongPtr(GetDlgItem(hDlg, IDC_CAM_LIST), GWLP_WNDPROC, (LONG_PTR)wpOrigListProc);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, FALSE);
		break;
	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
		case BN_CLICKED:
			switch(LOWORD(wParam))
			{
			case IDOK:
				EndDialog(hDlg, TRUE);
				break;
			case IDCANCEL:
				EndDialog(hDlg, FALSE);
				break;
			}
			break;
		case LBN_DBLCLK:
			{
				int nItem = (int)SendDlgItemMessage(hDlg, IDC_CAM_LIST, LB_GETCURSEL, 0, 0);
				m_strEnterCode[0] = nItem + 1 + '0';
				m_strEnterCode[1] = 0;
				m_nAnswerType = 1;
				EntryNextCI(hDlg);
			}
			break;
		}
		break;
	}

	return FALSE;
}
