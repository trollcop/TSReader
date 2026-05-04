// ATBoard.h: interface for the CATBoard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_)
#define AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "common.h"
#include <vector>
#include <frontend.h>
#include <string>

#include "Device.h"
//#include "../../UsbVideo/Software/Shared/LowlevelTypes.h"

#include "ATDV_API.h"
#include "AtRegisters.h"

class CIIRFilter;

typedef u32	TAtBoardId;

#ifdef _WINDOWS
#include <PSHPACK1.H>
#endif

/** DeviceTypeEnum\n
 *	Unified (for USB and PCI) unique device identifier.
 *	\note New boards/devices must be added here and in DeviceTableUSB and DeviceTablePCI.
 */
typedef enum _DeviceTypeEnum
{
	AT200 = 0,
	AT210,
	AT300,
	AT310,
	AT400,
	AT410,
	AT500,
	AT510,
	AT600,
	AT610,
	AT700,
	AT710,
	AT720,
	AT730,
	AT800,
	AT810,
	AT1400,
	AT1600,
	AT1700,
	AT1800,
	AT20,
	AT22,
	AT30,
	AT32,
	AT40,
	AT60,
	AT70,
	AT72,
	AT73,
	AT80,
	AT4,
	ATDEMO = 0xff
} DeviceTypeEnum;

/** FPGAModeEnum\n
 *	Enum representing the Mode of the device.
 */
typedef enum _FPGAModeEnum
{
	PLAY_DVB = 0,
    PLAY_SMPTE,
	REC_DVB,
	REC_SMPTE

} FPGAModeEnum;


/** _ATDEVICEINFO.\n
 *	Information block used to hold version and capabilities of a device.
 */
struct ATDV_API_API _ATDEVICEINFO
{
	u8		DeviceType;							///< DeviceTypeEnum values
	u8		DeviceFirmwareVersion;				///< A version number
	u8		DeviceCanPlay;						///< Indicate the board capability to Play a transport stream from the host
	u8		DeviceCanRecord;					///< Indicate the board capability to record a transport stream to the host
	u8		DeviceCanPlayRecordSimultaneously;	///< Indicate that Play and Record can be done simultaneously
	u8		DeviceHasDVB_S;						///< Indicate that device has dvb-s tuner
	u8		DeviceHasDVB_C;						///< Indicate that device has dvb-c tuner
	u8		DeviceHasDVB_T;						///< Indicate that device has dvb-t tuner
	u32		DriverMaxBlockSize;					///< The driver's maximum block size for play.
	u8		DeviceCanAudio;						///< Indicate the board capability to De-embed Audio
	char	FriendlyName[16];					///< Descriptive string. without USB or PCI indication.
	u8		GetBcdDevice;						///< Get BcdDevice info from usb device
} ;

#ifdef _WINDOWS
#include <POPPACK.H>
#endif

typedef _ATDEVICEINFO ATDEVICEINFO;

class CAtBoardManager;
class CDvbSource;
class CI2cBus;


/**@ingroup public_api*/
/*@{*/

/** CATBoard.\n
 *	Use this class to control an Alitronika Dvb Board.
 *	Each physical board present in the system has only one instance of
 *	this class as managed by the CAtBoardManger.
 *	@see CAtBoardManager.
 *	The internal registers may be accessed through the CAtRegisters class.
 */
class ATDV_API_API CATBoard
{
private:
	CDevice m_Device;

public:
			/** Get the device name in a user presentable form.
			 *	The USB or PCI suffix is NOT added to this string.
			 *	Example: AT400
			 *	@param	pName The address of a string buffer to receive the name.
			 *	@param	nLength The length of the string buffer.
			 */
	virtual void GetFriendlyName(char *pName, u32 &nLength);

			/** Get the unique identification of the board.
			 *	@note Currently this always returns idBoard=0 since it is reserved for future use.
			 *	@param	idBoard = 0
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetBoardId		(TAtBoardId& idBoard);

			/** Get information about the device's capabilities.
			 *	@param pDeviceInfo Pointer to a info structure to be filled.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetDeviceInfo	(ATDEVICEINFO* pDeviceInfo);

			/** Reset the device and clears all pending data.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ResetDevice		();

			/** Read the PID table from the device.\n
			 *	The newly read PID (Program ID) table can be accessed by using CAtRegisters.
			 *	@see CAtRegisters
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ReadPidTable	();

			/** Write the PID table to the device.\n
			 *	To access the PID (Program ID) table use CAtRegisters.
			 *	@see CAtRegisters
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	WritePidTable	();

			/** Get the number of packet synchronization errors since the
			 *	last call to ResetCounters.
			 *	@returns The number of packet synchronization errors.
			 */
	virtual	u32		GetSyncCount	();

			/**	Get the number of data errors since the
			 *	last call to ResetCounters.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	u32		GetErrorCount	();

			/** Resets the sync and error counters.
			 *	@see GetSyncCount()
			 *	@see GetErrorCount()
			 */
	virtual	void	ResetCounters	();

			 /**	Program the FPGA.\n
			 *	The FPA firmware may be updated at any time. Note however
			 *	that no recording or play should be in progress.\n
			 *	@param iFpgaEnum one of PLAY_DVB / PLAY_SMPTE / RECORD_DVB / RECORD_SMPTE
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual BOOL	ProgramFPGA(FPGAModeEnum iFpgaEnum);

			 /**	Program the FPGA.\n
			 *	The FPA firmware may be updated at any time. Note however
			 *	that no recording or play should be in progress.\n
			 *	The firmware file is the Altera .rbf (Raw Binary File).
			 *	@param pFileName Full path, name and extension of the FPGA firmware file.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ProgramFPGAFirmware(const char* pFileName);

			/** Get the current Firmware file name.\n
			 *	Initially the string is empty, but the default file
			 *	for the board has been loaded.
			 *	@note Do not forget to write the (current) register set after a
			 *	new firmware has been loaded.
			 *	@returns std::string to the name (const)
			 */
	virtual	const std::string & CATBoard::GetFPGAFirmwareName() const;

			/** Start recording (from the selected source?)
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StartRecording	();
			
			/** Stop a previously started recording.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StopRecording	();

			/** Get a block of recorded data.\n
			 *	Data may be taken from the driver
			 *	in large blocks, to improve efficiency.\n
			 *	This function is obsolete, use \ref GetRecordPacketDirect for better performance.
			 *	
			 *	@param pData Pointer to a memory location to put the data.
			 *	@param nSizeIn Max. size of bytes that pData can contain.
			 *	@param pSizeOut The number of bytes copied to pData.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetRecordPacket	(u8* pData,  u32 nSizeIn, u32* pSizeOut);
			
			/** Get Record Packet Direct.\n
			 *	Data may be taken from the driver
			 *	in large blocks, to improve efficiency.
			 *	Data is not copied to the driver, but the buffer is passed to it
			 *	and should not change.\n
			 *	The memory pointed to by pData will remain locked until the device driver has
			 *	finished processing it.
			 *	@param pData Pointer to a memory location to put the data.
			 *	@param nSizeIn Max. size of bytes that pData can contain.
			 *	@param pSizeOut The number of bytes copied to pData.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetRecordPacketDirect(u8* pData,  u32 nSizeIn, u32* pSizeOut);
			
			/** Start Playing.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StartPlaying	();

			/** Stop Playing.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StopPlaying		();

			/** Send a block of data.\n
			 *	Sends a block of data to the board. Data may be taken from the driver
			 *	in large blocks, to improve efficiency.\n
			 *	This function is obsolete, use \ref SendPlayPacketDirect for better performance.
			 *	
			 *	@param pData Packet data.
			 *	@param nSize Size of pData in bytes.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	SendPlayPacket	(u8* pData, u32 nSize);
			
			/** Send a block of data.\n
			 *	Sends a block of data to the board. Data may be taken from the driver
			 *	in large blocks, to improve efficiency.
			 *	Data is not copied to the driver, but the buffer is passed to it
			 *	and should not change.\n
			 *	The memory pointed to by pData will remain locked until the device driver has
			 *	finished processing it.
			 *	@param pData Packet data.
			 *	@param nSize Size of pData in bytes.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	SendPlayPacketDirect(u8* pData, u32 nSize);
			
			/** Send a command to the DVB source device.\n
			 *	A DVB board may have its own source, e.g. a tuner.
			 *	With this method one can send commands and set and retrieve information
			 *	to or from this device.\n
			 *	The type of source determines which commands are possible.
			 *	For the available commands see \ref dvbsource_cmd.\n
			 *	The function only applies to AT600, AT700 and AT800 boards.
			 *	
			 *	@param	nCmd Command code to send to the device.
			 *	@param	pArgs A pointer to the parameters accompanying the given command.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	DvbSource(u32 nCmd, void* pArgs);

			/**
			 *	Read the registers from the board's FPGA to be available
			 *	through CAtRegisters.
			 */
	virtual	void	GetRegisters();
	
			/**
			 *	Write any changed registers to the board's FPGA.
			 *	@param bForceWrite When TRUE, the all current register values are written. When FALSE (or not specified), changed registers are written.
			 */
	virtual	void	UpdateRegisters(BOOL bForceWrite = FALSE);

			/**
			 *	Initialize the filtering of the GetInputBitrate() to the interval
			 *	in milliseconds, the GetInputBitrate() is called periodically.
			 *	The interval may be between 50 and 200ms. Longer interval times may
			 *	make the filter instable.
			 *	\param UpdateIntervalMS	interval in milliseconds (max 200ms).
			 */
	virtual void	InitBitrateFilter(u32 UpdateIntervalMS);

			/**
			 *	Reads the current Packet Pulse count (the register must have been read)
			 *	and computes the bitrate, using a filter to smooth out variations.
			 *	This function must be called periodically, as initialized with
			 *	InitBitrateFilter().\n
			 *	The filter is a 8th order Bessel filter with 0.5 Hz roll-off.
			 *	\see InitBitrateFilter
			 *	\return The bitrate in bits/second.
			 */
	virtual u32		GetInputBitrate();

			/**
			 *	Shows wether the board is a USB or a PCI version.
			 *	\return TRUE for USB.
			 */
	virtual BOOL	IsUsbDevice();
	
			/**
			 *	Shows whether the USB device is high speed or not.
			 *	It is adviced to check the speed of the USB before initiating any
			 *	record or play function, since it can lead to buffer over/under flow.
			 *	\return TRUE for HIGH SPEED.
			 *	\note For PCI devices, this call always returns TRUE.
			 *	\version 1.0b (april 2005) and later.
			 */
	virtual BOOL	IsUsbDeviceHighSpeed();

			/**
			 *	Shows the USB BcdDevice value.
			 *	\return -1 when error, else BcdDevice value.
			 *	\note For PCI devices, this call always returns -1.
			 */
	int GetBcdDevice();

			/**
			 *	Shows the Status of the RX and TX fifo`s.
			 *	@param pData Pointer to a memory location of the data.
			 *	@param pData + 0 = Record fifo size (bytes)
			 *	@param pData + 1 = Record data in fifo (bytes)
			 *	@param pData + 2 = Play fifo size (bytes)
			 *	@param pData + 3 = Play data in fifo (bytes)
			 *	@param nSizeIn Max. size of bytes that pData can contain.
			 *	@param pSizeOut The number of bytes copied to pData.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	BOOL GetFifoStatus(u32* pData, u32 nSizeIn, u32 *pSizeOut);

	HANDLE GetDeviceHandle()	{ return m_Device.m_hDevice;	}

			/**
			 *	Mark the device as Inaccessible (or as Accessible)
			 *	This may be used for USB devices that are unplugged.
			 *	This may also change when a ProgramFPGA fails.
			 *	@returns 'true' --> Accessible
			 */
	BOOL GetDeviceEnable () { return m_bDeviceEnable; }

			/**
			 *	Mark the device as Accessible (or as Inaccessible)
			 *	This may be used for USB devices that are unplugged.
			 *	@param bFlg 'true' --> Accessible
			 */
	void SetDeviceEnable (BOOL bFlg = true) { m_bDeviceEnable = bFlg; }


protected:
			//Con-/destructor only available for: CAtBoardManager.
					CATBoard		(char* pDevName);
	virtual			~CATBoard		();

			BOOL	SetFpgaRegStartAddr(u8 nAddress);
			BOOL	SetFpgaRegs	(void *pData, u8 nStartAddr, u8 nSize);
			BOOL	GetFpgaRegs	(void *pData, u8 nStartAddr, u8 nSize);
			
			//Reference stuff made protected as it should be done ONLY via CAtBoardManager.
			/** @internal Release an instance reference.
			 *	@see AddRef()
			 *	@returns the number of references left.
			 */
			s32		Release(); //release a reference to the instance
			
			/** @internal Increase the current reference count of this instance.
			 *	@see Release()
			 */
			void	AddRef(); //add a reference to the instance.

#ifdef WIN32
#   pragma warning(push)
#   pragma warning(disable : 4251)
#endif // WIN32
protected:
	TSysMutex  m_mutexDevice;
	TSysMutex  m_FpgaMutex;
	ATREGISTRY m_iRegisters;		///< ACCESS THIS STRUCTURE ONLY VIA CAtRegisters !!!
	ATREGISTRY m_iRegistersBackup;	///< backup of m_iRegisters.
	CI2cBus		*m_pI2cBus;				
	CDvbSource  *m_pDvbSource;
	char		*m_pDeviceName;		
	std::string m_sFriendlyName;
	std::string m_FirmwareFname;	///< Fully qualified name for the firmware file.
	TSLONG		m_nRefCount;		///< Reference count of the instance
	u32			m_SyncCountOffset;	///< The 'Tare' value of the Sync counter
	u32			m_ErrCountOffset;	///< The 'Tare' value of the Error counter

	u32			m_BitrateUpdateInterval;	///< in milliseconds
	CIIRFilter	*m_pBitrateFlt;		///< Bitrate filter
	double		m_AvgPckPulseCnt;	///< Filtered Packet Pulse count
	BOOL		m_bDeviceEnable;	///< Indication that the device is not accessible. Further access is now disabled.
	dvb_frontend_parameters	m_frontparms;	///< Local restore buffer.

	//who are my friends??
	friend class CAtRegisters;
	friend class CAtBoardManager;
	friend class CATi2c;
#ifdef WIN32
public:
	static HWND s_hWnd;				///< Handle of Parent window for ::MessageBox error messages (may be NULL)
#   pragma warning(pop)
#endif // WIN32

};
/*@}*/

#ifdef WIN32
#   pragma warning(default : 4251)
#endif // WIN32


#endif // !defined(AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_)

