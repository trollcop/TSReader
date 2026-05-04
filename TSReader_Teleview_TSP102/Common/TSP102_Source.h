///////////////////////////////////////////////////////////////////////////////
// TSP102_Source.h
///////////////////////////////////////////////////////////////////////////////

#define TSP102_BASE_LABEL			"Teleview TSP102 "
#define TSP102_FORMAT_1				"DVB-ASI"
#define TSP102_FORMAT_0				"SMPTE-310M"

#ifdef TSP102_ASI
#define TSP102_TSIO_FORMAT			(1)
#endif

#ifdef TSP102_310M
#define TSP102_TSIO_FORMAT			(0)
#endif

#define TSP102_SUB_BANKS			(HW_BANK_NUMBER)
#define TSP102_SUB_BANK_KBYTES		(SUB_BANK_OFFSET_SIZE)
