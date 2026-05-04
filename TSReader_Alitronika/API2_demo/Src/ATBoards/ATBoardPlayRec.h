/*! Time-stamp: <@(#)ATBoardPlayRec.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardPlayRec.h
*
*  Project : ATDemoApp. Testappliction for Alitronika devices.
*			  Supports Linux and windows operating System.
*
*			: Devices supported (USB and PCI)
*				-AT20, AT200
*				-AT30, AT30R1, AT300
*				-AT4, AT40, AT40X, AT40R1, AT400
*
*  Package : 
*
*  Company : Engineering Spirit
*
*  Author  : P.Hoogervorst                              Date: 07/05/2007
*
*  Purpose : Implementation of methods for class 
*
*********************************************************************
* Version History:
*
* V 0.10  07/05/2007  BN : First Revision
*
*********************************************************************
*/

#ifndef __ATBOARD_PLAY_REC_H__
#define __ATBOARD_PLAY_REC_H__

/**********************************************************************************************************/
// includes
#include <AtBoardManager.h>
#include <ATBoard.h>
#include <frontend.h>

/**********************************************************************************************************/
// defines
#define SET_BITMASK(port, bitmask)	((port) |=  ((u32)(bitmask)))
#define CLR_BITMASK(port, bitmask)	((port) &= ~((u32)(bitmask)))
#define TST_BITMASK(port, bitmask)	((port) &   ((u32)(bitmask)))

#define DEFAULT_BITRATE		((u32)40000000)
#define TSDATA_BUFFERSIZE	((u32)1024*1024)

/**********************************************************************************************************/
// typedefs
typedef enum _EIOMode
{
	IOMODE_DVB,			///< The in- or output mode is DVB
	IOMODE_SMPE,		///< The in- or output mode is SMPTE
	IOMODE_RAW,			///< The in- or output mode is RAW
	IOMODE_SIZE
} EIOMode;

typedef enum _EIOSel
{
	IOSEL_NON,			///< The serial and SPI port are disabled 
	IOSEL_SER,			///< The serial port is enabled
	IOSEL_SPI,			///< The SPI port is enabled
	IOSEL_SERSPI		///< Both the serial and the SPI port are enabled (only for output)
} EIOSel;

typedef enum _ETSPSize
{
	TSPSIZE_188,		///< transports stream packet size is 188 bytes
	TSPSIZE_188P16,		///< transports stream packet size is 188 bytes and 16 bytes are added (only for output)
	TSPSIZE_204			///< transports stream packet size is 204 bytes
} ETSPSize;

typedef enum _EIOSpiMode
{
	SPIMODE_FCLO_188,	///< fixed 188 packet size at fixed 27MHz output clock 
	SPIMODE_VCLO_188,	///< fixed 188 packet size at variable output clock (default)
	SPIMODE_FCLO_204,	///< variable 204 packet or 188 packet size at fixed 27MHz output clock
	SPIMODE_VCLO_204	///< variable 204 packet or 188 packet size variable output clock
} EIOSpiMode;

/**********************************************************************************************************/
// class
class CATBoardPlayRec
{
// constructor
public:
	CATBoardPlayRec			   (void);
public:
// destructor
	virtual ~CATBoardPlayRec   (void);

// public functions
public:

	/*!
	 * open an ATboard
	 *
	 * @param BoardOpenParams : Parameter struct to open a ATboard
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	BOOL BoardOpen			   (SBoardOpenParams BoardOpenParams);
	
	/*!
	 * close an ATboard
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	BOOL BoardClose			   (void);
	
	/*!
	 * Get the pointer to the current opened ATboard
	 *
	 * @return CATBoard*  : pointer to the current opened ATBoard
	 */
	CATBoard* GetCurBoard	   (void);
	
	/*!
	 * Initialize play
	 *
	 * @param nFileBitrate : Bitrate of the file to play 
	 * @param bORemux : TRUE: enable bitrate remultiplexing, FALSE: disable bitrate remultiplexing
	 * @param nOutputBitrate : Output bitrate. Only used if bORemux is enabled, then it must be equal or higher as the nFileBitrate. If bORemux is disabled, Output nOutputBitrate must be set to MAX_ASIBITRATE
	 * @param OSel : Selects which output ports are enabled
	 * @param OMode : Selects the output standard
	 * @param OSpiMode : Selects the SPI output mode
	 * @param OTsPSize : Selects the output transport packet size
	 * @param bOHTP : TRUE: Hardware Generated Transport stream is enabled, FALSE: Hardware Generated Transport stream is disabled.
	 * @param bOCTP : TRUE: If Hardware Generated Transport stream is enbaled, counter packets are generated, FALSE, null packets are generated.
	 * @param nBurstSize : Output burst size. (Only for the serial ASI output)
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	virtual BOOL InitPlay	   (u32			nFileBitrate,
								BOOL		bORemux			= FALSE,
								u32			nOutputBitrate	= MAX_ASIBITRATE,
								EIOSel		OSel			= IOSEL_SER,
								EIOMode		OMode			= IOMODE_DVB,
								EIOSpiMode	OSpiMode		= SPIMODE_VCLO_188,
								ETSPSize	OTsPSize		= TSPSIZE_188,
								BOOL		bOHTP			= FALSE,
								BOOL		bOCTP			= FALSE,
								BYTE		nBurstSize		= 1);
		
	/*!
	 * Initialize play for derived class. If the initialization of the play must be changed or expanded by a derived class,
	 * The derived class should do that in this function
	 *
	 * @param nFileBitrate : Bitrate of the file to play 
	 * @param bORemux : TRUE: enable bitrate remultiplexing, FALSE: disable bitrate remultiplexing
	 * @param nOutputBitrate : Output bitrate. Only used if bORemux is enabled, then it must be equal or higher as the nFileBitrate. If bORemux is disabled, Output nOutputBitrate must be set to MAX_ASIBITRATE
	 * @param OSel : Selects which output ports are enabled
	 * @param OMode : Selects the output standard
	 * @param OSpiMode : Selects the SPI output mode
	 * @param OTsPSize : Selects the output transport packet size
	 * @param bOHTP : TRUE: Hardware Generated Transport stream is enabled, FALSE: Hardware Generated Transport stream is disabled.
	 * @param bOCTP : TRUE: If Hardware Generated Transport stream is enbaled, counter packets are generated, FALSE, null packets are generated.
	 * @param nBurstSize : Output burst size. (Only for the serial ASI output)
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	virtual BOOL _InitPlay	   (u32			nFileBitrate,
								BOOL		bORemux,
								u32			nOutputBitrate,
								EIOSel		OSel,
								EIOMode		OMode,
								EIOSpiMode	OSpiMode,
								ETSPSize	OTsPSize,
								BOOL		bOHTP,
								BOOL		bOCTP,
								BYTE		nBurstSize)		{return TRUE;}

	
	/*!
	 * Play a file
	 *
	 * @param pFilename : The name of the file to play
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	BOOL PlayFile			   (char * pFilename);

	/*!
	 * Initialize recording.
	 *
	 * @param IMode : Selects the input standard
	 * @param ISel : Selects which input port is enabled
	 * @param bILoop : TRUE: enable loopthrough, FALSE: disable loopthrough
	 * @param bIPassThroughSer : TRUE: enable passthrough to serial output, FALSE: disable passthrough
	 * @param bIPassThroughSpi : TRUE: enable passthrough to SPI output, FALSE: disable passthrough
	 * @param bITimeStamp : TRUE: enable timestamp, FALSE: disable loopthrough
	 * @param ISpiMode : Selects the SPI input mode
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	virtual BOOL InitRec	   (EIOMode		IMode			= IOMODE_DVB,
								EIOSel		ISel			= IOSEL_SER,
								BOOL		bILoop			= FALSE,
								BOOL		bIPassThroughSer= FALSE,
								BOOL		bIPassThroughSpi= FALSE,
								BOOL		bITimeStamp		= FALSE,
								EIOSpiMode	ISpiMode		= SPIMODE_VCLO_188);

	/*!
	 * Initialize recording for derived class. If the initialization of the recording must be changed or expanded by a derived class,
	 * The derived class should do that in this function
	 *
	 * @param IMode : Selects the input standard
	 * @param ISel : Selects which input port is enabled
	 * @param bILoop : TRUE: enable loopthrough, FALSE: disable loopthrough
	 * @param bIPassThroughSer : TRUE: enable passthrough to serial output, FALSE: disable passthrough
	 * @param bIPassThroughSpi : TRUE: enable passthrough to SPI output, FALSE: disable passthrough
	 * @param bITimeStamp : TRUE: enable timestamp, FALSE: disable loopthrough
	 * @param ISpiMode : Selects the SPI input mode
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	virtual BOOL _InitRec	   (EIOMode		IMode,
								EIOSel		ISel,
								BOOL		bILoop,
								BOOL		bIPassThroughSer,
								BOOL		bIPassThroughSpi,
								BOOL		bITimeStamp,
								EIOSpiMode	ISpiMode)		{return TRUE;}

	/*!
	 * Record a file
	 *
	 * @param pFilename : Filename to store the recording data in
	 * @param nFileSize : Requested size of the file to record
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	BOOL RecFile			   (char * pFilename, u32 nFileSize);

	/*!
	 * get status of the input transport stream
	 *
	 * @param &TsPSize : Get the input transport stream packet size
	 * @param &bInSync : Get the sync status (TRUE: in sync, FALSE: not in sync)
	 * @param &bCarrierDetect : Get the carrier detect status (TRUE: carrier is detected, FALSE: carrier is not detected)
	 * @param &bLocked : Get the lock status (TRUE: in lock, FALSE: not in lock)
	 * @param &nBitrate : Get the bitrate
	 *
	 * @return BOOL  : TRUE on success, else FALSE
	 */
	BOOL GetRecStatus	  (ETSPSize &TsPSize, BOOL &bInSync, BOOL &bCarrierDetect, BOOL &bLocked, u32 &nBitrate);


// private functions:
private:

	typedef enum _EProgFpgaMode
	{
		PROG_FPGA_DEFAULT,
		PROG_FPGA_PLAY,
		PROG_FPGA_REC,
		PROG_FPGA_MOD,
		PROG_FPGA_ENUM_SIZE
	} EProgFpgaMode;

	/*!
	* Programs the FPGA file to the device
	*
	* @param bPlay : TRUE: play file is programmed, FALSE: recording file is programmed
	* @param Mode : Selects the input or output standard
	*
	* @return BOOL  : TRUE on success, else FALSE
	*/
	BOOL ProgFpga(EProgFpgaMode ProgFpgaMode, EIOMode Mode);

// public variables
public:
	CATBoard* m_pAtBoard;
	char *m_pDevName;

};

#endif // __ATBOARD_PLAY_REC_H__
