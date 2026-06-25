#include <windows.h>
#include <commctrl.h>
#include <time.h>
#include "TSReader.h"
#include "util.h"
#include "sky_genres.h"
#include "bcdmux.h"

extern PVARIABLES v;
extern char gszAppName[];

// from sky_huffman.c
void sky_decode(unsigned char *s, unsigned char *d, int max_input_length, int max_decode_length);

// from tsreader.c
void ExpireOldEITData(int nServiceID);
void SaveEPGData(PEITEVENT pEITItem, int nChannelNumber);

time_t tm_base;
int event_id;
unsigned int epg_id;
unsigned int seq_low = 0xffffffff;
unsigned int seq_high = 0;
int nLocalEITCount = 0;

unsigned int extract_seq(unsigned char *b,unsigned int maxlen)
{
	return( b[4] | (b[3]<<8) );
}

// for an 4 bit value, return the genre name
char * getGenre(unsigned int type)
{
	return(sky_genre_main[ type & 0x0000000f ] );
}

// for an 4 bit value, return the parental rating category
char * getRating(unsigned int type)
{
	return(sky_rating[ type & 0x0000000f ] );
}

// for a given genre, return the sub gener for the given id
char * getSubGenre(unsigned int genre, unsigned int subgenre)
{
	switch(genre & 0x0000000f)
	{
		case 0x04:
			return(sky_genre_kids[ subgenre & 0x0000000f ] );
		case 0x06:
			return(sky_genre_entertainment[ subgenre & 0x0000000f ] );
		case 0x0c:
			return(sky_genre_movies1[ subgenre & 0x0000000f ] );
		case 0x0d:
			return(sky_genre_movies2[ subgenre & 0x0000000f ] );
		case 0x0e:
		case 0x0f:
			return(sky_genre_sport[ subgenre & 0x0000000f ] );
		default:
			// If in doubt, return undefined.
			return(sky_genre_sport[ 0 ] );
	}
}

// Accept a two digit sky epg midnight date and return a
// fully qualified unix timestamp. Sky deliver this in their dvb streams on each
// packet header. It's a two digit short int that determines which day the
// associated schedules are for. Where, 0xCDDA = FEB 28 2003.
// Returning a regular unix timestamp.
time_t getDay(unsigned short int d)
{
	unsigned int i;
	time_t t = 0x3e5ea680; // default base date, feb 28 2003

	// default the date to the base date if input is out of range.
	if( (d < 0xcdda) || ( d > 0xf800 ) )
		return(t);

	for(i=0; i < d - (unsigned int)0xcdda; i++)
		t += 24 * 60 * 60;
	return(t);
}

// For a given unix timestamp (representing the beginning of the day),
// add the epg time portion and return the timestamp.
time_t getStartTime(time_t midnight, unsigned short int delta)
{
	return(midnight+(2*delta));
}

// For a given unix timestamp (representing the beginning of the day),
// add the epg time portion and return the timestamp.
time_t getEndTime(time_t starttime, unsigned short int delta)
{
	return(starttime+(2*delta));
}

unsigned int dump_raw_bytes(unsigned char *b, unsigned char len)
{
	return len;
/*	int i;
	char szTemp[1024];
	char szTemp2[1024] = {""};
	char szTemp3[16];


	for (i = 0; i < len; i++)
	{
		wsprintf(szTemp3, "%02x ", b[i]);
		lstrcat(szTemp2, szTemp3);
	}
	wsprintf(szTemp, "SkyEPG: %s\n", szTemp2);
	dbg_printf(szTemp);

	return len;*/
}

// By looking at the dumps, it looks like a b5 table is in the following format...
// SS SS DD DD 
// 15 18 07 08 23 00 00 2a bf 34 6b e8 3b 90 3e 6d 64 6a
// SSSS = start time in seconds*2 from base time
// DDDD = program duration in seconds*2
unsigned int dump_table_b5(unsigned char *b, unsigned char len)
{
	int nDuration, nServiceID;
	unsigned short int t_start;
	struct tm *atsctime;
	time_t tm_start;
	EITEVENT thiseitevent;

	if (!v->fSkyEPGMapComplete)
		return (len);

	memset(&thiseitevent, 0, sizeof(thiseitevent));
	thiseitevent.nEventID = event_id;
	thiseitevent.nSource = 0xff;

	t_start = (b[2] << 8) | b[3];
	tm_start = getStartTime(tm_base, t_start);	
	atsctime = gmtime((time_t *)&tm_start);
	thiseitevent.stStartTime.wYear = (WORD)(atsctime->tm_year + 1900);
	thiseitevent.stStartTime.wMonth = (WORD)(atsctime->tm_mon + 1);
	thiseitevent.stStartTime.wDay = (WORD)(atsctime->tm_mday);
	thiseitevent.stStartTime.wHour = (WORD)(atsctime->tm_hour);
	thiseitevent.stStartTime.wMinute = (WORD)(atsctime->tm_min);
	thiseitevent.stStartTime.wSecond = (WORD)(atsctime->tm_sec);
	nDuration = (b[4] << 8 | b[5]) * 2;
	thiseitevent.stRunTime.wHour = (WORD)(nDuration / 3600);
	nDuration -= thiseitevent.stRunTime.wHour * 3600;
	thiseitevent.stRunTime.wMinute = (WORD)(nDuration / 60);
	nDuration -= thiseitevent.stRunTime.wMinute * 60;
	thiseitevent.stRunTime.wSecond = (WORD)(nDuration);

	nServiceID = v->epg_map[epg_id];
	if ( (nServiceID < MAX_EIT_CHANNEL_DATA) && (nServiceID > 0) )
	{
		BOOL fAddedNewEvent = FALSE; 
		PEITEVENT pNewEIT = NULL;

		EnterCriticalSection(&v->csEIT);

		// See if any items need to be expired
		ExpireOldEITData(nServiceID);
		
		// Now see about adding this item to the list
		if (v->pEvents[nServiceID] == NULL)
		{
			// No EIT data for this channel
			if (EventInPast(&thiseitevent, FALSE) == FALSE)
			{
				v->pEvents[nServiceID] = LocalAlloc(LPTR, sizeof(EITEVENT));
				if (v->pEvents[nServiceID] != NULL)
				{
					PEITEVENT pNewItem = v->pEvents[nServiceID];
					memset(thiseitevent.szEventName, 0, sizeof(thiseitevent.szEventName));
					sky_decode(&b[9], (unsigned char *)thiseitevent.szEventName, b[1]-7, sizeof(thiseitevent.szEventName));
					memcpy(v->pEvents[nServiceID], &thiseitevent, sizeof(EITEVENT));
					pNewEIT = v->pEvents[nServiceID];
					fAddedNewEvent = TRUE;
					v->nEITEvents++;
					nLocalEITCount++;
					PostMessage(v->hDlgSIParser, WM_USER + 2, SI_PARSER_EIT, nServiceID);
					if (v->fEPGSaveEnabled)
						SaveEPGData(pNewItem, nServiceID);
				}
			}
		}
		else
		{
			PEITEVENT pCurrent;

			pCurrent = v->pEvents[nServiceID];
			do
			{
				if (pCurrent->nEventID == thiseitevent.nEventID)
				{
					// already got this event
					break;
				}
				if (pCurrent->dwNextEvent == 0)
				{
					// end of the list without a match on the service ID
					// so add this new item to the end of the list
					if (EventInPast(&thiseitevent, FALSE) == FALSE)
					{
						PEITEVENT pNewEvent = LocalAlloc(LPTR, sizeof(EITEVENT));
						if (pNewEvent != NULL)
						{
							memset(thiseitevent.szEventName, 0, sizeof(thiseitevent.szEventName));
							sky_decode(&b[9], (unsigned char *)thiseitevent.szEventName, b[1] - 7, sizeof(thiseitevent.szEventName));
							memcpy(pNewEvent, &thiseitevent, sizeof(EITEVENT));
							pCurrent->dwNextEvent = (LONG_PTR)pNewEvent;
							pNewEIT = pNewEvent;
							fAddedNewEvent = TRUE;
							v->nEITEvents++;
							nLocalEITCount++;
							if (v->fEPGSaveEnabled)
								SaveEPGData(pNewEvent, nServiceID);
						}
					}
					break;
				}
				pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
			} while (TRUE);
		}
		LeaveCriticalSection(&v->csEIT);

		/*if (fAddedNewEvent == TRUE)
		{
			if (v->hWndEPGGrid != NULL && v->fEPGDisplayActive && v->fEPGUpdateRealtime)
				PostMessage(v->hWndEPGGrid, WM_USER + 2, SI_PARSER_EIT + nServiceID, (LPARAM)pNewEIT);
		}*/ // kills performance!
	}

	return(len);
}

// Program description? These seem to be variable length strings.
unsigned int dump_table_b9(unsigned char *b, unsigned char len)
{
	int nServiceID;

	if (!v->fSkyEPGMapComplete)
		return (len);

	nServiceID = v->epg_map[epg_id];
	if ( (nServiceID < MAX_EIT_CHANNEL_DATA) && (nServiceID > 0) )
	{
		PEITEVENT pCurrent;

		EnterCriticalSection(&v->csEIT);
		
		// Try to find a matching EIT entry
		pCurrent = v->pEvents[nServiceID];
		if (pCurrent != NULL)
		{
			do
			{
				if (pCurrent->nEventID == event_id)
				{
					// Found the event - add the description if we won't already have one
					if (pCurrent->szShortEventDescription == NULL)
					{
						char decomp_str[4096];

						memset(decomp_str, 0, sizeof(decomp_str));
						sky_decode(&b[2], (unsigned char *)decomp_str, b[1], sizeof(decomp_str));
						if (lstrlen(decomp_str) > 0)
						{
							pCurrent->szShortEventDescription = LocalAlloc(LPTR, lstrlen(decomp_str) + 1);
							lstrcpy(pCurrent->szShortEventDescription, decomp_str);
							if (v->fEPGSaveEnabled)
								SaveEPGData(pCurrent, nServiceID);
						}
					}
					break;
				}
				pCurrent = (PEITEVENT)pCurrent->dwNextEvent;
			} while (pCurrent != NULL);
		}
		LeaveCriticalSection(&v->csEIT);
	}

	return(len);
}

unsigned int dump_table_b0(unsigned char *b)
{
	unsigned int i;

	i = 2;
	while(i < b[1])
	{		
		switch(b[i])
		{
		case 0xb5:
			i += dump_table_b5(&b[2], b[1]);
			break;
		case 0xb9:
			i += dump_table_b9(&b[2], b[1]);
			break;
		default:
			i += dump_raw_bytes(&b[2], b[1]);
			break;
		}
	}

	return(i);
}

unsigned int dump_table_f0(unsigned char *b)
{
	unsigned int i;

	i = 2;
	while(i < b[1])
	{		
		switch(b[i])
		{
		case 0xb5:
			i = i + dump_table_b5(&b[2], b[1]);
			break;
		case 0xb9:
			i = i + dump_table_b9(&b[2], b[1]);
			break;
		default:
			i = i + dump_raw_bytes(&b[2], b[1]);
			break;
		}
	}

	return(i);
}

unsigned int dump_table_05(unsigned char *b)
{
	//printf(" DICT ");
	//for(i=1;i<9;i++) printf("%s ",dict_general_lookup(b[i]));
	//printf("\n");
	return(9);
} 

unsigned int dump_table_06(unsigned char *b)
{
	//printf(" DICT ");
	//for(i=1;i<9;i++) printf("%s ",dict_general_lookup(b[i]));
	//printf("\n");
	return(9);
}
 
//   bc 1b
//      06 22 85 7b 00 c3 1f ff ff
//      06 21 90 07 01 35 1f ff ff
//      06 22 9a 93 01 a0 1f ff ff
unsigned int dump_table_bc(unsigned char *b)
{
	unsigned int i;

	i=2;
	while(i < b[1])
	{		
		if(b[i]==0x06)
		{
			i=i+dump_table_06(&b[i]);
		}
		else
		{
			if(b[i]==0x05)
			{
				i=i+dump_table_05(&b[i]);
			}
			else
			{
				i=i+dump_raw_bytes(&b[2],b[1]);
			}
		}
	}
	return(i);
}
 
unsigned int dump_table_30(unsigned char *b)
{
	unsigned int i=2;

	if(b[2]==0xbc)
	{
		i=i+dump_table_bc(&b[2]);
	}
	else
	{
		//printf("ERROR COMMAND !=bc ");
		i=i+dump_raw_bytes(&b[2],b[1]);
	}
	return(i);
} 

unsigned int dump_table_31(unsigned char *b)
{
	unsigned int i=2;

	if(b[2]==0xbc)
	{
		i=i+dump_table_bc(&b[2]);
	}
	else
	{
		//printf("ERROR COMMAND !=bc ");
		i=i+dump_raw_bytes(&b[2],b[1]);
	}
	return(i);
}

BOOL decode_frame(unsigned char *b, unsigned int len, BOOL fPrimaryEPGPID)
{
	BOOL fRetVal = FALSE;
	unsigned int i;
	unsigned short int day = (b[8]<<8) | (b[9]);
	
	epg_id = extract_seq(b, len);
	tm_base = getDay(day);

	if (fPrimaryEPGPID)
	{
		unsigned int seq_tmp;

		seq_tmp = extract_seq(&b[0], len) << 8 | b[0];

		// Break out and terminate if the sequence has cycled.
		if(seq_tmp == seq_low)
		{
			// looks like we've cycled
			// When I was to grab the data associated with another pid, I comment out the
			// next line and we just grab N * iterations specified in the while clause.
			// This should probably be driven by a flag.... can't be arsed.
				fRetVal = TRUE;
		}
		if(seq_tmp > seq_high)
			seq_high = seq_tmp;
		if(seq_tmp < seq_low)
			seq_low = seq_tmp;
	}

	// scan all the bytes in the frame but stop when we reach the crc
	//  00 ab  <-- sequence ID
	//     b0 18
	//     b5 16 54 60 07 08 a5 00 00 2c af 2d 04 1a ae 30 87 d6 6c 6e 85 3f 3d 4f

	i = 10;
	while(i < len - 1)
	{
		// first we get the sequence ID
		event_id = b[i] << 8 | b[i + 1];

		i += 2;

		switch(b[i])
		{
		case 0xb0:
			i = i + dump_table_b0(&b[i]);
			break;
		case 0x30:
			i = i + dump_table_30(&b[i]);
			break;
		case 0xf0:
			i = i + dump_table_f0(&b[i]);
			break;
		case 0x31:
			i = i + dump_table_31(&b[i]);
			break;
		case 0x05:
			i = i + dump_table_05(&b[i]);
			break;
		}
	}
	return fRetVal;
}

void ParseSkyEPG(BYTE * pSectionPointer, int nPacketLength, BOOL fPrimaryEPGPID)
{
	while (nPacketLength)
	{
		int nTable = pSectionPointer[0];	// not used - just for clarity
		int nSectionLength = ((pSectionPointer[1] << 8) + pSectionPointer[2]) & 0xfff;

		if ( (nSectionLength <= 0) || (nSectionLength > 65535) )
			return;
		if (SourceHelper_CRC_Check(pSectionPointer, nSectionLength + 3) != TRUE)
		{
			v->nSIParserCRCs[SI_PARSER_STATS_EIT]++;
			return;
		}

		v->nSIParserPackets[SI_PARSER_STATS_EIT]++;
		if (decode_frame(pSectionPointer, nSectionLength, fPrimaryEPGPID) == TRUE)
		{
			seq_low = 0xffffffff;
			seq_high = 0;

			v->nSkyEPGPIDs[0]++; v->nSkyEPGPIDs[1]++;
			if (v->nSkyEPGPIDs[0] > 0x37)
				v->nSkyEPGPIDs[0] = 0x30;
			if (v->nSkyEPGPIDs[1] > 0x47)
				v->nSkyEPGPIDs[1] = 0x40;
			{
				dbg_printf("Sky_EPG: PIDS 0x%04x 0x%04x New events from prior %d\n", v->nSkyEPGPIDs[0], v->nSkyEPGPIDs[1], nLocalEITCount);
			}
			nLocalEITCount = 0;
		}
		nPacketLength -= nSectionLength + 3;
		pSectionPointer += nSectionLength + 3;
	}
}

void UpdateSkyEPGMap(int nBATID)
{
	int nBATIndex;
	int nTSIndex;
	int nChannelIndex;

	v->fSkyEPGMapComplete = FALSE;

	for (nBATIndex = 0; nBATIndex < MAX_BAT_ENTRIES; nBATIndex++)
	{
		if (v->bat[nBATIndex].bouquet_id == 0)
			return;
		if (v->bat[nBATIndex].bouquet_id == nBATID)
			break;
	}
	if (nBATIndex == MAX_BAT_ENTRIES)
		return;

	EnterCriticalSection(&v->csEIT);
	for (nChannelIndex = 0; nChannelIndex < MAX_EIT_CHANNEL_DATA; nChannelIndex++)
	{
		if (v->pChannelData[nChannelIndex] != NULL)
			v->pChannelData[nChannelIndex]->nLogicalChannelNumber = 0;
	}

	for (nTSIndex = 0; nTSIndex < MAX_BAT_TRANSPORT_ITEMS; nTSIndex++)
	{
		if (v->bat[nBATIndex].batts[nTSIndex].transport_stream_id == 0)
			break;
		if (v->bat[nBATIndex].batts[nTSIndex].transport_descriptors_length)
		{
			int nDescriptorsLength = v->bat[nBATIndex].batts[nTSIndex].transport_descriptors_length;
			BYTE * pDescriptors = v->bat[nBATIndex].batts[nTSIndex].transport_descriptors;

			while (nDescriptorsLength)
			{
				int nDescriptor = pDescriptors[0];
				int nDescriptorLength = pDescriptors[1];

				if (nDescriptor == 0xb1)
				{
					set_buf(BM_USER_THREAD, pDescriptors, 0, FALSE);
					{
						int descriptor_tag = get_bits(BM_USER_THREAD, 8);
						int descriptor_length = get_bits(BM_USER_THREAD, 8);

						int type = get_bits(BM_USER_THREAD, 8);
						descriptor_length--;
						while (descriptor_length > 9)
						{
							get_bits(BM_USER_THREAD, 8); /* flag */
							uint16_t service_id = get_bits(BM_USER_THREAD, 16) & 0xffff;
							get_bits(BM_USER_THREAD, 8); /* service_type */
							uint16_t bat_epg_id = get_bits(BM_USER_THREAD, 16) & 0xffff;
							uint16_t lcn = get_bits(BM_USER_THREAD, 16) & 0xffff; // lcn 0xffff means hidden channel
							get_bits(BM_USER_THREAD, 8); /* unknown */

							if (lcn != 0xffff)
							{
								int j;

								if (v->pChannelData[service_id] != NULL)
								{
									if (v->pChannelData[service_id]->nLogicalChannelNumber == 0)
										v->pChannelData[service_id]->nLogicalChannelNumber = lcn;
								}
								v->epg_map[bat_epg_id] = service_id;
								for (j = 0; j < 32; j++)
								{
									if (v->channel_maps[lcn][j] == 0)
									{
										v->channel_maps[lcn][j] = service_id | (bat_epg_id << 16);
										break;
									}
									else
									{
										if ((int)(v->channel_maps[lcn][j] & 0xffff) == service_id)
											break;
									}
								}
							}
							descriptor_length -= 9;
						}
					}
				}
				pDescriptors += nDescriptorLength + 2;
				nDescriptorsLength -= nDescriptorLength + 2;
			}
		}
	}

	v->fSkyEPGMapComplete = TRUE;
	LeaveCriticalSection(&v->csEIT);
}
