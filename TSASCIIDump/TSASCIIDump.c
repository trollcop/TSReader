#include <stdio.h>
#include <windows.h>

int main(int argc, char * argv[])
{
	int nOffset = 0;
	HANDLE hInputFile = CreateFile(argv[1], GENERIC_READ, 0, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	if (hInputFile == INVALID_HANDLE_VALUE)
	{
		printf("?Unable to open file %s\n", argv[1]);
		return 1;
	}

	do
	{
		int i;
		DWORD dwRead;
		BYTE bBuffer[188];

		ReadFile(hInputFile, bBuffer, 188, &dwRead, NULL);
		if (dwRead == 0)
			break;

		printf("%08x: ", nOffset);
		for (i = 0; i < 20; i++)
			printf("%02x ", bBuffer[i]);
		printf("\n");
		nOffset += 188;
	} while (nOffset < 0x1000);

	CloseHandle(hInputFile);

	return 0;
}