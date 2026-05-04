// Sky Seeker board types
#ifdef EIGHTPSK
 #ifndef DVBTECH
  #define TARGET_PRODUCT_ID 0x0001
 #else DVBTECH
  #define TARGET_PRODUCT_ID 0x0100
 #endif DVBTECH
#endif EIGHTPSK
#ifdef ALPSTUNER
 #define TARGET_PRODUCT_ID 0x0002
#endif ALPSTUNER
#ifdef SPI
 #ifndef HORIZON
  #define TARGET_PRODUCT_ID 0x0003
 #else HORIZON
  #define TARGET_PRODUCT_ID 0x0010
 #endif HORIZON
#endif SPI
#ifdef DVBS
 #define TARGET_PRODUCT_ID 0x0004
#endif DVBS
#ifdef CIELPLUS_SKY
 #define TARGET_PRODUCT_ID 0x0006
#endif CIELPLUS_SKY
#ifdef CIELPLUS_5000
 #define TARGET_PRODUCT_ID 0x0007
#endif CIELPLUS_5000

//EP1 out commands
#define outDNHardware 1
#define outI2CWRITE 2
#define outI2CWRITEEEPROM 3
#define outI2CREAD 4
#define outReadBCM4500RAM 5
#define outWriteBCM4500RAM 6
#define outDMAControl 7
#define outGetConfiguration 8
#define outWriteIntersil 9
#define outReadIntersil 10
#define out22KHzOnOff 11
#define outPOLARITY 12
#define outToneBurst 13
#define outDiSEqC 14
#define outSwitchSyncAsync 15
#define outTune 16
#define outLockStatus 17
#define outGetOverruns 20
#define outDishSwitch 21
#define outReadSignal 22
#define outSetupSharpFrequency 23
#define outSpectrumScan 24
#define outSendSkyIr 25
#define outStartStop38KHz 26
#define outEEPROMWriteEnable 27
#define outSwitchSyncAsyncExtended 29

#ifndef DVBTECH
//Intersil register definitions
 #define SR1		0x80
 #define DCL		0x40
 #define ISEL1		0x20
 #define ENT1		0x10
 #define LLC1		0x08
 #define VSEL1		0x04
 #define EN1		0x02
 #define OLF1		0x01

 #define SR2		0x80
 #define EN2		0x04
 #define OTF		0x01
#else DVBTECH
//ST register definitions
 #define PCL		0x80
 //#define ISEL1		0x40
#define ISEL1 0x00
 #define TEN		0x20
 #define LLC		0x10
 #define VSEL1		0x08
 #define EN1		0x04
 #define OTF		0x02
 #define OLF		0x01
#endif DVBTECH

