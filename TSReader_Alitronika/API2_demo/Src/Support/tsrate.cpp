
/**********************************************************************************************************/
// includes
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "msg.h"

/**********************************************************************************************************/
// defines
#define MAX_STRING_LENGTH 512

/**********************************************************************************************************/
// static variables
int		g_PID=-1;	// Default PID

/**********************************************************************************************************/
// functions
double	tsrate(FILE *in)
{
	unsigned char	ts_packet[188];
	int				pid;
	int				offset,p_offset=-1;
	double			pcr,p_pcr=-1;
	double			rate,r_min=1e20,r_max=0.0,r_ave=0.0;
	int				n=0;

	while ((unsigned char)fgetc(in) != 0x47)
		;
	fread(ts_packet, 187,1,in);

	while(1)
	{
		if(fread(ts_packet,188,1,in)!=1)	// EOF or other problem
			break;

		if(ts_packet[0]!=0x47)
		{
			Message("TS packet header missing");
			return 0.0;
		}

		pid=((int)(ts_packet[1]&0x1F)<<8)+ts_packet[2];
		if(pid!=g_PID&&g_PID!=-1)	// Filter PID
			continue;

		if(ts_packet[3]&0x20)	// Adaptation field present
		{
			int	af_length=ts_packet[4];

			if(af_length==0)
				continue;	// No flags

			if(ts_packet[5]&0x10)	// PCR flag
			{
				pcr=(double)ts_packet[6];
				pcr*=256.0;
				pcr+=(double)ts_packet[7];
				pcr*=256.0;
				pcr+=(double)ts_packet[8];
				pcr*=256.0;
				pcr+=(double)ts_packet[9];
				pcr*=2.0;
				if(ts_packet[10]&0x80)
					pcr+=1.0;
				pcr*=300.0;
				if(ts_packet[10]&0x01)
					pcr+=256.0;
				pcr+=(double)ts_packet[11];

				if(g_PID==-1)
					g_PID=pid;	// If any PID, from now on use only this one

				offset=ftell(in);

//				printf("PCR: @%ld %g in pid=%d\n",offset,pcr,pid);
//				printf("%02X%02X%02X%02X %02X %02X\n",ts_packet[6],ts_packet[7],ts_packet[8],ts_packet[9],ts_packet[10],ts_packet[11]);

				if(p_offset!=-1)
				{
					rate=27.0e6*((offset-p_offset)*8)/(pcr-p_pcr);

					if(rate<r_min)
						r_min=rate;
					if(rate>r_max)
						r_max=rate;
					r_ave+=rate;
					n++;
					if (n > 10)
						break;	// Sufficient accuracy is now obtained.

	//				printf("doffset=%d, dpcr=%g\n",(offset-p_offset)*8*188,pcr-p_pcr);
	//				printf("rate=%g\n",27.0e6*((offset-p_offset)*8)/(pcr-p_pcr));
				}

				p_offset=offset;
				p_pcr=pcr;
			}
		}
	}

//	char OutputString[MAX_STRING_LENGTH];
//	sprintf(OutputString, "%d PCRs sampled of PID %d (0x%x)",n,g_PID,g_PID);
//	Message (OutputString);
	if(n>0)
	{
//		sprintf(OutputString, "Average rate: %.1f bits/s (%d PCRs)",r_ave/n, n);
//		Message (OutputString);
	}
	
	return r_ave/n;
}
