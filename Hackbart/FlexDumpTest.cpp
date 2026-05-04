#include <windows.h>
#include <stdio.h>

HANDLE hOutputFile;

typedef struct _tagTuner 
{   
  DWORD Frequency;
  DWORD SymbolRate;
  DWORD FEC;
  DWORD Polarity;
  DWORD LNBFreq;
  DWORD LNBSelection;
  DWORD Diseqc;
  DWORD Modulation;
  DWORD BandWidth; // only Terrestrial
  DWORD QAMMode; // only Cable
} TUNER, *PTUNER;

typedef HRESULT __stdcall td_TCallbackFunc(BYTE * buf, int len_in);
typedef HRESULT (__stdcall * td_GetTuner) (PTUNER Tuner);
typedef HRESULT (__stdcall * td_SetTuner) (PTUNER Tuner);
typedef DWORD (__stdcall * td_GetTunerType) ();
typedef HRESULT (__stdcall * td_StopStream) ();
typedef HRESULT (__stdcall * td_StartStream) (td_TCallbackFunc tCallbackFunction);
 
HRESULT (__stdcall * GetTuner) (PTUNER Tuner);
HRESULT (__stdcall * SetTuner) (PTUNER Tuner);
DWORD (__stdcall * GetTunerType) ();
HRESULT (__stdcall * StopStream) ();
HRESULT   (__stdcall * StartStream) (td_TCallbackFunc tCallbackFunction);

HRESULT __stdcall TCallBackFunc(BYTE * buf, int len_in)
{
	DWORD dwWritten;
	WriteFile(hOutputFile, buf, len_in, &dwWritten, NULL);
	return 0;
}

int main(int argc, char * argv[])
{
	hOutputFile = CreateFile("test.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES) NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL);

	HMODULE hFlexDump = LoadLibrary("flexdump.dll");
	if (hFlexDump != NULL)
	{
		// Load library
		GetTuner = (td_GetTuner)GetProcAddress(hFlexDump, "GetTuner");
		SetTuner = (td_SetTuner)GetProcAddress(hFlexDump, "SetTuner");
		GetTunerType = (td_GetTunerType)GetProcAddress(hFlexDump, "GetTunerType");
		StopStream = (td_StopStream)GetProcAddress(hFlexDump, "StopStream");
		StartStream = (td_StartStream)GetProcAddress(hFlexDump, "StartStream");

		// Display tuner type
		DWORD tt;
		tt = GetTunerType();
		switch(tt)
		{
		case 0:
			printf("FlexDump: cable type\n");
			break;
		case 1:
			printf("FlexDump: satellite type\n");
			break;
		case 2:
			printf("FlexDump: terrestrial type\n");
			break;
		case 3:
			printf("FlexDump: ATSC type\n");
			break;
		default:
			printf("FlexDump: Unknown device %08x\n", tt);
			break;
		}

		// Get some data
		HRESULT hr = StartStream(TCallBackFunc);
		printf("StartStream returned %08x\n", hr);
		hr = StartStream(TCallBackFunc); // without this we get a GPF in StopStream()
		Sleep(5000);
		StopStream();
			
		// All done
		FreeLibrary(hFlexDump);
	}
	CloseHandle(hOutputFile);

	return 0;
}
