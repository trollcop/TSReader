typedef struct _tagEPGSchedule
{
	int nChannel;
	int nDuration;
	short wPreRoll;
	short wPostRoll;
	__int64 nStartTime;
	char szEventName[64];
} EPGSCHEDULE, *PEPGSCHEDULE;
