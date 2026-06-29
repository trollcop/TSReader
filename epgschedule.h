typedef struct _tagEPGSchedule
{
	int nChannel;
	int nDuration;
	short wPreRoll;
	short wPostRoll;
	int64_t nStartTime;
	char szEventName[64];
} EPGSCHEDULE, *PEPGSCHEDULE;
