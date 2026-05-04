//*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#* DTAPI.h *#*#*#*#*#*#*#*#*#* (C) 2000-2006 DekTec
//
// DTAPI: C++ API for DekTec's Digital-Video PCI Cards and USB-2 Adapters
//
// Main header file

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Change History -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//	By:   Date:   New Rev:	Change:
//	MG 2006.05.16 3.0.5.63	Add support for DTU-235
//	RD 2006.05.03 3.0.4.62	Add DTAPI_NW_SPEED_ constants
//	MG 2006.04.24 3.0.4.61	Add m_Flags member to DtTsIpPars structure
//	RD 2006.04.14 3.0.3.60	Add DTAPI_RXMODE_IPRAW, RawIpHeader
//	MG 2006.04.08 3.0.2.59	Add DtLoop class
//	MG 2006.03.20 3.0.1.58	Add implementation for GetIpPars and GetIpStat
//	MG 2006.03.13 3.0.1.57	Add overload for DtDevice::GetRefClkCnt with second parameter
//							which returns the clock frequency of the ref. clock counter
//	MG 2006.03.03 3.0.0.56	Added capability flags for supported frequency bands
//	RD 2006.03.13 3.0.0.56	Added ChangeTsIoConfig to DtDevice class
//	RD 2006.03.10 3.0.0.56	Added GetIpPars and GetIpStat to TsInpChannel class
//	MG 2006.02.23 3.0.0.55	Renamed DTAPI_TO_XXX defines (New: DTAPI_HWF2STR_XXX)
//	MG 2006.02.08 3.0.0.54	- Add DtapiInitDtTsIpParsFromIpString function
//							- Add DTAPI_MCAST_JOIN, DTAPI_MCAST_DROP defines
//	TD 2006.01.05 3.0.0.53	Added conditional DtApi namespace 
//	MG 2005.12.02 3.0.0.52	Add support for SDI in the output channel
//	MG 2005.11.10 3.0.0.51	- Add DTAPI_RFPLL_NO_LOCK, DTAPI_RFPLL_LOCK,
//							  DTAPI_TXSTUFF_MODE_OFF and DTAPI_TXSTUFF_MODE_ON defines
//							- Add support for SDI input
//	SD 2005.09.05 3.0.0.49	Add support for DTAPI_MOD_IQDIRECT and DTAPI_MOD_OFDM
//	SD 2005.08.11 3.0.0.48	Add TS-over-IP support in DTAPI for DTA-160
//	MG 2005.08.02 2.5.0.47	Added support for DTA-545 input channel
//	MG 2005.07.27 2.5.0.46	Added support for DTU-245
//	MG 2005.05.31 2.4.3.44	Added support for using the DLL version of the DTAPI
//	SD 2005.04.07 2.4.2.43  Added IpInpChannel
//	MG 2005.04.07 2.4.1.42	Made sure that TsRate returned by TsInpChannel::GetTsRateBps
//							is normalised to 188byte packets
//	SD 2005.01.28 2.4.0.41	Add support for DTA-105
//	MG 2005.01.13 2.3.0.40	Removed StartAddr parmeter from I2CRead and I2CWrite methods
//	MG 2004.12.29 2.3.0.39	- Added GetMaxFifoSize and GetReceivedByteCount methods to
//							  TsInpChannel class
//							- Added GetTransmitByteCount method to TsOutpChannel
//							- Added GetUsbSpeed method to DtDevice class
//	MG 2004.12.09 2.3.0.38	Added I2CRead and I2CWrite methods to DtDevice class
//	MG 2004.09.28 2.2.0.37	Added support for devices with multiple inputs (e.g. DTA-124)
//	MG 2004.08.18 2.1.2.36	- Added implementation for missing TsInpChannel functions
//							  for the DTU-225
//							- Fixed error in VpdReadRaw and VpdWriteRaw function
//							  for the DTU-2xx devices
//	SD 2004.08.13 2.1.1.35	Add support for modulator PCI cards (DTA-107, DTA-110)
//	MG 2004.06.27 2.1.0.34	Add support for DTU-205
//	SD 2004.06.27 2.0.2.33	Add UNICODE variants of VPD functions
//	MG 2004.05.31 2.0.2.32	Fixed bug in TsInpChannel::Read
//	SD 2004.04.25 2.0.1.31	Support to compile for Borland
//	SD 2004.03.17 2.0.0.30	Clean up
//	RZ 2004.02.17 2.0.0.29	Adapt for DtDevice
//	SD 2003.01.19 1.3.1.25	New method TsInpChannel::WriteLoopBackData
//	SD 2002.12.12 1.3.0.24	New methods PciCard::GetRefClkCnt, PciCard::LedControl,
//							TsInpChannel::ClearFifo, TsInpChannel::GetRxControl
//	SD 2002.11.11 1.2.1.21	Add definition of DTAPI_TXMODE_MIN16
//	SD 2002.11.11 1.2.0.20	Add declaration of DtapiCheckDeviceDriverVersion
//	SD 2002.11.11 1.1.0.19	Add support for DTA-140 DVB/ASI Input+Output board
//	SD 2002.03.22 1.0.2.6	- Added TsOutpChannel::ClearFifo
//							- Call ClearFifo i.s.o. Reset in Attach and Detach
//	SD 2002.01.15 0.9.1.2	Support included for TsInpChannel
//	SD 2001.09.10 0.9.1.2	For DTA-102: enable output before ClearFlags in Attach
//	SD 2001.06.14  0.09		Added bugfix and buildno to DTAPI / device-driver versions
//							Add PciCard::GetFirmwareVersion
//	SD 2001.05.31  0.08		Added DTAPI_TXMODE_192 for DTA-102 with Altera Rev A2
//	SD 2001.02.23  0.07		Added DTAPI_TXMODE_BURST for burst mode on DTA-100
//	SD 2001.02.09  0.06		Added functions and constants for DTAPI 0.06
//							Re-arranged error return codes (alphabetically)
//	SD 2000.10.15  0.00		Created

//#pragma once
#ifndef __DTAPI_H
#define __DTAPI_H

#ifdef _USE_DTAPIDLL
	#ifdef DTAPIDLL_EXPORTS
	#define DTAPI_API __declspec(dllexport)
	#else
	#define DTAPI_API __declspec(dllimport)
	#endif
#else
	// Not using the DLL so define DTAPI_API as empty
	#define DTAPI_API
#endif

//-.-.-.-.-.-.-.-.-.-.-.-.- Additional Libraries to be Linked In -.-.-.-.-.-.-.-.-.-.-.-.-
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DTAPI Version -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_VERSION_MAJOR		3
#define  DTAPI_VERSION_MINOR		0
#define  DTAPI_VERSION_BUGFIX		6
#define  DTAPI_VERSION_BUILD		65


//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Includes -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#include <windows.h>			// For windows type HANDLE


//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DTAPI Support Types +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

//if namespace usage is required
#ifdef _USE_NAMESPACE
namespace DtApi {
#endif

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Elementary Types -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
typedef unsigned long  DTAPI_RESULT;


//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtDeviceDesc -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//
// Structure describing a device.
//
struct DtDeviceDesc {	
	int  m_Category;			// Device category
	__int64  m_Serial;			// Unique serial number of the device
	int  m_PciBusNumber;		// PCI-bus number
	int  m_SlotNumber;			// PCI-slot number
	int  m_UsbAddress;			// USB address
	int  m_TypeNumber;			// Device type number
	int  m_DeviceId;			// Device ID
	int  m_VendorId;			// Vendor ID
	int  m_SubsystemId;			// Subsystem
	int  m_SubVendorId;			// Subsystem Vendor ID
	int  m_NumHwFuncs;			// #Hardware funtions hosted by device
	int  m_FirmwareVersion;		// Firmware version
	int  m_FirmwareVariant;		// Firmware variant
	int  m_NumDtInpChan;		// Number of input channels 
	int  m_NumDtOutpChan;		// Number of output channels 
	int  m_NumPorts;			// Number of physical ports
};

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Device Categories -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
// NOTE: Category values 32-63 are reserved for use in DTAPIplus
#define  DTAPI_CAT_PCI			0
#define  DTAPI_CAT_USB			1
#define  DTAPI_CAT_NDIS			2			// Not a real device category (intended for
											// getting  driver version use)

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtHwFuncDesc -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
//
// Structure describing a hardware function.
//
struct DtHwFuncDesc {
	DtDeviceDesc  m_DvcDesc;	// Device descriptor
	int  m_ChanType;			// Channel type (OR-able)
	int  m_StreamType;			// Transport-Stream type
	int  m_Flags;				// Capability flags (OR-able)
	int  m_IndexOnDvc;			// Index of hardware function
	int  m_Port;				// Physical port number
	unsigned char m_Ip[4];		// IP address (only valid for IP functions)
	unsigned char m_MacAddr[6];	// MAC address (only valid for IP functions)
};

//-.-.-.-.-.-.-.-.-.-.-.-.-.- Hardware-Function Channel Types -.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_CHAN_OUTPUT		1
#define  DTAPI_TS_OUTPUT		1			// Obsolete, for backward compatibility
#define  DTAPI_CHAN_INPUT		2
#define  DTAPI_TS_INPUT			2			// Obsolete, for backward compatibility

//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Hardware-Function Stream Types -.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_ASI_SDI			1
#define  DTAPI_TS_MOD			2
#define  DTAPI_TS_OVER_IP		3
#define  DTAPI_TS_SPI			4

//.-.-.-.-.-.-.-.-.-.-.-.-.-.- Hardware-Function Capabilities -.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_CAP_ASI			0x00000001
#define  DTAPI_CAP_BIDIR		0x00000002
#define  DTAPI_CAP_DBLBUF		0x00000004
#define  DTAPI_CAP_SDI			0x00000008

// Modulation-related
#define  DTAPI_CAP_OFDM			0x00000010
#define  DTAPI_CAP_QAM_A		0x00000100
#define  DTAPI_CAP_QAM_B		0x00000200
#define  DTAPI_CAP_QAM_C		0x00000400
#define  DTAPI_CAP_QPSK			0x00001000
#define  DTAPI_CAP_8VSB			0x00010000
#define  DTAPI_CAP_VHF			0x00100000		// VHF-band 47-470MHz
#define  DTAPI_CAP_UHF			0x00200000		// UHF-band 400-862MHz
#define  DTAPI_CAP_LBAND		0x00400000		// L-band 950-2150MHz

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- String Conversions Types -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-

// Device
// Device type number (e.g. "DTA-100", "DTA-102")
#define DTAPI_DVC2STR_TYPE_NMB			0	
// Device type number + location (e.g. "DTA-100 in slot 5");
#define DTAPI_DVC2STR_TYPE_AND_LOC		1

// Hardware Function
// Device type number (e.g. "DTA-100", "DTA-102")
#define DTAPI_HWF2STR_TYPE_NMB			0	
// Device type number + port (e.g. "DTA-124 port 1")
#define DTAPI_HWF2STR_TYPE_AND_PORT		1
// Device type number + location (e.g. "DTA-100 in slot 5");
#define DTAPI_HWF2STR_TYPE_AND_LOC		2
// Interface type (e.g. "DVB-ASI" or "DVB-C")
#define DTAPI_HWF2STR_ITF_TYPE			3
// Short version of interface type (e.g. "ASI" instead "DVB-ASI")
#define DTAPI_HWF2STR_ITF_TYPE_SHORT	4


//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtTsIpPars -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//
// Structure for storing parameters related to the transmission of Transport Streams
// over IP
//
struct DtTsIpPars {
	unsigned char  m_Ip[4];			// IP Address
	unsigned short  m_Port;			// IP port
	unsigned char  m_SrcFltIp[4];	// Source filter: IP address
	unsigned short  m_SrcFltPort;	// Source filter: IP port 
	int  m_MulticastTtl;			// TTL for multicast Tx
	int  m_NumTpPerIp;				// #TPs per IP packet
	int  m_Protocol;				// Protocol: UDP/RTP
	int  m_DiffServ;				// Differentiated Services
	int  m_FecMode;					// Error correction mode
	int  m_FecNumRows;				// ‘D’ = #rows in FEC matrix
	int  m_FecNumCols;				// ‘L’ = #columns in FEC matrix
	int  m_Flags;					// Optional controls/status flags
};


//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtTsIpStat -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//
// Structure for retrieving Ip statistics from drivers
// Statistics will be reset after read.
//
struct DtTsIpStat {
	unsigned int  m_TotNumIpPackets;
	unsigned int  m_LostIpPacketsBeforeFec;		// #Lost Packets before FEC 
	unsigned int  m_LostIpPacketsAfterFec;		// #Lost Packets after FEC
	unsigned int  m_LostIpPacketsBeforeSort;	// #Lost Packets before RTP sorting
	unsigned int  m_LostIpPacketsAfterSort;		// #Lost Packets after RTP sorting
};

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtRawIpHeader -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
//
// Header placed infront of all Ip Packets when DTAPI_RXMODE_IPRAW mode is used
//
struct DtRawIpHeader {
	unsigned short  m_Tag;			// 0x44A0h = ‘D’160
	unsigned short  m_Length;		// IP Packet Length;
	unsigned long  m_TimeStamp;		// Timestamp
};

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DtDevice +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// Class to represent a DEKTEC Device.
//
class DTAPI_API  DtDevice {

	// Public attributes
public:
	DtDeviceDesc  m_DvcDesc;	// Device Descriptor
	DtHwFuncDesc*  m_pHwf;		// Hardware functions, filled after Attach

	// Public access functions
public:
	int  Category(void)			{ return m_DvcDesc.m_Category; }
	int  FirmwareVersion(void)	{ return m_DvcDesc.m_FirmwareVersion; }
	int  TypeNumber(void)		{ return m_DvcDesc.m_TypeNumber; }
	bool IsAttached(void)		{ return m_Attached; }

	// Public member functions
public:
	DTAPI_RESULT  AttachToSerial(__int64 SerialNumber);
	DTAPI_RESULT  AttachToSlot(int PciBusNumber, int SlotNumber);
	DTAPI_RESULT  AttachToType(int TypeNumber, int DeviceNo=0);
	DTAPI_RESULT  Detach(void);
	DTAPI_RESULT  GetDescriptor(DtDeviceDesc& DvcDesc);
	DTAPI_RESULT  GetDeviceDriverVersion(int& DriverVersionMajor,
										 int& DriverVersionMinor);
	DTAPI_RESULT  GetDeviceDriverVersion(
								int& DriverVersionMajor, int& DriverVersionMinor,
								int& DriverVersionBugFix, int& DriverVersionBuild);
	DTAPI_RESULT  GetFirmwareVersion(int& FirmwareVersion);
	DTAPI_RESULT  GetIoConfig(int Port, int& IoConfig);
	DTAPI_RESULT  GetNwSpeed(int Port, bool& Enable, int& Speed);
	DTAPI_RESULT  GetRefClkCnt(int& RefClkCnt);
	DTAPI_RESULT  GetRefClkCnt(int& RefClkCnt, int& RefClkFreqHz);
	DTAPI_RESULT  GetUsbSpeed(int& UsbSpeed);
	DTAPI_RESULT  HwFuncScan(int NumEntries, int& NumEntriesResult,
							 DtHwFuncDesc* pHwFuncs);
	DTAPI_RESULT  I2CRead(int DvcAddr, char* pBuffer, int NumBytesToRead);
	DTAPI_RESULT  I2CWrite(int DvcAddr, char* pBuffer, int NumBytesToWrite);
	DTAPI_RESULT  LedControl(int LedControl);
	DTAPI_RESULT  SetIoConfig(int Port, int IoConfig);
	DTAPI_RESULT  SetNwSpeed(int Port, bool Enable, int Speed);
	DTAPI_RESULT  VpdDelete(char* pTag);
	DTAPI_RESULT  VpdDelete(wchar_t* pTag);
	DTAPI_RESULT  VpdRead(char* pTag, char* pVpdItem);
	DTAPI_RESULT  VpdRead(wchar_t* pTag, wchar_t* pVpdItem);
	DTAPI_RESULT  VpdWrite(char* pTag, char* pVpdItem);
	DTAPI_RESULT  VpdWrite(wchar_t* pTag, wchar_t* pVpdItem);

	// Encapsulated data
public:
	bool  m_Attached;			// Attached-to-hardware flag
	HANDLE  m_hDriver;			// Handle to PCI-card's device driver
	int*  m_pGenRegs;			// General registers

	// Constructor, destructor
public:
	DtDevice();
	virtual ~DtDevice();

	// Friends
	friend class  DtInpChannel;
	friend class  DtOutpChannel;
	friend class  TsInpChannel;
	friend class  TsOutpChannel;
};


//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtDevice Constants -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- I/O Configuration -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_IOCONFIG_NOTSUP	0
#define  DTAPI_IOCONFIG_INPUT	1
#define  DTAPI_IOCONFIG_OUTPUT	2
#define  DTAPI_IOCONFIG_DBLBUF	3
#define  DTAPI_IOCONFIG_IP		4

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Support Flags -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
// Used in DtInpChannel and DtOutpChannel
#define  DTAPI_SUPPORTS_TS		1
#define  DTAPI_SUPPORTS_SDI		2


//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DtInpChannel +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// Class to represent an input channel.
//
class DTAPI_API  DtInpChannel {

	friend class DtLoopWorker;	// allow access to protected members

	// Public attributes
public:
	DtHwFuncDesc  m_HwFuncDesc;	// Hardware function descriptor

	// Public access functions
public:
	int  Category(void)			{ return m_HwFuncDesc.m_DvcDesc.m_Category; }
	int  FirmwareVersion(void)	{ return m_HwFuncDesc.m_DvcDesc.m_FirmwareVersion; }
	int  GetChannelType(void)	{ return m_HwFuncDesc.m_ChanType; }
	int  GetIndexOnDevice(void)	{ return m_HwFuncDesc.m_IndexOnDvc; }
	bool IsAttached(void)		{ return (m_pOpaq != NULL); }
	int  TypeNumber(void)		{ return m_HwFuncDesc.m_DvcDesc.m_TypeNumber; }

public:
	DTAPI_RESULT  Attach(DtDevice* pDtDvc, int InpIndex=0, bool ProbeOnly=false);
	DTAPI_RESULT  AttachToPort(DtDevice* pDtDvc, int Port, bool ProbeOnly=false);
	DTAPI_RESULT  ClearFifo(void);
	DTAPI_RESULT  ClearFlags(int Latched);
	DTAPI_RESULT  Detach(int DetachMode);
	DTAPI_RESULT  Equalise(int EqualiserSetting);
	DTAPI_RESULT  GetDescriptor(DtHwFuncDesc& HwFunDesc);
	DTAPI_RESULT  GetFifoLoad(int& FifoLoad);
	DTAPI_RESULT  GetFlags(int& Flags, int& Latched);
	DTAPI_RESULT  GetIpPars(DtTsIpPars* pTsIpPars);
	DTAPI_RESULT  GetIpStat(DtTsIpStat* pTsIpStat);
	DTAPI_RESULT  GetMaxFifoSize(int& MaxFifoSize);
	DTAPI_RESULT  GetReceiveByteCount(int& ByteCount);
	DTAPI_RESULT  GetRxControl(int& RxControl);
	DTAPI_RESULT  GetRxMode(int& RxMode);
	DTAPI_RESULT  GetStatistics(int& ViolCount);
	DTAPI_RESULT  GetStatus(int& PacketSize, int& NumInv, int& ClkDet,
							int& AsiLock, int& RateOk, int& AsiInv);
	DTAPI_RESULT  GetTargetId(int& Present, int& TargetId);
	DTAPI_RESULT  GetTsRateBps(int& TsRate);
	DTAPI_RESULT  LedControl(int LedControl);
	DTAPI_RESULT  PolarityControl(int Polarity);
	DTAPI_RESULT  Read(char* pBuffer, int NumBytesToRead);
	DTAPI_RESULT  ReadDirect(char* pBuffer,	int NumBytesToRead, int& NumBytesRead);
	DTAPI_RESULT  ReadUsingDma(char* pBuffer, int NumBytesToRead);
	DTAPI_RESULT  Reset(int ResetMode);
	DTAPI_RESULT  SetLoopBackMode(int Mode);
	DTAPI_RESULT  SetIpPars(DtTsIpPars* pTsIpPars);
	DTAPI_RESULT  SetPower(int Power);
	DTAPI_RESULT  SetRxControl(int RxControl);
	DTAPI_RESULT  SetRxMode(int RxMode);
	DTAPI_RESULT  WriteLoopBackData(char* pBuffer, int NumBytesToWrite);

protected:
	bool  IsTsSupported()  { return ((m_Support & DTAPI_SUPPORTS_TS)!=0); }
	bool  IsSdiSupported() { return ((m_Support & DTAPI_SUPPORTS_SDI)!=0); }

	// Encapsulated data
protected:
	void*  m_pOpaq;				// Opaque data; doubles as 'Attached' flag
	int  m_Support;				// Channel capabilities supported by this object

	// Constructor, destructor
public:
	DtInpChannel();
	virtual ~DtInpChannel();
};

// Transport-Stream channel is a specialised version of DtInpChannel
class TsInpChannel : public DtInpChannel {
	// Constructor, destructor
public:
	TsInpChannel();
	virtual ~TsInpChannel();
};

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtInpChannel Constants -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

// Feature not supported
#define  DTAPI_NOT_SUPPORTED	-1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- ASI Polarity-Control Status -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_ASIINV_NORMAL	0
#define  DTAPI_ASIINV_INVERT	1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- ASI Input-Clock Lock -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_ASI_NOLOCK		0
#define  DTAPI_ASI_INLOCK		1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Clock Detector -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_CLKDET_FAIL		0
#define  DTAPI_CLKDET_OK		1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Input Rate Ok -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_INPRATE_LOW		0
#define  DTAPI_INPRATE_OK		1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- #Invalid Bytes per Packet -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_NUMINV_NONE		0
#define  DTAPI_NUMINV_16		1
#define  DTAPI_NUMINV_OTHER		2

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Packet Size -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_PCKSIZE_INV		0
#define  DTAPI_PCKSIZE_188		2
#define  DTAPI_PCKSIZE_204		3

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Receive Control -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_RXCTRL_IDLE		0
#define  DTAPI_RXCTRL_RCV		1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Receive Mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_RXMODE_ST188		0
#define  DTAPI_RXMODE_ST204		1
#define  DTAPI_RXMODE_STMP2		2
#define  DTAPI_RXMODE_STRAW		3
#define  DTAPI_RXMODE_STTRP		4
#define  DTAPI_RXMODE_IPRAW		5
#define  DTAPI_RX_TIMESTAMP		0x08		
#define  DTAPI_RXMODE_SDI		0x10		// Flag: can be OR-ed with other modes
#define  DTAPI_RXMODE_SDI_10BIT	0x20		// Flag: can be OR-ed with other modes
#define  DTAPI_RXMODE_SDI_STRAW (DTAPI_RXMODE_SDI | DTAPI_RXMODE_STRAW)



//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DtOutpChannel +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// Class to represent a transport-stream output channel.
//
class DTAPI_API DtOutpChannel {

	friend class DtLoopWorker;	// allow access to protected members

	// Public attributes
public:
	DtHwFuncDesc  m_HwFuncDesc;	// Hardware function descriptor

	// Public access functions
public:
	int  Category(void)			{ return m_HwFuncDesc.m_DvcDesc.m_Category; }
	int  FirmwareVersion(void)	{ return m_HwFuncDesc.m_DvcDesc.m_FirmwareVersion; }
	int  GetChannelType(void)	{ return m_HwFuncDesc.m_ChanType; }
	int  GetIndexOnDevice(void)	{ return m_HwFuncDesc.m_IndexOnDvc; }
	bool IsAttached(void)		{ return (m_pOpaq != NULL); }
	int  TypeNumber(void)		{ return m_HwFuncDesc.m_DvcDesc.m_TypeNumber; }

public:
	DTAPI_RESULT  Attach(DtDevice* pDtDvc, int OutpIndex=0, bool ProbeOnly=false);
	DTAPI_RESULT  AttachToPort(DtDevice* pDtDvc, int Port, bool ProbeOnly=false);
	DTAPI_RESULT  ClearFifo(void);
	DTAPI_RESULT  ClearFlags(int Latched);
	DTAPI_RESULT  Detach(int DetachMode);
	DTAPI_RESULT  GetDescriptor(DtHwFuncDesc& HwFunDesc);
	DTAPI_RESULT  GetExtClkFreq(int& ExtClkFreq);
	DTAPI_RESULT  GetFifoLoad(int& FifoLoad);
	DTAPI_RESULT  GetFifoSize(int& FifoSize);
	DTAPI_RESULT  GetFlags(int& Status, int& Latched);
	DTAPI_RESULT  GetMaxFifoSize(int& MaxFifoSize);
	DTAPI_RESULT  GetModControl(int& ModType, int& CodeRate, int& ParXtra1, int& ParXtra2);
	DTAPI_RESULT  GetRfControl(__int64& RfFreq, int& LockStatus);
	DTAPI_RESULT  GetTargetId(int& Present, int& TargetId);
	DTAPI_RESULT  GetTransmitByteCount(int& ByteCount);
	DTAPI_RESULT  GetTsRateBps(int& ClockGenMode, int& TsRate);
	DTAPI_RESULT  GetTxControl(int& TxControl);
	DTAPI_RESULT  GetTxMode(int& TxPacketMode, int& TxStuffMode);
	DTAPI_RESULT  ReadLoopBackData(char* pBuffer, int BytesToRead);
	DTAPI_RESULT  Reset(int ResetMode);
	DTAPI_RESULT  SetFifoSize(int FifoSize);
	DTAPI_RESULT  SetFifoSizeMax(void);
	DTAPI_RESULT  SetIpPars(DtTsIpPars* pTsIpPars);
	DTAPI_RESULT  SetLoopBackMode(int Mode);
	DTAPI_RESULT  SetModControl(int ModType, int CodeRate, int ParXtra1, int ParXtra2);
	DTAPI_RESULT  SetPower(int Power);
	DTAPI_RESULT  SetRfControl(__int64 RfFreq);
	DTAPI_RESULT  SetTsRateBps(int ClockGenMode, int TsRate);
	DTAPI_RESULT  SetTxControl(int TxControl);
	DTAPI_RESULT  SetTxMode(int TxPacketMode, int TxStuffMode);
	DTAPI_RESULT  SetTxPolarity(int TxPolarity);
	DTAPI_RESULT  Write(char* pBuffer, int NumBytesToWrite);
	DTAPI_RESULT  WriteDirect(char* pBuffer, int NumBytesToWrite);
	DTAPI_RESULT  WriteUsingDma(char* pBuffer, int NumBytesToWrite);

protected:
	bool  IsTsSupported()  { return ((m_Support & DTAPI_SUPPORTS_TS)!=0); }
	bool  IsSdiSupported() { return ((m_Support & DTAPI_SUPPORTS_SDI)!=0); }

	// Encapsulated data
protected:
	void*  m_pOpaq;				// Opaque data; doubles as 'Attached' flag
	int  m_Support;				// Channel capabilities supported by this object:
								// DTAPI_SUPPORTS_TS and/or DTAPI_SUPPORTS_SDI

	// Constructor, destructor
public:
	DtOutpChannel();
	virtual ~DtOutpChannel();
};

// Transport-Stream output channel is a specialised version of DtOutpChannel
class TsOutpChannel : public DtOutpChannel {
	// Constructor, destructor
public:
	TsOutpChannel();
	virtual ~TsOutpChannel();
};

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtOutpChannel Constants -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Clock-Generator Modes -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_TXCLOCK_INTERNAL	0
#define  DTAPI_TXCLOCK_EXTERNAL	1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Detach Mode flags -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_INSTANT_DETACH	1
#define  DTAPI_ZERO_OUTPUT		2
#define  DTAPI_TRISTATE_OUTPUT	4
#define  DTAPI_WAIT_UNTIL_SENT	8

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Equaliser Settings -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_EQUALISER_OFF	0
#define  DTAPI_EQUALISER_ON		1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- LED Control -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_LED_OFF			0
#define  DTAPI_LED_GREEN		1
#define  DTAPI_LED_RED			2
#define  DTAPI_LED_YELLOW		3
#define  DTAPI_LED_HARDWARE		4

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Loop-Back Mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_NO_LOOPBACK		0
#define  DTAPI_LOOPBACK_MODE	1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Modulation Parameters -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_MOD_QPSK			0
#define  DTAPI_MOD_BPSK			1
#define  DTAPI_MOD_QAM4			3
#define  DTAPI_MOD_QAM16		4
#define  DTAPI_MOD_QAM32		5
#define  DTAPI_MOD_QAM64		6
#define  DTAPI_MOD_QAM128		7
#define  DTAPI_MOD_QAM256		8
#define  DTAPI_MOD_OFDM			9
#define  DTAPI_MOD_IQDIRECT		15
#define  DTAPI_MOD_1_2			0		// Code rate 1/2
#define  DTAPI_MOD_2_3			1		// Code rate 2/3
#define  DTAPI_MOD_3_4			2		// Code rate 3/4
#define  DTAPI_MOD_4_5			3		// Code rate 4/5
#define  DTAPI_MOD_5_6			4		// Code rate 5/6
#define  DTAPI_MOD_6_7			5		// Code rate 6/7
#define  DTAPI_MOD_7_8			6		// Code rate 7/8
#define  DTAPI_MOD_ROLLOFF_12	0		// Roll-off factor 12%
#define  DTAPI_MOD_ROLLOFF_13	1		// Roll-off factor 13%
#define  DTAPI_MOD_ROLLOFF_15	2		// Roll-off factor 15%
#define  DTAPI_MOD_ROLLOFF_18	3		// Roll-off factor 18%

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Modulation Parameters (OFDM) -.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_MOD_OFDM_BW_MSK	0x0000000F
#define  DTAPI_MOD_OFDM_5MHZ	0x00000001
#define  DTAPI_MOD_OFDM_6MHZ	0x00000002
#define  DTAPI_MOD_OFDM_7MHZ	0x00000003
#define  DTAPI_MOD_OFDM_8MHZ	0x00000004
#define  DTAPI_MOD_OFDM_CO_MSK	0x000000F0
#define  DTAPI_MOD_OFDM_QPSK	0x00000010
#define  DTAPI_MOD_OFDM_QAM16	0x00000020
#define  DTAPI_MOD_OFDM_QAM64	0x00000030
#define  DTAPI_MOD_OFDM_GU_MSK	0x00000F00
#define  DTAPI_MOD_OFDM_G_1_32	0x00000100
#define  DTAPI_MOD_OFDM_G_1_16	0x00000200
#define  DTAPI_MOD_OFDM_G_1_8	0x00000300
#define  DTAPI_MOD_OFDM_G_1_4	0x00000400
#define  DTAPI_MOD_OFDM_IL_MSK	0x0000F000
#define  DTAPI_MOD_OFDM_INDEPTH	0x00001000
#define  DTAPI_MOD_OFDM_NATIVE	0x00002000
#define  DTAPI_MOD_OFDM_MD_MSK	0x000F0000
#define  DTAPI_MOD_OFDM_2K		0x00010000
#define  DTAPI_MOD_OFDM_4K		0x00020000
#define  DTAPI_MOD_OFDM_8K		0x00030000
#define  DTAPI_MOD_OFDM_S48		0x00100000
#define  DTAPI_MOD_OFDM_S49		0x00200000
#define  DTAPI_MOD_OFDM_DIS4849	0x00400000

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Packet Transmit Mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_TXMODE_188		0
#define  DTAPI_TXMODE_204		1
#define  DTAPI_TXMODE_ADD16		2
#define  DTAPI_TXMODE_RAW		3
#define  DTAPI_TXMODE_192		4		// Supported on DTA-102 Firmware Rev >= 2
#define  DTAPI_TXMODE_130		5		// Supported on DTA-102 Firmware Rev >= 7
#define  DTAPI_TXMODE_MIN16		6		// Supported on DTA-100 Firmware Rev >= 5
										// and          DTA-102 Firmware Rev >= 8
#define  DTAPI_TXMODE_MASK		0x0F	// Mask for TxMode without burst flag
#define  DTAPI_TXMODE_BURST		0x10	// Can be OR-ed with one of 188/192/204/ADD16/RAW
#define	 DTAPI_TXMODE_SDI		0x40
#define	 DTAPI_TXMODE_SDI_10BIT	0x80
#define  DTAPI_TXMODE_SDI_RAW	(DTAPI_TXMODE_SDI | DTAPI_TXMODE_RAW)

#define  DTAPI_TXSTUFF_MODE_OFF	0		// Stuff-mode is disabled
#define  DTAPI_TXSTUFF_MODE_ON	1		// Stuff-mode is enabled

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Transmit Polarity -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_TXPOL_NORMAL		0
#define  DTAPI_TXPOL_INVERTED	1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Polarity Control -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_POLARITY_AUTO	0
#define  DTAPI_POLARITY_NORMAL	2
#define  DTAPI_POLARITY_INVERT	3

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Power Mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_POWER_OFF		0
#define  DTAPI_POWER_ON			1

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Reset Mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_FIFO_RESET		0
#define  DTAPI_FULL_RESET		1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Status Flags -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_RX_DMA_PENDING	1
#define  DTAPI_RX_FIFO_OVF		2
#define  DTAPI_RX_SYNC_ERR		4
#define  DTAPI_RX_RATE_OVF		8
#define  DTAPI_RX_TARGET_ERR	16

#define  DTAPI_TX_DMA_PENDING	1
#define  DTAPI_TX_FIFO_UFL		2
#define  DTAPI_TX_SYNC_ERR		4
#define  DTAPI_TX_READBACK_ERR	8
#define  DTAPI_TX_TARGET_ERR	16

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Target Adapter Present -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_NO_CONNECTION	0
#define  DTAPI_DVB_SPI_SINK		1		// For output channels
#define  DTAPI_DVB_SPI_SOURCE	1		// For input channels
#define  DTAPI_TARGET_PRESENT	2
#define  DTAPI_TARGET_UNKNOWN	3

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Transmit Control -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
#define  DTAPI_TXCTRL_IDLE		0
#define  DTAPI_TXCTRL_HOLD		1
#define  DTAPI_TXCTRL_SEND		3

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- RF PLL lock status -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_RFPLL_NO_LOCK	0
#define  DTAPI_RFPLL_LOCK		1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- USB-Speed Modes -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_USB_FULL_SPEED	0
#define  DTAPI_USB_HIGH_SPEED	1

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- SDRAM sizes -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_SDRAM_SIZE_8MB	0
#define  DTAPI_SDRAM_SIZE_16MB	1
#define  DTAPI_SDRAM_SIZE_32MB	2


//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ DtOutpChannel +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// DtLoop class represents an object that provides functionality to loop MPEG-2
// transport-streams from a DtInpChannel to a DtOutpChannel object
//
class DTAPI_API DtLoop {

	friend class DtLoopWorker;

	// Public access functions
public:
	bool  IsStarted();

public:
	DTAPI_RESULT  AttachToInput(DtInpChannel* pDtInp);
	DTAPI_RESULT  AttachToOutput(DtOutpChannel* pDtOutp);
	DTAPI_RESULT  Detach();
	DTAPI_RESULT  DetachFromInput();
	DTAPI_RESULT  DetachFromOutput();
	DTAPI_RESULT  SetStuffingMode(int Mode, int TsRate);
	DTAPI_RESULT  Start(bool Start=true);

	// Internal methods
protected:
	
	

	// Encapsulated data
protected:
	DtLoopWorker* m_pWorker;	// Internal worker class
	
	// Constructor, destructor
public:
	DtLoop();
	virtual ~DtLoop();
};


//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ Global DTAPI Functions +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=

DTAPI_API DTAPI_RESULT  DtapiCheckDeviceDriverVersion(void);
DTAPI_API DTAPI_RESULT  DtapiDeviceScan(int NumEntries, int& NumEntriesResult,
										DtDeviceDesc* DvcDescArr);
DTAPI_API DTAPI_RESULT  DtDeviceDesc2String(DtDeviceDesc* pDvcDesc, int StringType, 
											char* pString, int StringLength);
DTAPI_API DTAPI_RESULT  DtDeviceDesc2String(DtDeviceDesc* pDvcDesc, int StringType, 
											wchar_t* pString, int StringLength);
DTAPI_API DTAPI_RESULT  DtapiDtHwFuncDesc2String(DtHwFuncDesc* pHwFunc, int StringType, 
												 char* pString, int StringLength);
DTAPI_API DTAPI_RESULT  DtapiDtHwFuncDesc2String(DtHwFuncDesc* pHwFunc, int StringType, 
												 wchar_t* pString, int StringLength);
DTAPI_API DTAPI_RESULT  DtapiGetDeviceDriverVersion(int&, int&, int&, int&);
DTAPI_API DTAPI_RESULT  DtapiGetDeviceDriverVersion(int, int&, int&, int&, int&);
DTAPI_API DTAPI_RESULT  DtapiGetVersion(int& LibVersion, int& LibVersionMinor);
DTAPI_API DTAPI_RESULT  DtapiGetVersion(int& LibVersion, int& LibVersionMinor,
										int& LibVersionBugFix, int& LibVersionBuild);
DTAPI_API DTAPI_RESULT  DtapiHwFuncScan(int NumEntries, int& NumEntriesResult,
										DtHwFuncDesc* pHwFuncs);
DTAPI_API DTAPI_RESULT  DtapiInitDtTsIpParsFromIpString(DtTsIpPars& TsIpPars,
														const char* pDstIp,
														const char* pSrcIp);
DTAPI_API DTAPI_RESULT  DtapiInitDtTsIpParsFromIpString(DtTsIpPars& TsIpPars,
														const wchar_t* pDstIp,
														const wchar_t* pSrcIp);
DTAPI_API DTAPI_RESULT  DtapiIpAddr2Str(char* pStr, int StrLen, unsigned char* pIpAddr);
DTAPI_API DTAPI_RESULT  DtapiIpAddr2Str(wchar_t* pStr, int StrLen,
										unsigned char* pIpAddr);
DTAPI_API DTAPI_RESULT  DtapiStr2IpAddr(unsigned char* pIpAddr, const char* pStr);
DTAPI_API DTAPI_RESULT  DtapiStr2IpAddr(unsigned char* pIpAddr, const wchar_t* pStr);

const char*  DtapiResult2Str(DTAPI_RESULT DtapiResult);


//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ Return Codes +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//
// NOTE: ERROR CODES 0x1100-0x12FF ARE RESERVED FOR USE IN THE DTAPIplus

#define  DTAPI_OK					0
#define  DTAPI_E					0x1000
#define  DTAPI_E_ATTACHED			(DTAPI_E + 0)
#define  DTAPI_E_BUF_TOO_SMALL		(DTAPI_E + 1)
#define  DTAPI_E_DEV_DRIVER			(DTAPI_E + 2)
#define  DTAPI_E_EEPROM_FULL		(DTAPI_E + 3)
#define  DTAPI_E_EEPROM_READ		(DTAPI_E + 4)
#define  DTAPI_E_EEPROM_WRITE		(DTAPI_E + 5)
#define  DTAPI_E_EEPROM_FORMAT		(DTAPI_E + 6)
#define  DTAPI_E_FIFO_FULL			(DTAPI_E + 7)
#define  DTAPI_E_IN_USE				(DTAPI_E + 8)
#define  DTAPI_E_INVALID_BUF		(DTAPI_E + 9)
#define  DTAPI_E_INVALID_CGMODE		(DTAPI_E + 10)
#define  DTAPI_E_INVALID_FLAGS		(DTAPI_E + 11)
#define  DTAPI_E_INVALID_MODE		(DTAPI_E + 12)
#define  DTAPI_E_INVALID_RATE		(DTAPI_E + 13)
#define  DTAPI_E_INVALID_SIZE		(DTAPI_E + 14)
#define  DTAPI_E_KEYWORD			(DTAPI_E + 15)
#define  DTAPI_E_NO_DEVICE			(DTAPI_E + 16)
#define  DTAPI_E_NO_DTA_CARD		(DTAPI_E + 16)	// BACKWARD COMPATIBILITY
#define  DTAPI_E_NO_LOOPBACK		(DTAPI_E + 17)
#define  DTAPI_E_NO_SUCH_DEVICE		(DTAPI_E + 18)
#define  DTAPI_E_NO_SUCH_OUTPUT		(DTAPI_E + 19)
#define  DTAPI_E_NO_DT_OUTPUT		(DTAPI_E + 20)
#define  DTAPI_E_NO_TS_OUTPUT		(DTAPI_E + 20)
#define  DTAPI_E_NOT_ATTACHED		(DTAPI_E + 21)
#define  DTAPI_E_NOT_FOUND			(DTAPI_E + 22)
#define  DTAPI_E_NOT_SUPPORTED		(DTAPI_E + 23)
#define  DTAPI_E_PCICARD			(DTAPI_E + 24)	// BACKWARD COMPATIBILITY
#define  DTAPI_E_DEVICE				(DTAPI_E + 24)
#define  DTAPI_E_TOO_LONG			(DTAPI_E + 25)
#define  DTAPI_E_UNDERFLOW			(DTAPI_E + 26)
#define  DTAPI_E_NO_SUCH_INPUT		(DTAPI_E + 27)
#define  DTAPI_E_NO_DT_INPUT		(DTAPI_E + 28)
#define  DTAPI_E_NO_TS_INPUT		(DTAPI_E + 28)
#define  DTAPI_E_DRIVER_INCOMP		(DTAPI_E + 29)
#define  DTAPI_E_INTERNAL			(DTAPI_E + 30)
#define  DTAPI_E_OUT_OF_MEM			(DTAPI_E + 31)
#define  DTAPI_E_INVALID_ROLLOFF	(DTAPI_E + 32)
#define  DTAPI_E_IDLE				(DTAPI_E + 33)
#define  DTAPI_E_INSUF_LOAD			(DTAPI_E + 34)
#define  DTAPI_E_INVALID_BANDWIDTH	(DTAPI_E + 35)
#define  DTAPI_E_INVALID_CONSTEL	(DTAPI_E + 36)
#define  DTAPI_E_INVALID_GUARD		(DTAPI_E + 37)
#define  DTAPI_E_INVALID_INTERLVNG	(DTAPI_E + 38)
#define  DTAPI_E_INVALID_TRANSMODE	(DTAPI_E + 39)
#define  DTAPI_E_INVALID_TSTYPE		(DTAPI_E + 40)
#define  DTAPI_E_NO_IPPARS			(DTAPI_E + 41)
#define  DTAPI_E_NO_TSRATE			(DTAPI_E + 42)
#define  DTAPI_E_NOT_IDLE			(DTAPI_E + 43)
#define  DTAPI_E_INVALID_ARG		(DTAPI_E + 44)
#define  DTAPI_E_NDIS_DRIVER		(DTAPI_E + 45)
#define  DTAPI_E_DST_MAC_ADDR		(DTAPI_E + 46)
#define  DTAPI_E_NO_SUCH_PORT		(DTAPI_E + 47)
#define  DTAPI_E_WINSOCK			(DTAPI_E + 48)
#define  DTAPI_E_MULTICASTJOIN		(DTAPI_E + 49)
#define  DTAPI_E_EMBEDDED			(DTAPI_E + 50)
#define  DTAPI_E_LOCKED				(DTAPI_E + 51)

//+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ IP Tx / Rx Constants +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- IP Tx / Rx Protocol -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_PROTO_UDP			0
#define  DTAPI_PROTO_RTP			1
#define  DTAPI_PROTO_AUTO			2
#define  DTAPI_PROTO_UNKN			2

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- IP Tx / Rx Fec mode -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define  DTAPI_FEC_DISABLE			0
#define  DTAPI_FEC_2D				1

//=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+ General Network Constants +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+

//.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Ethernet Speed -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
#define DTAPI_NWSPEED_AUTO			0		// Set
#define DTAPI_NWSPEED_NOLINK		0		// Get
#define DTAPI_NWSPEED_10MB_HALF		1
#define DTAPI_NWSPEED_10MB_FULL		2
#define DTAPI_NWSPEED_100MB_HALF	3
#define DTAPI_NWSPEED_100MB_FULL	4
#define DTAPI_NWSPEED_1GB_MASTER	5
#define DTAPI_NWSPEED_1GB_SLAVE		6

//=+=+=+=+=+=+=+=+=+=+=+=+=+=+ BACKWARD COMPATIBILITY SECTION +=+=+=+=+=+=+=+=+=+=+=+=+=+=

// Map old PciCard class on DtDevice
typedef class DtDevice PciCard;

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- DtapiHwFunc -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
//
// Structure that describes a hardware function on a PCI card.
//
struct DtapiHwFunc {		
	int  m_nPciBusNumber;		// PCI-bus number
	int  m_nSlotNumber;			// PCI-slot number
	int  m_nTypeNumber;			// PCI-card type number
	int  m_nDeviceId;			// Device ID of PCI card
	int  m_nVendorId;			// Vendor ID of PCI card
	int  m_nSubsystemId;		// Subsystem ID of PCI card
	int  m_nSubVendorId;		// Subsystem Vendor ID of PCI card
	int  m_nHwFuncType;			// Hardware-function type
	int  m_nHwFuncFlags;		// Hardware-function flags
	int  m_nIndexOnCard;		// Index of hardware function
};

//-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- Global functions -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-
DTAPI_API DTAPI_RESULT DtapiPciScan(int NumFuncEntries, int& NumFuncEntriesResult,
									 DtapiHwFunc* pHwFuncs);

//=+=+=+=+=+=+=+=+=+=+=+=+ END OF BACKWARD COMPATIBILITY SECTION +=+=+=+=+=+=+=+=+=+=+=+=+

//if namespace usage is required
#ifdef _USE_NAMESPACE
}       //end of namespace
#endif

#endif //#ifndef __DTAPI_H
