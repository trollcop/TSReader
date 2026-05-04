#include "stdafx.h"
#include "MicronasApi.h"

#define  TUNER_PHILIPS_TD_1316AL_MK2
#include "bsp_tuner_tables.h"

//DtDevice *g_DtDevice;

CMicronasDevice::CMicronasDevice( DtDevice *device )
{
   m_DtDevice = device;
   //g_DtDevice = device;
   
   demodAddr       = DRX3973DDefaultAddr_g;
   demodCommAttr   = DRX3973DDefaultCommAttr_g;
   demod           = DRX3973DDefaultDemod_g;
   demodulators[0] = &demod;
   demodulators[1] = NULL;
}

CMicronasDevice::~CMicronasDevice()
{
}

BOOL CMicronasDevice::Initialize()
{    
   demod.myI2CDevAddr                = &demodAddr;
   demod.myCommonAttr                = &demodCommAttr;
   demod.myExtAttr                   = &demodExtAttr;
   demod.myCommonAttr->microcode     = MC_IMAGE_ADDR;
   demod.myCommonAttr->microcodeSize = MC_IMAGE_SIZE;
   demod.myTuner                     = NULL;

   demodCommAttr.intermediateFreq = 36125;

   if( DRXBSP_I2C_Init() != DRX_STS_OK )
      return FALSE;
   
   if( DRXBSP_HST_Init() != DRX_STS_OK )
      return FALSE;
   
   if( DRX_Init( demodulators ) != DRX_STS_OK )
      return FALSE;
   
   if( DRX_Open( &demod, m_DtDevice ) != DRX_STS_OK )
      return FALSE;
        
   return TRUE;
}

BOOL CMicronasDevice::Uninitialize()
{
   DRX_Close( &demod, m_DtDevice );
   //DRX_Term();
   DRXBSP_HST_Term();
   DRXBSP_I2C_Term();
   
   return TRUE;
}

BOOL CMicronasDevice::WriteIic( UCHAR *Data, ULONG Length )
{
   return TRUE;
}

DRXStatus_t CMicronasDevice::GetLockStatus( pDRXLockStatus_t lockStatus )
{
   return DRX_Ctrl( &demod, DRX_CTRL_LOCK_STATUS, lockStatus, m_DtDevice);
}

DRXStatus_t CMicronasDevice::GetSignalQuality( pDRXSigQuality_t sigQuality )
{
   return DRX_Ctrl( &demod, DRX_CTRL_SIG_QUALITY, sigQuality , m_DtDevice);
}

DRXStatus_t CMicronasDevice::SetChannel(DRXChannel_t *channelParams)
{
   return DRX_Ctrl(&demod, DRX_CTRL_SET_CHANNEL, channelParams, m_DtDevice);
}

DRXStatus_t CMicronasDevice::GetChannel(DRXChannel_t *channelParams)
{
   return DRX_Ctrl(&demod, DRX_CTRL_GET_CHANNEL, channelParams, m_DtDevice);
}

DRXStatus_t CMicronasDevice::GetSymbol(DRXComplex_t *symbolPoint)
{
   return DRX_Ctrl(&demod, DRX_CTRL_CONSTEL, symbolPoint, m_DtDevice);
}

DRXStatus_t CMicronasDevice::GetVersions(DRXVersionList_t *versionList)
{
   return DRX_Ctrl(&demod, DRX_CTRL_VERSION, versionList, m_DtDevice);
}

void CMicronasDevice::SetDtDevice()
{
   //g_DtDevice = m_DtDevice;
}