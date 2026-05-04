/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********  main.c                                               **********/
/**********                                                       **********/
/**********                                                       **********/
/**********  This is a test harness for the IneoQuest MDI         **********/
/**********  functions.  It takes a PCAP file as input.           **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <direct.h>
#include <string.h>
#include "mditest.h"
#include "mdi.h"
#include "pcap.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                   Local Definitions                   **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#define MAXARGS		2
#define MINARGS		1

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                  Static Declarations                  **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
static tFRAME frame;
static tMDISTREAM *pStream;
static MDITIME lastTime;
static MDITIME sampleTime;
static unsigned int nRecords;
static int hdrBytes = 42;
static int mtspSize = 188;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                        Swap32                         **********/
/**********                                                       **********/
/**********  Byte-swap a 32-bit value.                            **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
unsigned __int32 Swap32(unsigned __int32 n32)
{
	__asm mov eax,n32
	__asm bswap eax
	__asm mov n32,eax

	return n32;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                        Swap16                         **********/
/**********                                                       **********/
/**********  Byte-swap a 16-bit value.                            **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
unsigned __int16 Swap16(unsigned __int16 n16)
{
	__asm mov ax,n16
	__asm xchg al,ah
	__asm mov n16,ax

	return n16;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                          Punt                         **********/
/**********                                                       **********/
/**********  This function is called before an error exit.  When  **********/
/**********  running this program under the debuffer this allows  **********/
/**********  me to see the error message before the program       **********/
/**********  terminates.                                          **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void Punt(void)
{
#ifdef _DEBUG
	getchar();
#endif
	exit(1);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                         Usage                         **********/
/**********                                                       **********/
/**********  Display the usage error message.                     **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void Usage(void)
{
	printf("usage: mdi infile [outfile] [-v] [-hNNN] [-mNNN]\n");
	printf("       infile refers to a PCAP file\n");
	printf("       outfile defaults to mdi.out\n");
	printf("       -v specifies VLAN headers (data offset = 46)\n");
	printf("       -hNNN specifies custom header size where NNN is a decimal number\n");
	printf("       -mNNN specifies custom MTSP size (default = 188)\n");
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                         main                          **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
int main(int argc, char *argv[])
{
	int i;
	char *pArg;
	char *arg[MAXARGS];
	int nArgs = 0;
	tMDISAMPLE sample;
	FILE *hIn;						/* handle of input file */
	FILE *hOut;						/* handle of output file */

	/* Set the default output filename. */
	arg[1] = "mdi.xls";

	/* Process command line arguments. */
	for (i = 1; i < argc; ++i)
	{
		pArg = argv[i];

		/* Process switches (arguments preceded by a '-' character). */
		if (*pArg == '-')
		{
			switch (tolower(pArg[1]))
			{
			case '?':
			default:
				Usage();
				Punt();

			/* VLAN switch, forces header size to 46. */
			case 'v':
				hdrBytes = 46;
				break;

			/* HEADER switch, allows any value for header size. */
			case 'h':
				sscanf(&pArg[2], "%u", &hdrBytes);
				break;

			/* MTSP switch, allows any value for MTSP size.  */
			case 'm':
				sscanf(&pArg[2], "%u", &mtspSize);
				break;
			}
			continue;
		}

		/* If not a switch then place the argument
		   into the next free arg slot.  If no free
		   slots left then display a usage error and exit. */
		if (nArgs < MAXARGS)
			arg[nArgs++] = pArg;
		else
		{
			Usage();
			Punt();
		}
	}

	/* If we didn't get enough arguments
	   then display a usage error and exit. */
	if (nArgs < MINARGS)
	{
		Usage();
		Punt();
	}

	/* Open the designated input file for reading. */
	hIn = fopen(arg[0], "rb");
	if (!hIn)
	{
		printf("error: cannot open %s\n", arg[0]);
		Punt();
	}

	/* Perform a simple PCAP validation test. */
	if (0 != pcapValidate(hIn))
	{
		printf("error: %s is not a valid PCAP file\n", arg[0]);
		Punt();
	}

	/* Open the output file for writing. */
	hOut = fopen(arg[1], "wt");
	if (!hOut)
	{
		printf("error: cannot create %s\n", arg[1]);
		Punt();
	}

	/* Allocate a buffer to contain a single record from the PCAP file. */
	frame.pBuffer = malloc(65536);

	/* Begin monitoring the stream. */
	pStream = mdiStreamInit(mtspSize);

	nRecords = 0;

	while (0 == pcapRead(&frame, hIn))
	{
		if (1 == ++nRecords)
		{
			lastTime = frame.ts;
			sampleTime = frame.ts + 1000000000;
		}
		else
		{
			if (frame.ts >= sampleTime)
			{
				mdiSample(sampleTime - lastTime, pStream, &sample);
#if MDI_USE_FLOAT
				fprintf(hOut, "% 8.3f % 5u % 9u\n\n", sample.delayFactor / 1000, sample.lossCount, sample.bitrate);
#else
				fprintf(hOut, "% 4u.%03u % 5u % 9u\n\n", sample.delayFactor / 1000, sample.delayFactor % 1000, sample.lossCount, sample.bitrate);
#endif
				sampleTime += 1000000000;
			}
		}
		/* Process a single PCAP record.  */
		mdiPacket(frame.ts, (void *)((unsigned char *)frame.pBuffer + hdrBytes), frame.nBytes - hdrBytes, pStream);
	}

	/* Stop monitoring the stream. */
	mdiStreamStop(pStream);

	/* Cleanup. */
	fclose(hOut);
	fclose(hIn);
	free(frame.pBuffer);
	return 0;
}
