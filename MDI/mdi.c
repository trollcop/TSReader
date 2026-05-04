#ifdef PRO
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********  MDI.c                                                **********/
/**********                                                       **********/
/**********                                                       **********/
/**********  Functions that compute MDI for a transport stream.   **********/
/**********                                                       **********/
/**********  Copyright 2005 IneoQuest Technologies, Inc.          **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#include <stdlib.h>
#include <string.h>
#include "mdi.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                   Local Definitions                   **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
#define STREAM_FLAG_PCRERROR	0x01
#define STREAM_FLAG_INDEXERROR	0x02

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                      mdiStreamInit                    **********/
/**********                                                       **********/
/**********  This function should be called once for each stream  **********/
/**********  to be monitored.                                     **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
tMDISTREAM *mdiStreamInit(unsigned int mtspSize)
{
	tMDISTREAM *pStream;

	/* Allocate all necessary memory. */
	pStream = calloc(1, sizeof(tMDISTREAM) + (MDI_MAX_SAMPLES * sizeof(MDIULONG))
			+ (MDI_MAX_SAMPLES * sizeof(MDIULONG)) + (8192 * sizeof(MDIUBYTE)));

	if (0 != pStream)
	{
		pStream->timeArray = (MDIULONG *)(pStream + 1);
		pStream->bytesArray = (MDIULONG *)(&pStream->timeArray[MDI_MAX_SAMPLES]);
		pStream->pidArray = (MDIUBYTE *)(&pStream->bytesArray[MDI_MAX_SAMPLES]);
		memset((void *)pStream->pidArray, 0, 8192 * sizeof(MDIUBYTE));

		pStream->mtspSize = mtspSize;
	}

	pStream->pcrPid = (MDIUSHORT)-1;

	return pStream;
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                     mdiStreamStop                     **********/
/**********                                                       **********/
/**********  Call this function when done monitoring a stream.    **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void mdiStreamStop(tMDISTREAM *pStream)
{
	if (0 != pStream)
		free(pStream);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                       mdiPacket                       **********/
/**********                                                       **********/
/**********  This function will get called once for every packet  **********/
/**********  that arrives belonging to the indicated stream.      **********/
/**********                                                       **********/
/**********  This function first stores away the arrival time     **********/
/**********  (stored as delta from previous arrival time) and     **********/
/**********  the payload size.                                    **********/
/**********                                                       **********/
/**********  If the payload contains a PCR record from the        **********/
/**********  designated PCR PID then a new PCR bitrate will be    **********/
/**********  computed and the virtual buffer size between the     **********/
/**********  last PCR record and this one will be calculated.     **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void mdiPacket(MDITIME ts, void *pPayload, unsigned int nPayloadBytes, tMDISTREAM *pStream)
{
	union
	{
		MDIUBYTE *p8;
		MDIUSHORT *p16;
		MDIULONG *p32;
		MDIULONG n;		void *pVoid;
	} u;
	MDIUSHORT pid;
	MDIULONG pcr, workingPcr;
	MDIULONG indicators;
	MDIULONG vBuffer;
	MDIFLOAT drainrate;
	unsigned int i;
	int ccDiff;

	/* Check to see if we're about to exceed the bounds
	   of our packet array.  If we are, set an error
	   flag and exit. */
	if (pStream->index >= MDI_MAX_SAMPLES)
	{
		pStream->flags |= STREAM_FLAG_INDEXERROR;
		return;
	}

	/* Force the payload size to be a multiple of the MTSP size. */
	nPayloadBytes = (nPayloadBytes / pStream->mtspSize) * pStream->mtspSize;

	/* Save the delta time since the previous packet. */
	pStream->bytesArray[pStream->index] = nPayloadBytes;
	if (0 == pStream->lastTime)
		pStream->timeArray[pStream->index] = 0;
	else
		pStream->timeArray[pStream->index] = (unsigned __int32)(ts - pStream->lastTime);
	pStream->lastTime = ts;

	/* Save the payload size. */
	pStream->bytesArray[pStream->index++] = nPayloadBytes;

	for (u.pVoid = pPayload; nPayloadBytes >= pStream->mtspSize;
			u.n += pStream->mtspSize, nPayloadBytes -= pStream->mtspSize)
	{
		pStream->nBytes += pStream->mtspSize;

		/* Skip this MTSP if it does not start with a sync byte. */
		if (0x47 != u.p8[0])
		{
			++pStream->syncErrors;
			pStream->flags |= STREAM_FLAG_PCRERROR;
			continue;
		}

		/* Extract the PID number. */
		pid = ((u.p8[1] & 0x1F) << 8) + u.p8[2];

		/* Assume no adaptation field. */
		indicators = 0;

		/* Check for an adaptation field. */
		if ((0 != (u.p8[3] & 0x20)) && (0 != u.p8[4]))
			indicators = u.p8[5];

		/* If this discontinuity indicator is not set
		   then check for a CC error. */
		if (0 == (indicators & 0x80))
		{
			/* Ignore the stuffing PID. */
			if (pid != 8191)
			{
				/* If we have seen this PID before
				   then check for a CC error. */
				if (0 != (pStream->pidArray[pid] & 0x10))
				{
					/* If there is no payload then this CC
					   should equal the last CC. */
					if (0 != (ccDiff = abs(((pStream->pidArray[pid] + ((u.p8[3] & 0x10) ? 1 : 0)) & 0x0F) - (u.p8[3] & 0x0F))))
					{
						/* CC error detected.  Increment the stream's CC error count. */
						pStream->ccErrors += ccDiff;

						/* We won't be able to compute the PCR bitrate for
						   this segment due to missing MTSP(s). */
						pStream->flags |= STREAM_FLAG_PCRERROR;
					}
				}

				/* Store the CC value OR'ed with 0x10. */
				pStream->pidArray[pid] = 0x10 + (u.p8[3] & 0x0F);
			}
		}
		else
		{
			/* If there is a discontinuity in the PCR PID then we
			   cannot compute the PCR bitrate for this segment. */
			if (pStream->pcrPid == pid)
			{
				pStream->flags |= STREAM_FLAG_PCRERROR;
				continue;
			}
		}

		/* Is there a PCR present? */
		if (0 != (indicators & 0x10))
		{
			/* Compute the PCR value as ((PCR_base * 300) + PCR_extension).
			   Ignore the top 16 bits of PCR_base in order to keep things
			   within the constraints of 32-bit arithmetic.
			   PCR rollover has to be dealt with no matter what, this just
			   forces it to be dealt with 65536 times more often.
			   Because of this, successive PCRs must not be more than
			   1.45 seconds ((2^17) / 90000) apart. */
			pcr = (u.p8[8] << 9) + (u.p8[9] << 1) + ((u.p8[10] >> 7) & 0x01);
			pcr *= 300;
			pcr += ((u.p8[10] & 0x01) << 8) + u.p8[11];

			/* If we do not yet have a designated PCR PID then simply save
			   this PID as the PCR PID. */
			if ((MDIUSHORT)-1 == pStream->pcrPid)
			{
				pStream->pcrPid = pid;
				pStream->index = 0;
				pStream->nBytes = 0;
				pStream->lastPcr = pcr;
				pStream->flags &= ~STREAM_FLAG_PCRERROR;
			}
			else
			{
				/* Is this PID the designated PCR PID? */
				if (pid == pStream->pcrPid)
				{
					if (0 == (pStream->flags & STREAM_FLAG_PCRERROR))
					{
						/* If PCR rollover has occured we OR in the
						   next bit (2^17) multiplied by 300. */
						if ((workingPcr = pcr )< pStream->lastPcr)
							workingPcr += (131072 * 300);

						/* Compute the PCR drainrate. */
#if MDI_USE_FLOAT
						drainrate = pStream->nBytes;
						drainrate /= abs(workingPcr - pStream->lastPcr);
						drainrate = 1000000000.0 / (drainrate * 27000000.0);
#else
						drainrate = abs(workingPcr - pStream->lastPcr) * 37;
						drainrate = (drainrate + (drainrate / 1000) + (drainrate / 1000000)) / pStream->nBytes;
#endif

						/* Compute flow-rate balance values using
						   the newly-minted PCR drainrate. */
						for (i = 0; i < pStream->index; ++i)
						{
							pStream->frbCurrent -= (pStream->timeArray[i] / drainrate);
							pStream->frbMin = min(pStream->frbMin, pStream->frbCurrent);
							pStream->frbCurrent += (pStream->bytesArray[i] /** 8*/);
							pStream->frbMax = max(pStream->frbMax, pStream->frbCurrent);
						}

						/* Compute the virtual buffer size. If this value is
						   greater than the currently saved vBuffer value then
						   save it. */
						vBuffer = (pStream->frbMax - pStream->frbMin) /*/ 8*/;
						pStream->vBuffer = (MDIUSHORT)max(pStream->vBuffer, vBuffer);
					}

					/* Save these values for bitrate computation
					   over the complete sample time. */
					for (i = 0; i < pStream->index; ++i)
					{
						pStream->processedTime += pStream->timeArray[i];
						pStream->processedBytes += pStream->bytesArray[i] / pStream->mtspSize;
					}

					/* Clear the PCR error flag. */
					pStream->flags &= ~STREAM_FLAG_PCRERROR;

					/* Reset for next PCR interval. */
					pStream->index = 0;
					pStream->nBytes = 0;
					pStream->lastPcr = pcr;
				}
			}
		}
	}
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/**********                                                       **********/
/**********                       mdiSample                       **********/
/**********                                                       **********/
/**********  This function gets called periodically - by          **********/
/**********  convention, once a second.  It returns the current   **********/
/**********  maximum virtual buffer size.                         **********/
/**********                                                       **********/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void mdiSample(MDITIME ts, tMDISTREAM *pStream, tMDISAMPLE *pSample)
{
	pSample->vBuffer = pStream->vBuffer;
	pStream->vBuffer = 0;

	pSample->lossCount = pStream->ccErrors;
	pStream->ccErrors = 0;

	if (pStream->processedTime)
		pSample->bitrate = ((pStream->processedBytes * 100000) / (pStream->processedTime / 10000))
		* (pStream->mtspSize * 8);

	if (pSample->bitrate)
		pSample->delayFactor = ((pSample->vBuffer * 100000) / pSample->bitrate) * 80;

	pStream->processedBytes = pStream->processedTime = 0;
	pStream->frbCurrent = pStream->frbMin = pStream->frbMax = 0;
}

#endif PRO