#define MDAPI_START_FILTER           0x01020020		
#define MDAPI_STOP_FILTER            0x01020021		

typedef struct  
{
	char FilterName[5];
	unsigned char FilterId;
	unsigned short PID;
} TPIDFilters;

typedef struct  
{
	unsigned short CA_Typ;
	unsigned short ECM;
	unsigned short EMM;
} TCA_System;

typedef struct 
{
	char				Name[30];
	char				Anbieter[30];
	char				Land[30];
    unsigned long		freq;
    unsigned char		Typ;
	unsigned char		volt;              
	unsigned char		afc;
	unsigned char		diseqc;            
	unsigned int		srate;         
	unsigned char		qam;               
	unsigned char		fec;   
	unsigned char		norm;
	unsigned short		tp_id;        
	unsigned short		Video_pid;        
	unsigned short		Audio_pid;
    unsigned short		TeleText_pid;          
	unsigned short		PMT_pid;
    unsigned short		PCR_pid;
	unsigned short		ECM_PID;
	unsigned short		SID_pid;
	unsigned short		AC3_pid;
	unsigned char		TVType; 
	unsigned char		ServiceTyp;
    unsigned char		CA_ID;
	unsigned short		Temp_Audio;
	unsigned short		Filteranzahl;
    TPIDFilters			Filters[12];
	unsigned short		CA_Anzahl;
    TCA_System			CA_System[6];
    char				CA_Land[5];
    unsigned char		Merker;
    unsigned short		Link_TP;
    unsigned short		Link_SID;
    unsigned char		Dynamisch;

    char Extern_Buffer[16];
} TPROGRAM;

typedef struct
{
	unsigned short	DLL_ID;
	unsigned short	Filter_ID;
	unsigned short	Pid;
	unsigned char	Name[32];
	DWORD			Irq_Call_Adresse;
	int				Running_ID;
} TSTART_FILTER;


