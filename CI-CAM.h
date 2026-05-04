#define ME0						1
#define ME1						2
#define MMI0					3
#define MMI1					4
#define MMI0_ClOSE				5
#define MMI1_CLOSE				6
#define NON_CI_INFO				0

#define PCMSG_NULL              0x00
#define PCMSG_APPLICATION_INFO  0x01
#define PCMSG_CA_INFO           0x02
#define PCMSG_CA_PMT            0x03
#define PCMSG_CA_PMT_REPLY      0x04
#define PCMSG_DATETIME_ENQ      0x05
#define PCMSG_DATETIME          0x06
#define PCMSG_ENQ               0x07
#define PCMSG_ANSWER            0x08
#define PCMSG_ENTER_MENU        0x09
#define PCMSG_MENU           	0x0A
#define PCMSG_MENU_ANSWER       0x0B
#define PCMSG_LIST              0x0C
#define PCMSG_GET_MMI           0x0D
#define PCMSG_CLOSE_MMI			0x0e

#define	CAM_DEFAULT				0
#define	CAM_CONAX				1
#define	CAM_CRYPTOWORKS			2
#define CAM_ASTON				3

typedef struct MMIInfoStruct
{
	char Header[256];			// Header message
	char SubHeader[256];			// Sub header message
	char ButtomLine[256];			// Status message
	char MenuItem[9][42];			// Menu list info
	int  ItemCount;

	BOOL EnqFlag;

	BOOL Blind_Answer;			// TRUE: only entry the password
	int  Answer_Text_Length;		// Answer text length
	char Prompt[256];			// Prompt

	int  Answer;				// Answer type
	char AnswerStr[256];
}MMI_Info;

typedef struct AppInfoStruct
{
	unsigned int app_type;			// Application type; 1: CA; 2: EPG
	unsigned int application_manufacture;
	unsigned int manufacture_code;
	char application_info[64];		// Application info;
}App_Info;
