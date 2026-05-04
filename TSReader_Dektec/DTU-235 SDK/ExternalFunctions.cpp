#include "stdafx.h"
#include "Board1077.h"

extern CBoard1077App theApp;

bool CheckCard( int CardIndex )
{
   if( CardIndex < 0 || CardIndex >= MAX_CARDS || theApp.pBoard1077Hal[CardIndex] == NULL )
   {
      return false;
   }
   else
   {
      return true;
   }
}

BOOL WINAPI B1077Uninitialize( int CardIndex )
{
   BOOL RetVal;

   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   RetVal = theApp.pBoard1077Hal[CardIndex]->Uninitialize();

   delete theApp.pBoard1077Hal[CardIndex];

   theApp.pBoard1077Hal[CardIndex] = NULL;

   return RetVal;
}

BOOL WINAPI B1077Initialize( int CardIndex )
{
   BOOL RetVal = FALSE;

   if( theApp.pBoard1077Hal[CardIndex] == NULL && theApp.SerialNumber[CardIndex] != 0 )
   {
      theApp.pBoard1077Hal[CardIndex] = new CBoard1077Hal;

      if( theApp.pBoard1077Hal[CardIndex] != NULL )
      {
         RetVal = theApp.pBoard1077Hal[CardIndex]->Initialize( theApp.SerialNumber[CardIndex] );
      }
   }
   return RetVal;
}

BOOL WINAPI B1077DoesExist( int CardIndex )
{
   BOOL RetVal = FALSE;
   int DeviceCount = 0;
   DtDeviceDesc DvcDescArr[64];
   bool SerialInList;
   __int64 NewSerialNumber[MAX_CARDS] = {0};

   DtapiDeviceScan( 64, DeviceCount, DvcDescArr );

   //First, Add the existing ones to the new list.
   for( int i=0; i<DeviceCount; i++ )
   {
      if( DvcDescArr[i].m_TypeNumber == 235 )
      {
         for( int j=0; j<MAX_CARDS; j++ )
         {
            if( theApp.SerialNumber[j] == DvcDescArr[i].m_Serial )
            {
               NewSerialNumber[j] = DvcDescArr[i].m_Serial;
               break;
            }
         }
      }
   }

   //Second, Add the new ones to the new list.
   for( int i=0; i<DeviceCount; i++ )
   {
      if( DvcDescArr[i].m_TypeNumber == 235 )
      {
         SerialInList = false;
         
         //Look to see if the serial number is in the list
         for( int j=0; j<MAX_CARDS; j++ )
         {
            if( NewSerialNumber[j] == DvcDescArr[i].m_Serial )
            {
               SerialInList = true;
               break;
            }
         }

         //Add it if it is not.
         if( SerialInList == false )
         {
            for( int j=0; j<MAX_CARDS; j++ )
            {
               if( NewSerialNumber[j] == 0 )
               {
                  NewSerialNumber[j] = DvcDescArr[i].m_Serial;
                  break;
               }
            }
         }
      }
   }
   
   for( int j=0; j<MAX_CARDS; j++ )
   {
      theApp.SerialNumber[j] = NewSerialNumber[j];
   }


   if( theApp.SerialNumber[CardIndex] != 0 )
   {
      RetVal = TRUE;
   }
   return RetVal;
}

long WINAPI B1077GetTunerFrequencyHz( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }

   return theApp.pBoard1077Hal[CardIndex]->GetTunerFrequencyHz();
}

void WINAPI B1077SetTunerFrequencyHz( int CardIndex, long Frequency )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetTunerFrequencyHz( Frequency );
}

short WINAPI B1077GetAdcNbRssi( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetAdcNbRssi();
}

short WINAPI B1077GetAdcWbRssi( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetAdcWbRssi();
}

short WINAPI B1077GetAdcTunerAGC( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetAdcTunerAGC();
}

short WINAPI B1077GetAdcFineAGC( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetAdcFineAGC();
}

short WINAPI B1077GetAdcTemperature( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetAdcTemperature();
}

BOOL WINAPI B1077GetIoRfAttenuation( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetIoRfAttenuation();
}

void WINAPI B1077SetIoRfAttenuation( int CardIndex, BOOL RfAttenuation )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetIoRfAttenuation( RfAttenuation );
}

BOOL WINAPI B1077DemodGetSymbols( int CardIndex, short FAR* SymbolsArray, short SymbolsArrayLength )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->DemodGetSymbols( SymbolsArray, SymbolsArrayLength );
}

BOOL WINAPI B1077DacSetAGC( int CardIndex, short Level )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->DacSetAGC( Level );
}

BOOL WINAPI B1077ExecuteRecord( int CardIndex, LPCTSTR strFilename, long lngPackets )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->ExecuteRecord( strFilename, lngPackets );
}

void WINAPI B1077ExecuteStop( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->ExecuteStop();
}

long WINAPI B1077FlashRead( int CardIndex, long Address, short FAR* Data, long Length )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return 0;
}

long WINAPI B1077FlashWrite( int CardIndex, long Address, short FAR* Data, long Length )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return 0;
}

void WINAPI B1077FlashErase( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
}

void WINAPI B1077Versions( int CardIndex, long FAR* Dll, long FAR* Driver, long FAR* Firmware )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }

   theApp.pBoard1077Hal[CardIndex]->Versions( Driver, Firmware );
   //*Dll = 0x01000000;  // 1.0.0.0
   //*Dll = 0x01000001;  // 1.0.0.1
   //*Dll = 0x01000100;  // 1.0.1.0
   *Dll = 0x01010000;  // 1.1.0.0
}

void WINAPI B1077RegisterCommunicationErrorHandle( int CardIndex, COMMUNICATION_ERROR_CALLBACK registerHandle )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->RegisterCommunicationErrorHandle( registerHandle );
}

void WINAPI B1077RegisterRecordProgressHandle( int CardIndex, RECORD_PROGRESS_CALLBACK registerHandle )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->RegisterRecordProgressHandle( registerHandle );
}

void WINAPI B1077RegisterRecordStatusHandle( int CardIndex, RECORD_STATUS_CALLBACK registerHandle )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->RegisterRecordStatusHandle( registerHandle );
}

double WINAPI B1077GetChannelLevel( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0.0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelLevel();
}

double WINAPI B1077GetNbLevel( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 0.0;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetNbLevel();
}

BOOL WINAPI B1077GetCalTime( int CardIndex, __int64 FAR* Time )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetCalTime( Time );
}

BOOL WINAPI B1077GetSerialNumber( int CardIndex, __int64 FAR* Serial )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetSerialNumber( Serial );
}

BOOL WINAPI B1077GetBoardRevision( int CardIndex, char FAR* revision )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetBoardRevision(revision);
}

BOOL WINAPI B1077DongleBoard( int CardIndex, LPCTSTR DongleXml )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->DongleBoard( DongleXml );
}

BOOL WINAPI B1077IsDongled( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->IsDongled();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

int WINAPI B1077GetLockStatus( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   
   DRXLockStatus_t lockStatus;
   DRXStatus_t status = theApp.pBoard1077Hal[CardIndex]->GetLockStatus( &lockStatus );

   if( lockStatus == DRX_LOCKED )
   {
      return 2; //Full MPEG Lock
   }
   else if( lockStatus == DRX3973D_FEC_LOCK )
   {
      return 1; //FEC Lock (Partial Lock)
   }
   else// if( lockStatus == DRX_NEVER_LOCK || lockStatus == DRX_NOT_LOCKED )
   {
      return 0; //No lock at all
   }
}

UCHAR B1077GetChannelBandwidth( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelBandwidth();
}
void B1077SetChannelBandwidth( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelBandwidth(mode);   
}
UCHAR B1077GetFftMode( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetFftMode();
}
void B1077SetFftMode( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetFftMode(mode);   
}
UCHAR B1077GetChannelGuard( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelGuard();
}
void B1077SetChannelGuard( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelGuard(mode);
}
UCHAR B1077GetChannelConstellation( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelConstellation();
}
void B1077SetChannelConstellation( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelConstellation(mode);
}
UCHAR B1077GetChannelCoderate( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelCoderate();
}
void B1077SetChannelCoderate( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelCoderate(mode);
}
UCHAR B1077GetChannelHierarchy( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelHierarchy();
}
void B1077SetChannelHierarchy( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelHierarchy(mode);
}
UCHAR B1077GetChannelMirror( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelMirror();
}
void B1077SetChannelMirror( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelMirror(mode);
}
UCHAR B1077GetChannelPriority( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelPriority();
}
void B1077SetChannelPriority( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelPriority(mode);
}
UCHAR B1077GetChannelClassification( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return 255;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelClassification();
}
void B1077SetChannelClassification( int CardIndex, UCHAR mode )
{
   if( CheckCard( CardIndex ) == false )
   {
      return;
   }
   theApp.pBoard1077Hal[CardIndex]->SetChannelClassification(mode);
}


BOOL WINAPI B1077SetChannelParameters( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->SetChannelParameters();
}

BOOL WINAPI B1077GetChannelParameters( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetChannelParameters();
}

BOOL WINAPI B1077GetSignalQuality( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetSignalQuality();
}
double WINAPI B1077GetMER( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetMER();
}
double WINAPI B1077GetPreVitberBiBER( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetPreVitberBiBER();
}
double WINAPI B1077GetPostVitberBiBER( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetPostVitberBiBER();
}

short WINAPI B1077GetPacketError( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }
   return theApp.pBoard1077Hal[CardIndex]->GetPacketError();
}

BOOL WINAPI B1077GetSymbols( int CardIndex, short FAR* SymbolsArray, short SymbolsArrayLength )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->DemodGetSymbols( SymbolsArray, SymbolsArrayLength );
}
BOOL WINAPI B1077GetIoExtAGC( int CardIndex)
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->GetIoExtAGC();
}
BOOL WINAPI B1077SetIoExtAGC( int CardIndex, BOOL bNewValue)
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->SetIoExtAGC(bNewValue);
}
BOOL WINAPI B1077SetCalData( int CardIndex, long Table, long Index1, long Index2, short Value, short Password)
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->SetCalData(Table, Index1, Index2, Value, Password);
}
BOOL WINAPI B1077SaveCalData( int CardIndex)
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->SaveCalData();
}
short WINAPI B1077GetAdcVoltage( int CardIndex )
{
   if( CheckCard( CardIndex ) == false )
   {
      return FALSE;
   }

   return theApp.pBoard1077Hal[CardIndex]->GetAdcVoltage();
}
