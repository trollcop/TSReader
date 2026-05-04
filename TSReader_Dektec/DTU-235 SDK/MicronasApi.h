#ifndef __MICRONASAPI_H__
   #define __MICRONASAPI_H__

#include "DTAPI.h"
#include "drx3973d.h"
#include "drx3973d_map.h"
#include "drx3973d_mc.h"
#include "bsp_i2c.h"
#include "bsp_tuner_tables.h"

class CMicronasDevice
{
   public:
      CMicronasDevice( DtDevice *device );
      virtual ~CMicronasDevice();
      // Dektec Device Object
      DtDevice *m_DtDevice;
      // Micronas Device Variables
      I2CDeviceAddr_t demodAddr;
      DRXCommonAttr_t demodCommAttr;   
      DRX3973DData_t demodExtAttr;
      DRXDemodInstance_t demod;      
      pDRXDemodInstance_t demodulators[2];  
      
      BOOL Initialize();
      BOOL Uninitialize();
      BOOL WriteIic( UCHAR *Data, ULONG Length );
      DRXStatus_t GetLockStatus( pDRXLockStatus_t lockStatus );
      DRXStatus_t GetSignalQuality( pDRXSigQuality_t sigQuality );
      DRXStatus_t GetSignalStrength( pu16_t sigStrength );
      DRXStatus_t SetChannel(DRXChannel_t *channelParams);
      DRXStatus_t GetChannel(DRXChannel_t *channelParams);
      DRXStatus_t GetSymbol(DRXComplex_t *symbolPoint);
      DRXStatus_t GetVersions(DRXVersionList_t *versionList);

      void SetDtDevice();
};

#endif