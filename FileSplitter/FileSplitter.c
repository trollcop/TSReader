#include <windows.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
	HANDLE hInputFile = CreateFile("c:\\1026_H10M12_CH22-2.ts", GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	HANDLE hOutputFile = CreateFile("c:\\1026_H10M12_CH22-2.small.ts", GENERIC_WRITE, 0, (LPSECURITY_ATTRIBUTES)NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	int i;

	for (i = 0; i < 100; i++)
	{
		DWORD dwRead, dwWritten;
		BYTE buffer[1024];

		ReadFile(hInputFile, buffer, sizeof(buffer), &dwRead, NULL);
		WriteFile(hOutputFile, buffer, dwRead, &dwWritten, NULL);
	}
	CloseHandle(hInputFile);
	CloseHandle(hOutputFile);
	

	/*HANDLE hInputFile = CreateFile("c:\\JimDeCarlo.small.ts", GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	int i;

	for (i = 0; i < 100; i++)
	{
		int j;
		DWORD dwRead;
		BYTE buffer[188];

		ReadFile(hInputFile, buffer, sizeof(buffer), &dwRead, NULL);
		for (j = 0; j < 4; j++)
			printf("%02x ", buffer[j]);
		printf("\n");
	}
	CloseHandle(hInputFile);*/

	return 0;
}
