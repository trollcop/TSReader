#ifndef __ALI_TYPES_H__
#define __ALI_TYPES_H__

/*
 *	common types used in multiple classes
 */

#include "AT_APIDefs.h"
#include "common.h"

#define ATDEV_INFO_MAX_STR_LEN		30
#define BOARDPOS					int

#ifdef _WINDOWS
#include <PSHPACK1.H>
#endif

/** \brief struct with standard version information
*
*	Information block used to hold version information.
*	Version information may be requested from multiple sources, like
*	the API and the driver.
*/
struct ATDV_API_API _ATVERSIONINFO
{
	u8 Mayor;			///< The mayor part of the version number
	u8 Minor;			///< The minor part of the version number
	u8 Micro;			///< The micro part of the version number
	u8 Nano;			///< The nano part of the version number
	u32 temp1;			///< Empty not used
	u32 temp2;			///< Empty not used
	u32 temp3;			///< Empty not used
};

typedef _ATVERSIONINFO ATVERSIONINFO;

/** \brief struct with the device location
*
*	Information block used to hold device location
*	
*/
struct ATDV_API_API _ATDEVICELOCATION
{
	u8 x;				///< Bus number
	u8 y;				///< Device number
	u8 z;				///< Function
	u8 Slot;			///< Slot number, -1 when not available	
};

typedef _ATDEVICELOCATION ATDEVICELOCATION;

#define TYPE_DVB_C_ANNEXA	1
#define TYPE_DVB_C_ANNEXB	2
#define TYPE_DVB_C_ANNEXC	3

/** \brief struct with the device information.
*
*	Information block used to hold version and capabilities of a device.
*/
struct ATDV_API_API _ATDEVICEINFO
{
	u8		DeviceType;								///< DeviceTypeEnum values
	u8		DeviceFirmwareVersion;					///< A version number
	u8		DeviceCanPlay;							///< Indicate the board capability to Play a transport stream from the host
	u8		DeviceCanRecord;						///< Indicate the board capability to record a transport stream to the host
	u8		DeviceCanPlayRecordSimultaneously;		///< Indicate that Play and Record can be done simultaneously
	u8		DeviceHasDVB_S;							///< Indicate that device has dvb-s tuner
	u8		DeviceHasDVB_C;							///< Indicate that device has dvb-c tuner
	u8		DeviceHasDVB_T;							///< Indicate that device has dvb-t tuner
	u8		DeviceHasDVB_S_Mod;						///< Indicate that device has dvb-s modulator
	u8		DeviceHasDVB_C_Mod;						///< Indicate that device has dvb-c modulator
	u8		DeviceHasDVB_T_Mod;						///< Indicate that device has dvb-t modulator
	u32		DriverMaxBlockSize;						///< The driver's maximum block size for play.
	u8		DeviceCanAudio;							///< Indicate the board capability to De-embed Audio
	char	FriendlyName[ATDEV_INFO_MAX_STR_LEN];	///< Descriptive string. without USB or PCI indication.
	char	FPGAFileName[ATDEV_INFO_MAX_STR_LEN];	///< FPGAfilename string without extensions (like _PLAY.hwcfg)
	u8		GetBcdDevice;							///< Get BcdDevice info from usb device
	u8		IsDeviceUsb;							///< TRUE: USB device, FALSE: PCI device
	__int64	iSerialNum;								///< device serial number
	ATDEVICELOCATION	DevLoc;						///< device location (only for PCI)
} ;

typedef _ATDEVICEINFO ATDEVICEINFO;

struct ATDV_API_API _SBoardOpenParams
{
	char*		m_pBoardName;		///< "unfriendly" device name
	BOARDPOS	m_BoardPos;			///< position in the device list
} ;

typedef _SBoardOpenParams SBoardOpenParams;

#ifdef _WINDOWS
#include <POPPACK.H>
#endif

#endif	// __ALI_TYPES_H__
