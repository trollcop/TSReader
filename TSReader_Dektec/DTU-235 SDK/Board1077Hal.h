#pragma once
#include <fstream>
// Dektec
#include "DTAPI.h"
// Micronas
#include "drx3973d.h"
#include "drx3973d_mc.h"         /* file containing firmware (microcode) */
// Micronas API
#include "MicronasApi.h"

//Call Backs
typedef VOID (WINAPI* COMMUNICATION_ERROR_CALLBACK)(LPCTSTR);
typedef VOID (WINAPI* RECORD_PROGRESS_CALLBACK)(long);
typedef VOID (WINAPI* RECORD_STATUS_CALLBACK)(long);


enum EepromNames
{
   EEPROM_1,
   EEPROM_2,
   EEPROM_3
};

const int CALDATA_CURRENT_VERSION = 4;
const int CALDATA_OLD_VERSION_NO_VOLT = 3;
const int CAL_NUM_LIN_DBMV = 63;
const int CAL_NUM_FREQ = 75;
const int CAL_NUM_TUNER_DBMV = 27;
const int CAL_EEPROM = EEPROM_1;

const int CAL_VOLT_NUM_FREQ = 15;

const int EQ_DATA_CURRENT_VERSION = 1;
const int EQ_NUM_FREQ = 134;
const int EQ_EEPROM_1_MAX_BYTES = 0x8000;
const int EQ_EEPROM_1 = EEPROM_2;
const int EQ_EEPROM_2 = EEPROM_3;

const long TUNER_MINIMUM = 50000000;
const long TUNER_MAXIMUM = 860000000;

#define IIC_ADDR_TUNER     0xC6
#define IIC_ADDR_DEMOD     0x18
#define IIC_ADDR_ADC       0x50
#define IIC_ADDR_DAC       0x58 
#define IIC_ADDR_IO        0x4C
#define IIC_ADDR_IO2       0x4A
#define IIC_ADDR_IO_EE     0xAC
#define IIC_ADDR_EEPROM_1  0xAA
#define IIC_ADDR_EEPROM_2  0xA8
#define IIC_ADDR_EEPROM_3  0xAE

const int CARRIER_OFFSET = 1750000;

enum RecStatus
{
   REC_STATUS_UNKNOWN_ERROR = -1,
   REC_STATUS_DONE,
   REC_STATUS_NO_PSYNC,
   REC_STATUS_LOST_PSYNC,
   REC_STATUS_CANNOT_CREATE_FILE,
};

enum IoExpander
{
   //Io Expander
   IO_NOT_RED_LED = 0x01,           //Bit 0 = !Red LED
   IO_NOT_GREEN_LED = 0x02,         //Bit 1 = !Green LED
   // Nothing                       //Bit 2 = Nothing   
   IO_FPGA_IO = 0x08,               //Bit 3 = FPGA I/O
   IO_EXTERNAL_RF_AGC = 0x10,       //Bit 4 = External RF AGC
   IO_EXTERNAL_IF_AGC = 0x20,       //Bit 5 = External IF AGC
   IO_TUNER_ENABLE = 0x40,          //Bit 6 = Tuner IIC Enable
   IO_EEPROM_WRITE_PROTECT = 0x80   //Bit 7 = !WC of the Io Expander's Eeprom
};

enum IoExpander2
{
   //Io Expander
   IO_ATTEN_1 = 0x01,               //Bit 0 = Attenuator 1
   IO_ATTEN_2 = 0x02,               //Bit 1 = Attenuator 2
   IO_ATTEN_4 = 0x04,               //Bit 2 = Attenuator 4
   IO_ATTEN_8 = 0x08,               //Bit 3 = Attenuator 8
   IO_ATTEN_16 = 0x10,              //Bit 4 = Attenuator 16
   IO_ATTEN_PATH = 0x20,            //Bit 5 = Attenuator Path ( Bit 5 Must != Bit 6 )
   IO_NON_ATTEN_PATH = 0x40,        //Bit 6 = Non-Attenuator Path 
   IO_ATTEN_LATCH = 0x80            //Bit 7 = Latch Pin
};


class CBoard1077Hal
{
public:
   CBoard1077Hal(void);
   virtual ~CBoard1077Hal(void);

   CMicronasDevice *m_MicronasDevice;
   
	BOOL Initialize(short CardIndex);
   BOOL Initialize(__int64 SerialNumber) ;
	BOOL Uninitialize();
	long GetTunerFrequencyHz();
	bool SetTunerFrequencyHz(long nNewValue);
	short GetAdcNbRssi();
	short GetAdcWbRssi();
	short GetAdcTunerAGC();
	short GetAdcFineAGC();
	short GetAdcTemperature();
   short GetAdcVoltage();
	BOOL GetIoRfAttenuation();
	BOOL SetIoRfAttenuation(BOOL bNewValue);
	BOOL GetIoExtAGC();
	BOOL SetIoExtAGC(BOOL bNewValue);
	BOOL DemodGetSymbols(short FAR* SymbolsArray, short SymbolsArrayLength);
	BOOL DacSetAGC(short Level);
	BOOL ExecuteRecord(LPCTSTR strFilename, long lngPackets);
	void ExecuteStop();
	void Versions( long FAR* Driver, long FAR* Firmware );
   void RegisterCommunicationErrorHandle( VOID (WINAPI* registerHandle)(LPCTSTR) );
   void RegisterRecordProgressHandle( VOID (WINAPI* registerHandle)(long) );
   void RegisterRecordStatusHandle( VOID (WINAPI* registerHandle)(long) );
   double GetChannelLevel();
   double GetNbLevel();
   bool SaveCalData();
   bool SetCalData( long Table, long Index1, long Index2, short Value, short Password );
   bool GetCalTime( __int64 *Time );
   bool GetSerialNumber( __int64 *Serial );
   bool DongleBoard( LPCTSTR DongleXml );
   bool IsDongled();

   DRXStatus_t GetLockStatus( pDRXLockStatus_t lockStatus );
   UCHAR GetChannelBandwidth();
   bool SetChannelBandwidth( UCHAR mode );
   UCHAR GetFftMode();
   bool SetFftMode( UCHAR mode );
   UCHAR GetChannelGuard();
   bool SetChannelGuard( UCHAR mode );
   UCHAR GetChannelConstellation();
   bool SetChannelConstellation( UCHAR mode );
   UCHAR GetChannelCoderate();
   bool SetChannelCoderate( UCHAR mode );
   UCHAR GetChannelHierarchy();
   bool SetChannelHierarchy( UCHAR mode );
   UCHAR GetChannelMirror();
   bool SetChannelMirror( UCHAR mode );
   UCHAR GetChannelPriority();
   bool SetChannelPriority( UCHAR mode );
   UCHAR GetChannelClassification();
   bool SetChannelClassification( UCHAR mode );
   bool SetChannelParameters();
   bool GetChannelParameters();

   bool GetSignalQuality();
   double GetMER();
   double GetPreVitberBiBER();
   double GetPostVitberBiBER();
   short GetPacketError();
   bool GetBoardRevision( char *rev );

private:
   DtDevice m_DtDevice;
   BOOL SetupCard();
   bool WriteIic( UCHAR *Data, ULONG Length );
   bool ReadIic( UCHAR Address, UCHAR *Data, ULONG Length );
   static UINT RecordThread( LPVOID pParam );
   void FireCommunicationError( LPCTSTR ErrorMessage );
   void FireRecordProgress( long Percent );
   void FireRecordStatus( long Message );

   double TempAdjust();
   double TempAdjustTunerGain();
   double Polate( short InVal, int NumElements, short InArray[], short OutArray[], bool IncrementingOrder );
   bool WriteEeprom( USHORT Address, UCHAR *Data, ULONG Length, int EepromName );
   bool ReadEeprom( USHORT Address, UCHAR *Data, ULONG Length, int EepromName );
   bool LoadCalData();
   bool LoadEqData();
   void FFT( double *Real, double *Imaginary, long Length, double Direction );

   COMMUNICATION_ERROR_CALLBACK m_pCommunicationErrorCallback;
   RECORD_PROGRESS_CALLBACK m_pRecordProgressCallback;
   RECORD_STATUS_CALLBACK m_pRecordStatusCallback;

   DRXChannel_t m_channelParams;
   DRXChannel_t m_getChannelParams;
   DRXSigQuality_t m_getSignalQuality;

   bool m_bRecording;
   bool m_Dongled;
   bool m_NeedToCheckDongle;

   bool m_CalDataOk;
   struct CalDataHeader_T
   {
      __time64_t CalTime;
      unsigned short TableVersion;
      unsigned short CalVotage;
      unsigned long  Reserved1;
      unsigned long  Reserved2;
      unsigned long  Reserved3;
      unsigned long  Reserved4;
      unsigned long  Reserved5;
   }CalDataHeader;

   struct CalData_T
   {
      short LinCountsNb[64];
      short LinCountsWb[64];
      short MaxAgcErr[76];
      short AttenOnErr[76];
      
      struct Tuner_T
      {
         short AgcCounts[32];
         short TunerGain[32];
      }Tuner[76];
      
      short Temp;
   }CalData;

   enum AttenType
   {
      ATTEN_ON,
      ATTEN_OFF,
      ATTEN_TOTAL
   };

   enum CalData_E
   {
      CAL_LINCOUNTS_NB,
      CAL_LINCOUNTS_WB,
      CAL_MAX_AGC_ERR,
      CAL_ATTEN_ON_ERR,
      CAL_TUNER_AGC,
      CAL_TUNER_GAIN,
      CAL_TEMP,
   };

   struct CalDataConst_T
   {
      short LinDbmv[CAL_NUM_LIN_DBMV];
      short LevelFreq[CAL_NUM_FREQ];
   }static CalDataConst;

};
