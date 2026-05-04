typedef enum
{
	XDS_IDLE = 0,
	XDS_COLLECT_DATA = 1,
	XDS_PROCESS_DATA = 2
} XDS_STATUS;

typedef enum
{
	TYPE_CHAR_STANDARD = 0,
	TYPE_MISC1,
	TYPE_MISC2,
	TYPE_MIDROW,
	TYPE_PREAMBLE_ADDRESS,
	TYPE_ATTRIBUTE_1,
	TYPE_ATTRIBUTE_2,
	TYPE_ATTRIBUTE_3,
	TYPE_CLOSED_GROUP_EXT,
	TYPE_CHAR_EXT1,
	TYPE_CHAR_EXT2,
	TYPE_CHAR_EXT3,
	TYPE_XDS,
	TYPE_UNKNOWN
} CCTYPE;

typedef enum
{
	COUNTER_CC608_F1,
	COUNTER_CC608_F2,
	COUNTER_CC708
} COUNTER_CC;

#define CC608_PAGES 8
#define CC708_SERVICES 64
#define CAPTION_ROWS 15
#define CAPTION_COLUMNS 32

#define ATTRIBUTE_UNDERLINE 1
#define ATTRIBUTE_ITALIC 2

#define CC_SYSTEM_ATSC		0x00000001
#define CC_SYSTEM_CCUBE		0x00000002
#define CC_SYSTEM_SCTE_20	0x00000004
#define CC_SYSTEM_SCIATL	0x00000008
#define CC_SYSTEM_DIRECTV	0x00000010
#define CC_SYSTEM_SCTE_21	0x00000020
#define CC_SYSTEM_321_33F	0x00000040
#define CC_SYSTEM_UNDEF		0x80000000

#define CC708_PACKET_IDLE 0
#define CC708_PACKET_RECEIVE 1

#define CC708_BLOCK_CHARACTER 0
#define CC708_BLOCK_ESCAPE 1

#define MAX_BUFFERED_PICTURE_DATA 512

void InitCCDecoder();
int GetCCDataType(int code);

typedef struct tag_ClosedCaptionVariables
{
	int nTemporalReference;
	int nPictureCodingType;
	int nH264UserBufferSize;

	int n608CursorRow[CC608_PAGES];
	int n608CursorColumn[CC608_PAGES];
	int n708CursorRow[CC708_SERVICES];
	int n708CursorColumn[CC708_SERVICES];

	int n608WriteChannel;
	int nIFrame608Count, nPFrame608Count;
	int nIFrame708Count, nPFrame708Count;
	int n708PacketState;
	int n708PacketSequence;
	int n708CurrentPacketLength;
	int n708PacketWriteOffset;
	int n708SequenceErrors;
	int n708DataErrors;
	int n708BlockStatus[CC708_SERVICES];
	int n708EscapeRemaining[CC708_SERVICES];
	int n708EscapeOffset[CC708_SERVICES];
	
	int nUserDataCount;
	int nPriorUserDataCount;
	int nCaptionAreaCount[3];	// line-21 field 1, 2 then DTVCC
	DWORD dwSystemType;

	WCHAR currentChar[4];		// only need 2 ut this keeps aligned
	int previousType;
	int xdsState;
	int xdsPosition;
	int xdsLastControlCode;
	int xdsLastType;

	BOOL fDisplay708;
	BOOL f708ServiceActive[CC708_SERVICES];

	HFONT hCaptionFixedFont;
	HFONT hCaptionSmallFont;
	HICON hInfoIcon;

	BYTE bIFrameCC608[MAX_BUFFERED_PICTURE_DATA], bIFrameCC708[MAX_BUFFERED_PICTURE_DATA];
	BYTE bPFrameCC608[MAX_BUFFERED_PICTURE_DATA], bPFrameCC708[MAX_BUFFERED_PICTURE_DATA];
	BYTE b708Packet[1024];

	BYTE b608CurrentAttribute[CC608_PAGES];
	BYTE b608DisplayAttributes[CC608_PAGES][CAPTION_ROWS][CAPTION_COLUMNS];
	WCHAR b608DisplayBuffer[CC608_PAGES][CAPTION_ROWS][CAPTION_COLUMNS];

	BYTE b708CurrentAttribute[CC708_SERVICES];
	BYTE b708DisplayAttributes[CC708_SERVICES][CAPTION_ROWS][CAPTION_COLUMNS];
	WCHAR b708DisplayBuffer[CC708_SERVICES][CAPTION_ROWS][CAPTION_COLUMNS];
	BYTE b708EscapeBuffer[CC708_SERVICES][32];

	BYTE bXDSLength[16 * 256];
	BYTE bXDSBuffer[16 * 256][32];
	BYTE xdsBuffer[256];

	char szChannelDescription[256];

	BYTE bH264UserBuffer[1024];

} CLOSEDCAPTIONVARIABLES, *PCLOSEDCAPTIONVARIABLES;


