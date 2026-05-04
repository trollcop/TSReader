#include <windows.h>
#include <stdio.h>

int nPIDContinuity[8192];
int nContinuityErrors = 0;

void CheckPIDContinuity(int nPID, BYTE * pBuffer)
{
	int nCurrentContinuity;
	BOOL fError = FALSE;

	int nAdaptation = (pBuffer[3] >> 4) & 0x03;
	nCurrentContinuity = pBuffer[3] & 0x0f;

	if (nPIDContinuity[nPID] == -1)
		nPIDContinuity[nPID] = nCurrentContinuity;
	if (nPIDContinuity[nPID] != nCurrentContinuity)
	{
		if (nAdaptation != 2)
		{
			if (nPIDContinuity[nPID] != nCurrentContinuity)
			{
				// See if this is a duplicate packet on the PID
				int nPreviousContinuity = nPIDContinuity[nPID] - 1;

				if (nPreviousContinuity == -1)
					nPreviousContinuity = 0x0f;
				if (nPreviousContinuity != nCurrentContinuity)
				{
					nContinuityErrors++;
				}
				nPIDContinuity[nPID] = nCurrentContinuity;
				fError = TRUE;
			}
		}
	}
	printf("%1x %1d ", nCurrentContinuity, nAdaptation);
	if (fError == TRUE)
		printf("************\n");
	else
		printf("\n");

	/*if (nPID == 0x1ffb)
	{
		DWORD dwWritten;
		char szTemp[128];
		char szErrorString[] = {"******************"};

		if (fError == FALSE)
			szErrorString[0] = 0;
		wsprintf(szTemp, "%02d %s\r\n", nPIDContinuity[nPID], szErrorString);
		WriteFile(hDebugFile, szTemp, lstrlen(szTemp), &dwWritten, NULL);
	}*/

	if (nAdaptation != 2)
	{
		nPIDContinuity[nPID]++;
		nPIDContinuity[nPID] &= 0x0f;
	}
}

int main(int argc, char * argv[])
{
	int i;
	int nTargetPID;
	HANDLE hFile;

	if (argc < 3)
	{
		printf("?Usage: ContinuityCheck.exe filename hex-PID\n");
		return -1;
	}

	hFile = CreateFile(argv[1], GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("?Unable to open file %s\n", argv[1]);
		return -1;
	}
	sscanf(argv[2], "%x", &nTargetPID);

	for (i = 0; i < 8192; i++)
		nPIDContinuity[i] = -1;
	
	printf("Processing file %s with PID 0x%04x", argv[1], nTargetPID);
	do
	{
		DWORD dwRead;
		int nPID;
		BYTE buffer[188];

		ReadFile(hFile, buffer, 188, &dwRead, NULL);
		if (dwRead != 188)
			break;
		if (buffer[0] != 0x47)
		{
			printf("?Sync missing\n");
			break;
		}
		nPID = (buffer[1] << 8 | buffer[2]) & 0x1fff;
		if (nPID == nTargetPID)
			CheckPIDContinuity(nPID, buffer);
	} while (TRUE);

	printf("Total continuity errors: %d\n", nContinuityErrors);

	CloseHandle(hFile);
	return 0;
}
