#include "stdafx.h"
#include "Board1077Hal.h"

#include <math.h>
#include <fstream>
#include <iostream>

using namespace std;

#ifdef ALLOW_DOT_NET
using namespace System;
using namespace System::IO;
using namespace System::Xml;
using namespace System::Xml::Serialization;
using namespace System::Text;
#endif

CBoard1077Hal::CalDataConst_T CBoard1077Hal::CalDataConst = 
{
   {
      -48, -47, -46, -45, -44, -43, -42, -41, -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, 
      -28, -27, -26, -25, -24, -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9, 
      - 8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10, 11,
       12,  13,  14 
   },
   {
       57,  63,  69,  79,  85,  91,  97, 103, 109, 115, 127, 139, 151, 157, 163, 169, 183, 195, 207, 219, 
      231, 243, 255, 267, 279, 291, 303, 315, 327, 339, 351, 363, 375, 387, 399, 411, 423, 435, 441, 447, 
      453, 465, 479, 491, 503, 515, 527, 539, 551, 563, 575, 587, 599, 611, 623, 635, 647, 659, 671, 683, 
      695, 707, 719, 731, 743, 755, 767, 779, 791, 803, 815, 827, 839, 851, 857 
   }
};

CBoard1077Hal::CBoard1077Hal(void)
{
   m_pCommunicationErrorCallback = NULL;
   m_pRecordProgressCallback = NULL;
   m_pRecordStatusCallback = NULL;

   m_CalDataOk = false;
   m_Dongled = false;
   m_NeedToCheckDongle = true;

   m_channelParams.fftmode        = DRX_FFTMODE_AUTO;
   m_channelParams.guard          = DRX_GUARD_AUTO;
   m_channelParams.constellation  = DRX_CONSTELLATION_AUTO;
   m_channelParams.coderate       = DRX_CODERATE_AUTO;
   m_channelParams.hierarchy      = DRX_HIERARCHY_AUTO;
   m_channelParams.mirror         = DRX_MIRROR_AUTO;
   m_channelParams.priority       = DRX_PRIORITY_HIGH;
   m_channelParams.classification = DRX_CLASSIFICATION_AUTO;

   m_getChannelParams.bandwidth = DRX_BANDWIDTH_UNKNOWN;
}

CBoard1077Hal::~CBoard1077Hal(void)
{
}

/////////////////////////////////////////////////////////////////////////////
// CBoard1077Hal message handlers

BOOL CBoard1077Hal::Initialize(short CardIndex) 
{
   BOOL RetVal = TRUE;
   DTAPI_RESULT DtapiResult;

   DtapiResult = m_DtDevice.AttachToType( 235, CardIndex );
   
   m_MicronasDevice = new CMicronasDevice( &m_DtDevice );

   switch( DtapiResult )
   {
      case DTAPI_OK:
         break;     
      case DTAPI_E_ATTACHED:     
         FireCommunicationError( "DTAPI_E_ATTACHED" );
         break;     
     case DTAPI_E_INTERNAL:      
         FireCommunicationError( "DTAPI_E_INTERNAL" );
         break;     
      case DTAPI_E_NO_DEVICE:     
         FireCommunicationError( "DTAPI_E_NO_DEVICE" );
         break;     
      case DTAPI_E_NO_SUCH_DEVICE:
         FireCommunicationError( "DTAPI_E_NO_SUCH_DEVICE" );
         break;     
      case DTAPI_E_DRIVER_INCOMP: 
         FireCommunicationError( "DTAPI_E_DRIVER_INCOMP" );
         break;
      default:
         FireRecordProgress( DtapiResult );
   }

   if( DtapiResult != DTAPI_OK )
   {
      RetVal = FALSE;
   }
   else
   {
      RetVal = SetupCard();
   }
   return RetVal;
}

BOOL CBoard1077Hal::Initialize(__int64 SerialNumber) 
{
   DTAPI_RESULT DtapiResult;
   BOOL RetVal = TRUE;

   DtapiResult = m_DtDevice.AttachToSerial( SerialNumber );

   m_MicronasDevice = new CMicronasDevice( &m_DtDevice );

   if( DtapiResult != DTAPI_OK )
   {
      RetVal = FALSE;
   }
   else
   {  
/*
      UCHAR *Data2 = new UCHAR[256];
      // Read something back from address 0
      ReadEeprom( 0, Data2, 256, CAL_EEPROM );
*/
      RetVal = SetupCard();
   }
   
   return RetVal;
}

BOOL CBoard1077Hal::SetupCard() 
{
   UCHAR Data[2];
   bool  bolErrorMsg;

   // Set up IoExpander
   Data[0] = IIC_ADDR_IO;
   Data[1] = IO_EEPROM_WRITE_PROTECT | IO_FPGA_IO | IO_NOT_RED_LED | IO_NOT_GREEN_LED;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError( "Initialize: Unable to Write to the IoDevice" );
   }
   
   // Set up IoExpander2
   Data[0] = IIC_ADDR_IO2;
   Data[1] = IO_NON_ATTEN_PATH | IO_ATTEN_LATCH;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError( "Initialize: Unable to Write to the IoDevice 2" );
   }
   
   // Latch Low
   Data[0] = IIC_ADDR_IO2;
   Data[1] = IO_NON_ATTEN_PATH;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg = false )
      FireCommunicationError( "Initialize: Failed on Set Latch Low" );

   // Latch High
   Data[0] = IIC_ADDR_IO2;
   Data[1] = IO_NON_ATTEN_PATH | IO_ATTEN_LATCH;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg == false )
      FireCommunicationError( "Initialize: Failed on Set Latch High" );
      
   // Load the Calibration Data
   m_CalDataOk = LoadCalData();   

   return m_MicronasDevice->Initialize();
}

BOOL CBoard1077Hal::Uninitialize() 
{
   m_DtDevice.Detach();
   m_MicronasDevice->Uninitialize();
   return TRUE;
}

//  Little Helper Functions
bool CBoard1077Hal::WriteIic( UCHAR *Data, ULONG Length )
{  
   bool RetVal = false;
   DTAPI_RESULT DtResult;

   DtResult = m_DtDevice.I2CWrite( Data[0], (char*)(Data + 1), Length - 1 );

   RetVal = DtResult == DTAPI_OK;
   
   return RetVal;
}

bool CBoard1077Hal::ReadIic( UCHAR Address, UCHAR *Data, ULONG Length )
{
   bool RetVal = false;
   DTAPI_RESULT DtResult;

   DtResult = m_DtDevice.I2CRead( Address, (char*)Data, Length );

   RetVal = DtResult == DTAPI_OK;

   return RetVal;
}

void CBoard1077Hal::FireCommunicationError(LPCTSTR ErrorMessage)
{
   if( m_pCommunicationErrorCallback != NULL )
   {
      m_pCommunicationErrorCallback( ErrorMessage );
   }
}

void CBoard1077Hal::FireRecordProgress(long Percent)
{
   if( m_pRecordProgressCallback != NULL )
   {
      m_pRecordProgressCallback( Percent );
   }
}

void CBoard1077Hal::FireRecordStatus(long Message)
{
   if( m_pRecordStatusCallback != NULL )
   {
      m_pRecordStatusCallback( Message );
   }
}

void CBoard1077Hal::RegisterCommunicationErrorHandle( COMMUNICATION_ERROR_CALLBACK registerHandle )
{
   m_pCommunicationErrorCallback = registerHandle;
}

void CBoard1077Hal::RegisterRecordProgressHandle( RECORD_PROGRESS_CALLBACK registerHandle )
{
   m_pRecordProgressCallback = registerHandle;
}

void CBoard1077Hal::RegisterRecordStatusHandle( RECORD_STATUS_CALLBACK registerHandle )
{
   m_pRecordStatusCallback = registerHandle;
}

// ****************************************************************************
//
// TunerFrequencyHz 
//
// GetTunerFrequencyHz - Returns the current value of the TunerFrequencyHz.
// SetTunerFrequencyHz - Sets the Tuner to the supplied Frequency.
//
// ****************************************************************************

long CBoard1077Hal::GetTunerFrequencyHz() 
{
   return m_channelParams.frequency;
}

bool CBoard1077Hal::SetTunerFrequencyHz(long nNewValue) 
{
   nNewValue = nNewValue * 1000;
   
   ULONG ulngNominalFreq;
   UCHAR byteData[5];
   UCHAR byteNewBand;
   UCHAR Data[2];
   bool  bolErrorMsg;

   if( nNewValue < TUNER_MINIMUM || nNewValue > TUNER_MAXIMUM)
   {
      FireCommunicationError("SetTunerFrequencyHz: Invalid Frequency");
      return false;
   }
   
   // Turn On the Tuner's Line.
   bolErrorMsg = ReadIic( IIC_ADDR_IO, &Data[1], 1 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError( "SetTunerFrequencyHz: Unable to Read from the IoDevice" );
      return FALSE;
   }
   
   Data[0] = IIC_ADDR_IO;
   Data[1] |= IO_TUNER_ENABLE;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError( "SetTunerFrequencyHz: Unable to Write to the IoDevice" );
      return FALSE;
   }
   
   // Determine the New Band
   //  50 MHz to 162 MHz = 1
   // 162 MHz to 448 MHz = 2
   // 448 MHz to 858 MHz = 4
   
   if( nNewValue <= 162000000 )
   {
      byteNewBand = 1;
   }
   else if ( nNewValue <= 448000000 )
   {
      byteNewBand = 2;
   }
   else
   {
      byteNewBand = 4;
   }

   ulngNominalFreq = ( nNewValue + 36125000 ) / 62500;

   byteData[0] = IIC_ADDR_TUNER;
   byteData[1] = (UCHAR)( ( ulngNominalFreq & 0x7FFF ) >> 8 );
   byteData[2] = (UCHAR)( ulngNominalFreq & 0x00FF );
   
   // Tuner Pll Charge Pump Settings
   // Pll step = 62.5 kHz
   if( nNewValue <= 366000000 )
   {
      // Low Charge Pump
      byteData[3] = 0xBE;
   }
   else if( nNewValue > 366000000 && nNewValue <= 448000000 )
   {
      // Medium Charge Pump
      byteData[3] = 0xF6;
   }
   else if( nNewValue > 448000000 && nNewValue <= 646000000 )
   {
      // Low Charge Pump
      byteData[3] = 0xBE;
   }
   else if( nNewValue > 646000000 && nNewValue <= 790000000 )
   {
      // Medium Charge Pump
      byteData[3] = 0xF6;
   }
   else if( nNewValue > 790000000 )
   {
      // High Charge Pump
      byteData[3] = 0xFE;
   }
   
   if( m_channelParams.bandwidth == DRX_BANDWIDTH_8MHZ )
   {
      byteData[4] = byteNewBand; // Enable 8 Mhz saw filter
   }
   else if( m_channelParams.bandwidth == DRX_BANDWIDTH_7MHZ 
      || m_channelParams.bandwidth == DRX_BANDWIDTH_6MHZ)
   {
      byteData[4] = 0x08 | byteNewBand; // Enable 7 Mhz saw filter      
   }
   else
   {
      byteData[4] = byteNewBand; // Enable 8 Mhz saw filter (assume it's 8)
   }
 
   //Write to the Tuner
   bolErrorMsg = WriteIic( byteData, 5 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("SetTunerFrequencyHz: Unable to Write to the Tuner");
      return false;
   }
   
   //Turn the Tuner Line back Off.
   Data[0] = IIC_ADDR_IO;
   Data[1] &= ~IO_TUNER_ENABLE;

   bolErrorMsg = WriteIic( Data, 2 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError( "SetTunerFrequencyHz: Unable to Write to the IoDevice" );
      return FALSE;
   }
   
   m_channelParams.frequency = nNewValue / 1000;

   return true;
}

// ****************************************************************************
//
// DemodGetSymbols 
//
// Gets the Symbols's from the Demod and stores them in the provided array.
//
// ****************************************************************************

BOOL CBoard1077Hal::DemodGetSymbols(short FAR* SymbolsArray, short SymbolsArrayLength) 
{  
   DRXStatus_t commandStatus;
   DRXComplex_t point;

   //Check to see if we have enough room to put the Results
   if(SymbolsArrayLength < 2)
   {
      FireCommunicationError("DemodGetSymbols: Not enough room for the Symbols");
      return FALSE;
   }

   for(int x = 0; x < SymbolsArrayLength/2; x++)
   {
      commandStatus = m_MicronasDevice->GetSymbol(&point);
      
      if( commandStatus != DRX_STS_OK )
      {
         FireCommunicationError("DemodGetSymbols: Read Failed");
         return FALSE;
      }

      SymbolsArray[(x*2)] = point.im;
      SymbolsArray[(x*2)+1] = point.re;
   }

   return TRUE;
}

// ****************************************************************************
//
// AdcNbRssi 
//
// GetAdcNbRssi - Returns the Conversion on NbRssi.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcNbRssi() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0xA8; // channel 2
   byteData[2] = 0xA8; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcNbRssi: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcNbRssi: Unable to read the NbRssi Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// AdcWbRssi 
//
// GetAdcWbRssi - Returns the Conversion on WbRssi.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcWbRssi() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0xB8; // channel 3
   byteData[2] = 0xB8; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcWbRssi: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcWbRssi: Unable to read the WbRssi Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// AdcTunerAGC 
//
// GetAdcTunerAGC - Returns the Conversion on TunerAGC.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcTunerAGC() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0xC8; // channel 4
   byteData[2] = 0xC8; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcTunerAGC: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcTunerAGC: Unable to read the TunerAGC Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// AdcFineAGC 
//
// GetAdcFineAGC - Returns the Conversion on FineAGC.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcFineAGC() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0xD8; // channel 5
   byteData[2] = 0xD8; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcFineAGC: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcFineAGC: Unable to read the FineAGC Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// AdcTemperature 
//
// GetAdcTemperature - Returns the Conversion on Temperature.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcTemperature() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0x00E8; // channel 6
   byteData[2] = 0x00E8; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcTemperature: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcTemperature: Unable to read the Temperature Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// GetAdcVoltage 
//
// GetAdcVoltage - Returns the Conversion on Voltage.
//
// ****************************************************************************

short CBoard1077Hal::GetAdcVoltage() 
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_ADC;
   byteData[1] = 0x0088; // channel 0
   byteData[2] = 0x0088; // Discarding the First Value.

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcVoltage: Unable to Write to the ADC");
      return 0;
   }

   bolErrorMsg = ReadIic( IIC_ADDR_ADC, byteData, 2 );

   if( bolErrorMsg == false)
   {
      FireCommunicationError("GetAdcVoltage: Unable to read the Voltage Conversion from the ADC");
      return 0;
   }

   return (( byteData[0] & 0x00FF ) << 4) | (( byteData[1] & 0x00F0 ) >> 4);
}

// ****************************************************************************
//
// IoRfAttenuation 
//
// GetIoRfAttenuation - Returns the state of the Rf Attenuation line.
// SetIoRfAttenuation - Sets the state of the Rf Attenuation line.
//
// ****************************************************************************

BOOL CBoard1077Hal::GetIoRfAttenuation() 
{
   BOOL bIoRfAttenuation = FALSE;
   UCHAR Data = 0;
   bool bErrorMsg;

   bErrorMsg = ReadIic( IIC_ADDR_IO2, &Data, 1 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "GetIoRfAttenuation: Unable to Read from the IIC IoDevice" );
   }

   if( Data & IO_ATTEN_PATH )
   {
      bIoRfAttenuation = TRUE;
   }

   return bIoRfAttenuation;
}

BOOL CBoard1077Hal::SetIoRfAttenuation(BOOL bNewValue) 
{
   BOOL bModifiedFlag = TRUE;
   UCHAR Data[2];
   bool bErrorMsg;

   bErrorMsg = ReadIic( IIC_ADDR_IO2, &Data[1], 1 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "SetIoRfAttenuation: Unable to Read from the IoDevice" );
      return FALSE;
   }

   Data[0] = IIC_ADDR_IO2;

   if( bNewValue == TRUE )
   {
      Data[1] |= IO_ATTEN_PATH;
      Data[1] &= ~IO_NON_ATTEN_PATH;
   }
   else
   {
      Data[1] |= IO_NON_ATTEN_PATH;
      Data[1] &= ~IO_ATTEN_PATH;
   }

   bErrorMsg = WriteIic( Data, 2 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "SetIoRfAttenuation: Unable to Write to the IIC IoDevice" );
   }

   return bModifiedFlag;
}

// ****************************************************************************
//
// IoExtAGC 
//
// GetIoExtAGC - Returns the state of the External AGC line.
// SetIoExtAGC - Sets the state of the External AGC line.
//
// ****************************************************************************

BOOL CBoard1077Hal::GetIoExtAGC() 
{
   BOOL bIoExtAGC = FALSE;
   UCHAR Data = 0;
   bool bErrorMsg;

   bErrorMsg = ReadIic( IIC_ADDR_IO, &Data, 1 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "GetIoExtAGC: Unable to Read from the IoDevice" );
   }

   if( Data & IO_EXTERNAL_RF_AGC )
   {
      bIoExtAGC = TRUE;
   }

   return bIoExtAGC;
}

BOOL CBoard1077Hal::SetIoExtAGC(BOOL bNewValue) 
{
   BOOL bModifiedFlag = TRUE;
   UCHAR Data[2];
   bool bErrorMsg;

   bErrorMsg = ReadIic( IIC_ADDR_IO, &Data[1], 1 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "SetIoExtAGC: Unable to Read from the IoDevice" );
      return FALSE;
   }

   Data[0] = IIC_ADDR_IO;

   if( bNewValue == TRUE )
   {
      Data[1] |= IO_EXTERNAL_RF_AGC;
   }
   else
   {
      Data[1] &= ~IO_EXTERNAL_RF_AGC;
   }

   bErrorMsg = WriteIic( Data, 2 );

   if( bErrorMsg = false )
   {
      FireCommunicationError( "SetIoExtAGC: Unable to Write to the IoDevice" );
      return FALSE;
   }

   return bModifiedFlag;
}

// ****************************************************************************
//
// DacSetAGC 
//
// Sets the Dac's AGC level = Vref * Level/256.
//
// ****************************************************************************

BOOL CBoard1077Hal::DacSetAGC(short Level)
{
   UCHAR byteData[3];
   bool  bolErrorMsg;

   byteData[0] = IIC_ADDR_DAC;
   byteData[1] = 0x00;
   byteData[2] = (UCHAR)Level;

   bolErrorMsg = WriteIic( byteData, 3 );

   if( bolErrorMsg == false )
   {
      FireCommunicationError("DacSetAGC: Unable to Write to the DAC");
      return FALSE;
   }

   return TRUE;
}

struct SRecordInfo
{
   CString strFilename;
   long lngPackets;
   CBoard1077Hal *pBoard1077Hal;
};

BOOL CBoard1077Hal::ExecuteRecord( LPCTSTR strFilename, long lngPackets ) 
{
   BOOL RetVal = FALSE;
   SRecordInfo *pRecordInfo;

   pRecordInfo = new SRecordInfo;

   if( pRecordInfo != NULL )
   {
      pRecordInfo->lngPackets = lngPackets;
      pRecordInfo->pBoard1077Hal = this;
      pRecordInfo->strFilename = strFilename;

      m_bRecording = true;

      AfxBeginThread( RecordThread, pRecordInfo, THREAD_PRIORITY_HIGHEST );

      Sleep( 100 );

      RetVal = TRUE;
   }

   return RetVal;
}

void CBoard1077Hal::ExecuteStop() 
{
   m_bRecording = false;
}

UINT CBoard1077Hal::RecordThread( LPVOID pParam )
{
   SRecordInfo *pRecordInfo = (SRecordInfo *)pParam;

   ULONG BytesRead;
   ULONG BytesWritten;
   unsigned __int64 TotalBytesRead = 0;
   unsigned __int64 ReadSize = (__int64)pRecordInfo->lngPackets * (__int64)188;
   unsigned __int64 BuffSize;
   const ULONG P_BUFF_SIZE = 0x100000;
   char *pBuff = new char[P_BUFF_SIZE];
   int RecPosition = -1;
   int CurPosition;
   int FailedReads = 0;
   bool bFindSync = true;
   DTAPI_RESULT DtapiResult;
   TsInpChannel TsIn;

   CString DirectoryName = pRecordInfo->strFilename;

   DirectoryName = DirectoryName.Left( DirectoryName.ReverseFind( '\\' ) );

   BOOL RetValue = CreateDirectory( DirectoryName, NULL );
   
   DtapiResult = TsIn.Attach( &pRecordInfo->pBoard1077Hal->m_DtDevice );

   if( DtapiResult == DTAPI_OK )
   {
      TsIn.SetRxControl( DTAPI_RXCTRL_RCV );
      Sleep( 50 );

      int RetryCount = 0;
      HANDLE hWriteFile = INVALID_HANDLE_VALUE;
   
      while( hWriteFile == INVALID_HANDLE_VALUE && RetryCount < 10 )
      {
         hWriteFile = CreateFile( pRecordInfo->strFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

         if( hWriteFile == INVALID_HANDLE_VALUE )
         {
            Sleep( 100 );
         }

         RetryCount++;
      }

      if( hWriteFile == INVALID_HANDLE_VALUE )
      {
            pRecordInfo->pBoard1077Hal->FireRecordStatus( REC_STATUS_CANNOT_CREATE_FILE );
      }
      else
      {
         BytesRead = 0;

         while( TotalBytesRead < ReadSize && pRecordInfo->pBoard1077Hal->m_bRecording == true )
         {
            BuffSize = min( ReadSize - TotalBytesRead, P_BUFF_SIZE );

            int FifoLoad = 0;
            TsIn.GetFifoLoad( FifoLoad );

            BytesRead = min( FifoLoad, (int)BuffSize );

            DtapiResult = TsIn.Read( pBuff, BytesRead );

            if( DtapiResult != DTAPI_OK )
            {
               BytesRead = 0;
            }

            if( bFindSync == true && BytesRead > 0 )
            {
               for( int i=0; i< 188; i++ )
               {
                  if( pBuff[i+188*0] == 0x47 &&
                        pBuff[i+188*1] == 0x47 &&
                        pBuff[i+188*2] == 0x47 &&
                        pBuff[i+188*3] == 0x47 &&
                        pBuff[i+188*4] == 0x47 )
                  {
                     break;
                  }
               }

               if( i == 188 )
               {
                  pRecordInfo->pBoard1077Hal->FireRecordStatus( REC_STATUS_NO_PSYNC );
                  break;
               }
               else
               {
                  BytesRead -= i;
                  WriteFile( hWriteFile, pBuff + i, BytesRead, &BytesWritten, NULL );
                  TotalBytesRead += BytesRead;
                  BytesRead = 0;
               }

               bFindSync = false;
            }


            WriteFile( hWriteFile, pBuff, BytesRead, &BytesWritten, NULL );

            TotalBytesRead += BytesRead;

            if( BytesRead == 0 )
            {
               FailedReads++;

               if( FailedReads > 50 )
               {
                  FailedReads++;
               }
               if( FailedReads > 100 )
               {
                  pRecordInfo->pBoard1077Hal->FireRecordStatus( REC_STATUS_NO_PSYNC );
                  break;
               }

               Sleep( 50 );
            }
            else
            {
               FailedReads = 0;
            }

            //Calculate Percent Done.
            CurPosition = (ULONG)((TotalBytesRead * 100)/ReadSize);

            if( CurPosition != RecPosition )
            {
               RecPosition = CurPosition;

               pRecordInfo->pBoard1077Hal->FireRecordProgress( RecPosition );
            }
         }

         if( TotalBytesRead >= ReadSize )
         {
            pRecordInfo->pBoard1077Hal->FireRecordStatus( REC_STATUS_DONE );
         }

         CloseHandle( hWriteFile );
      }
      
      TsIn.Detach( DTAPI_INSTANT_DETACH );
   }
   else
   {
      pRecordInfo->pBoard1077Hal->FireRecordStatus( REC_STATUS_UNKNOWN_ERROR );
   }

   delete [] pBuff;

   delete pRecordInfo;

   return 0;
}

void CBoard1077Hal::Versions( long FAR* Driver, long FAR* Firmware ) 
{
   int major, minor, bug, build;
   m_DtDevice.GetDeviceDriverVersion( major, minor, bug, build );

   *Driver = major << 24 | minor << 16 | bug << 8 | build;

   m_DtDevice.GetFirmwareVersion( major );

   *Firmware = major << 24;
}

double CBoard1077Hal::GetChannelLevel()
{
   short RawWb;
   short RawAgc;
   double ChannelLevel;
   short intLoop;
   short ClosestFreq;
   short Freq;
   double LowerTunerGain;
   double UpperTunergain;
   double InterpolatedTg;
   double CalVolt;
   double CurVolt;
   double clippedInterpolatedTg;

   Freq = (short)( (GetTunerFrequencyHz() * 1000) / 1000000 );

   CalVolt = CalDataHeader.CalVotage / 100.0;
   CurVolt = GetAdcVoltage() / 500.0;

   RawWb = GetAdcWbRssi();
   RawAgc = GetAdcTunerAGC();

   // Find Channel Level Based on WbRssi Value
   ChannelLevel = Polate( RawWb, CAL_NUM_LIN_DBMV, CalData.LinCountsWb, CalDataConst.LinDbmv, true );

   // Subtract Frequency-based Error from Channel Level
   ChannelLevel -= Polate( Freq, CAL_NUM_FREQ, CalDataConst.LevelFreq, CalData.MaxAgcErr, true ) / 100.0;
   
   // If Attentuator is on, subtract Attenuator Error (Frequency-based)
   if( GetIoRfAttenuation() == TRUE )
   {
      ChannelLevel -= Polate( Freq, CAL_NUM_FREQ, CalDataConst.LevelFreq, CalData.AttenOnErr, true ) / 100.0;
   }

   //Find closest freq for tuner gain
   for( intLoop=0; intLoop<(CAL_NUM_FREQ - 1); intLoop++ )
   {
      if( Freq < CalDataConst.LevelFreq[intLoop + 1] )
      {
         ClosestFreq = intLoop;
         break;
      }
   }
  
   // If the Frequency is above the highest Frequency in the array, set it to the highest Frequency element
   if( intLoop == CAL_NUM_FREQ - 1 )
   {
      ClosestFreq = CAL_NUM_FREQ - 1;
      InterpolatedTg = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq].AgcCounts, CalData.Tuner[ClosestFreq].TunerGain, false );
   }
   else  // Otherwise, calculate the LowerTunerGain and UpperTunderGain and Interpolate the two
   {
      LowerTunerGain = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq].AgcCounts, CalData.Tuner[ClosestFreq].TunerGain, false );
      UpperTunergain = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq + 1].AgcCounts, CalData.Tuner[ClosestFreq + 1].TunerGain, false );
      InterpolatedTg = LowerTunerGain + ( Freq - CalDataConst.LevelFreq[ClosestFreq] ) / ( CalDataConst.LevelFreq[ClosestFreq + 1] - CalDataConst.LevelFreq[ClosestFreq] ) * ( UpperTunergain - LowerTunerGain );
   }
   // Temperature & Voltage Correct the Tuner Gain Offset after Interpolation
   InterpolatedTg = InterpolatedTg / 100;
   
   if( InterpolatedTg > -5 )
   {
      clippedInterpolatedTg = InterpolatedTg;
   }
   else
   {
      clippedInterpolatedTg = -5;
   }
   // Tuner Gain Voltage Correction
   InterpolatedTg += (2 * clippedInterpolatedTg * ( CurVolt - CalVolt ));
   // Tuner Gain Temperature Correction
   InterpolatedTg += clippedInterpolatedTg * TempAdjustTunerGain();
   // Subtracting-in Voltage and Temperature Tuner Gain Correction into Channel Level
   ChannelLevel -= InterpolatedTg;
   // Adding in Overall Temperature correction to Channel Level
   ChannelLevel += TempAdjust();

   if( ( ChannelLevel < 15.0 ) && ( GetIoRfAttenuation() == TRUE ) )
   {
      SetIoRfAttenuation( FALSE );
      Sleep( 200 );
      ChannelLevel = GetChannelLevel();
   }
   else if( ( ChannelLevel > 20.0 ) && ( GetIoRfAttenuation() == FALSE ) )
   {
      SetIoRfAttenuation( TRUE );
      Sleep( 200 );
      ChannelLevel = GetChannelLevel();
   }

   //Adding channel level correction for channel width over 6 Mhz
   if( m_channelParams.bandwidth == DRX_BANDWIDTH_6MHZ )
   {
      // Add nothing
   }
   else if( m_channelParams.bandwidth == DRX_BANDWIDTH_7MHZ )
   {
      ChannelLevel += .67;
   }
   else if( m_channelParams.bandwidth == DRX_BANDWIDTH_8MHZ )
   {
      ChannelLevel += 1.25;
   }
   return ChannelLevel;
}

double CBoard1077Hal::GetNbLevel()
{
   short RawNb;
   short RawAgc;
   double NbLevel;
   short intLoop;
   short ClosestFreq;
   short Freq;
   double LowerTunerGain;
   double UpperTunergain;
   double InterpolatedTg;
   double CalVolt;
   double CurVolt;
   double clippedInterpolatedTg;

   Freq = (short)( (GetTunerFrequencyHz() * 1000) / 1000000 );

   CalVolt = CalDataHeader.CalVotage / 100.0;
   CurVolt = GetAdcVoltage() / 500.0;

   RawNb = GetAdcNbRssi();
   RawAgc = GetAdcTunerAGC();

   NbLevel = Polate( RawNb, CAL_NUM_LIN_DBMV, CalData.LinCountsNb, CalDataConst.LinDbmv, true );

   NbLevel -= Polate( Freq, CAL_NUM_FREQ, CalDataConst.LevelFreq, CalData.MaxAgcErr, true ) / 100.0;

   if( GetIoRfAttenuation() == TRUE )
   {
      NbLevel -= Polate( Freq, CAL_NUM_FREQ, CalDataConst.LevelFreq, CalData.AttenOnErr, true ) / 100.0;
   }

   //Find closest freq for tuner gain
   for( intLoop=0; intLoop<(CAL_NUM_FREQ - 1); intLoop++ )
   {
      if( Freq < CalDataConst.LevelFreq[intLoop + 1] )
      {
         ClosestFreq = intLoop;
         break;
      }
   }

   if( intLoop == CAL_NUM_FREQ - 1 )
   {
      ClosestFreq = CAL_NUM_FREQ - 1;
      InterpolatedTg = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq].AgcCounts, CalData.Tuner[ClosestFreq].TunerGain, false );
   }
   else
   {
      LowerTunerGain = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq].AgcCounts, CalData.Tuner[ClosestFreq].TunerGain, false );
      UpperTunergain = Polate( RawAgc, CAL_NUM_TUNER_DBMV, CalData.Tuner[ClosestFreq + 1].AgcCounts, CalData.Tuner[ClosestFreq + 1].TunerGain, false );
      InterpolatedTg = LowerTunerGain + ( Freq - CalDataConst.LevelFreq[ClosestFreq] ) / ( CalDataConst.LevelFreq[ClosestFreq + 1] - CalDataConst.LevelFreq[ClosestFreq] ) * ( UpperTunergain - LowerTunerGain );
   }
   // Temperature Correct the Tuner Gain Offset after Interpolation
   InterpolatedTg = InterpolatedTg / 100;
   
   if( InterpolatedTg > -5 )
   {
      clippedInterpolatedTg = InterpolatedTg;
   }
   else
   {
      clippedInterpolatedTg = -5;
   }
   // Tuner Gain Voltage Correction
   InterpolatedTg += (2 * clippedInterpolatedTg * ( CurVolt - CalVolt ));
   // Tuner Gain Temperature Correction
   InterpolatedTg += clippedInterpolatedTg * TempAdjustTunerGain();
   // Subtracting-in Voltage and Temperature Tuner Gain Correction into Nb Level
   NbLevel -= InterpolatedTg;
   // Adding in Overall Temperature correction to Nb Level
   NbLevel += TempAdjust();
   
   return NbLevel;
}


double CBoard1077Hal::TempAdjust()
{
   double TempAdjuster;

   if( CalData.Temp == 0 )
   {
      TempAdjuster = 0.0;
   }
   else
   {
      TempAdjuster = ( GetAdcTemperature() - CalData.Temp ) / 100.0;
   }

   return TempAdjuster;
}


double CBoard1077Hal::TempAdjustTunerGain()
{
   double TempAdjuster;
   
   if( CalData.Temp == 0 )
   {
      TempAdjuster = 0.0;
   }
   else
   {
      TempAdjuster = (GetAdcTemperature() - CalData.Temp) / 500.0;
   }
   
   return TempAdjuster;
}


double CBoard1077Hal::Polate( short InVal, int NumElements, short InArray[], short OutArray[], bool IncrementingOrder )
{
   int intLoop;

   if( IncrementingOrder == true )
   {
      if( InVal < InArray[0] )
      {
         return (double)OutArray[0];
      }

      for( intLoop = 0; intLoop < ( NumElements - 1); intLoop++ )
      {
         if( ( InVal >= InArray[intLoop] ) && ( InVal < InArray[intLoop + 1] ) )
         {
            return OutArray[intLoop] + ( (double)( InVal - InArray[intLoop] ) * ( OutArray[intLoop + 1] - OutArray[intLoop] ) / (double)( InArray[intLoop + 1] - InArray[intLoop] ) );
         }
      }
   }
   else
   {
      if( InVal > InArray[0] )
      {
         return OutArray[0];
      }

      for( intLoop = 0; intLoop < ( NumElements - 1); intLoop++ )
      {
         if( (InVal <= InArray[intLoop] ) && ( InVal > InArray[intLoop + 1] ) )
         {
            return OutArray[intLoop] + ( (double)( InVal - InArray[intLoop] ) * ( OutArray[intLoop + 1] - OutArray[intLoop] ) / (double)( InArray[intLoop + 1] - InArray[intLoop] ) );
         }
      }
   }

   return (double)OutArray[NumElements - 1];
}

bool CBoard1077Hal::WriteEeprom( USHORT Address, UCHAR *Data, ULONG Length, int EepromName )
{
   const ULONG MAX_BYTES_TO_WRITE = 32;
   UCHAR AddressHigh;
   UCHAR AddressLow;
   ULONG BytesToWrite;
   ULONG BytesWritten = 0;
   UCHAR WriteBuffer[MAX_BYTES_TO_WRITE + 3];
   UCHAR IicAddress;

   switch( EepromName )
   {
      case EEPROM_1:
         IicAddress = IIC_ADDR_EEPROM_1;
         break;

      case EEPROM_2:
         IicAddress = IIC_ADDR_EEPROM_2;
         break;

      case EEPROM_3:
         IicAddress = IIC_ADDR_EEPROM_3;
         break;
   }
   
   while( BytesWritten < Length )
   {
      AddressHigh = (UCHAR)( Address >> 8 );
      AddressLow = (UCHAR)Address;

      WriteBuffer[0] = IicAddress;
      WriteBuffer[1] = AddressHigh;
      WriteBuffer[2] = AddressLow;

      BytesToWrite = min( 64 - ( AddressLow & 0x3F ), MAX_BYTES_TO_WRITE );
      BytesToWrite = min( BytesToWrite, Length - BytesWritten );

      for( ULONG i=0; i<BytesToWrite; i++ )
      {
         WriteBuffer[i+3] = Data[i + BytesWritten];
      }

      WriteIic( WriteBuffer, BytesToWrite + 3 );

      //Sleep, because we cannot determine when a write cycle is done
      Sleep( 10 );

      BytesWritten += BytesToWrite;
      Address += (USHORT)BytesToWrite;
   }
   return true;
}

bool CBoard1077Hal::ReadEeprom( USHORT Address, UCHAR *Data, ULONG Length, int EepromName )
{
   UCHAR WriteBuff[4];
   const int MAX_BYTES_TO_TRY = 62;
   UCHAR IicAddress;

   switch( EepromName )
   {
      case EEPROM_1:
         IicAddress = IIC_ADDR_EEPROM_1;
         break;

      case EEPROM_2:
         IicAddress = IIC_ADDR_EEPROM_2;
         break;

      case EEPROM_3:
         IicAddress = IIC_ADDR_EEPROM_3;
         break;
   }

   WriteBuff[0] = IicAddress;
   WriteBuff[1] = (UCHAR)( Address >> 8 );
   WriteBuff[2] = (UCHAR)Address;
   WriteBuff[3] = 0xFF;

   WriteIic( WriteBuff, 3 );

   for( unsigned int i=0; i<Length; i+=MAX_BYTES_TO_TRY )
   {
      ReadIic( IicAddress, Data + i, min( Length - i, MAX_BYTES_TO_TRY ) );
   }

   return true;
}

bool CBoard1077Hal::SaveCalData()
{
   USHORT HeaderSize = sizeof( CalDataHeader );
   UCHAR Data[2];
   bool RetVal;

   CalDataHeader.CalTime = CTime::GetCurrentTime().GetTime();
   CalDataHeader.TableVersion = CALDATA_CURRENT_VERSION;
   CalDataHeader.CalVotage = GetAdcVoltage() / 5;
   CalDataHeader.Reserved1 = 0;
   CalDataHeader.Reserved2 = 0;
   CalDataHeader.Reserved3 = 0;
   CalDataHeader.Reserved4 = 0;
   CalDataHeader.Reserved5 = 0;

   //Turn Off the Write Protect Line.
   RetVal = ReadIic( IIC_ADDR_IO, &Data[1], 1 );

   if( RetVal == false )
   {
      FireCommunicationError( "SaveCalData: Unable to Read from the IoDevice" );
      return FALSE;
   }

   Data[0] = IIC_ADDR_IO;
   Data[1] &= ~IO_EEPROM_WRITE_PROTECT;

   RetVal = WriteIic( Data, 2 );

   if( RetVal == false )
   {
      FireCommunicationError( "SaveCalData: Unable to Write to the IoDevice" );
      return FALSE;
   }
   
   RetVal = WriteEeprom( 0, (UCHAR*)&CalDataHeader, HeaderSize, CAL_EEPROM );

   RetVal = WriteEeprom( HeaderSize, (UCHAR*)&CalData, sizeof( CalData ), CAL_EEPROM );

   //Turn the Write Protect Line back On.
   Data[0] = IIC_ADDR_IO;
   Data[1] |= IO_EEPROM_WRITE_PROTECT;

   RetVal = WriteIic( Data, 2 );

   if( RetVal == false )
   {
      FireCommunicationError( "SaveCalData: Unable to Write to the IoDevice" );
      return FALSE;
   }
   
   return RetVal;
}


bool CBoard1077Hal::LoadCalData()
{
   USHORT HeaderSize = sizeof( CalDataHeader );
   bool RetVal;

   RetVal = ReadEeprom( 0, (UCHAR*)&CalDataHeader, HeaderSize, CAL_EEPROM );

   if( RetVal == false || !( CalDataHeader.TableVersion == CALDATA_CURRENT_VERSION || CalDataHeader.TableVersion == CALDATA_OLD_VERSION_NO_VOLT ) )
   {
      FireCommunicationError("LoadCalData: Unable to Load Calibration Data Header");
      return false;
   }

   if( CalDataHeader.TableVersion == CALDATA_OLD_VERSION_NO_VOLT )
   {
      CalDataHeader.CalVotage = 490;
   }

   RetVal = ReadEeprom( HeaderSize, (UCHAR*)&CalData, sizeof( CalData ), CAL_EEPROM );

   if( RetVal == false )
   {
      FireCommunicationError("LoadCalData: Unable to Load Calibration Data");
      return false;
   }

   return true;
}

bool CBoard1077Hal::SetCalData( long Table, long Index1, long Index2, short Value, short Password )
{
   bool RetVal = true;

   if( Password != 0x7771 )
   {
      return false;
   }

   switch( Table )
   {
      case CAL_LINCOUNTS_NB:
         if( Index1 < 0 || Index1 >= CAL_NUM_LIN_DBMV )
         {
            RetVal = false;
            break;
         }

         CalData.LinCountsNb[Index1] = Value;
         break;

      case CAL_LINCOUNTS_WB:
         if( Index1 < 0 || Index1 >= CAL_NUM_LIN_DBMV )
         {
            RetVal = false;
            break;
         }

         CalData.LinCountsWb[Index1] = Value;
         break;

      case CAL_MAX_AGC_ERR:
         if( Index1 < 0 || Index1 >= CAL_NUM_FREQ )
         {
            RetVal = false;
            break;
         }

         CalData.MaxAgcErr[Index1] = Value;
         break;

      case CAL_ATTEN_ON_ERR:
         if( Index1 < 0 || Index1 >= CAL_NUM_FREQ )
         {
            RetVal = false;
            break;
         }

         CalData.AttenOnErr[Index1] = Value;
         break;

      case CAL_TUNER_AGC:
         if( Index1 < 0 || Index1 >= CAL_NUM_FREQ || Index2 < 0 || Index2 >= CAL_NUM_TUNER_DBMV )
         {
            RetVal = false;
            break;
         }

         CalData.Tuner[Index1].AgcCounts[Index2] = Value;
         break;

      case CAL_TUNER_GAIN:
         if( Index1 < 0 || Index1 >= CAL_NUM_FREQ || Index2 < 0 || Index2 >= CAL_NUM_TUNER_DBMV )
         {
            RetVal = false;
            break;
         }

         CalData.Tuner[Index1].TunerGain[Index2] = Value;
         break;

      case CAL_TEMP:
         CalData.Temp = Value;
         break;
   }

   return RetVal;
}

bool CBoard1077Hal::GetCalTime( __int64 *Time )
{
   *Time = CalDataHeader.CalTime;
   return m_CalDataOk;
}

bool CBoard1077Hal::GetSerialNumber( __int64 *Serial )
{
   DTAPI_RESULT DtResult;
   DtDeviceDesc DtDevDesc;

   DtResult = m_DtDevice.GetDescriptor( DtDevDesc );

   *Serial = DtDevDesc.m_Serial;
   
   return DtResult == DTAPI_OK;
}

bool CBoard1077Hal::GetBoardRevision( char *rev )
{
   DTAPI_RESULT DtResult;

   DtResult = m_DtDevice.VpdRead("EC", rev);

   return DtResult == DTAPI_OK;
}

bool CBoard1077Hal::DongleBoard( LPCTSTR DongleXml )
{
#ifdef ALLOW_DOT_NET
   //Set up Encryption stuff
   UnicodeEncoding *ByteConverter = new UnicodeEncoding();

   System::Int64 iSerial;
   GetSerialNumber( &iSerial );

   System::Byte SerialNumber[] = ByteConverter->GetBytes( iSerial.ToString() );
   System::Byte UnDongle[] = ByteConverter->GetBytes( "0" );
   System::Byte EncrypedData[];
   System::Byte DecrypedData[];
   System::Byte HashCode[] = { 0xA5, 0xAA, 0x5A, 0x55 };
   
   XmlSerializer *Serializer = new XmlSerializer( __typeof( System::Byte[] ) );

   StringReader *Reader = new StringReader( DongleXml );

   EncrypedData = (System::Byte[]) Serializer->Deserialize( Reader );
   DecrypedData = (System::Byte[]) EncrypedData->Clone();

   for( int i=0; i<DecrypedData->Length; i++ )
   {
      DecrypedData[i] ^= HashCode[i % 4];
   }

   bool IsValid = true;

   for( int i=0; i<SerialNumber->Length; i++ )
   {
      if( SerialNumber[i] != DecrypedData[i] )
      {
         IsValid = false;
         break;
      }
   }

   //Check for Un-Dongle
   if( IsValid == false )
   {
      IsValid = true;

      for( int i=0; i<UnDongle->Length; i++ )
      {
         if( UnDongle[i] != DecrypedData[i] )
         {
            IsValid = false;
            break;
         }
      }
   }

   if( IsValid == true )
   {
      UCHAR WriteBuffer[6];
      UCHAR Data[2];
      bool RetVal;

      //Turn Off the Write Protect Line.
      RetVal = ReadIic( IIC_ADDR_IO, &Data[1], 1 );

      if( RetVal = false )
      {
         FireCommunicationError( "DongleBoard: Unable to Read from the IoDevice" );
         return FALSE;
      }

      Data[0] = IIC_ADDR_IO;
      Data[1] &= ~IO_EEPROM_WRITE_PROTECT;

      RetVal = WriteIic( Data, 2 );

      if( RetVal = false )
      {
         FireCommunicationError( "DongleBoard: Unable to Write to the IoDevice" );
         return FALSE;
      }
      
      for( int i=0; i<20; i+=4 )
      {
         WriteBuffer[0] = IIC_ADDR_IO_EE;
         WriteBuffer[1] = i;

         for( ULONG j=0; j<4; j++ )
         {
            WriteBuffer[j+2] = EncrypedData[j + i];
         }

         RetVal = WriteIic( WriteBuffer, 6 );

         if( RetVal == false )
         {
            return false;
         }
      }
      
      //Turn the Write Protect Line back On.
      Data[0] = IIC_ADDR_IO;
      Data[1] |= IO_EEPROM_WRITE_PROTECT;

      RetVal = WriteIic( Data, 2 );

      if( RetVal = false )
      {
         FireCommunicationError( "DongleBoard: Unable to Write to the IoDevice" );
         return FALSE;
      }

      m_NeedToCheckDongle = true;
   }

   return IsValid;
#else
   return false;
#endif
}

bool CBoard1077Hal::IsDongled()
{
#ifdef ALLOW_DOT_NET
   UCHAR WriteBuffer[2];
   UCHAR ReadBuffer[20];

   if( m_NeedToCheckDongle == true )
   {
      WriteBuffer[0] = IIC_ADDR_IO_EE;
      WriteBuffer[1] = 0;
      WriteIic( WriteBuffer, 2 );

      ReadIic( IIC_ADDR_IO_EE, ReadBuffer, 20 );

      //Set up Encryption stuff
      UnicodeEncoding *ByteConverter = new UnicodeEncoding();

      System::Byte DecrypedData[] = new Byte[20];
      System::Byte HashCode[] = { 0xA5, 0xAA, 0x5A, 0x55 };

      for( int i=0; i<20; i++ )
      {
         DecrypedData[i] = ReadBuffer[i] ^ HashCode[i % 4];
      }

      System::Int64 iSerial;
      GetSerialNumber( &iSerial );

      System::Byte SerialNumber[] = ByteConverter->GetBytes( iSerial.ToString() );

      m_Dongled = true;

      for( int i=0; i<SerialNumber->Length; i++ )
      {
         if( SerialNumber[i] != DecrypedData[i] )
         {
            m_Dongled = false;
            break;
         }
      }

      m_NeedToCheckDongle = false;
   }

   return m_Dongled;
#else
   return false;
#endif
}

void CBoard1077Hal::FFT( double *Real, double *Imaginary, long Length, double Direction )
{
   int i, j, k, m, istep;
   double arg;
   double cwR, cwI, ctR, ctI;

   j = 0;
   k = 1;
   for(i=0; i<Length; i++)
   {
      if (i <= j)
      {
         ctR = Real[j];
         ctI = Imaginary[j];
         Real[j] = Real[i];
         Imaginary[j] = Imaginary[i];
         Real[i] = ctR;
         Imaginary[i] = ctI;
      }
      m = Length/2;
      while (j > m-1)
      {
         j = j - m;
         m = m/2;
         if (m < 1)
            break;
      }
      j = j + m;
   }
   do
   {
      istep = 2*k;
      for (m=0; m<k; m++)
      {
         arg = 3.14159265*Direction*m/k;
         cwR = cos(arg);
         cwI = sin(arg);
         for (i=m; i<Length; i+=istep)
         { 
            ctR = (cwR * Real[i+k] - cwI * Imaginary[i+k]);
            ctI = (cwI * Real[i+k] + cwR * Imaginary[i+k]);
            Real[i+k] = Real[i] - ctR;
            Imaginary[i+k] = Imaginary[i] - ctI;
            Real[i] = Real[i] + ctR;
            Imaginary[i] = Imaginary[i] + ctI;
         }
      }
      k = istep;
   }while (k < Length);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

DRXStatus_t CBoard1077Hal::GetLockStatus( pDRXLockStatus_t lockStatus )
{
	return m_MicronasDevice->GetLockStatus( lockStatus );
}

UCHAR CBoard1077Hal::GetChannelBandwidth()
{
   return m_getChannelParams.bandwidth;
}
bool CBoard1077Hal::SetChannelBandwidth( UCHAR mode )
{
   m_channelParams.bandwidth = (DRXBandwidth_t)mode;
   return true;
}

UCHAR CBoard1077Hal::GetFftMode()
{
   return m_getChannelParams.fftmode;
}
bool CBoard1077Hal::SetFftMode( UCHAR mode )
{
   m_channelParams.fftmode = (DRXFftmode_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelGuard()
{
   return m_getChannelParams.guard;
}
bool CBoard1077Hal::SetChannelGuard( UCHAR mode )
{
   m_channelParams.guard = (DRXGuard_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelConstellation()
{
   return m_getChannelParams.constellation;
}
bool CBoard1077Hal::SetChannelConstellation( UCHAR mode )
{
   m_channelParams.constellation = (DRXConstellation_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelCoderate()
{
   return m_getChannelParams.coderate;
}
bool CBoard1077Hal::SetChannelCoderate( UCHAR mode )
{
   m_channelParams.coderate = (DRXCoderate_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelHierarchy()
{
   return m_getChannelParams.hierarchy;
}
bool CBoard1077Hal::SetChannelHierarchy( UCHAR mode )
{
   m_channelParams.hierarchy = (DRXHierarchy_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelMirror()
{
   return m_getChannelParams.mirror;
}
bool CBoard1077Hal::SetChannelMirror( UCHAR mode )
{
   m_channelParams.mirror = (DRXMirror_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelPriority()
{
   return m_getChannelParams.priority;
}
bool CBoard1077Hal::SetChannelPriority( UCHAR mode )
{
   m_channelParams.priority = (DRXPriority_t)mode;
   return true;
}
UCHAR CBoard1077Hal::GetChannelClassification()
{
   return m_getChannelParams.classification;
}
bool CBoard1077Hal::SetChannelClassification( UCHAR mode )
{
   m_channelParams.classification = (DRXClassification_t)mode;
   return true;
}

bool CBoard1077Hal::SetChannelParameters()
{
   DRXStatus_t status;
   status = m_MicronasDevice->SetChannel( &m_channelParams );
   
   if(status == DRX_STS_OK)
      return true;

   return false;     
}

bool CBoard1077Hal::GetChannelParameters()
{
   DRXStatus_t status;
   status = m_MicronasDevice->GetChannel( &m_getChannelParams );
   status = DRX_STS_OK;
   
   
   if(status == DRX_STS_OK)
      return true;

   return false;
}


bool CBoard1077Hal::GetSignalQuality()
{
   DRXStatus_t status;
   status = m_MicronasDevice->GetSignalQuality( &m_getSignalQuality  );
   
   if(status == DRX_STS_OK)
      return true;

   return false;
}

/**< in steps of 0.1 dB */
double CBoard1077Hal::GetMER()
{
   //BG - 11/03/2006
   //Due to low MER concerns, Terry authorized the use of a pulled from the air value in order to fix difference
   //until property testing equipment can be located and used to determine actual value that should be used.
   
   //return (((double)m_getSignalQuality.MER) * (double)0.1) + 4.0;
   
   //BJ - 03/15/2007
   //MER concerns continued even after the above original fix.  A new MER value calculation was created to apply
   //suitable offsets to the value.  Removed the MER correction to maintain cohesiveness between code and applied
   //fix to the .NET dll in project netBoard1077.
   
   return (((double)m_getSignalQuality.MER) * (double)0.1);
}
/**< in steps of 1/scaleFactorBER */
double CBoard1077Hal::GetPreVitberBiBER()
{
   return ((double)m_getSignalQuality.preViterbiBER) / ((double)m_getSignalQuality.scaleFactorBER);
}
/**< in steps of 1/scaleFactorBER */
double CBoard1077Hal::GetPostVitberBiBER()
{
   return ((double)m_getSignalQuality.postViterbiBER) / ((double)m_getSignalQuality.scaleFactorBER);
}
short CBoard1077Hal::GetPacketError()
{
   return m_getSignalQuality.packetError;
}