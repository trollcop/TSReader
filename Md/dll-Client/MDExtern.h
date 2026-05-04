/* Beispiel einer externen DLL in MultiDec */


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


#define MDAPI_DVB_COMMAND            0x01020060




HINSTANCE	MultiDecInstance;
HINSTANCE	DLLInstance;
HWND    	MultiDecWindow;
BOOL    	MultiDecLog;
int         MultiDec_DLL_ID;
  

typedef struct TPIDFilters {
         char FilterName[5];
		 unsigned char FilterId;
		 unsigned short PID;
};


typedef struct TCA_System {
		 unsigned short CA_Typ;
		 unsigned short ECM;
		 unsigned short EMM;
};


typedef struct TProgramm
{
	char              Name[30];
	char              Anbieter[30];
	char              Land[30];
    unsigned long     freq;
    unsigned char     Typ;
	unsigned char     volt;              
	unsigned char     afc;
	unsigned char     diseqc;            
	unsigned int      srate;         
	unsigned char     qam;               
	unsigned char     fec;   
	unsigned char     norm;
	unsigned short    tp_id;        
	unsigned short    Video_pid;        
	unsigned short    Audio_pid;
    unsigned short    TeleText_pid;          
	unsigned short    PMT_pid;
    unsigned short    PCR_pid;
	unsigned short    ECM_PID;
	unsigned short    SID_pid;
	unsigned short    AC3_pid;
	unsigned char     TVType; 
	unsigned char     ServiceTyp;
    unsigned char     CA_ID;
	unsigned short    Temp_Audio;
	unsigned short    Filteranzahl;
    struct TPIDFilters		  Filters[12];
	unsigned short    CA_Anzahl;
    struct TCA_System		  CA_System[6];
    char              CA_Land[5];
    unsigned char     Merker;
    unsigned short    Link_TP;
    unsigned short    Link_SID;
    unsigned char     Dynamisch;

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
    struct TTPCat TPCat[8];
};


#define TPSIZE   256

struct TProgramm ProgrammNeu[TPSIZE];


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
	unsigned short DLL_ID;
	unsigned short Filter_ID;
	unsigned short Pid;
	unsigned char Name[32];
	DWORD Irq_Call_Adresse;
	int Running_ID;
};


struct TDVB_COMMAND{
	unsigned short Cmd_laenge;
	unsigned short Cmd_Buffer[32];
};


struct TSTART_FILTER StartFilter;

unsigned char OSD_Key_Value;
HANDLE OSD_KEY_EVENT;