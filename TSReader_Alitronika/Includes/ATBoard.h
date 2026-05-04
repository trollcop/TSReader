// ATBoard.h: interface for the CATBoard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_)
#define AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef _LINUX
#pragma warning (push)
#pragma warning (disable : 4231 4251 4275)
#endif

#include "common.h"
#include <vector>
#include <frontend.h>
#include <string>

#include "Device.h"

#include "ATDV_API.h"
#include "AtRegisters.h"
#include "DVBSource_Error.h"
#include "types.h"

class CIIRFilter;

typedef u32	TAtBoardId;

#ifdef _WINDOWS
#include <PSHPACK1.H>
#endif

#define MAX_PARBITRATE	108000000L
#define MAX_ASIBITRATE	214000000L

//#define _DEBUG_AT2800

/** DeviceTypeEnum\n
 *	Unified (for USB and PCI) unique device identifier.
 *	\note New boards/devices must be added here and in DeviceTableUSB and DeviceTablePCI.
 */
typedef enum _DeviceTypeEnum
{
	AT200 = 0,
	AT300,
	AT400,
	AT500,
	AT600,
	AT700,
	AT720,
	AT730,
	AT800,
	AT820,
	AT20,
	AT30,
	AT40,
	AT40X,
	AT60,
	AT70,
	AT72,
	AT73,
	AT80,
	AT82,
	AT4,
	AT2600,
	AT2700,
	AT2800,
	AT2900,
	AT49,		// BDA device not supported in this API
	AT69,		// BDA device not supported in this API
	AT79,		// BDA device not supported in this API
	AT89,		// BDA device not supported in this API	
	AT8,		
	AT30R1,
	AT40R1,
	AT8C,		// BDA device not supported in this API
	AT1800,		// 800 stand alone
	AT3800,		// AT2800 stand alone
	AT2780,
	AT4R1,
	AT70X,
	AT72X,
	AT80X,
	AT700R1,
	AT720R1,
	AT800R1,
	AT1800R1,
	ATDEMO = 0xff
} DeviceTypeEnum;

/** FPGAModeEnum\n
 *	Enum representing the Mode of the device.
 */
typedef enum _FPGAModeEnum
{
	PLAY_DVB = 0,							///< select DVB play FPGA file
	PLAY_SMPTE,								///< select SMPTE play FPGA file
	PLAY_RAW,								///< select RAW play FPGA file
	REC_DVB,								///< select DVB record FPGA file
	REC_SMPTE,								///< select SMPTE record FPGA file
	REC_RAW,								///< select RAW record FPGA file
	MODULATOR,								///< select DVB play and record modulator FPGA file (at2600, at2700, at2800, at2780 and at2900)
	MODULATOR_CARRIER,						///< ONLY FOR TEST PURPOSE!! select DVB play and record modulator carrier FPGA file (at2600, at2700, at2800, at2780 and at2900)
	FPGA_MODE_ENUM_SIZE						///< sizeof enum
} FPGAModeEnum;


#ifdef _WINDOWS
#include <POPPACK.H>
#endif

class CAtBoardManager;
class CDvbSource;
class CI2cBus;
class CModulatorInterface;
class CSigQuality;

/**@ingroup public_api*/
/*@{*/
 
/**
 *	\brief The control interface for an AT-device.
 *
 *	Use this class to control an Alitronika Dvb Board.
 *	Each physical board present in the system has only one instance of
 *	this class as managed by the CAtBoardManger.
 *
 *	@see CAtBoardManager.
 *	The internal registers may be accessed through the CAtRegisters class.
 */
class ATDV_API_API CATBoard
{
private:
	CDevice m_Device;

public:
			/** \brief Get the device name in a user presentable form.
			 *
			 *	Get the device name in a user presentable form.
			 *	The USB or PCI suffix is NOT added to this string.
			 *	Example: AT400
			 *	@param	pName The address of a string buffer to receive the name.
			 *	@param	nLength The length of the string buffer.
			 */
	virtual void GetFriendlyName(char *pName, u32 &nLength);

			/** \brief Get the fpga filename in a user presentable form.
			 *
			 *	Get the fpga filename in a user presentable form.
			 *	This name can be different from the device name
			 *	The USB or PCI suffix is NOT added to this string.
			 *	Example: AT400
			 *	@param	pName The address of a string buffer to receive the name.
			 *	@param	nLength The length of the string buffer.
			 */
	virtual	void GetFpgaFileName(char *pName, u32 &nLength);

			 /** \brief Get the unique identification of the board.
			 *
			 *	Get the unique identification of the board.
			 *	@note Currently this always returns idBoard=0 since it is reserved for future use.
			 *	@param	idBoard = 0
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetBoardId		(TAtBoardId& idBoard);

			/** \brief Get information about the device's capabilities.
			 *
			 *	Get information about the device's capabilities.
			 *	@param pDeviceInfo Pointer to a info structure to be filled.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	GetDeviceInfo	(ATDEVICEINFO* pDeviceInfo);

			/** \brief Reset the device and clears all pending data.
			 *
			 *	Reset the device and clears all pending data.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ResetDevice		();

			/** \brief Read the PID table from the device.
			 *
			 *	Read the PID table from the device.\n
			 *	The newly read PID (Program ID) table can be accessed by using CAtRegisters.
			 *	@see CAtRegisters
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ReadPidTable	(u32 *& PidList, int & Size);

			/** \brief Write the PID table to the device.
			 *
			 *	Write the PID table to the device.
			 *	To access the PID (Program ID) table use CAtRegisters.
			 *	@see CAtRegisters
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	WritePidTable	(u32 * PidList, UINT Size);

			/** \brief Clear the PID table in the device.
			 *
			 *	Clear the PID table in the device.
			 *	To access the PID (Program ID) table use CAtRegisters.
			 *	@see CAtRegisters
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual BOOL	ClearPidTable	();

			/** \brief Get the number of packet synchronization errors
			 *
			 *	Get the number of packet synchronization errors since the
			 *	last call to ResetCounters.
			 *	@returns The number of packet synchronization errors.
			 */
	virtual	u32		GetSyncCount	();

			/**	\brief Get the number of data errors
			 *
			 *	Get the number of data errors since the
			 *	last call to ResetCounters.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	u32		GetErrorCount	();

			/** \brief Resets the sync and error counters.
			 *
			 *	Resets the sync and error counters.
			 *	@see GetSyncCount()
			 *	@see GetErrorCount()
			 */
	virtual	void	ResetCounters	();

			/** \brief Check if the currently programmed FPGA file, is up to date
			*
			*	Check if the currently programmed FPGA file, is up to date
			*	@param iFpgaEnum one of PLAY_DVB / PLAY_SMPTE / RECORD_DVB / RECORD_SMPTE
			*	@returns TRUE on success, FALSE on failure.
			*/
	virtual BOOL IsFPGAUpToDate(FPGAModeEnum iFpgaEnum);

			 /** \brief Program the FPGA with standard configuration.
			 *
			 *	The FPA firmware may be updated at any time. Note however
			 *	that no recording or play should be in progress.\n
			 *	@param iFpgaEnum one of PLAY_DVB / PLAY_SMPTE / RECORD_DVB / RECORD_SMPTE
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual BOOL	ProgramFPGA(FPGAModeEnum iFpgaEnum);

			 /** \brief Program the FPGA with a specific configuration.
			 *
			 *	The FPA firmware may be updated at any time. Note however
			 *	that no recording or play should be in progress.\n
			 *	The firmware file is the Altera .rbf (Raw Binary File).
			 *	@param pFileName Full path, name and extension of the FPGA firmware file.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	ProgramFPGAFirmware(const char* pFileName);

			/** \brief Get the current Firmware file name.
			 *
			 *	Get the current Firmware file name.\n
			 *	Initially the string is empty, but the default file
			 *	for the board has been loaded.
			 *	@note Do not forget to write the (current) register set after a
			 *	new firmware has been loaded.
			 *	@returns std::string to the name (const)
			 */
	virtual	const std::string & GetFPGAFirmwareName() const;

			/** \brief Start recording
			 *
			 *	Start recording (from the selected source).
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StartRecording	();
			
			/** \brief Stop a previously started recording.
			 *
			 *	Stop a previously started recording.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StopRecording	();

			/** \brief Get a block of recorded data.
			 *
			 *	Get a block of recorded data.\n
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
			
			/** \brief Get Record Packet Direct.
			 *
			 *	Get Record Packet Direct.\n
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
			
			/** \brief Start Playing.
			 *
			 *	Start Playing.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StartPlaying	();

			/** \brief Stop Playing.
			 *
			 *	Stop Playing.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	StopPlaying		();

			/** \brief Send a block of data.
			 *
			 *	Send a block of data.\n
			 *	Sends a block of data to the board. Data may be taken from the driver
			 *	in large blocks, to improve efficiency.\n
			 *	This function is obsolete, use \ref SendPlayPacketDirect for better performance.
			 *	
			 *	@param pData Packet data.
			 *	@param nSize Size of pData in bytes.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	SendPlayPacket	(u8* pData, u32 nSize);
			
			/** \brief Send a block of data.
			 *
			 *	Send a block of data.\n
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
			
			/** \brief Send a command to the DVB source device.
			 *
			 *	Send a command to the DVB source device.\n
			 *	A DVB board may have its own source, e.g. a tuner or modulator.
			 *	With this method one can send commands and set and retrieve information
			 *	to or from this device.\n
			 *	The type of source determines which commands are possible.
			 *	For the available commands see \ref dvbsource_cmd.\n
			 *	The function only applies to AT600, AT700, AT720, AT730, AT800 receiver boards and 
			 *	the AT2600, AT2700, AT2800, AT2780 and the AT3800 modulator boards.
			 *	See the Examples on how to control them.
			 *	
			 *	@param	nCmd Command code to send to the device.
			 *	@param	pArgs A pointer to the parameters accompanying the given command.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual	BOOL	DvbSource(u32 nCmd, void* pArgs);

			/** \brief Read the registers from the board
			 *
			 *	Read the registers from the board's FPGA to be available
			 *	through CAtRegisters.
			 */
	virtual	void	GetRegisters();
	
			/** \brief Write any changed registers
			 *
			 *	Write any changed registers to the board's FPGA.
			 *	@param bForceWrite When TRUE, the all current register values are written. When FALSE (or not specified), changed registers are written.
			 */
	virtual	void	UpdateRegisters(BOOL bForceWrite = FALSE);

			/** \brief Initialize the stability filter of the bitrate
			 *
			 *	Initialize the filtering of the GetInputBitrate() to the interval
			 *	in milliseconds. This tells the API that the application will call the GetInputBitrate()
			 *	periodically with an the specified time interval. The application must call this from
			 *	a timer, with a value between 50 and 200ms.\n
			 *	The 'UpdateIntervalMS' parameter is used for the digital smoothing filter to create the
			 *	correct smoothing effect.
			 *	\see GetInputBitrate
			 *	\param UpdateIntervalMS	interval in milliseconds (max 200ms).
			 */
	virtual void	InitBitrateFilter(u32 UpdateIntervalMS);

			/** \brief Return the stabilized bitrate number
			 *
			 *	Computes the current bitrate from the current register values
			 *	(the register must have been read).\n
			 *	A digital filter is used to smooth out variations.
			 *	This function must be called periodically, as initialized with
			 *	InitBitrateFilter().\n
			 *	The filter is a 8th order Bessel filter with 0.5 Hz roll-off.\n
			 *	If the bitrate can not be read (no signal or error), 0 is returned.\n
			 *	If you do not want a filtered/stabilized bitrate indication, or can not
			 *	set a timer for a fixed interval, use \ref GetRawInputBitrate instead.
			 *	\see InitBitrateFilter
			 *	\return The bitrate in bits/second or zero.
			 */
	virtual u32		GetInputBitrate();

			/** \brief Return the un-filtered bitrate number
			 *
			 *	Computes the current bitrate from the current register values
			 *	(the register must have been read).\n
			 *	If the bitrate can not be read (no signal or error), 0 is returned.
			 *	\return The bitrate in bits/second or zero.
			 */
	virtual u32		GetRawInputBitrate();

			/** \brief See if it is a USB device
			 *
			 *	Shows whether the board is a USB or a PCI version.
			 *	\return TRUE for USB.
			 */
	virtual BOOL	IsUsbDevice();
	
			/** \brief See if it is a Hish-Speed (480Mb/s) USB device
			 *
			 *	Shows whether the USB device is high speed or not.
			 *	It is adviced to check the speed of the USB before initiating any
			 *	record or play function, since it can lead to buffer over/under flow.
			 *	\return TRUE for HIGH SPEED.
			 *	\note For PCI devices, this call always returns TRUE.
			 *	\version 1.0b (april 2005) and later.
			 */
	virtual BOOL	IsUsbDeviceHighSpeed();

			/** \brief Shows the USB BcdDevice value.
			 *
			 *	Shows the USB BcdDevice value.
			 *	\return -1 when error, else BcdDevice value.
			 *	\note For PCI devices, this call always returns -1.
			 */
	virtual int	GetBcdDevice();

			/** \brief Shows the Send/Receive Fifo Status
			 *
			 *	Shows the Status of the RX and TX fifo`s.
			 *	@param pData Pointer to a memory location of the data.
			 *	@param pData + 0 = Record fifo size (bytes)
			 *	@param pData + 1 = Record data in fifo content (bytes)
			 *	@param pData + 2 = Play fifo size (bytes)
			 *	@param pData + 3 = Play data in fifo content (bytes)
			 *	@param pData + 4 = Transfer buffer size (bytes)
			 *	@param pData + 5 = Transfer buffer content (bytes)
			 *	@param nSizeIn Max. size of bytes that pData can contain.
			 *	@param pSizeOut The number of bytes copied to pData.
			 *	@returns TRUE on success, FALSE on failure.
			 *
			 *	Example :
			 *
			 *	u32 Data[6];
			 *	u32 nSizeIn = 6*sizeof(u32);
			 *	u32 SizeOut;
			 *	double dValue[3];
			 *
			 *	if(TheATBoard.GetFifoStatus(Data, nSizeIn, &SizeOut))
			 *	{
			 *		dValue[0] = ((double)Data[1]/(double)Data[0]) * 100;    // = Record data in fifo content / Record fifo size
			 *		dValue[1] = ((double)Data[3]/(double)Data[2]) * 100;    // = Record data in fifo content / Record fifo size
			 *		dValue[2] = ((double)Data[5]/(double)Data[4]) * 100;    // = Transfer buffer content / Transfer buffer size
			 *	}
			 *
			 *
			 */
	virtual BOOL GetFifoStatus(u32* pData, u32 nSizeIn, u32 *pSizeOut);

			/** \brief Clears the TX fifo`s in the driver.
			 *
			 *	Clears the TX fifo`s in the driver.
			 *	@returns TRUE on success, FALSE on failure.
			 */
	virtual BOOL ResetPlayFifo(void);


			/** \brief See if the device is accessible
			 *
			 *	See if the device is Inaccessible (or as Accessible)
			 *	This may be used for USB devices that are unplugged.
			 *	This may also change when a ProgramFPGA fails.
			 *	@returns 'true' --> Accessible
			 */
	virtual BOOL GetDeviceEnable ();

			/** \brief Mark the device as Accessible
			 *
			 *	Mark the device as Accessible (or as Inaccessible)
			 *	This may be used for USB devices that are unplugged.
			 *	@param bFlg 'true' --> Accessible
			 */
	virtual void SetDeviceEnable (BOOL bFlg = true);


			/** \brief Get the API version number
			 *
			 *	Get the version number of the API
			 *	@returns ATVERSIONINFO
 			 */
	virtual	ATVERSIONINFO *		GetAPIVersion();

			/** \brief Get the filename / location of the API used
			 *
			 *	Get the filename / location of the API used
			 *	This may be used for USB devices that are unplugged.
			 *	@returns string with filepath and filename
			 */
	virtual char *				GetAPIFileLocation();

			/** \brief Get the driver version number
			 *
			 *	Get the version number of the DRIVER
			 *  Currently only working for USB devices.
			 *	@returns ATVERSIONINFO. For USB, version number, for PCI, Mayor = -1;
 			 */
	virtual ATVERSIONINFO *		GetDriverVersion();

	/** \brief Get the error string of the last error that occurred.
	*
	*	When a call to a low-level function to a AT-device fails, 
	*	the description of the error is left in this string pointer.\n
	*	When retrieved, the string is cleared.\n
	*	Note that the pointer points to a static string in the API and does NOT need free or delete.
	*	@returns pointer to the string, or NULL.
	*/
	virtual const char *		GetLastErrorString();

	/** \brief Get the FPGA programming status for the USB device
	 *
	 *	Get the FPGA programmed status for the USB device
	 *
	 *  @returns 'TRUE' when programmed;
 	 */
	virtual	BOOL		IsUSBFPGAProgrammed();

	/** \brief Read data from the EEprom
	 *
	 *	Read data from the EEprom.\n
	 *	This function starts reading the EEProm data at user address 0 + nOffset.
	 *	The number of bytes read is set with the size.
	 *	If the offset and size are out of range this function will return False.
	 *	
	 *	@param	pData	A pointer to the data to be read from the EEPROM
	 *	@param	nOffset An address offset
	 *	@param	nSize	The size of the data to be read.
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 */
	virtual BOOL		GetEEpromData(u8* pData, u16 nOffset, u32 & nSize);

	/** \brief Write data to the EEprom
	 *
	 *	Write data to the EEprom\n
	 *	This function starts writing the EEProm data at user address 0 + nOffset.
	 *	The number of bytes to be written is set with the size.
	 *	If the offset and size are out of range this function will return False.
	 *
	 *	@param	pData	A pointer to the data to be stored in the EEPROM
	 *	@param	nOffset An address offset
	 *	@param	nSize	The size of the data to be read.
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 */
	virtual BOOL		WriteEEpromData(u8* pData, u16 nOffset, u32 & nSize);

	/** \brief Read data from the private EEprom section
	 *
	 *	Read data from the private EEprom section.\n
	 *	This function starts reading the EEProm data at private address 0 + nOffset.
	 *	The number of bytes read is set with the size.
	 *	If the offset and size are out of range this function will return False.\n
	 *
	 *	@param	pData			A pointer to the data to be read from the EEPROM
	 *	@param	nOffset			An address offset
	 *	@param	nSize			The size of the data to be read.
	 *	@param	nMagicWord		Password for this function
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 *
	 *	\internal This function must not be used by application programmers. The area of EEProm
	 *	may contain data that is used to calibrate the AT-device. If it is damaged,
	 *	the device may not work (properly) any more. For this reason, the password
	 *	is not made available.
	 */
	virtual	BOOL		GetPrivateEEpromData(u8* pData, u16 nOffset, u32 & nSize, u16 nMagicWord);

	/** \brief Write data to the private EEprom section
	 *
	 *	Write data to the private EEprom section.\n
	 *	This function starts writing the EEProm data at private address 0 + nOffset.
	 *	The number of bytes to be written is set with the size.
	 *	If the offset and size are out of range this function will return False.\n
	 *
	 *	@param	pData			A pointer to the data to be stored in the EEPROM
	 *	@param	nOffset			An address offset
	 *	@param	nSize			The size of the data to be read.
	 *	@param	nMagicWord		Password for this function
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 *
	 *	\internal This function must not be used by application programmers. The area of EEProm
	 *	may contain data that is used to calibrate the AT-device. If it is damaged,
	 *	the device may not work (properly) any more. For this reason, the password
	 *	is not made available.
	 */
	virtual	BOOL		WritePrivateEEpromData(u8* pData, u16 nOffset, u32 & nSize, u16 nMagicWord);
	
	/** \brief Get the device type of the board
	 *
	 *	Get the device type of the board.\n
	 *	@returns The device type.
	 */
	virtual DeviceTypeEnum	GetDeviceType();

	/** \brief Get the serial number of the board
	 *
	 *	Get the serial number of the board.\n
	 *	This function reads the Serial number of the ATboard
	 *	@returns The serial number.
	 */
	virtual __int64		GetSerialNumber();
	
	/** \brief Get the location of the device
	 *
	 *	Get the location of the AT board used
	 *	slot number, x(bus),y(position),z(function)
	 *	@returns FALSE when failed
	 */
	virtual BOOL		GetDeviceLocation(ATDEVICELOCATION * pDevLocation);

	/** \brief Get the location of the device
	 *
	 *	Get the location of the AT board as a string
	 *	slot number, x(bus),y(position),z(function)
	 *	String is in the form of Slot n (X, Y, Z)
	 * 
	 *	@returns  FALSE when failed
	 */
	virtual BOOL		GetDeviceLocationString(char * pDeviceLocation, u32 &nLength);

	/** \brief Read data from tuners (dvb-s, dvb-c, dvb-t) direct by FX2
	 *
	 *	Read data from the Tuner.\n
	 *	This function starts reading the tuner data at user address 0 + nOffset.
	 *	The number of bytes read is set with the size.
	 *	
	 *	@param	pData		A pointer to the data to be read from the EEPROM
	 *	@param	nSize		The size of the data to be read.
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 */
	virtual BOOL		GetI2cFX2(u8* pData, u32 nSize);

	/** \brief Write data to tuners (dvb-s, dvb-c, dvb-t) direct by FX2
	 *
	 *	Write data to the Tuner\n
	 *	This function starts writing the tuner data at user address 0 + nOffset.
	 *	The number of bytes to be written is set with the size.
	 *
	 *	@param	pData		A pointer to the data to be stored in the EEPROM
	 *	@param	nSize		The size of the data to be read.
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 */
	virtual BOOL		SetI2cFX2(u8* pData, u32 nSize);

	/** \brief set slave address and start address direct by FX2
	 *
	 *	@param	nSlvAdrs	I2C slave address of the tuner
	 *	@param	nOffset		An address offset
	 *
	 *	@returns TRUE on success, FALSE on failure.
	 */
	virtual BOOL		SetI2cFX2Adrs(u8 nSlvAdrs, u8 nOffset);

	/** \brief Store FPGA registers to the private EEprom section
	*
	*	Store FPGA registers to the private EEprom section.\n
	*	This function can be used to store the FPGA register setting
	*	in EEPROM so that at next power-up, the latest settings can be
	*	restored.
	*
	*	@returns TRUE on success, FALSE on failure.
	*/
	virtual BOOL		StoreFpgaRegs();

	/** \brief Restore FPGA registers from the private EEprom section
	*
	*	Restores FPGA registers from the private EEprom section.\n
	*	This function can be used to restore the latest FPGA register
	*	settings from EEPROM at power-up.
	*
	*	@returns TRUE on success, FALSE on failure.
	*/
	virtual BOOL		ReStoreFpgaRegs();


	/** \brief Get the error, if exists, after a DVBSource call
	*
	*	The DVBSource functions communicate with the tuners.
	*	When an error occurs during the communication, the function will fail.
	*	Use this call to get the error number
	*
	*	Error are a ENUM defined in DVBSource_Error.h	
	*
	*	User GETDVBSourceErrorString to get an error string
	*
	*	@returns DVB_SOURCE_NO_ERROR when there a no errors, else DVB_SOURCE_ERROR type
	*/
	virtual DVBSOURCE_ERROR GetDVBSourceError();


	/** \brief Get the error description, if exists, after a DVBSource call
	*
	*	The DVBSource functions communicate with the tuners.
	*	When an error occurs during the communication, the function will fail.
	*	Use this call to get the error string
	*
	*	User GETDVBSourceError to get an error number
	*
	*	@returns char * with text string
	*/
	virtual char * GetDVBSourceErrorString();

protected:
			//Con-/destructor only available for: CAtBoardManager.
					CATBoard		(SBoardOpenParams *pBoardOpenParams, ATDEVICEINFO *pDeviceInfo);
	virtual			~CATBoard		();

			BOOL	SetFpgaRegStartAddr(u8 nAddress);
			static void	SetErrorMessage(const char *pStr);
public:
			BOOL	SetFpgaRegs	(void *pData, u8 nStartAddr, u8 nSize);
			BOOL	GetFpgaRegs	(void *pData, u8 nStartAddr, u8 nSize);
protected:
			
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


protected:
	TSysMutex				m_mutexDevice;
	TSysMutex				m_FpgaMutex;
	ATREGISTRY				m_iRegisters;			///< ACCESS THIS STRUCTURE ONLY VIA CAtRegisters !!!
	ATREGISTRY				m_iRegistersBackup;		///< backup of m_iRegisters.
	CI2cBus					*m_pI2cBus;				
	CI2cBus					*m_pI2cBusFx2;				
	CModulatorInterface		*m_pModInt;
	CDvbSource				*m_pDvbSource;
	char					*m_pDeviceName;		
//	std::string				m_sFriendlyName;
//	std::string				m_sFPGAFileName;
	std::string				m_FirmwareFname;		///< Fully qualified name for the firmware file.
	std::string				m_NxtFirmwareFname;		///< Fully qualified name for next to program firmware file.
	TSLONG					m_nRefCount;			///< Reference count of the instance
	u32						m_SyncCountOffset;		///< The 'Tare' value of the Sync counter
	u32						m_ErrCountOffset;		///< The 'Tare' value of the Error counter

	u32						m_BitrateUpdateInterval;///< in milliseconds
	CIIRFilter				*m_pBitrateFlt;			///< Bitrate filter
	BOOL					m_bDeviceEnable;		///< Indication that the device is not accessible. Further access is now disabled.
	dvb_frontend_parameters	m_frontparms;			///< Local restore buffer.
	CSigQuality				*m_pSigQuality;			///< Signal quality calculations
	
	ATDEVICEINFO			m_AtDeviceInfo;			///< local stored AT device information 
	
	//who are my friends??
	friend class CAtRegisters;
	friend class CAtBoardManager;
	friend class CATi2c;
	friend class CATi2cFX2;
	friend class CModulatorTerrestrial;
	friend class CATEeprom;
public:
	SBoardOpenParams		m_BoardOpenParams;		///< parameter list from which the board is opened
#ifndef LINUX
	static HWND s_hWnd;				///< Handle of Parent window for ::MessageBox error messages (may be NULL)
#endif
};
/*@}*/


#ifndef _LINUX
#   pragma warning(pop)
#endif // WIN32

#endif // !defined(AFX_ATBOARD_H__71654D2A_E95F_463B_8D21_7BE971D7389B__INCLUDED_)

