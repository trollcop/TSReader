//#define TERRATECx

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <math.h>

#include "CyAPI.h"
#include "..\sources.h"

#ifndef TERRATEC
 #include "sasem_commands.h"
#endif TERRATEC

PSOURCESTRUCT ss;
BOOL fNeedTuneDialog = TRUE;
BOOL fDoneInit;
BOOL f256QAM;
int nFrequency;
#ifdef TERRATEC
int nBandwidth;
BOOL fSpectrumInversion;
#endif TERRATEC
CRITICAL_SECTION csSignal;
HANDLE hInstance;

CCyUSBDevice * USBDevice = NULL;
CCyBulkEndPoint * MPEGInEpt = NULL;   
OVERLAPPED outOvLap;
OVERLAPPED inOvLap; 

#ifndef TERRATEC
CCyBulkEndPoint * DownloadOutEpt = NULL;   
OVERLAPPED outOvLap2;
#endif TERRATEC

char szLastSignalReport[128] = {"n/a"};        
char szLastTune[128] = {"n/a"};

#ifndef TERRATEC
 #ifndef AW
  #define DEBUG_NAME "Sasem USB"
  #ifndef QAM
   #ifdef ENCODER
    char gszSourceName[] = {"Sasem OnAir USB Encoder"};
   #else ENCODER
    char gszSourceName[] = {"Sasem OnAir USB 8VSB"};
   #endif ENCODER
  #else QAM
   char gszSourceName[] = {"Sasem OnAir USB QAM"};
  #endif QAM
 #else AW
  #define DEBUG_NAME "AutumnWave USB"
  #ifndef QAM
   char gszSourceName[] = {"Autumn Wave OnAir USB 8VSB"};
  #else QAM
   char gszSourceName[] = {"Autumn Wave OnAir USB QAM"};
  #endif QAM
 #endif AW
 #else TERRATEC
  #define DEBUG_NAME "Terratec Cinergy T2"
   char gszSourceName[] = {"Terratec Cinergy T²"};
 #endif TERRATEC

extern "C" {
	BOOL __cdecl SourceHelper_ValidateSourceContainer(PSOURCESTRUCT pss);
}

BOOL SendUSB(BYTE * bOut, int nSize)
{
	LONG nBytesSent;
	int nRetVal;

	UCHAR  * outContext = USBDevice->BulkOutEndPt->BeginDataXfer(bOut, nSize, &outOvLap); 
	USBDevice->BulkOutEndPt->WaitForXfer(&outOvLap, 1000);
	nRetVal = USBDevice->BulkOutEndPt->FinishDataXfer(bOut, nBytesSent, &outOvLap, outContext);

	return nRetVal;
}

#ifndef TERRATEC
BOOL SendUSBEP2(BYTE * bOut, int nSize)
{
	LONG nBytesSent;

	UCHAR  * outContext = DownloadOutEpt->BeginDataXfer(bOut, nSize, &outOvLap2); 
	DownloadOutEpt->WaitForXfer(&outOvLap2, 1000);
	return (DownloadOutEpt->FinishDataXfer(bOut, nBytesSent, &outOvLap2, outContext));
}
#endif TERRATEC

int ReceiveUSB(BYTE * bIn, int nSize)
{
	int nRetVal;
	LONG nBytesReceived;

	UCHAR  *inContext = USBDevice->BulkInEndPt->BeginDataXfer(bIn, nSize, &inOvLap); 
	USBDevice->BulkInEndPt->WaitForXfer(&inOvLap, 500); 
	nRetVal = USBDevice->BulkInEndPt->FinishDataXfer(bIn, nBytesReceived, &inOvLap,inContext);

	return nBytesReceived;
}

void SetupLastTune()
{
#ifndef TERRATEC
 #ifndef QAM
  #ifdef ENCODER
	wsprintf(szLastTune, "Encoder Mode");
  #else ENCODER
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetATSCChannelFromFrequency(ss->nFrequency), ss->nFrequency);
  #endif ENCODER
 #else QAM
	wsprintf(szLastTune, "Channel %d (%d MHz)", SourceHelper_GetQAMChannelFromFrequency(ss->nFrequency), ss->nFrequency);
 #endif QAM
#else TERRATEC
	sprintf(szLastTune, "%.3f MHz", (double)ss->nFrequency / 1000.0);
	{
		char szTemp[128];
		wsprintf(szTemp, DEBUG_NAME": Tuner %s\n", szLastTune);
		OutputDebugString(szTemp);
	}
#endif TERRATEC
}

int GetSasemSignalLevel()
{
#ifndef AW
	BYTE bIn1[2], bIn2[2], bIn[1];
	BYTE bOut1[] = {0x08, 0x0e, 0x01, 0x4a, 0x38, 0x00, 0x00, 0x00, 0x00, 0xca, 0x8b};
	BYTE bOut2[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x20, 0x90, 0xf5, 0x83};
	BYTE bOut3[] = {0x08, 0x0e, 0x01, 0x4b, 0x01, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x66};

	SendUSB(bOut1, sizeof(bOut1));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn1, sizeof(bIn1));

	SendUSB(bOut3, sizeof(bOut3));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn2, sizeof(bIn2));

	return bIn1[1] << 8 | bIn2[1];
#else AW
	BYTE bIn1[2], bIn2[2], bIn[1];
	BYTE bOut1[] = {0x08, 0x0e, 0x01, 0x71, 0x38, 0x00, 0x00, 0x00, 0x00, 0xca, 0x8b};
	BYTE bOut2[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x20, 0x90, 0xf5, 0x83};
	BYTE bOut3[] = {0x08, 0x0e, 0x01, 0x72, 0x01, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x66};

	SendUSB(bOut1, sizeof(bOut1));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn1, sizeof(bIn1));

	SendUSB(bOut3, sizeof(bOut3));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn2, sizeof(bIn2));

	return bIn1[1] << 8 | bIn2[1];
#endif AW
}

#ifdef QAM
double QAM_SNR(int DATA)
{
	double snr;

	if((DATA>1021)&&(DATA<=1023))
		snr = 00.0 ;
	else if((DATA>773)&&(DATA<=1021))
		snr = 10.0 ;
	else if((DATA>769)&&(DATA<=773))
		snr = 18.0 ;
	else if((DATA>765)&&(DATA<=769))
		snr =18.5 ;
	else if((DATA>761)&&(DATA<=765))
		snr =19.0 ;
	else if((DATA>758)&&(DATA<=761))
		snr =19.5 ;
	else if((DATA>755)&&(DATA<=758))
		snr =20.0 ;
	else if(DATA==755)
		snr =20.5 ;
	else if(DATA==754)
		snr =21.0 ;
	else if((DATA>750)&&(DATA<=753))
		snr =21.5 ;
	else if((DATA>746)&&(DATA<=750))
		snr =22.0 ;
	else if((DATA>742)&&(DATA<=746))
		snr =22.5 ;
	else if((DATA>738)&&(DATA<=742))
		snr =23.0 ;
	else if((DATA>733)&&(DATA<=738))
		snr =23.5 ;
	else if((DATA>728)&&(DATA<=733))
		snr =24.0 ;
	else if((DATA>723)&&(DATA<=728))
		snr =24.5 ;
	else if((DATA>715)&&(DATA<=723))
		snr =25.0 ;
	else if((DATA>707)&&(DATA<=715))
		snr =25.5 ;
	else if((DATA>700)&&(DATA<=707))
		snr =26.0 ;
	else if((DATA>692)&&(DATA<=700))
		snr =26.5 ;
	else if((DATA>686)&&(DATA<=692))
		snr =27.0 ;
	else if((DATA>684)&&(DATA<=686))
		snr =27.1 ;
	else if((DATA>682)&&(DATA<=684))
		snr =27.2 ;
	else if((DATA>679)&&(DATA<=682))
		snr =27.3 ;
	else if((DATA>676)&&(DATA<=679))
		snr =27.4 ;
	else if((DATA>674)&&(DATA<=676))
		snr =27.5 ;
	else if((DATA>672)&&(DATA<=674))
		snr =27.6 ;
	else if((DATA>670)&&(DATA<=672))
		snr =27.7 ;
	else if((DATA>666)&&(DATA<=670))
		snr =27.8 ;
	else if((DATA>663)&&(DATA<=666))
		snr =27.9 ;
	else if((DATA>660)&&(DATA<=663))
		snr =28.0 ;
	else if((DATA>657)&&(DATA<=660))
		snr =28.1 ;
	else if((DATA>654)&&(DATA<=657))
		snr =28.2 ;
	else if((DATA>650)&&(DATA<=654))
		snr =28.3 ;
	else if((DATA>647)&&(DATA<=650))
		snr =28.4 ;
	else if((DATA>645)&&(DATA<=647))
		snr =28.5 ;
	else if((DATA>643)&&(DATA<=645))
		snr =28.6 ;
	else if((DATA>640)&&(DATA<=643))
		snr =28.7 ;
	else if((DATA>637)&&(DATA<=640))
		snr =28.8 ;
	else if((DATA>634)&&(DATA<=637))
		snr =28.9 ;
	else if((DATA>631)&&(DATA<=634))
		snr =29.0 ;
	else if((DATA>628)&&(DATA<=631))
		snr =29.1 ;
	else if((DATA>625)&&(DATA<=628))
		snr =29.2 ;
	else if((DATA>621)&&(DATA<=625))
		snr =29.3 ;
	else if((DATA>617)&&(DATA<=621))
		snr =29.4 ;
	else if((DATA>613)&&(DATA<=617))
		snr =29.5 ;
	else if((DATA>609)&&(DATA<=613))
		snr =29.6 ;
	else if((DATA>605)&&(DATA<=609))
		snr =29.7 ;
	else if((DATA>602)&&(DATA<=605))
		snr =29.8 ;
	else if((DATA>598)&&(DATA<=602))
		snr =29.9 ;
	else if((DATA>594)&&(DATA<=598))
		snr =30.0 ;
	else if((DATA>589)&&(DATA<=594))
		snr =30.1 ;
	else if((DATA>585)&&(DATA<=589))
		snr =30.2 ;
	else if((DATA>581)&&(DATA<=585))
		snr =30.3 ;
	else if((DATA>577)&&(DATA<=581))
		snr =30.4 ;
	else if((DATA>572)&&(DATA<=577))
		snr =30.5 ;
	else if((DATA>569)&&(DATA<=572))
		snr =30.6 ;
	else if((DATA>564)&&(DATA<=569))
		snr =30.7 ;
	else if((DATA>560)&&(DATA<=564))
		snr =30.8 ;
	else if((DATA>556)&&(DATA<=560))
		snr =30.9 ;
	else if((DATA>551)&&(DATA<=556))
		snr =31.0 ;
	else if((DATA>547)&&(DATA<=551))
		snr =31.1 ;
	else if((DATA>542)&&(DATA<=547))
		snr =31.2 ;
	else if((DATA>537)&&(DATA<=542))
		snr =31.3 ;
	else if((DATA>532)&&(DATA<=537))
		snr =31.4 ;
	else if((DATA>527)&&(DATA<=532))
		snr =31.5 ;
	else if((DATA>522)&&(DATA<=527))
		snr =31.6 ;
	else if((DATA>517)&&(DATA<=522))
		snr =31.7 ;
	else if((DATA>512)&&(DATA<=517))
		snr =31.8 ;
	else if((DATA>507)&&(DATA<=512))
		snr =31.9 ;
	else if((DATA>501)&&(DATA<=507))
		snr =32.0 ;
	else if((DATA>495)&&(DATA<=501))
		snr =32.1 ;
	else if((DATA>490)&&(DATA<=495))
		snr =32.2 ;
	else if((DATA>484)&&(DATA<=490))
		snr =32.3 ;
	else if((DATA>478)&&(DATA<=484))
		snr =32.4 ;
	else if((DATA>472)&&(DATA<=478))
		snr =32.5 ;
	else if((DATA>467)&&(DATA<=472))
		snr =32.6 ;
	else if((DATA>461)&&(DATA<=467))
		snr =32.7 ;
	else if((DATA>455)&&(DATA<=461))
		snr =32.8 ;
	else if((DATA>448)&&(DATA<=455))
		snr =32.9 ;
	else if((DATA>442)&&(DATA<=448))
		snr =33.0 ;
	else if((DATA>437)&&(DATA<=442))
		snr =33.1 ;
	else if((DATA>431)&&(DATA<=437))
		snr =33.2 ;
	else if((DATA>425)&&(DATA<=431))
		snr =33.3 ;
	else if((DATA>419)&&(DATA<=425))
		snr =33.4 ;
	else if((DATA>412)&&(DATA<=419))
		snr =33.5 ;
	else if((DATA>407)&&(DATA<=412))
		snr =33.6 ;
	else if((DATA>400)&&(DATA<=407))
		snr =33.7 ;
	else if((DATA>393)&&(DATA<=400))
		snr =33.8 ;
	else if((DATA>388)&&(DATA<=393))
		snr =33.9 ;
	else if((DATA>381)&&(DATA<=388))
		snr =34.0 ;
	else if((DATA>375)&&(DATA<=381))
		snr =34.1 ;
	else if((DATA>369)&&(DATA<=375))
		snr =34.2 ;
	else if((DATA>362)&&(DATA<=369))
		snr =34.3 ;
	else if((DATA>356)&&(DATA<=362))
		snr =34.4 ;
	else if((DATA>350)&&(DATA<=356))
		snr =34.5 ;
	else if((DATA>343)&&(DATA<=350))
		snr =34.6 ;
	else if((DATA>337)&&(DATA<=343))
		snr =34.7 ;
	else if((DATA>331)&&(DATA<=337))
		snr =34.8 ;
	else if((DATA>324)&&(DATA<=331))
		snr =34.9 ;
	else if(DATA<=324)
		snr =35.0 ;
	else 
		snr =35.0 ;

	return snr;
}

BOOL IsSasemQAMLocked()
{
#ifndef AW
	BYTE bOut4[] = {0x08, 0x0e, 0x01, 0x69, 0x01, 0x00, 0x00, 0x00, 0x00, 0xec, 0x2b};
	BYTE bOut5[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0xc0, 0x38, 0x24, 0x86};
	BYTE bIn[1], bIn3[2];
	
	SendUSB(bOut4, sizeof(bOut4));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut5, sizeof(bOut5));
	ReceiveUSB(bIn3, sizeof(bIn3));

	return ((bIn3[1] & 0x0f) == 0x0f);
#else AW
	/*DWORD signalPower = 688128;
	DWORD lComData;
	double snr;
	int test;
	
	BYTE bIn1[2], bIn2[2], bIn[1];
	BYTE bOut1[] = {0x08, 0x0e, 0x01, 0x1a, 0x38, 0x00, 0x00, 0x00, 0x00, 0xca, 0x8b};
	BYTE bOut2[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x20, 0x90, 0xf5, 0x83};
	BYTE bOut3[] = {0x08, 0x0e, 0x01, 0x1b, 0x01, 0x00, 0x00, 0x00, 0x00, 0xbc, 0x66};

	SendUSB(bOut1, sizeof(bOut1));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn1, sizeof(bIn1));

	SendUSB(bOut3, sizeof(bOut3));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn2, sizeof(bIn2));
	lComData = bIn1[1] << 8 | bIn2[1];

	{
		BYTE bIn1[2], bIn2[2], bIn[1];
		BYTE bOut1[] = {0x08, 0x0e, 0x01, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0xca, 0x8b};
		BYTE bOut2[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x20, 0x90, 0xf5, 0x83};
		
		SendUSB(bOut1, sizeof(bOut1));
		ReceiveUSB(bIn, sizeof(bIn));
		SendUSB(bOut2, sizeof(bOut2));
		ReceiveUSB(bIn1, sizeof(bIn1));
		test = bIn1[1];
	}

	if (f256QAM) // QAM 256
		signalPower = 696320;

	if ((lComData != 0xFFFF) && (lComData !=0))
	{
		snr = 10 * log10(double(signalPower / lComData ));
	}
	else
	{
		snr = 0;
	}
	{
		char szTemp[128];
		sprintf(szTemp, "SNR = %.3f lComData = %d test = %d\n", snr, lComData, test);
		OutputDebugString(szTemp);
	}
	return (snr > 0);
	*/
	BYTE bIn1[2], bIn[1];
	BYTE bOut1[] = {0x08, 0x0e, 0x01, 0x8a, 0x38, 0x00, 0x00, 0x00, 0x00, 0xca, 0x8b};
	BYTE bOut2[] = {0x09, 0x00, 0x01, 0x0e, 0x05, 0x00, 0x00, 0x00, 0x20, 0x90, 0xf5, 0x83};

	SendUSB(bOut1, sizeof(bOut1));
	ReceiveUSB(bIn, sizeof(bIn));
	SendUSB(bOut2, sizeof(bOut2));
	ReceiveUSB(bIn1, sizeof(bIn1));
	if ((bIn1[1] & 0x0f) == 0x0f)
		return TRUE;
	return FALSE;
#endif AW
}
#endif QAM

void SendTunerFrequency(int nFrequency)
{
#ifndef AW
	int nTuneFrequency = (nFrequency + 44) * 16;
	BYTE bOut[11];

	bOut[0] = 0x08;
	bOut[1] = 0x61;
	bOut[2] = 0x04;
	bOut[3] = nTuneFrequency >> 8;
	bOut[4] = nTuneFrequency & 0xff;
	bOut[5] = 0x8e;
	if (ss->nFrequency <= 85)	// channels 2-6
		bOut[6] = 0xa1;
	else if (ss->nFrequency < 400)	// channels 7-14
		bOut[6] = 0x91;
	else
		bOut[6] = 0x31;
	bOut[7] = 0x00;
	bOut[8] = 0x00;
	bOut[9] = 0x00;
	bOut[10] = 0x00;
	SendUSB(bOut, sizeof(bOut));
	Sleep(50);
#else AW
	int nTuneFrequency = (nFrequency + 44) * 16;
	BYTE bOut[11];

	bOut[0] = 0x08;
	bOut[1] = 0x61;
	bOut[2] = 0x04;
	bOut[3] = nTuneFrequency >> 8;
	bOut[4] = nTuneFrequency & 0xff;
	bOut[5] = 0xce;
	if (ss->nFrequency <= 85)	// channels 2-6
		bOut[6] = 0x01;
	else if (ss->nFrequency < 400)	// channels 7-14
		bOut[6] = 0x02;
	else
		bOut[6] = 0x04;
	bOut[7] = 0xff;
	bOut[8] = 0x00;
	bOut[9] = 0x00;
	bOut[10] = 0x00;
	SendUSB(bOut, sizeof(bOut));
	Sleep(50);
#endif AW
}

BOOL TSReader_Tune()
{
	BOOL fLocked = TRUE;
#ifndef TERRATEC
	int nOffset = 0;
	int nCheckSignal = 0;
	BYTE bUSBReceiver[64];

	SetupLastTune();
 #ifndef QAM
  #ifdef ENCODER
   #define USB_TUNER_COMMANDS usbanalog_configure
  #else ENCODER
   #ifndef AW
    #define USB_TUNER_COMMANDS usb_tune_8vsb
   #else AW
    #define USB_TUNER_COMMANDS usbaw_tune
   #endif AW
  #endif ENCODER
 #else QAM
  #ifndef AW
   #define USB_TUNER_COMMANDS usb_tune_cable
  #else AW
   #define USB_TUNER_COMMANDS usbaw_cable
  #endif AW
 #endif QAM
	while (USB_TUNER_COMMANDS[nOffset].nLength)
	{
		switch(nOffset)
		{
#ifdef ENCODER
		case -1:	// just to stop the compiler bitching
			break;
#else ENCODER
 #ifndef QAM
  #ifndef AW
		case 78:
		case 88:
  #else AW
		case 871:
  #endif AW
 #else QAM
  #ifndef AW
		case 84:
  #else AW
		case 908:
  #endif AW
 #endif QAM
			SendTunerFrequency(ss->nFrequency);
			break;
#endif ENCODER
		default:
			if (USB_TUNER_COMMANDS[nOffset].nEndPoint == 0x01)
			{
				SendUSB(USB_TUNER_COMMANDS[nOffset].pOutData, USB_TUNER_COMMANDS[nOffset].nLength);
			}
			else
			{
				int nRXLength;
				nRXLength = ReceiveUSB(bUSBReceiver, USB_TUNER_COMMANDS[nOffset].nLength);
			}
			break;
		}
		nOffset++;
	}	

#ifndef ENCODER
 #ifndef QAM
  #ifdef AW
	Sleep(250);
  #endif AW
	GetSasemSignalLevel();
	nCheckSignal = GetSasemSignalLevel();
	{
		char szTemp[128];
		wsprintf(szTemp, "Signal level = %05d %x ", nCheckSignal, nCheckSignal);
		OutputDebugString(szTemp);
	}
	if (nCheckSignal == 0xffff)
		fLocked = FALSE;
 #else QAM
	f256QAM = TRUE;
#ifdef AW
	Sleep(250);
#endif AW
	fLocked = IsSasemQAMLocked();
 #endif QAM

 #ifdef QAM
	if (!fLocked)
	{
		// Try 64QAM
		f256QAM = FALSE;
		nOffset = 0;
		while (usbqam64switch[nOffset].nLength)
		{
			if (usbqam64switch[nOffset].nEndPoint == 0x01)
				SendUSB(usbqam64switch[nOffset].pOutData, usbqam64switch[nOffset].nLength);
			else
			{
				int nRXLength = ReceiveUSB(bUSBReceiver, usbqam64switch[nOffset].nLength);
			}
			nOffset++;
		}
	}
	Sleep(250);		// give it time to lock
	fLocked = IsSasemQAMLocked();
 #endif QAM
#else ENCODER
	fLocked = TRUE;
#endif ENCODER

#else TERRATEC
	DWORD dwStartLockTime;
	BYTE bOut[9];
	BYTE bInDummy[2];

	fLocked = FALSE;
	SetupLastTune();
	bOut[0] = 0x04;				// tune command
	bOut[4] = (ss->nFrequency >> 24) & 0xff;
	bOut[3] = (ss->nFrequency >> 16) & 0xff;
	bOut[2] = (ss->nFrequency >> 8) & 0xff;
	bOut[1] = ss->nFrequency & 0xff;
	switch(ss->nBandwidth)
	{
	case 0:
		bOut[5] = 6;
		break;
	case 1:
		bOut[5] = 7;
		break;
	case 2:
		bOut[5] = 8;
		break;
	}
	bOut[6] = 0;				// TPS parameters
	bOut[7] = 0;
	bOut[8] = 0;				// Flags (all auto)
	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString(DEBUG_NAME": Tune command failed\n");
	ReceiveUSB(bInDummy, 0);

	// Wait for a potential lock
	dwStartLockTime = GetTickCount();
	while (TRUE)
	{
		BYTE bTunerData[25];

		if ((GetTickCount() - dwStartLockTime) > 3000)
		{
			OutputDebugString(DEBUG_NAME": No lock after 3 seconds\n");
			break;
		}
		// See if we're locked
		bOut[0] = 0x05;
		SendUSB(bOut, 1);
		ReceiveUSB(bTunerData, sizeof(bTunerData));
		if (bTunerData[23] == 0xff)
		{
			fLocked = TRUE;
			break;	// we've locked!
		}
		Sleep(50);
	}

#endif TERRATEC
	if (!fLocked)
	{
		if (ss->fQuietMode == FALSE)
			MessageBox(ss->hWndTSReader, "Failed to lock signal", gszSourceName, MB_ICONWARNING);
		return FALSE;
	}
	return TRUE;
}

void EnableDMA(BOOL fEnable)
{
#ifndef ENCODER
	BYTE bOut[2];

#ifndef TERRATEC
	if (fEnable)
		bOut[0] = 0xa0;
	else
		bOut[0] = 0xa1;
	bOut[1] = 0x00;
#else TERRATEC
	bOut[0] = 0x03;
	bOut[1] = fEnable;
#endif TERRATEC

	if (!SendUSB(bOut, sizeof(bOut)))
		OutputDebugString(DEBUG_NAME": EnableDMA failed\n");
#ifdef TERRATEC
	{
		BYTE bInDummy[2];
		ReceiveUSB(bInDummy, 0);
	}
#endif TERRATEC
#else ENCODER
/*
	int nOffset = 0;
	BYTE bUSBReceiver[64];

	if (!fEnable)
		return;

	while (usbanalog_start[nOffset].nLength)
	{
		if (usbanalog_start[nOffset].nEndPoint == 0x01)
		{
			SendUSB(usbanalog_start[nOffset].pOutData, usbanalog_start[nOffset].nLength);
		}
		else
		{
			int nRXLength;
			nRXLength = ReceiveUSB(bUSBReceiver, usbanalog_start[nOffset].nLength);
		}
		nOffset++;
	}	
*/
#endif ENCODER
}

void CheckLockStatus()
{
#ifndef ENCODER
	EnterCriticalSection(&csSignal);
#ifndef TERRATEC
 #ifndef QAM
	{
		int nSignalLevel = GetSasemSignalLevel();
		if (nSignalLevel < 0xffff)
		{
#ifndef AW
			double fSNR = 10 * log10(double(25 * 1024) / nSignalLevel);
#else AW
			double fSNR = 10 * log10(double((25 * 32 * 32) / nSignalLevel));
#endif AW
			//double fSNR = 31.0 - ((double)nSignalLevel / 32.0);
			if (fSNR < 0.0)
				fSNR = 0.0;
			sprintf(szLastSignalReport, "Locked SNR %.1f dB", fSNR);
		}
		else
		{
			lstrcpy(szLastSignalReport, "Unlocked");
		}
	}
 #else QAM
	if (IsSasemQAMLocked() == TRUE)
	{
		if (f256QAM)
			lstrcpy(szLastSignalReport, "Locked: 256QAM");
		else
			lstrcpy(szLastSignalReport, "Locked: 64QAM");
	}
	else
		lstrcpy(szLastSignalReport, "Unlocked");
 #endif QAM
#else TERRATEC
#endif TERRATEC
	LeaveCriticalSection(&csSignal);
#endif ENCODER
}

#define READ_FROM_SASEM_SIZE 128 * 1024
DWORD WINAPI ReadMPEGINThread(LPVOID lpv)
{
	int nReadPtr = 0;
	int nCurrentBuffer;
	BOOL fFirstPacket = TRUE;
	BOOL fRestart = TRUE;
	int nSignalCheckCounter = 64;
	//HANDLE hDebug;

	OutputDebugString(DEBUG_NAME": +ReadMPEGINThread\n");
	//hDebug = CreateFile("c:\\tsreader.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	ss->fReadThreadTerminated = FALSE;
	ss->fTerminateReadThread = FALSE;

	// Setup the pipe used to sync up with the TS packets and 
	// feed TSReader	
	SourceHelper_StartSyncThread(ss, FALSE);

RestartSasemDataThread:
	fRestart = FALSE;
	LONG nTransferLength = READ_FROM_SASEM_SIZE;
	MPEGInEpt->SetXferSize(nTransferLength);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 8;
	PUCHAR *buffers = new PUCHAR[nQueueSize];
	PUCHAR *contexts = new PUCHAR[nQueueSize];
	OVERLAPPED * inMPEGOvLap = new OVERLAPPED[nQueueSize];

	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{ 
       buffers[nCurrentBuffer] = new UCHAR[nTransferLength];
	   inMPEGOvLap[nCurrentBuffer].Internal = inMPEGOvLap[nCurrentBuffer].InternalHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].Offset = inMPEGOvLap[nCurrentBuffer].OffsetHigh = 0;
	   inMPEGOvLap[nCurrentBuffer].hEvent = CreateEvent(NULL, false, false, NULL);
	}

	// Queue-up the first batch of transfer requests
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	   contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
	EnableDMA(TRUE);

	nCurrentBuffer = 0;
	//OutputDebugString(DEBUG_NAME": enter main read loop\n");
	while (!ss->fTerminateReadThread)
	{
		LONG nReceiveLength = 0;
		
		if (!MPEGInEpt->WaitForXfer(&inMPEGOvLap[nCurrentBuffer], 2500))
		{
			char szTemp[128];
			wsprintf(szTemp, DEBUG_NAME": WaitForXfer() timed out buffer = %d ***********\n", nCurrentBuffer);
			OutputDebugString(szTemp);
			fRestart = TRUE;
			break;
		}

		if (MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer],
			contexts[nCurrentBuffer]))
		{
			if (fFirstPacket == TRUE)
				fFirstPacket = FALSE;
			else
			{
				SourceHelper_SyncData(buffers[nCurrentBuffer], nReceiveLength);
				{
					//DWORD dwWritten;
					//WriteFile(hDebug, buffers[nCurrentBuffer], nReceiveLength, &dwWritten, NULL);
				}
			}
		}
		else
			Sleep(1);

	    contexts[nCurrentBuffer] = MPEGInEpt->BeginDataXfer(buffers[nCurrentBuffer], nTransferLength, &inMPEGOvLap[nCurrentBuffer]);
		nCurrentBuffer++;
		if (nCurrentBuffer == nQueueSize)
			nCurrentBuffer = 0;
		if (nSignalCheckCounter++ > 64)
		{
			nSignalCheckCounter = 0;
			CheckLockStatus();
		}
	}
	OutputDebugString(DEBUG_NAME": left main read loop\n");
	CheckLockStatus();

	MPEGInEpt->Abort();
	for (nCurrentBuffer = 0; nCurrentBuffer < nQueueSize; nCurrentBuffer++)
	{
		// Wait for all the queued requests to be cancelled 
		LONG nReceiveLength = 0;

		MPEGInEpt->FinishDataXfer(buffers[nCurrentBuffer], nReceiveLength, &inMPEGOvLap[nCurrentBuffer], contexts[nCurrentBuffer]);
		CloseHandle(inMPEGOvLap[nCurrentBuffer].hEvent);
		delete [] buffers[nCurrentBuffer];
	}

	delete [] buffers;
	delete [] contexts;
	delete [] inMPEGOvLap;

	EnableDMA(FALSE);
	if (fRestart == TRUE && !ss->fTerminateReadThread)
	{
		//SendTunerFrequency(ss->nFrequency);
		goto RestartSasemDataThread;
	}

	SourceHelper_StopSyncThread();

	ss->fReadThreadTerminated = TRUE;
	ss->fTerminateReadThread = FALSE;
	EnterCriticalSection(&ss->csTSBuffersInUse);
	ss->nTSBuffersInUse = -1000;
	LeaveCriticalSection(&ss->csTSBuffersInUse);
	CloseHandle(ss->hReadDataThread);
	//CloseHandle(hDebug);
	OutputDebugString(DEBUG_NAME": -ReadMPEGINThread\n");
	return 0;
	goto RestartSasemDataThread;	// to stop the compiler bitching
}

BOOL TSReader_Start()
{
	DWORD dwThreadID;

	OutputDebugString(DEBUG_NAME": enter Start()\n");

	ss->hReadDataThread = CreateThread(NULL, 0, ReadMPEGINThread, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
	SourceHelper_SetWorkerThreadPriorities(FALSE);
	ResumeThread(ss->hReadDataThread);

	OutputDebugString(DEBUG_NAME": leave Start()\n");
	return TRUE;
}

BOOL TSReader_Stop()
{
	OutputDebugString(DEBUG_NAME": enter Stop()\n");

	ss->fTerminateReadThread = TRUE;
	while (ss->fReadThreadTerminated == FALSE)
		Sleep(50);

#ifdef ENCODER
	{
		int nOffset = 0;
		BYTE bUSBReceiver[64];

		while (usbanalog_stop[nOffset].nLength)
		{
			if (usbanalog_stop[nOffset].nEndPoint == 0x01)
			{
				SendUSB(usbanalog_stop[nOffset].pOutData, usbanalog_stop[nOffset].nLength);
			}
			else
			{
				int nRXLength;
				nRXLength = ReceiveUSB(bUSBReceiver, usbanalog_stop[nOffset].nLength);
			}
			nOffset++;
		}
	}
#endif ENCODER
	
	OutputDebugString(DEBUG_NAME": leave Stop()\n");
	return TRUE;
}

BOOL OpenUSBDriver()
{
	int d = 0;
	GUID OASISguid;

	OASISguid.Data1 = 0xFA58C45D;
	OASISguid.Data2 = 0x5B19; OASISguid.Data3 = 0x428b;
	OASISguid.Data4[0] = 0xA2; OASISguid.Data4[1] = 0xD1; OASISguid.Data4[2] = 0x27; OASISguid.Data4[3] = 0x07;
	OASISguid.Data4[4] = 0x85; OASISguid.Data4[5] = 0x6D; OASISguid.Data4[6] = 0x7E; OASISguid.Data4[7] = 0x19;
	
	USBDevice = new CCyUSBDevice(NULL, OASISguid);
    int nDevices = USBDevice->DeviceCount(); 
	if (nDevices == 0)
	{
		MessageBox(NULL, "Unable to locate the "DEBUG_NAME" interface. Make sure you've switched to the DTVWorks driver.", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	// Find the Sasem device by it's vendor number
    do
	{
		USBDevice->Open(d);   // Open automatically  calls Close() if necessary 

		int vID = USBDevice->VendorID; 
        int pID  = USBDevice->ProductID;
#ifndef TERRATEC
 #ifndef AW
		if ( (vID == 0x11BA) && (pID == 0x1001 || pID == 0x1002) )
 #else AW
		if ( (vID == 0x11BA) && (pID == 0x1003 || pID == 0x1101) )
 #endif AW
#else TERRATEC
		if (vID == 0x0ccd && pID == 0x0038)
#endif TERRATEC
			break;
		USBDevice->Close();
		d++;
    } while (d < nDevices);

	if (d == nDevices)
	{
		MessageBox(NULL, "Unable to locate the "DEBUG_NAME" interface", gszSourceName, MB_ICONSTOP);
		return FALSE;
	}

	if (USBDevice->EndPoints == NULL)
	{
		OutputDebugString(DEBUG_NAME": NULL endpoint from the USB device\n");
		return FALSE;
	}

	if (USBDevice->BulkOutEndPt == NULL)
	{
		OutputDebugString(DEBUG_NAME": Couldn't find control out endpoint\n");
		return FALSE;
	}

	return TRUE;
}

void CloseOvelapppedEvents()
{
	CloseHandle(outOvLap.hEvent);
	CloseHandle(inOvLap.hEvent);
#ifndef TERRATEC
	CloseHandle(outOvLap2.hEvent);
#endif TERRATEC
}

#ifdef TERRATEC
void SetTerratecSleepStatus(int nEnable)
{
	BYTE bOut[2];
	BYTE bDummy[2];

	bOut[0] = 0x09;		// sleep mode select
	bOut[1] = nEnable;
	SendUSB(bOut, sizeof(bOut));
	ReceiveUSB(bDummy, 0);
}
#endif TERRATEC

#define SASEM_FIRMWARE_LENGTH 256 * 1024
#define SASEM_FIRMWARE_CHUNK_SIZE 32 * 1024

BOOL TSReader_Init(PSOURCESTRUCT pss)
{
	int nIndex;
	int nEndPointCount;

	if (SourceHelper_ValidateSourceContainer(pss) == FALSE)
		return FALSE;
	OutputDebugString(DEBUG_NAME": Init\n");
	lstrcpy(szLastSignalReport, "n/a");
	InitializeCriticalSection(&csSignal);

	ss = pss;

	outOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT"); 
	inOvLap.hEvent = CreateEvent(NULL, false, false, "CYUSB_IN"); 
#ifndef TERRATEC
	outOvLap2.hEvent = CreateEvent(NULL, false, false, "CYUSB_OUT2"); 
#endif TERRATEC

	MPEGInEpt = (CCyBulkEndPoint *)NULL;
#ifndef TERRATEC
	DownloadOutEpt = (CCyBulkEndPoint *)NULL;
#endif TERRATEC

	if (OpenUSBDriver() == FALSE)
		return FALSE;

	// Find the USB pipe we use for data transfer (endpoint IN 4 for Sasem, IN 2 for TerraTec)

	nEndPointCount = USBDevice->EndPointCount();  
	for (nIndex = 1; nIndex < nEndPointCount; nIndex++)
	{
#ifndef TERRATEC
		if (USBDevice->EndPoints[nIndex]->Address == 0x84)
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
		if (USBDevice->EndPoints[nIndex]->Address == 0x02)
			DownloadOutEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
#else TERRATEC
		if (USBDevice->EndPoints[nIndex]->Address == 0x82)
			MPEGInEpt = (CCyBulkEndPoint *)USBDevice->EndPoints[nIndex];
#endif TERRATEC
	}
#ifndef TERRATEC
	if (MPEGInEpt == (CCyBulkEndPoint *)NULL || DownloadOutEpt == (CCyBulkEndPoint *)NULL)
#else TERRATEC
	if (MPEGInEpt == (CCyBulkEndPoint *)NULL)
#endif TERRATEC
	{
		OutputDebugString(DEBUG_NAME": Couldn't find endpoinst\n");
		return FALSE;
	}

#ifndef TERRATEC
	if (!fDoneInit)
	{
		int nOffset = 0;
		int i;
		int nFirmwareFileSize, nFirmwareOffset;
		HANDLE hFirmwareFile;
		HANDLE hMap;
		BYTE * pFirmware;
		char szFirmwareFilename[MAX_PATH];
		BYTE bUSBReceiver[512];

		// Load the firmware from the Sasem driver file
		GetModuleFileName(ss->hTSReaderInst, szFirmwareFilename, sizeof(szFirmwareFilename));
		for (i = lstrlen(szFirmwareFilename); i > 0; i--)
		{
			if (szFirmwareFilename[i] == '\\')
			{
				szFirmwareFilename[i + 1] = '\0';
				break;
			}
		}		
		lstrcat(szFirmwareFilename, "USBHDTV.SYS");
		hFirmwareFile = CreateFile(szFirmwareFilename, GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		if (hFirmwareFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(NULL, "Unable to locate the Sasem/Autumn Wave driver file. Please locate the file called USBHDTV.SYS\nand copy it into the TSReader folder.\n\nThis file can be found in one of the following locations:\n\n\tC:\\Program Files\\OnAir USB HDTV\nor\n\tC:\\Windows\\System32\\Drivers", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
		nFirmwareFileSize = GetFileSize(hFirmwareFile, NULL);
		hMap = CreateFileMapping(hFirmwareFile, NULL, PAGE_READONLY | SEC_COMMIT, 0,  0, NULL);
		if (hMap == NULL)
		{
			OutputDebugString(DEBUG_NAME": CreateFileMapping failed\n");
			return FALSE;
		}
		pFirmware = (BYTE *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		if (pFirmware == NULL)
		{
			OutputDebugString(DEBUG_NAME": MapViewOfFile failed\n");
			return FALSE;
		}
		// Locate encoder firmware
		for (nFirmwareOffset = 0; nFirmwareOffset < nFirmwareFileSize; nFirmwareOffset++)
		{
			if (   pFirmware[3 + nFirmwareOffset] == 0x00
				&& pFirmware[2 + nFirmwareOffset] == 0x00
				&& pFirmware[1 + nFirmwareOffset] == 0x0d
				&& pFirmware[0 + nFirmwareOffset] == 0xa7
				&& pFirmware[7 + nFirmwareOffset] == 0xaa
				&& pFirmware[6 + nFirmwareOffset] == 0x55
				&& pFirmware[5 + nFirmwareOffset] == 0xbb
				&& pFirmware[4 + nFirmwareOffset] == 0x66)
			{
				break;
			}
		}
		if (nFirmwareOffset == nFirmwareFileSize)
		{
			MessageBox(NULL, "Unable to locate firmware in Sasem/Autumn Wave driver file. Please contact rod@coolstf.com for advice!", gszSourceName, MB_ICONSTOP);
			return FALSE;
		}
			
		fDoneInit = TRUE;
		while (usb_setup[nOffset].nLength)
		{
			switch(nOffset)
			{
			case -1:
				break;
			default:
				switch(usb_setup[nOffset].nEndPoint)
				{
				case 0x01:
					SendUSB(usb_setup[nOffset].pOutData, usb_setup[nOffset].nLength);
					if (nOffset < 100)
						Sleep(10);
					break;
				case 0x02:
					{
						int nRemaining = SASEM_FIRMWARE_LENGTH;
						BYTE * bCurrentFirmware = (BYTE *)LocalAlloc(LPTR, SASEM_FIRMWARE_CHUNK_SIZE);

						while (nRemaining)
						{
							int nCurrentChunkOffset;

							for (nCurrentChunkOffset = 0; nCurrentChunkOffset < SASEM_FIRMWARE_CHUNK_SIZE; nCurrentChunkOffset += 4)
							{
								bCurrentFirmware[nCurrentChunkOffset + 0] = pFirmware[nFirmwareOffset + 3];
								bCurrentFirmware[nCurrentChunkOffset + 1] = pFirmware[nFirmwareOffset + 2];
								bCurrentFirmware[nCurrentChunkOffset + 2] = pFirmware[nFirmwareOffset + 1];
								bCurrentFirmware[nCurrentChunkOffset + 3] = pFirmware[nFirmwareOffset + 0];
								nFirmwareOffset += 4;
							}
							SendUSBEP2(bCurrentFirmware, SASEM_FIRMWARE_CHUNK_SIZE);
							nRemaining -= SASEM_FIRMWARE_CHUNK_SIZE;
						}

						LocalFree(bCurrentFirmware);
					}
					break;
				case 0x81:
					{
						int nRXLength;

						nRXLength = ReceiveUSB(bUSBReceiver, usb_setup[nOffset].nLength);
					}
					break;				
				}
				break;
			}
			nOffset++;
		}
		
		UnmapViewOfFile(pFirmware);
		CloseHandle(hMap);
		CloseHandle(hFirmwareFile);
	}
#else TERRATEC
	{
		BYTE bOut[1];
		BYTE bIn[3];
		char szTemp[128];

		bOut[0] = 0x08;			// firmware/sleep status request
		SendUSB(bOut, sizeof(bOut));
		ReceiveUSB(bIn, sizeof(bIn));

		wsprintf(szTemp, DEBUG_NAME": Firmware version = 0x%x Sleep = 0x%x\n", bIn[0] << 8 | bIn[1], bIn[2]);
		OutputDebugString(szTemp);
		if (bIn[2] && 0x01)
			SetTerratecSleepStatus(0x00); // wakeup
	}

#endif TERRATEC

	return TRUE;
}

BOOL TSReader_DeInit()
{
	OutputDebugString(DEBUG_NAME": +DeInit\n");

#ifdef TERRATEC
	//if (MPEGInEpt != NULL)
	//	SetTerratecSleepStatus(0x01); // sleep
#endif TERRATEC

	CloseOvelapppedEvents();

	{
		char szTemp[128];
		wsprintf(szTemp, DEBUG_NAME": USBDevice->EndPoints = 0x%08x\n", USBDevice->EndPoints);
		OutputDebugString(szTemp);
	}
	if (USBDevice->EndPoints != NULL)
		delete USBDevice;

	DeleteCriticalSection(&csSignal);
	OutputDebugString(DEBUG_NAME": -DeInit\n");
	return TRUE;
}

BOOL TSReader_TuneDialog(HWND hWnd)
{
#ifdef ENCODER
	return TRUE;
#else ENCODER
	OutputDebugString(DEBUG_NAME": TSReader_TuneDialog\n");

	if (ss->fDontTune == TRUE)
		fNeedTuneDialog = FALSE;

	if (fNeedTuneDialog)
	{
		OutputDebugString(DEBUG_NAME": TSReader_TuneDialog tuning dialog is required\n");
		ss->fQuietMode = FALSE;
#ifndef TERRATEC
 #ifndef QAM
		if (SourceHelper_ATSCTuneDialog(hWnd) == FALSE)
 #else QAM
		if (SourceHelper_QAMTuneDialog(hWnd) == FALSE)
 #endif QAM
#else TERRATEC
		if (SourceHelper_DVBTTuneDialog(hWnd) == FALSE)
#endif TERRATEC
			return FALSE;
	}
	else
	{
		OutputDebugString(DEBUG_NAME": TSReader_TuneDialog tune NOT required\n");
		ss->nFrequency = nFrequency;
#ifdef TERRATEC
		ss->nBandwidth = nBandwidth;
#endif TERRATEC
		fNeedTuneDialog = TRUE;
	}

	return TRUE;
#endif ENCODER
}

BOOL TSReader_PIDManagement(BOOL fAdd, int nPID, BOOL fTemporary)
{
	return TRUE;
}

BOOL TSReader_GetDescription(char * szDescription, char * szCommandLineParameters, BOOL * fCanBeStopped, int * nMaxPIDs, DWORD * dwCapabilities)
{
	if (szDescription != NULL)
		lstrcpy(szDescription, gszSourceName);
	if (fCanBeStopped != NULL)
		*fCanBeStopped = FALSE;
	if (nMaxPIDs != NULL)
		*nMaxPIDs = 8192;
	if (szCommandLineParameters != NULL)
		lstrcpy(szCommandLineParameters, "freq");	
	if (dwCapabilities != NULL)
		*dwCapabilities = 0;

	return TRUE;
}

BOOL TSReader_ParseCommandLine(PSOURCESTRUCT ss, char * szCommandLine, BOOL fQuiet)
{
#ifndef ENCODER
	fNeedTuneDialog = TRUE;
	if (lstrlen(szCommandLine))
	{
		int nConversionCount;

#ifdef TERRATEC
		nConversionCount = sscanf(szCommandLine,
								  "%d %d %d", 
								  &nFrequency,
								  &fSpectrumInversion,
								  &nBandwidth);
		if (nConversionCount < 3)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq inversion bandwidth\n"
					   "\n"
					   "freq = frequency to tune in KHz\n"
					   "inversion = inverted spectrum (0 or 1)\n"
					   "bandwidth = bandwidth of signal (0 = 6, 1 = 7, 2 = 8 MHz)",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
#else TERRATEC
		nConversionCount = sscanf(szCommandLine,
								  "%d", 
								  &nFrequency);
		if (nConversionCount < 1)
		{
			if (!fQuiet)
				MessageBox(NULL,
					   "Usage for this source: freq\n"
					   "\n"
					   "freq = frequency to tune in MHz or prefix with 0 for channel number, e.g. 022 for channel 22",
					   gszSourceName,
					   MB_OK | MB_ICONSTOP);
			return FALSE;
		}
		if (*szCommandLine == '0')
 #ifndef QAM
			nFrequency = SourceHelper_GetFrequencyFromATSCChannel(nFrequency);
 #else QAM
			nFrequency = SourceHelper_GetFrequencyFromQAMChannel(nFrequency);
 #endif QAM
#endif TERRATEC
			fNeedTuneDialog = FALSE;
	}
	else
		fNeedTuneDialog = TRUE;
#endif ENCODER

	return TRUE;
}

BOOL TSReader_IsPIDActive(int nPID)
{
	return TRUE;
}

BOOL TSReader_SetChannel(int nChannel)
{
	return TRUE;
}

BOOL TSReader_GetSignalString(char * szString)
{
	EnterCriticalSection(&csSignal);
	lstrcpy(szString, szLastSignalReport);
	LeaveCriticalSection(&csSignal);
	return TRUE;
}

BOOL TSReader_GetTunerString(char * szString)
{
	lstrcpy(szString, szLastTune);
	return TRUE;
}

BOOL TSReader_SendDiSEqC(BYTE * bCommand, int nLength)
{
	return TRUE;
}

int TSReader_GetSyncLossCount(BOOL fReset)
{
	return SourceHelper_GetSyncLossCount(fReset);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		hInstance = (HINSTANCE)hModule;
		fDoneInit = FALSE;
		break;
    case DLL_PROCESS_DETACH:
		break;
    }
    return TRUE;
}


/*

TunerIface(0,INIM_DMODE_TUNER_ID,&lComData);


 if(lComData==LGTDVSH062F_LG3303) // 5th NIM module(NEW)
 {
  TunerIface(0,INIM_DMODE_SNR_REGISTER,&lComData);
  if(m_LockedTunerMode==1 ) // VSB mode in case
  {

   if ((lComData != 0xFFFFFF) && (lComData !=0))
   {
    snr = 10 * log10(double((25 * 32 * 32) / lComData ));
   }
   else
   {
    snr = 0;
   }
  }
  else if(m_LockedTunerMode==2 ) // QAM mode in case
  {
   DWORD signalPower;
   signalPower = 688128;
   if (PbankPtr->nLockedFlag1 == 3) // QAM 256
   {
    signalPower = 696320;
   }
   if ((lComData != 0xFFFF) && (lComData !=0))
   {
    snr = 10 * log10(double(signalPower / lComData ));
   }
   else
   {
     snr = 0;
   }
  }
 }
 else // old Tuner / methode
 {
  if(m_LockedTunerMode==2 ) //QAM mode
  {
   int DATA = 0;
   nReturn = FastLgVsbSetReg(0,I2C_LG3302_VSB_ADDRESS, 0x00, 0x1a, 1);
   if(nReturn) 
   {
    if(!FastLgVsbGetReg(1,I2C_LG3302_VSB_ADDRESS, &dwData2, 1))
    {
     dwData2 = 0;
     bFailed = TRUE;
    }
   }
   else
   {
    bFailed = TRUE;
    TRACE(" Failed Get SNR Value");
   }
   nReturn = FastLgVsbSetReg(1,I2C_LG3302_VSB_ADDRESS, 0x00, 0x1b, 1);
   if(nReturn)
   {
    if(!FastLgVsbGetReg(1,I2C_LG3302_VSB_ADDRESS, &dwData1, 1))
    {
     bFailed = TRUE;
     dwData1 = 0;
    }
   }
   else
   {
    bFailed = TRUE;
    TRACE(" Failed Get SNR Value\r\n");
   }

 

  
   if(bFailed)
   {
    snr = 0;
   }
   else
   {
    DATA = ((dwData2&0x03) << 8) | dwData1;
    snr = QAM_SNR(DATA);
   }
  }
  else
  {
   nReturn = FastLgVsbSetReg(0,I2C_LG3302_VSB_ADDRESS, 0x00, 0x4a, 1);
   if(nReturn) 
   {
    if(!FastLgVsbGetReg(1,I2C_LG3302_VSB_ADDRESS, &dwData2, 1))
    {
     dwData2 = 0;
     bFailed = TRUE;
    }
   }
   else
   {
    bFailed = TRUE;
   }
   nReturn = FastLgVsbSetReg(1,I2C_LG3302_VSB_ADDRESS, 0x00, 0x4b, 1);
   if(nReturn)
   {
    if(!FastLgVsbGetReg(1,I2C_LG3302_VSB_ADDRESS, &dwData1, 1))
    {
     dwData1 = 0;
     bFailed = TRUE;
    }
   }
   else
   {
    bFailed = TRUE;
   }

 

   if(bFailed)
   {
    snr = 0;
   }
   else
   {
    dwTotal = (dwData2 << 8) | dwData1;
    snr = 10 * log10(double(25 * 1024)/dwTotal);
   }
  }
 }

 

 

double CQTVSet::QAM_SNR(int DATA)
{
 double snr;

 

 //TRACE("CQTVSet::QAM_SNR()\r\n");
 
 if((DATA>1021)&&(DATA<=1023))
  snr = 00.0 ;
 else if((DATA>773)&&(DATA<=1021))
  snr = 10.0 ;
 else if((DATA>769)&&(DATA<=773))
  snr = 18.0 ;
 else if((DATA>765)&&(DATA<=769))
  snr =18.5 ;
 else if((DATA>761)&&(DATA<=765))
  snr =19.0 ;
 else if((DATA>758)&&(DATA<=761))
  snr =19.5 ;
 else if((DATA>755)&&(DATA<=758))
  snr =20.0 ;
 else if(DATA==755)
  snr =20.5 ;
 else if(DATA==754)
  snr =21.0 ;
 else if((DATA>750)&&(DATA<=753))
  snr =21.5 ;
 else if((DATA>746)&&(DATA<=750))
  snr =22.0 ;
 else if((DATA>742)&&(DATA<=746))
  snr =22.5 ;
 else if((DATA>738)&&(DATA<=742))
  snr =23.0 ;
 else if((DATA>733)&&(DATA<=738))
  snr =23.5 ;
 else if((DATA>728)&&(DATA<=733))
  snr =24.0 ;
 else if((DATA>723)&&(DATA<=728))
  snr =24.5 ;
 else if((DATA>715)&&(DATA<=723))
  snr =25.0 ;
 else if((DATA>707)&&(DATA<=715))
  snr =25.5 ;
 else if((DATA>700)&&(DATA<=707))
  snr =26.0 ;
 else if((DATA>692)&&(DATA<=700))
  snr =26.5 ;
 else if((DATA>686)&&(DATA<=692))
  snr =27.0 ;
 else if((DATA>684)&&(DATA<=686))
  snr =27.1 ;
 else if((DATA>682)&&(DATA<=684))
  snr =27.2 ;
 else if((DATA>679)&&(DATA<=682))
  snr =27.3 ;
 else if((DATA>676)&&(DATA<=679))
  snr =27.4 ;
 else if((DATA>674)&&(DATA<=676))
  snr =27.5 ;
 else if((DATA>672)&&(DATA<=674))
  snr =27.6 ;
 else if((DATA>670)&&(DATA<=672))
  snr =27.7 ;
 else if((DATA>666)&&(DATA<=670))
  snr =27.8 ;
 else if((DATA>663)&&(DATA<=666))
  snr =27.9 ;
 else if((DATA>660)&&(DATA<=663))
  snr =28.0 ;
 else if((DATA>657)&&(DATA<=660))
  snr =28.1 ;
 else if((DATA>654)&&(DATA<=657))
  snr =28.2 ;
 else if((DATA>650)&&(DATA<=654))
  snr =28.3 ;
 else if((DATA>647)&&(DATA<=650))
  snr =28.4 ;
 else if((DATA>645)&&(DATA<=647))
  snr =28.5 ;
 else if((DATA>643)&&(DATA<=645))
  snr =28.6 ;
 else if((DATA>640)&&(DATA<=643))
  snr =28.7 ;
 else if((DATA>637)&&(DATA<=640))
  snr =28.8 ;
 else if((DATA>634)&&(DATA<=637))
  snr =28.9 ;
 else if((DATA>631)&&(DATA<=634))
  snr =29.0 ;
 else if((DATA>628)&&(DATA<=631))
  snr =29.1 ;
 else if((DATA>625)&&(DATA<=628))
  snr =29.2 ;
 else if((DATA>621)&&(DATA<=625))
  snr =29.3 ;
 else if((DATA>617)&&(DATA<=621))
  snr =29.4 ;
 else if((DATA>613)&&(DATA<=617))
  snr =29.5 ;
 else if((DATA>609)&&(DATA<=613))
  snr =29.6 ;
 else if((DATA>605)&&(DATA<=609))
  snr =29.7 ;
 else if((DATA>602)&&(DATA<=605))
  snr =29.8 ;
 else if((DATA>598)&&(DATA<=602))
  snr =29.9 ;
 else if((DATA>594)&&(DATA<=598))
  snr =30.0 ;
 else if((DATA>589)&&(DATA<=594))
  snr =30.1 ;
 else if((DATA>585)&&(DATA<=589))
  snr =30.2 ;
 else if((DATA>581)&&(DATA<=585))
  snr =30.3 ;
 else if((DATA>577)&&(DATA<=581))
  snr =30.4 ;
 else if((DATA>572)&&(DATA<=577))
  snr =30.5 ;
 else if((DATA>569)&&(DATA<=572))
  snr =30.6 ;
 else if((DATA>564)&&(DATA<=569))
  snr =30.7 ;
 else if((DATA>560)&&(DATA<=564))
  snr =30.8 ;
 else if((DATA>556)&&(DATA<=560))
  snr =30.9 ;
 else if((DATA>551)&&(DATA<=556))
  snr =31.0 ;
 else if((DATA>547)&&(DATA<=551))
  snr =31.1 ;
 else if((DATA>542)&&(DATA<=547))
  snr =31.2 ;
 else if((DATA>537)&&(DATA<=542))
  snr =31.3 ;
 else if((DATA>532)&&(DATA<=537))
  snr =31.4 ;
 else if((DATA>527)&&(DATA<=532))
  snr =31.5 ;
 else if((DATA>522)&&(DATA<=527))
  snr =31.6 ;
 else if((DATA>517)&&(DATA<=522))
  snr =31.7 ;
 else if((DATA>512)&&(DATA<=517))
  snr =31.8 ;
 else if((DATA>507)&&(DATA<=512))
  snr =31.9 ;
 else if((DATA>501)&&(DATA<=507))
  snr =32.0 ;
 else if((DATA>495)&&(DATA<=501))
  snr =32.1 ;
 else if((DATA>490)&&(DATA<=495))
  snr =32.2 ;
 else if((DATA>484)&&(DATA<=490))
  snr =32.3 ;
 else if((DATA>478)&&(DATA<=484))
  snr =32.4 ;
 else if((DATA>472)&&(DATA<=478))
  snr =32.5 ;
 else if((DATA>467)&&(DATA<=472))
  snr =32.6 ;
 else if((DATA>461)&&(DATA<=467))
  snr =32.7 ;
 else if((DATA>455)&&(DATA<=461))
  snr =32.8 ;
 else if((DATA>448)&&(DATA<=455))
  snr =32.9 ;
 else if((DATA>442)&&(DATA<=448))
  snr =33.0 ;
 else if((DATA>437)&&(DATA<=442))
  snr =33.1 ;
 else if((DATA>431)&&(DATA<=437))
  snr =33.2 ;
 else if((DATA>425)&&(DATA<=431))
  snr =33.3 ;
 else if((DATA>419)&&(DATA<=425))
  snr =33.4 ;
 else if((DATA>412)&&(DATA<=419))
  snr =33.5 ;
 else if((DATA>407)&&(DATA<=412))
  snr =33.6 ;
 else if((DATA>400)&&(DATA<=407))
  snr =33.7 ;
 else if((DATA>393)&&(DATA<=400))
  snr =33.8 ;
 else if((DATA>388)&&(DATA<=393))
  snr =33.9 ;
 else if((DATA>381)&&(DATA<=388))
  snr =34.0 ;
 else if((DATA>375)&&(DATA<=381))
  snr =34.1 ;
 else if((DATA>369)&&(DATA<=375))
  snr =34.2 ;
 else if((DATA>362)&&(DATA<=369))
  snr =34.3 ;
 else if((DATA>356)&&(DATA<=362))
  snr =34.4 ;
 else if((DATA>350)&&(DATA<=356))
  snr =34.5 ;
 else if((DATA>343)&&(DATA<=350))
  snr =34.6 ;
 else if((DATA>337)&&(DATA<=343))
  snr =34.7 ;
 else if((DATA>331)&&(DATA<=337))
  snr =34.8 ;
 else if((DATA>324)&&(DATA<=331))
  snr =34.9 ;
 else if(DATA<=324)
  snr =35.0 ;
 else 
  snr =35.0 ;
 
 return snr;
}



*/