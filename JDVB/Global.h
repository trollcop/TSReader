
#ifndef GLOBAL_H___
#define GLOBAL_H___


#define IDT_UDP_TIMER 100
#define IDT_DVB_SOURCE_TIMER		500

class DVBSource;
class TSPacketQueue;
class TSPidFactory;
class UDPPidFilter;
class TSTarget;

extern HWND hWnd;
extern HWND g_UDPPidDlg;
extern HWND g_DVBSourceDlg;
extern HWND g_cProgramm;
extern TSTarget * g_defaultVideoOutDevice;
extern DVBSource *g_DVBSource;



extern int g_cProgrammIndex;




#endif