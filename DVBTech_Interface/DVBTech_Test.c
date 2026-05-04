#include <windows.h>
#include <conio.h>
#include <stdio.h>

// Typedefs and pointers to the DLL functions
typedef void (* td_Start) ();
typedef void (* td_Stop) ();
typedef BOOL (* td_Tune) (int nFrequency, int nSymbolRate, int nPolarity, int nModulationMode, int nCodeRate, 
		                  int nLNBFrequency, BOOL f22KHz, int nDiSEqCInput, int nTuneTimeoutMS);
typedef BOOL (* td_GetSignal) (BOOL * fLocked, double * dSNR);
typedef int  (* td_ReadTransportData) (unsigned char * pBuffer, int nMaxSize, int nTimeoutMS);

void (* Start) ();
void (* Stop) ();
BOOL (* Tune) (int nFrequency, int nSymbolRate, int nPolarity, int nModulationMode, int nCodeRate, 
		       int nLNBFrequency, BOOL f22KHz, int nDiSEqCInput, int nTuneTimeoutMS);
BOOL (* GetSignal) (BOOL * fLocked, double * dSNR);
int  (* ReadTransportData) (unsigned char * pBuffer, int nMaxSize, int nTimeoutMS);

// Modulation modes
#define MOD_DVB_QPSK 0						// DVB-S QPSK
#define MOD_TURBO_QPSK 1					// Turbo QPSK
#define MOD_TURBO_8PSK 2					// Turbo 8PSK (also used for Trellis 8PSK DVB-SNG)
#define MOD_TURBO_16QAM 3					// Turbo 16QAM (also used for Trellis 16QAM DVB-SNG)
#define MOD_DCII_C_QPSK 4					// Digicipher II Combo
#define MOD_DCII_I_QPSK 5					// Digicipher II I-stream
#define MOD_DCII_Q_QPSK 6					// Digicipher II Q-stream
#define MOD_DCII_C_OQPSK 7					// Digicipher II offset QPSK

// FEC code rates
#define FEC_DVB_QPSK_12		0				// DVB-S 1/2 
#define FEC_DVB_QPSK_23		1				// DVB-S 2/3
#define FEC_DVB_QPSK_34		2				// DVB-S 3/4
#define FEC_DVB_QPSK_56		3				// DVB-S 5/6
#define FEC_DVB_QPSK_78		4				// DVB-S 7/8
#define FEC_DVB_QPSK_AUTO	5				// DVB-S Automatic rate

#define FEC_TURBO_QPSK_14	0				// Turbo QPSK 1/4
#define FEC_TURBO_QPSK_12	1				// Turbo QPSK 1/2
#define FEC_TURBO_QPSK_34	2				// Turbo QPSK 3/4
#define FEC_TURBO_8PSK_23	0				// Turbo 8PSK 2/3
#define FEC_TURBO_8PSK_34I	1				// Turbo 8PSK 3/4 I
#define FEC_TURBO_8PSK_34II	2				// Turbo 8PSK 3/4 II
#define FEC_TURBO_8PSK_56	3				// Turbo 8PSK 5/6
#define FEC_TURBO_8PSK_89	4				// Turbo 8PSK 8/9
#define FEC_TURBO_16QAM_34	0				// Turbo 16QAM 3/4

#define FEC_DCII_QPSK_511	0				// DCII 5/11
#define FEC_DCII_QPSK_12	1				// DCII 1/2
#define FEC_DCII_QPSK_35	2				// DCII 3/5
#define FEC_DCII_QPSK_23	3				// DCII 2/3
#define FEC_DCII_QPSK_34	4				// DCII 3/4
#define FEC_DCII_QPSK_45	5				// DCII 4/5
#define FEC_DCII_QPSK_56	6				// DCII 5/6
#define FEC_DCII_QPSK_78	7				// DCII 7/8
#define FEC_DCII_QPSK_AUTO	8				// DCII Automatic rate

#define DISEQC_NONE			0				// No DiSEqC port selection
#define DISEQC_PORT1		1				// DiSEqC port 1
#define DISEQC_PORT2		2				// DiSEqC port 2
#define DISEQC_PORT3		3				// DiSEqC port 3
#define DISEQC_PORT4		4				// DiSEqC port 4
#define DISEQC_TB_A			5				// Tonebust A
#define DISEQC_TB_B			6				// Tonebust B

// Code
int main(int argc, char * argv[])
{
	HANDLE hInterfaceDLL;
	BOOL fRetVal;

	// Load the DVBTech Interface Library and get pointers to it's functions
	hInterfaceDLL = LoadLibrary("DVBTech_Interface.dll");
	if (hInterfaceDLL == NULL)
	{
		printf("?Unable to load DVBTech_Interface.dll\n");
		return 0;
	}
	Start = (td_Start)GetProcAddress(hInterfaceDLL, "Start");
	Stop = (td_Stop)GetProcAddress(hInterfaceDLL, "Stop");
	Tune = (td_Tune)GetProcAddress(hInterfaceDLL, "Tune");
	GetSignal = (td_GetSignal)GetProcAddress(hInterfaceDLL, "GetSignal");
	ReadTransportData = (td_ReadTransportData)GetProcAddress(hInterfaceDLL, "ReadTransportData");

	// Tune to a transponder
	fRetVal = Tune(12050,				// Frequency in MHz
		           19510,				// Symbol rate in KSps
				   0,					// 14v(V/RHCP) = 0 18v(H/LHCP) = 1
				   MOD_DCII_C_QPSK,		// Modulation mode
				   FEC_DCII_QPSK_AUTO,	// Auto code rate
				   10750,				// LNB Frequency in MHz
				   0,					// 22 KHz tone (0 = off, 1 = on)
				   DISEQC_PORT1,		// DiSEqC port
				   5000);				// Five seconds max to lock
	if (fRetVal)
	{
		// Now we're locked, read the data and get the signal every now and then
		HANDLE hOutputFile;

		hOutputFile = CreateFile("dvbtech.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL,
			                     CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);
		if (hOutputFile != INVALID_HANDLE_VALUE)
		{
			int nTotalWritten = 0;
			DWORD dwNextSignalTime = GetTickCount();

			printf("Receiving TS - hit any key to quit\n");
			Start();
			while (!_kbhit())
			{
				int nReturnedData;
				BOOL fLocked;
				double dSNR;
				BYTE bBuffer[100 * 188];

				// Read data from the interface - MUST be a buffer size divisible by 188
				nReturnedData = ReadTransportData(bBuffer, sizeof(bBuffer), 1000);
				if (nReturnedData)
				{
					DWORD dwWritten;
					WriteFile(hOutputFile, bBuffer, nReturnedData, &dwWritten, NULL);
					nTotalWritten += (int)dwWritten;
				}
				
				// Output signal/statistics every second
				if (GetTickCount() > dwNextSignalTime)
				{
					fRetVal = GetSignal(&fLocked, &dSNR);
					printf("GetSignal = %d Locked = %d SNR = %.2f dB\t%d MB written\n",
						   fRetVal, fLocked, dSNR,
						   nTotalWritten / 1024 / 1024);
					dwNextSignalTime = GetTickCount() + 1000;
				}
			}
			Stop();
			CloseHandle(hOutputFile);
			printf("\n");
		}
		else
			printf("?Unable to open dvbtech.ts for output\n");
	}
	else
		printf("?Failed to lock signal\n");

	// All done with the library
	FreeLibrary(hInterfaceDLL);

	return 0;
}
