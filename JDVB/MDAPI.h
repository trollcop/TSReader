
#ifndef MDAPI_H___
#define MDAPI_H___


#define MAX_CA_SYSTEMS		48
#define MD_MAX_CHANNEL		50
#define MD_MAX_TRANSPONDER	50
#define MD_MAX_SATELLITE	10
#define MAX_PID_IDS  32
#define TPSIZE   256

typedef int BOOL;
typedef unsigned long DWORD;


struct TPIDFilters {
         char FilterName[5];
		 unsigned char FilterId;
		 unsigned short PID;
};

struct TCA_System {
		 unsigned short CA_Typ;
		 unsigned short ECM;
		 unsigned short EMM;
		 unsigned int   Provider_Id;   // Neu -> Viaccess und Seca Provider in der Kanalliste
};

struct TProgramm
{
	char              Name[30];
	char              Anbieter[30];
	char              Land[30];
    unsigned long     freq;
	unsigned char     volt;              
	unsigned char     diseqc;            
	unsigned int      srate;         
	unsigned char     qam;               
	unsigned char     fec;   
	unsigned char     norm;
	unsigned short    tp_id;        
	unsigned short    Video_pid;        
	unsigned short    Audio_pid;
    unsigned short    TeleText_pid;          /* Teletext PID */
	unsigned short    PMT_pid;
    unsigned short    PCR_pid;
	unsigned short    ECM_PID;
	unsigned short    SID_pid;
	unsigned short    AC3_pid;
	unsigned char     TVType; //  == 00 PAL ; 11 == NTSC    
	unsigned char     ServiceTyp;
    unsigned char     CA_ID;
	unsigned short    Temp_Audio;
	unsigned short    Filteranzahl;
    struct TPIDFilters Filters[MAX_PID_IDS];
	unsigned short    CA_Anzahl;
    struct TCA_System CA_System[MAX_CA_SYSTEMS];
    char    CA_Land[5];
    unsigned char Merker;		 //constant in FreeDec	
    unsigned short Link_TP;
    unsigned short Link_SID;	
    unsigned char Dynamisch;	 //constant in FreeDec 2.2
    unsigned short Lists[12];    // Contains 12 Favorite-Sortings 
	int      Current_i_id;       //constant in FreeDec 2.2  
	int      Current_o_id;       //constant in FreeDec 2.2  
	int      Next_i_id;          //constant in FreeDec 2.2
	int      Next_o_id;          //constant in FreeDec 2.2

    char Extern_Buffer[16];
};


struct TTransponder {
		int ttk;
        int power;
		int diseqc;
        unsigned int freq;
        int volt;
        int qam;
        unsigned int srate;
        int fec;
    	int sync;              /* sync from decoder */
    	int afc;               /* frequency offset in Hz */
	    unsigned short agc;             /* gain */
	    unsigned short nest;            /* noise estimation */
        unsigned int vber;            /* viterbi bit error rate */
        unsigned short ts_id;
    	unsigned int sig;               
    	unsigned int err;               
	};


struct TProgrammNummer {
	    int RealNummer;
		int VirtNummer;
};


struct TTPCat {
    unsigned char  TAG;
    unsigned char  DesLen;
	unsigned short CA_ID;
	unsigned short EMM;
	unsigned short BufferLen;
    unsigned char  Buffer[64];
};

struct TTPCatio {
	int TPCatAnzahl;
    struct TTPCat TPCat[32];
};

struct TOSD_START {
        int DLL_Id;
	    unsigned char BitTiefe;
	    int x1;
	    int y1;
	    int x2;
	    int y2;
		BOOL Input;
};


struct TOSD_DRAW {
	unsigned short x;
	unsigned short y;
	unsigned short Sizex;
	unsigned short Sizey;
    unsigned short Color;
};

struct TOSD_SETFONT {
	unsigned short Typ;
	unsigned short Fg_Color;
	unsigned short Bg_Color;
};

struct TOSD_SETEXT {
	unsigned short x;
	unsigned short y;
	char Zeile[128];
};

struct TSTART_FILTER {
	unsigned short DLL_ID;				//Indentifying DLL (host assigned)
	unsigned short Filter_ID;			//Filter_ID internal number (client assigned)
	unsigned short Pid;					//requested Pid
	unsigned char Name[32];				//Name of filter
	DWORD Irq_Call_Adresse;				//CallBack routine
	int Running_ID;
};


struct TDVB_COMMAND{
	unsigned short Cmd_laenge;
	unsigned short Cmd_Buffer[32];
};


#define MDAPI_GET_TRANSPONDER        0x01020000
#define MDAPI_SET_TRANSPONDER        0x01020001

#define MDAPI_GET_PROGRAMM           0x01020010
#define MDAPI_SET_PROGRAMM           0x01020011
#define MDAPI_RESCAN_PROGRAMM        0x01020012
#define MDAPI_SAVE_PROGRAMM          0x01020013
#define MDAPI_GET_PROGRAMM_NUMMER    0x01020014
#define MDAPI_SET_PROGRAMM_NUMMER    0x01020015

#define MDAPI_START_FILTER           0x01020020
#define MDAPI_STOP_FILTER            0x01020021

#define MDAPI_SCAN_CURRENT_TP        0x01020030
#define MDAPI_SCAN_CURRENT_CAT       0x01020031

#define MDAPI_START_OSD              0x01020040
#define MDAPI_OSD_DRAWBLOCK          0x01020041
#define MDAPI_OSD_SETFONT            0x01020042
#define MDAPI_OSD_TEXT               0x01020043
#define MDAPI_SEND_OSD_KEY           0x01020044
#define MDAPI_STOP_OSD               0x01020049

#define MDAPI_GET_VERSION            0x01020100
#define MDAPI_DVB_COMMAND            0x01020060

//own definitions

#define MDAPI_ON_SEND_DLL_ID_NAME 0
#define MDAPI_ON_START            1
#define MDAPI_ON_MENU_SELECT	  2
#define MDAPI_ON_CHANNEL_CHANGE   3
#define MDAPI_ON_HOT_KEY          4
#define MDAPI_ON_OSD_KEY          5
#define MDAPI_ON_FILTER_CLOSE     6
#define MDAPI_ON_REC_PLAY         7
#define MDAPI_ON_EXIT             8
#define MDAPI_ON_VIDEOTEXT_STREAM 9








#endif