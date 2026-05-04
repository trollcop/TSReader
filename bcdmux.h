int PS__StartWriting();
int PS__StopWriting();
int PS__TranslateToProgramStream(BYTE * pPacketData, int nLength);

// Bit manipulation prototypes
enum BM_BUFFERS
{
	BM_PARSER_THREAD = 0,
	BM_CC_THREAD,
	BM_USER_THREAD,
	BM_PS_MUXER,
	BM_CI_INTERFACE,
	BM_DVBINT,
	BM_ARCHIVE_THREAD,
	BM_MPEG2_THREAD,
	BM_MPEG2_THREAD_END = BM_MPEG2_THREAD + 32,//REAL_MAX_ES_PARSERS,
	BM_MAX,
};

void set_buf(int index, unsigned char* buf, int bufsize, BOOL clear);
void set_bits(int index, unsigned int val, int bits);
unsigned int get_bits(int index, int bits);
unsigned int get_bit_pos(int index);
unsigned int get_byte_pos(int index);
void set_bit_pos(int index, unsigned int newpos);
