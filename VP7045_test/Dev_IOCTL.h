#ifndef DEV_IOCTL_H
#define DEV_IOCTL_H

#define Ezusb_IOCTL_INDEX  0x0800


// {7A7B9E46-3B87-4fe6-BF59-54AEA2DF0D02}
DEFINE_GUID(GUID_TWINHANDST_USBDEV, 
0x7a7b9e46, 0x3b87, 0x4fe6, 0xbf, 0x59, 0x54, 0xae, 0xa2, 0xdf, 0xd, 0x2);

//Unused in this driver, only return TRUE 
#define	DST_SET_INFO					      CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+200,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS) 

//Unused in this driver, only return TRUE 
#define DST_TW_RESETPIPE					   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+214,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                                                                      
#define IOCTL_TH_USB_SCAN_START				CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+202,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   

//Unused in this driver, only return TRUE
#define IOCTL_TH_USB_SCAN_WAITING			CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+203,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
//Unused in this driver, only return TRUE                                                   
#define IOCTL_TH_USB_SCAN_END					CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+204,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                                                                                                                                                 

#define IOCTL_TH_USB_START_CAP 				CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+205,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                 

#define IOCTL_TH_USB_STOP_CAP					CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+206,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)    

#define IOCTL_TH_USB_GET_WPTR_OFFSET		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+207,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   

#define IOCTL_TH_USB_SET_PLD_PID     	   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+209,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   
                                                   
#define IOCTL_TH_USB_GET_PLD_PID     	   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+221,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)  
                                                   
#define IOCTL_TH_USB_GET_SIGNAL_Q_S	   	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+211,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)  

#define DST_GET_RC				            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+216,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
#define ACTTV_IOCTL_GET_KEYCODE           CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+217,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
#define ACTTV_IOCTL_MAKE_REMOTEBUF        CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+218,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define DST_START_RC				            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+219,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define DST_STOP_RC				            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+220,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)


//************** These interfaces are only in VP704x, different from other USB driver *************//

#define IOCTL_TH_USB_RW_FX2_REG            CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+300,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
#define IOCTL_TH_USB_SET_PLD_STATUS     	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+301,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   

#define IOCTL_TH_USB_GET_FW_VERSION   		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+302,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)    

#define IOCTL_TH_USB_GET_VENDER_STRING		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+303,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_TH_USB_GET_PRODUCT_STRING	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+304,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   

#define IOCTL_TH_USB_SET_TUNER_POWER     	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+305,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   

#define IOCTL_TH_USB_GET_USB_SPEED     	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+306,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)   
                                                   
#define IOCTL_TH_USB_GET_RC_VAL     		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+307,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                                                                                                       

#define IOCTL_TH_USB_SET_EE_VAL     		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+308,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)      
                                                   
#define IOCTL_TH_USB_GET_EE_VAL     		CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+309,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)                                                         

#define IOCTL_TH_USB_GET_DRIVER_VERSION	CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+310,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS) 


#define IOCTL_TH_USB_GET_DRIVER_INFO       CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   Ezusb_IOCTL_INDEX+311,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
                                                   
#define	 DATABUF_SIZE	3*4096*255



//Twinhan Vender request
#define TH_COMMAND_IN                     0xC0
#define TH_COMMAND_OUT                    0xC1

//TH_COMMAND_OUT request type
#define FX2_REG_READ	                     0x00
#define FX2_REG_WRITE	                  0x01
#define TUNER_REG_READ	                  0x03
#define TUNER_REG_WRITE	                  0x04
#define RC_VAL_READ  	                  0x05
#define SET_TUNER_POWER	                  0x06
#define GET_USB_SPEED	                  0x07
#define SET_PLD_STATUS 	                  0x08
#define LOCK_TUNER_COMMAND                0x09
#define TUNER_SIGNAL_READ                 0x0A
#define FW_VERSION_READ  	               0x0B
#define VENDER_STRING_READ                0x0C
#define PRODUCT_STRING_READ               0x0D
#define SET_PLD_PID	                     0x0E
#define GET_PLD_PID  	                  0x0F
#define SET_EE_VALUE  	                  0x10
#define GET_EE_VALUE  	                  0x11


#define TUNER_REG_QUALITY_2 0x0A  //VIT_ERR_CNT
#define TUNER_REG_QUALITY_1 0x0B
#define TUNER_REG_QUALITY_0 0x0C
#define TUNER_REG_STRENGTH  0x09  //SNR

#define RC_NO_DATA                        0x44

#define Tuner_Power_ON                    1
#define Tuner_Power_OFF                   0

#define Tuner_Lock                        1
#define Tuner_UnLock                      0

#define PLD_ON                            1
#define PLD_OFF                           0

#define USB_SPEED_LOW                     0
#define USB_SPEED_FULL                    1
#define USB_SPEED_HIGH                    2

typedef struct
{
	unsigned char address;					// 0
	unsigned char frequencyMSB; 			// 1
	unsigned char frequencyLSB;  			// 2
	unsigned char tunerStep;     			// 3
	unsigned char symbolRateHSB;    		// 4
	unsigned char symbolRateMSB;    		// 5
	unsigned char symbolRateLSB;    		// 6
	unsigned char flag;						// 7
	unsigned char checkSum;		   		// 8
}	TUNER_DATA, *P_TUNER_DATA ;

typedef struct _REMOTE_EVENT
{
	HANDLE hEvent;
	LARGE_INTEGER DueTime; // requested DueTime in 100-nanosecond units
	PVOID pRemoteKey; 
} REMOTE_EVENT, *PREMOTE_EVENT;

typedef struct _DriverInfo {
    unsigned char  Version_Major;           // in BCD Ex., 3.2    =====> 0x32
    unsigned char  Version_Minor;           // 2.1    =====> 0x21
    unsigned char  FW_Version_Major;        // Ex., 1
    unsigned char  FW_Version_Minor;        // 5    =====> 1.5
    char Date_Time[22];                     // Ex.,"2004-12-20 18:30:00" or  "DEC 20 2004 10:22:10"  with compiler __DATE__ and __TIME__  definition s
    char Company[8];                        // Ex.,"TWINHAN" 
    char SupportHWInfo[32];                 // Ex.,"PCI DVB CX-878 with MCU series", "PCI ATSC CX-878 with MCU series", "7020/7021 USB-Sat", , "7045/7046 USB-Ter",.....................
    char Reserved[190 ];
} DriverInfo, *P_DriverInfo;

#endif //DEV_IOCTL_H
