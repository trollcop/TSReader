/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1992-1993  Microsoft Corporation

Module Name:

    devioctl.h

Abstract:

    This module contains

Author:

    Andre Vachon (andreva) 21-Feb-1992


Revision History:


--*/

// begin_winioctl

#ifndef _MYDEVIOCTL_
#define _MYDEVIOCTL_
#ifndef _DEVIOCTL_
#define _DEVIOCTL_

// begin_ntddk begin_nthal
//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//

#define DEVICE_TYPE ULONG

#define FILE_DEVICE_BEEP                0x00000001
#define FILE_DEVICE_CD_ROM              0x00000002
#define FILE_DEVICE_CD_ROM_FILE_SYSTEM  0x00000003
#define FILE_DEVICE_CONTROLLER          0x00000004
#define FILE_DEVICE_DATALINK            0x00000005
#define FILE_DEVICE_DFS                 0x00000006
#define FILE_DEVICE_DISK                0x00000007
#define FILE_DEVICE_DISK_FILE_SYSTEM    0x00000008
#define FILE_DEVICE_FILE_SYSTEM         0x00000009
#define FILE_DEVICE_INPORT_PORT         0x0000000a
#define FILE_DEVICE_KEYBOARD            0x0000000b
#define FILE_DEVICE_MAILSLOT            0x0000000c
#define FILE_DEVICE_MIDI_IN             0x0000000d
#define FILE_DEVICE_MIDI_OUT            0x0000000e
#define FILE_DEVICE_MOUSE               0x0000000f
#define FILE_DEVICE_MULTI_UNC_PROVIDER  0x00000010
#define FILE_DEVICE_NAMED_PIPE          0x00000011
#define FILE_DEVICE_NETWORK             0x00000012
#define FILE_DEVICE_NETWORK_BROWSER     0x00000013
#define FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014
#define FILE_DEVICE_NULL                0x00000015
#define FILE_DEVICE_PARALLEL_PORT       0x00000016
#define FILE_DEVICE_PHYSICAL_NETCARD    0x00000017
#define FILE_DEVICE_PRINTER             0x00000018
#define FILE_DEVICE_SCANNER             0x00000019
#define FILE_DEVICE_SERIAL_MOUSE_PORT   0x0000001a
#define FILE_DEVICE_SERIAL_PORT         0x0000001b
#define FILE_DEVICE_SCREEN              0x0000001c
#define FILE_DEVICE_SOUND               0x0000001d
#define FILE_DEVICE_STREAMS             0x0000001e
#define FILE_DEVICE_TAPE                0x0000001f
#define FILE_DEVICE_TAPE_FILE_SYSTEM    0x00000020
#define FILE_DEVICE_TRANSPORT           0x00000021
#define FILE_DEVICE_UNKNOWN             0x00000022
#define FILE_DEVICE_VIDEO               0x00000023
#define FILE_DEVICE_VIRTUAL_DISK        0x00000024
#define FILE_DEVICE_WAVE_IN             0x00000025
#define FILE_DEVICE_WAVE_OUT            0x00000026
#define FILE_DEVICE_8042_PORT           0x00000027
#define FILE_DEVICE_NETWORK_REDIRECTOR  0x00000028

//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

//
// Define the method codes for how buffers are passed for I/O and FS controls
//

#define METHOD_BUFFERED                 0
#define METHOD_IN_DIRECT                1
#define METHOD_OUT_DIRECT               2
#define METHOD_NEITHER                  3

//
// Define the access check value for any access
//
//
// The FILE_READ_ACCESS and FILE_WRITE_ACCESS constants are also defined in
// ntioapi.h as FILE_READ_DATA and FILE_WRITE_DATA. The values for these
// constants *MUST* always be in sync.
//


#define FILE_ANY_ACCESS                 0
#define FILE_READ_ACCESS          ( 0x0001 )    // file & pipe
#define FILE_WRITE_ACCESS         ( 0x0002 )    // file & pipe
#endif // _DEVIOCTL_
#define MaxPlxDmaChannels 2	// Channels are assigned permanently to transfer directions (if supported by hardware)
#define MaxPidCounters 4
#define TxDir 0
#define RxDir 1
// OptionFlags bit specifications (OR in bit to	OptionFlags configuration value to enable option)
// These bits have nothing to do with any hardware register bits with similar function.
// They are simply an interface to the driver
#define RLSetLow 1 // HotLink RL pin set HIGH during normal receive operation (otherwise it's LOW)
#define FrSz204 2 // Transmit interframe stuffing generated for 204 byte packet (otherwise 188 byte)
#define RxFrSz204 0x40 // Receive 204 byte packets only (otherwise 188 byte or autosize) 
#define PktSyncEnbl 4  // Enable Transport Stream packet synchronization for receive
#define RxAutoSize 8	// Enable firmware to automatically accept/discover 188 or 204 byte packets
#define RxInvPktSync 0x10	// Enable firmware sync to work with inverted sync bytes
#define RxDualSyncMode 0x20	// DualSync mode 


//following options are supported on the new FD card only
#define RxPidFilterEnable 0x80 // Enable Pid Filter Mode 
#define RxPidFilterInvert 0x8000
#define RxModeMask 0x700
#define TxModeMask 0x7000
#define TxClockMask 0x30000
#define SetTxClock(x) ((x)<<16)&TxClockMask
#define GetTxClock(x) ((x)&TxClockMask)>>16
#define ClearTxClock(x) ((x)&(TxClockMask<<16))
#define SetRxMode(x) ((x)<<8)&RxModeMask
#define SetTxMode(x)	((x)<<12)&TxModeMask
#define GetRxMode(x) (((x)&RxModeMask)>>8)
#define ClearRxMode(x) (x) &= ~RxModeMask;
#define GetTxMode(x) (((x)&TxModeMask)>>12)
#define ClearTxMode(x) (x) &= ~TxModeMask;
// following supported by Fpga_Id >4
#define RxTimestampEnable 0x800
#define PcrRestamp 0x40000
// following supported by revised FD card
#define RxRecoverTimeStamp 0x80000
#define TxApplyTimeStamp 0x100000
#define InsertRxNullPackets 0x200000
#define InsertTxNullPackets 0x400000
#define DvbRxOptions (RxDualSyncMode|RxInvPktSync|RxAutoSize|PktSyncEnbl|RLSetLow|RxFrSz204|RxPidFilterEnable|RxPidFilterInvert|RxModeMask|RxTimestampEnable|RxRecoverTimeStamp|InsertRxNullPackets)
#define DvbTxOptions (FrSz204|TxModeMask|TxClockMask|PcrRestamp|PcrRestamp|TxApplyTimeStamp|InsertTxNullPackets)
// following supported by new rev DVB with SMPTE firmware
#define TxBypassEnable 0x800000
// Tx 8 bit character format is TxMode = 0, 10 bit = 1 for these cards
#define RxBypassEnable 0x1000000
// Rx 8 bit character format is RxMode = 0, 10 bit = 1 for these cards
#define TxSdiPllPal 0x4000000

// Receive modes
#define FD_RxNoSync 0	// set by none of the following
#define FD_188Sync 0x1	// set by PktSyncEnbl
#define FD_204Sync 0x2	// set by FrSz204 &&	PktSyncEnbl
#define FD_AutoSync 0x3	  // set by  RxAutoSize
#define FD_Auto204to188 0x4
#define FD_204to188 0x5
// Tx clock modes
#define FD_OnBoardClk 0
#define FD_ExtClk 1
#define FD_RecoveredRecClk 2
// Transmit modes
#define FD_188Byte 0
#define FD_204Byte 1
#define FD_SyncRx188to204 2
#define FILE_DEVICE_DVB_PORT 32768
	 
// Read channel statistics, zero counters, initialize minimums
#define IOCTL_DVB_RD_ST CTL_CODE(FILE_DEVICE_DVB_PORT,2048,METHOD_BUFFERED,FILE_ANY_ACCESS)

// read channel statistics, don't zero counters
#define IOCTL_DVB_RD_ST_NZ CTL_CODE(FILE_DEVICE_DVB_PORT,2050,METHOD_BUFFERED,FILE_ANY_ACCESS)

// read channel configuration
#define IOCTL_DVB_RD_CFG CTL_CODE(FILE_DEVICE_DVB_PORT,2051,METHOD_BUFFERED,FILE_ANY_ACCESS)

// set channel configuration
#define IOCTL_DVB_SET_CFG CTL_CODE(FILE_DEVICE_DVB_PORT,2049,METHOD_BUFFERED,FILE_ANY_ACCESS)

// read current PLX global status condition immediately
#define IOCTL_DVB_GET_STATUS CTL_CODE(FILE_DEVICE_DVB_PORT,2052,METHOD_BUFFERED,FILE_ANY_ACCESS)

// wait for status condition change and then return current PLX global status condition
#define IOCTL_DVB_GET_STATUS_CHANGE CTL_CODE(FILE_DEVICE_DVB_PORT,2057,METHOD_BUFFERED,FILE_ANY_ACCESS)

// read & zero Firmware bytecount register
#define IOCTL_DVB_GET_TX_BYTE_COUNT CTL_CODE(FILE_DEVICE_DVB_PORT,2053,METHOD_BUFFERED,FILE_ANY_ACCESS)

// pulse reframe pin from default state
#define IOCTL_DVB_RESET_REFRAME CTL_CODE(FILE_DEVICE_DVB_PORT,2056,METHOD_BUFFERED,FILE_ANY_ACCESS)

// force completion of active Receive Dma buffer
#define IOCTL_DVB_RX_PURGE CTL_CODE(FILE_DEVICE_DVB_PORT,2054,METHOD_BUFFERED,FILE_ANY_ACCESS)

// Immediately set stuffing in hardware registers
#define IOCTL_DVB_TX_SET_STUFF CTL_CODE(FILE_DEVICE_DVB_PORT,2058,METHOD_BUFFERED,FILE_ANY_ACCESS)

// Reset Rx and Tx and Plx software reset
#define IOCTL_DVB_RESET_ALL CTL_CODE(FILE_DEVICE_DVB_PORT,2059,METHOD_BUFFERED,FILE_ANY_ACCESS)
// Reset Rx functions
#define IOCTL_DVB_RESET_RX CTL_CODE(FILE_DEVICE_DVB_PORT,2060,METHOD_BUFFERED,FILE_ANY_ACCESS)
// Reset Tx functions
#define IOCTL_DVB_RESET_TX CTL_CODE(FILE_DEVICE_DVB_PORT,2061,METHOD_BUFFERED,FILE_ANY_ACCESS)
// Read Rx Pid counters
#define IOCTL_DVB_RX_PID_CTR CTL_CODE(FILE_DEVICE_DVB_PORT,2062,METHOD_BUFFERED,FILE_ANY_ACCESS)
// Put std. messages in system log
#define IOCTL_DVB_RX_PID_STOP_MSG CTL_CODE(FILE_DEVICE_DVB_PORT,2063,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_DVB_RX_PID_START_MSG CTL_CODE(FILE_DEVICE_DVB_PORT,2064,METHOD_BUFFERED,FILE_ANY_ACCESS)
// Access the 27MHZ 32 bit counter
#define IOCTL_DVB_READ_27MHZ_CTR CTL_CODE(FILE_DEVICE_DVB_PORT,2068,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_DVB_WRITE_27MHZ_CTR CTL_CODE(FILE_DEVICE_DVB_PORT,2069,METHOD_BUFFERED,FILE_ANY_ACCESS)


// returned by IOCTL_DVB_RD_ST/IOCTL_DVB_RD_ST_NZ
struct DVBStats {
	ULONG StartDma[MaxPlxDmaChannels] // The channel Dma controller was started
		,NumLost // The ISR could not start the DSP (Deferred Service Procedure)
		,NumExErr // Dma completion error (should never occur !)
		,MaxDspIntCount[MaxPlxDmaChannels] // The DSP for this channel has been started by the ISR
		,NumPciAbts[MaxPlxDmaChannels] // Pci abort interrupt signalled from hardware
		,MaxPend[MaxPlxDmaChannels] // Maximum NumPend occuring since start or last IOCTL_DVB_RD_ST
		,MaxDmaQued[MaxPlxDmaChannels] // Maximum NumDmaQued occuring since start or last IOCTL_DVB_RD_ST
		,MaxQued[MaxPlxDmaChannels] // Maximum NumQued occuring since start or last IOCTL_DVB_RD_ST
		,MinPend[MaxPlxDmaChannels] // Minimum NumPend occuring since start or last IOCTL_DVB_RD_ST
		,MinDmaQued[MaxPlxDmaChannels] // Minimum NumDmaQued occuring since start or last IOCTL_DVB_RD_ST
		,MinQued[MaxPlxDmaChannels] // Minimum NumQued occuring since start or last IOCTL_DVB_RD_ST
		,NumPend[MaxPlxDmaChannels] // Number of buffers active for Dma transfer and pending completion
		,NumDmaQued[MaxPlxDmaChannels] // Number of buffers queued for Dma activation
		,NumQued[MaxPlxDmaChannels] // Number of buffers queued on the device
		,NumInts[MaxPlxDmaChannels] // Number of Dma interrupts generated by the channel
		,NumFifoErrs[MaxPlxDmaChannels]; // Interrupt count at which FIFO overflow occured (0= no errors)
 };
enum BoardType {DvBTransmit,DvBReceive,DvBBoth,DvBNewRev,DvBRxX4,DvBSdi,DvBMm,DvBMmR};

// set/returned by IOCTL_DVB_SET_CFG/IOCTL_DVB_RD_CFG
struct DVBCfg {
	BOOLEAN DirSupported[MaxPlxDmaChannels]; // Hardware supports transfer in this direction
	int OptionFlags // Bit flags controlling additional BOOLEAN selections
		,MaxTransferSize[MaxPlxDmaChannels] // Maximum Dma transfer size allowed for this channel
		,MaxBuffers[MaxPlxDmaChannels] // Maximum number of buffers that may be simultaneously active on this
		// channel (additional buffers will be queued)
		,FifoAe[MaxPlxDmaChannels] // Hardware almost empty limit for FIFO's (do not change)
		,FifoAf[MaxPlxDmaChannels] // Hardware almost full limit for FIFO's (do not change)
		,Stuffing  // Hardware transmit stuffing control parameter
		,FT0,FT1 // Fine tuning parameters
		,FpgaId
		,RxPidMonitor[MaxPidCounters]
		,NumPidCounters
		,HighByteStuff	   // Bits 8 -> 15 of byte stuffing value
		,PlxDmaThreshold; 
 };

// returned by IOCTL_DVB_GET_STATUS/IOCTL_DVB_GET_STATUS_CHANGE
struct DVBStatus {
	ULONG CFGStatus  // Status conditions from register 1	
		,CFG2Status	// Status conditions from register 2
		,RxCFGStatus  // status condition from the the Rx Cmd/Sts register
		,TxCFGStatus // status condition from the the Tx Cmd/Sts register
		,CFGSetCmd	// Value written to CFG register							
		,CFG2SetCmd	// Value written to CFG2 register
		,RxCFGSetCmd  // value written to the Rx Cmd/Sts register
		,TxCFGSetCmd;  // value written to the Tx Cmd/Sts register
		enum BoardType BrdTyp;  // type of board
		__int64 CrMaturity;
		ULONG FTSetCmd  // Value written to fine tune register
		,NumStsInt[MaxPlxDmaChannels]; // Number of status change interrupts processed
};
// The following bits are accumulated since the last status read IoCtl. If On, this means
// the interrupt or condition has occured at least once during this period.
// Cfg status bits (in CFGStatus and CFG2Status condition register values)
// in CFGStatus
// Cfg2 status bits (in CFG2Status register value)
#define AcquireSync 0x40	// DvB firmware has acquired packet sync (ie. byte 0x47 received)
#define LossOfSync 0x4	// DvB firmware has lost sync (ie. illegal packet size)
#define CarrierDetect 0x200  // DvB firmware carrier detect change in status
#define DisabledFifoInterrupt 0x80	 // driver has disabled Fifo interrupt source (it's hung)
#define DisabledLocalInterrupt 0x800 // driver has disabled Local interupt source (it's hung)
#define FD_RxOvrIntStatus 0x20000
#define FD_LossOfSyncIntStatus 0x40000
#define FD_AcquireSyncIntStatus 0x80000
#define FD_CarrierDetectIntStatus 0x100000
#define RFD_PktNotMature 0x4000000
#define RFD_RXNoSignal 0x8000000	  // no black burst signal

//	These bits are CURRENT status conditions in CFGStatus, valid at the time of the IoCtl return.
// Original HD card pair
#define RESET_RX_FIFO_OVI 0x4000000	// Fifo overrun interrupt has occured
#define SyncStatus 0x20	// DvB firmware is in sync
#define AutoSize204 0x8	//	DvB firmware discovered 204 byte packets received
#define CarrierDetectStatus 0x400  // Dvb carrier detected status
#define RxPacketsStopped	0x8000	// Packets have stopped being received (for at least .5 sec)

// FD card
#define FD_RxOvrStatus 0x200	 // Rx overflow
#define FD_PktSyncStatus 0x400  // Packet sync
#define FD_CarrierDetectStatus 0x1000
#define FD_204PktSizeStatus 0x1000000	 // 204 byte Rx packets

// These bits are fixed, depending on hardware programming
#define PktSize204 0x2	// 204 byte packets have been specified for receive
#define AutoSizeEnbl 0x10 // DvB firmware will discover packet size
#define EnblPktSync 0x1 // DvB firmware search for packet synchronization is enabled
// the following is valid for the WDM driver ONLY
// {AB48D685-6AA2-11d3-8EC3-525400E402A7}
DEFINE_GUID(DVBLINK_GUID,0xab48d685, 0x6aa2, 0x11d3, 0x8e, 0xc3, 0x52, 0x54, 0x0, 0xe4, 0x2, 0xa7);

#endif

