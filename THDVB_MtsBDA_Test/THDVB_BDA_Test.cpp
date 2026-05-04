// THDVB_BDA_Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "graph.h"


CBDAFilterGraph* g_pfg = NULL;

void GetMMIInfo()
{
    THCIState CIState;
    THMMIInfo MMIInfo;
    BOOL      bFlag = TRUE;
	ULONG     Count=0;
	DWORD     dwStatus;
	HANDLE    hCIEvent;

	hCIEvent = CreateEvent( 
                            NULL,   // lpEventAttributes
                            TRUE,   // bManualReset
                            FALSE,  // bInitialState
                            NULL);  // lpName

	ResetEvent(hCIEvent);
	
	if (!g_pfg->THBDA_IOCTL_CI_EVENT_CREATE_Fun(hCIEvent)) {
		cout << "Cannot create CI event!" <<endl;
		return ;
	}

	if (!g_pfg->THBDA_IOCTL_CI_INIT_MMI_Fun()) {
		cout << "Cannot init MMI!" <<endl;
		g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
		return ;
	}

    g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIState);
	
    if (!CIState.ulCIState) {
		// Wait CAM insert
		cout << "Please insert the CAM ..." <<endl;

		dwStatus = WaitForSingleObject(hCIEvent, 6000);
		ResetEvent(hCIEvent);

		g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIState);
		if (!CIState.ulCIState) {
			cout << "--*--No CAM insert--*--" <<endl;
			g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
			g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
			return;
		}

		// Wait CAM initial
		cout << "Wait the CAM initial..." <<endl;

		dwStatus = WaitForSingleObject(hCIEvent, 6000);
		ResetEvent(hCIEvent);

		g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIState);
		if (!CIState.ulCIState) {
			cout << "--*--No CAM insert--*--" <<endl;
			g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
			g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
			return;
		}
    }

	BOOL bFirstFlag = TRUE;
    
    while (bFlag)
    {
		if (!bFirstFlag || CIState.ulEventMessage == MMI_STATUS_GET_MENU_INIT || CIState.ulEventMessage == MMI_STATUS_ANSWER_SEND)
		{
			dwStatus = WaitForSingleObject(hCIEvent, INFINITE);
			ResetEvent(hCIEvent);

			g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIState);

			if (CIState.ulEventMessage == MMI_STATUS_GET_MENU_FAIL) {
				cout << "Get MMI error!" <<endl;
				//g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
				//break;
			}
			else if (CIState.ulEventMessage == MMI_STATUS_GET_MENU_CLOSE) {
				cout << "MMI close!" <<endl;
				break;
			}
		}

		bFirstFlag = FALSE;

		g_pfg->THBDA_IOCTL_CI_GET_MMI_Fun(&MMIInfo);

		Count = 0;
        switch (MMIInfo.Type)
        {
            case 0:
                g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
				g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
                bFlag = FALSE;
                break;
            case 1:
            case 2:
            {
                cout <<" ========== MMI Infomation ========== "<< endl; 
                
                for (int i=0; i<MMIInfo.ItemCount; i++)
                {
                    cout << i + 1 << "."<< MMIInfo.MenuItem[i] << endl;
                }

				cout << "0.Return"<< endl;

                cout << "----------------" << endl;
                cout << MMIInfo.ItemCount + 1 << ".exit MMI" << endl;

                int iSel = 0;
                cout << endl << endl << "Please Input the code: ";
                cin >> iSel;
		        
                if (iSel == MMIInfo.ItemCount + 1) {
                    g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
					g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
                    return;
                }

                if (MMIInfo.Type == 1) { //Menu	
                    while ((iSel<0) || (iSel>MMIInfo.ItemCount))
                    {
                        cout <<"*********error code ,try again********" << endl;
                        cin >> iSel;
                    }
                    //answer here
                    MMIInfo.Answer = iSel;
                    g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
                }
                else if (MMIInfo.Type == 2) {			
                    MMIInfo.Answer = 0;
                    g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
                }
                
                //continue;
				break;
            }
            case 3:
                cout << endl << MMIInfo.Prompt << "(" 
                    << MMIInfo.Answer_Text_Length << "bytes):";
                
                cin >> MMIInfo.AnswerStr;			
			    
                MMIInfo.Answer = 3;
			    
                g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
			    
                //continue;
				break;
            
            default:
                bFlag = FALSE;
				MMIInfo.Answer = 0;
				MMIInfo.Type = 1;
                g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
				cout <<" No MMI information!! "<< endl; 
                break;
        }
    }

	g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
	g_pfg->THBDA_IOCTL_CI_EVENT_CLOSE_Fun(hCIEvent);
}

void GetMMIInfoPolling()
{
    THCIStateOld CIStateOld;
    THMMIInfo MMIInfo;
    BOOL      bFlag = TRUE;
	ULONG     Count=0;
	
    g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIStateOld);
	
    if (!CIStateOld.ulCIState) {
        cout << "--*--No CAM insert--*--" <<endl;
        return;
    }
    
    g_pfg->THBDA_IOCTL_CI_INIT_MMI_Fun();

    while (bFlag)
    {
		// Delay 6 s, or 3-5 s 
        if (!g_pfg->THBDA_IOCTL_CI_GET_MMI_Fun(&MMIInfo)) {
			Sleep(100);
			if (Count++<60) { 
				g_pfg->THBDA_IOCTL_CI_GET_STATE_Fun(&CIStateOld);

				if (CIStateOld.ulMMIState == MMI_STATUS_GET_MENU_CLOSE1_OLD) {
					cout << " --!!-- MMI Close --!!--\n" <<endl;
					break;
				}
				continue;
			}
			else {
				cout << " --!!-- Timeout %d s --!!--\n" <<endl;
				break;
			}
		}

		Count = 0;
        switch (MMIInfo.Type)
        {
            case 0:
                g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
                bFlag = FALSE;
                break;
            case 1:
            case 2:
            {
                cout <<" ========== MMI Infomation ========== "<< endl; 
                
                for (int i=0; i<MMIInfo.ItemCount; i++)
                {
                    cout << i + 1 << "."<< MMIInfo.MenuItem[i] << endl;
                }

                cout << "----------------" << endl;
                cout << MMIInfo.ItemCount + 1 << ".exit MMI" << endl;

                int iSel = 0;
                cout << endl << endl << "Please Input the code: ";
                cin >> iSel;
		        
                if (iSel == MMIInfo.ItemCount + 1) {
                    g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
                    return;
                }

                if (MMIInfo.Type == 1) { //Menu	
                    while ((iSel<0) || (iSel>MMIInfo.ItemCount))
                    {
                        cout <<"*********error code ,try again********" << endl;
                        cin >> iSel;
                    }
                    //answer here
                    MMIInfo.Answer = iSel;
                    g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
                }
                else if (MMIInfo.Type == 2) {			
                    MMIInfo.Answer = 0;
                    g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
                }
                
                continue;
            }
            case 3:
                cout << endl << MMIInfo.Prompt << "(" 
                    << MMIInfo.Answer_Text_Length << "bytes):";
                
                cin >> MMIInfo.AnswerStr;			
			    
                MMIInfo.Answer = 3;
			    
                g_pfg->THBDA_IOCTL_CI_ANSWER_Fun(&MMIInfo);
			    
                continue;
            
            default:
                bFlag = FALSE;
                break;
        }
    }

	g_pfg->THBDA_IOCTL_CI_CLOSE_MMI_Fun();
}

void GetSmartCardInfo()
{
	RAW_CMD_INFO rwCmdInfo;
	BYTE pData[256] = {0x9F, 0x8E, 0x00, 0x01, 0x06, 0x00};

	rwCmdInfo.dwSessionType = SESSION_TYPE_CA;
	rwCmdInfo.dwRawCmdBuffSize = 6;
	rwCmdInfo.pRawCmdBuff = (PVOID)pData;

	g_pfg->THBDA_IOCTL_CI_SEND_RAW_CMD_Fun((PBYTE)&rwCmdInfo, sizeof(RAW_CMD_INFO));
	g_pfg->THBDA_IOCTL_CI_GET_RAW_CMD_DATA_Fun((PBYTE)&rwCmdInfo, sizeof(RAW_CMD_INFO));
}

int main(int argc, char* argv[])
{
	HRESULT hr = CoInitialize (NULL);
    if (FAILED (hr))
    {
        printf("Err: Failed to initialize COM library!\0");
        return -1;
    }

	g_pfg = new CBDAFilterGraph();
	if (!g_pfg)
    {
        printf("Err: Failed to create the filter graph!");
        return -1;
    }

	if (FAILED(g_pfg->BuildGraph(L"MYDVB-S"))) {
        printf("Err: Could not Build the DVB-S BDA FilterGraph.");
		return -1;
    }

	THAppInfo AppInfo;
	g_pfg->THBDA_IOCTL_CI_GET_APP_INFO_Fun(&AppInfo);

	//GetSmartCardInfo();

	UINT  nPMTPID, nVideoPID = 0, nAudioPID = 0;
	UINT  nSIDNum = 0; //Program SID number, optional, set it to 0 for default

	BOOLEAN bLocked = false;
	LONG lSignalQuality = 0;		
    LONG lSignalStrength = 0;
	BOOL bExit = FALSE;
	char str[256];
	int iMenuSelect = 0;

	while(bExit == FALSE)
	{
		printf("-------------------------------------------\n");
		printf("|       THDVB Mantis BDA Test Menu        |\n");
		printf("-------------------------------------------\n");
        printf("|  1. Get device and driver Info          |\n");
        printf("|  2. Set DiSeqC/22k config(IOCTL)        |\n");		
		printf("|  3. Lock tuner                          |\n");		
        printf("|  4. Play                                |\n");		
		printf("|  5. Get signal quality & strength       |\n");		
		printf("|  6. Stop                                |\n");
		printf("|  7. Enable CAM-(A/V PID's in play)      |\n");
		printf("|  8. Enable CAM-(A/V PID's from Parser)  |\n");
		printf("|  9. Get CAPMT_Reply-(A/V PID's in play) |\n");
		printf("| 10. MMI (Polling mode)                  |\n");
		printf("| 11. MMI (Event mode)                    |\n");
		printf("|  0. Exit                                |\n");
        printf("|  Please Select Menu(0-8): ");
		memset(str, 0, 256);
		scanf("%s", str);
		printf("-------------------------------------------\n");
		
		iMenuSelect = atoi(str);		
		if(iMenuSelect < 0 || iMenuSelect > 11)
		{
			printf("Err: Please try again !!\n");
			continue;
		}

		switch(iMenuSelect)
		{
			case 1: // Get device and driver Info
			{				
				DEVICE_INFO DevInfo;
				memset(&DevInfo, 0, sizeof(DEVICE_INFO));
				DriverInfo  DrvInfo;
				memset(&DrvInfo, 0, sizeof(DriverInfo));
				g_pfg->THBDA_IOCTL_GET_DEVICE_INFO_Fun(&DevInfo);
				g_pfg->THBDA_IOCTL_GET_DRIVER_INFO_Fun(&DrvInfo);
				printf("Device Info: Device_Name=%s, Device_TYPE=%d, MAC=%x-%x-%x-%x-%x-%x \n", 
						DevInfo.Device_Name, DevInfo.Device_TYPE, 
						DevInfo.MAC_ADDRESS[0], DevInfo.MAC_ADDRESS[1], DevInfo.MAC_ADDRESS[2], DevInfo.MAC_ADDRESS[3], DevInfo.MAC_ADDRESS[4], DevInfo.MAC_ADDRESS[5]);
				printf("Driver Info: Company=%s, Version=%02x%02x\n",
						DrvInfo.Company, DrvInfo.Version_Major, DrvInfo.Version_Minor);
			}
			break;

			case 2: // Set DiSeqC/22k config(IOCTL)
			{
				UINT  nTemp;
				
				printf("DiSEqc (DiSEqC_NULL:0 | DiSEqC_A:1 | DiSEqC_B:2 | DiSEqC_C:3 | DiSEqC_D:4): ");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.DiSEqC_Port = nTemp;
				printf("LNB Power (LNB_POWER_ON:1 | LNB_POWER_OFF:0):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.LNB_POWER = nTemp;
				printf("LNBLOF LowBand: (MHz):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.ulLNBLOFLowBand = nTemp;
				printf("LNBLOF HighBand (MHz):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.ulLNBLOFHighBand = nTemp;
				printf("LNBLOF HiLoSW (MHz):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.ulLNBLOFHiLoSW = nTemp;

				printf("LNB Tone_Data_Burst (Tone_Data_OFF:0 | Tone_Burst_ON:1 | Data_Burst_ON:2):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.Tone_Data_Burst = nTemp;
				printf("22KHz Tone (F22K_Output_HiLo:0 | F22K_Output_Off:1 | F22K_Output_On:2):");
				scanf("%d", &nTemp);
				g_pfg->m_LNB_Data.f22K_Output = nTemp;

				if (!g_pfg->THBDA_IOCTL_SET_LNB_DATA_Fun(&g_pfg->m_LNB_Data)) {
					printf("THBDA_IOCTL_SET_LNB_DATA_Fun failed\n");
				}

			}
			break;

			case 3: // Lock tuner
			{
				printf("Frequency(MHz): ");
				scanf("%d", &g_pfg->m_ulCarrierFrequency);
				g_pfg->m_ulCarrierFrequency *= 1000;

				if (g_pfg->m_TunerFilterType == CLSID_DVBSNetworkProvider) {
					printf("Sample Rate(KHz): ");
					scanf("%d", &g_pfg->m_ulSymbolRate);				
					printf("Signal Polarisation (POLARITY_H:1 | POLARITY_V:0):");
					scanf("%d", &g_pfg->m_SignalPolarisation);
				}
				else if (g_pfg->m_TunerFilterType == CLSID_DVBTNetworkProvider) {
					printf("Bandwidth(MHz): ");
					scanf("%d", &g_pfg->m_ulBandwidth);				
				}
				else if (g_pfg->m_TunerFilterType == CLSID_DVBCNetworkProvider) {
					printf("Sample Rate(KHz): ");
					scanf("%d", &g_pfg->m_ulSymbolRate);				
					printf("QAM(1:16, 2:32, 3:64, 7:128, 11:256): ");
					scanf("%d", &g_pfg->m_ulQAM);
				}

				g_pfg->ChangeSetting();

			}
			break;

			case 4: // Play
			{
				printf("Video PID: ");
				scanf("%d", &nVideoPID);
				printf("Audio PID: ");
				scanf("%d", &nAudioPID);

				if(FAILED(g_pfg->RunGraph()))
				{
					printf("Err: Could not play the FilterGraph.");
					return -1;
				}

				Sleep(1000);
				g_pfg->SetVideoAndAudioPIDs(nVideoPID, nAudioPID);
			}
			break;

			case 5: // Get signal quality & strength
			{				
				g_pfg->GetTunerStatus(&bLocked, &lSignalQuality, &lSignalStrength);
				printf("Locked=%d, SignalQuality=%d, SignalStrength=%d \n", bLocked, lSignalQuality, lSignalStrength);
			}
			break;

			case 6: // Stop
			{
				if(FAILED(g_pfg->StopGraph()))
				{
					printf("Err: Could not stop the FilterGraph.");
					return -1;
				}
			}
			break;
			
			case 7: // Enable CAM-(A/V PID's in play) 
			{
				if (!g_pfg->m_fGraphRunning) {
					printf("Select option 'Play' in advance!! \n");
					break;
				}

				UINT  PIDAry[10], nPIDNum = 0, nTemp = 2;
				printf("PMT PID: ");
				scanf("%d", &nPMTPID);

				nPIDNum = 2;
				PIDAry[0] = nVideoPID;
				PIDAry[1] = nAudioPID;

				BYTE pBuff_CAPMT[4096];
				UINT byBuffSize = 0;
				if (FAILED(g_pfg->ComposeCAPMT(CA_PMT_LIST_MGT_ONLY, CA_PMT_CMDID_Decrambleing, nPMTPID, nSIDNum, PIDAry, nPIDNum, pBuff_CAPMT, &byBuffSize)))
				{
					printf("Err: Could not Enable CAM");
					return -1;
				}

				if (byBuffSize > 0) {
					g_pfg->THBDA_IOCTL_CI_SEND_PMT_Fun(pBuff_CAPMT, byBuffSize);
				}
			}
			break;

			case 8: // Enable CAM-(A/V PID's from Parser)
			{
				if (!g_pfg->m_fGraphRunning) {
					printf("Select option 'Play' in advance!! \n");
					break;
				}

				printf("PMT PID: ");
				scanf("%d", &nPMTPID);

				BYTE pBuff_CAPMT[4096];
				UINT byBuffSize = 0;
				if (FAILED(g_pfg->ComposeCAPMT(CA_PMT_LIST_MGT_ONLY, CA_PMT_CMDID_Decrambleing, nPMTPID, nSIDNum, NULL, 0, pBuff_CAPMT, &byBuffSize)))
				{
					printf("Err: Could not Enable CAM");
					return -1;
				}

				if (byBuffSize > 0) {
					g_pfg->THBDA_IOCTL_CI_SEND_PMT_Fun(pBuff_CAPMT, byBuffSize);
				}
			}
			break;

			case 9: // Get CAPMT_Reply-(A/V PID's in play)
			{
				if (!g_pfg->m_fGraphRunning) {
					printf("Select option 'Play' in advance!! \n");
					break;
				}

				UINT  PIDAry[10], nPIDNum = 0, nTemp = 2;
				printf("PMT PID: ");
				scanf("%d", &nPMTPID);

				nPIDNum = 2;
				PIDAry[0] = nVideoPID;
				PIDAry[1] = nAudioPID;

				BYTE pBuff_CAPMT[4096];
				UINT byBuffSize = 0;
				if (FAILED(g_pfg->ComposeCAPMT(CA_PMT_LIST_MGT_ONLY, CA_PMT_CMDID_Query, nPMTPID, nSIDNum, PIDAry, nPIDNum, pBuff_CAPMT, &byBuffSize)))
				{
					printf("Err: Could not Enable CAM");
					return -1;
				}

				if (byBuffSize > 0) {
					g_pfg->THBDA_IOCTL_CI_SEND_PMT_Fun(pBuff_CAPMT, byBuffSize);
				}

				Sleep(1000);
				BYTE pBuff_CAPMTReply[1024];
				g_pfg->THBDA_IOCTL_CI_GET_PMT_REPLY_Fun(pBuff_CAPMTReply, 1024);
			}
			break;

			case 10: // MMI (Polling mode)
			{
				GetMMIInfoPolling();
			}
			break;

			case 11: // MMI (Event mode)
			{
				GetMMIInfo();
			}
			break;

			case 0: //Exit
			{
				bExit = TRUE;
			}
			break;
		}
	}

	
	delete g_pfg;
	CoUninitialize ();
	return 0;
}

