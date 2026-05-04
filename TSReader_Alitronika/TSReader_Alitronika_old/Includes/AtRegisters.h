// AtRegisters.h: interface for the CAtRegisters class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ATREGISTERS_H__A71704CB_5507_4A48_A386_F9A9A9D9DB1D__INCLUDED_)
#define AFX_ATREGISTERS_H__A71704CB_5507_4A48_A386_F9A9A9D9DB1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "AtRegDefs.h"

class CATBoard;
class CAtRegisters;

#define SDRAMFIFO_SZ	(8*1024*1024)	///< in FIFO Bytes

#ifdef _WINDOWS
#include <PSHPACK1.H>
#endif

/** @defgroup public_api_regs	The AT device registers.
 */
/*@{*/

/**
 *	The (accessible) registers of an AT Device.<br>
 *	see: \ref _ATREGISTRY
 */
struct	ATDV_API_API _ATREGISTRY
{
	/* ------------- READ / WRITE REGISTERS ----------------- */
	DWORD	m_RecordConfig;			///< Input configuration register (R/W)
	DWORD	m_PlayConfig;			///< Output configuration register (R/W)

	/**
	 * The Play bitrate smoother regulates the bitrate of the Play-from-PC action.
	 * It is equivalent to the File output bitrate. If the data comes
	 * from an ASI or Parallel input, it can be set to the highest possible
	 * bitrate, since that data is already at a correct rate.<br>
	 * Expressed in bits per second. (R/W)
	 * \note The \ref m_OutputBitrate is used to set the actual output bitrate.
	 */
	DWORD	m_PlayBitrate;			///< Play bitrate (bits per second).

	/**
	 * The bitrate at the output of Play or a Hardware generated stream.
	 * Remux is performed if NULL packet insertion is on. (R/W)
	 * Otherwise the m_OutputBitrate must be equal to the \ref m_PlayBitrate.<br>
	 * Expressed in bits per second.\n
	 * AT-devices realize Re-muxing in hardware. No software or transfer overhead is introduced.
	 */
	DWORD	m_OutputBitrate;		///< Output bitrate after optional re-mux (bits per second).

	/**
	 *	The size of the output burst length 0~188 / 0~204.\n
	 *	The DVB-ASI standards specify that data is transmitted at a constant bit rate
	 *	of 270 Mbit/s. Considering the 8 bit data bytes are 8b/10b encoded, this correspond
	 *	to fixed symbol rate of 27 Msymbol/s.\n
	 *	But the transport bit rate could be much less, special character, K28.5 (Comma) is
	 *	used as "Stuffing" to make up the difference.
	 *	The transport packets may be transmitted as a burst of contiguous bytes or as
	 *	individual bytes spread out in time.\n
	 *	Normally the later is used, since spreading out the data in time has less demand on
	 *	the receiver's input buffer.\n
	 *	Alitronika boards use this mode of transmission as default.\n
	 *	m_BurstSize defines the number of bytes sent as burst block.
	 *	By setting the this to 1, data comes out smoothed (a burst of 1).\n
	 * (Read-only)
	 *	\par
	 *	Applicable to DVB_ASI mode only.
	 */
	DWORD	m_BurstSize;			///< Output burst size (1..Packet size).

	DWORD	m_AudioChanSelectBits0;	///< Selected audio channels per bits (R/W)
	DWORD	m_AudioChanSelectBits1;	///< Selected audio channels per bits (R/W)

	/* ------------- READ-ONLY REGISTERS ----------------- */

	DWORD	m_Status;				///< Status during Recordings (Read-only)

	/**
	 * Bitrate measurement is done by counting the bytes (8 bits) in a
	 * PCR interval. The interval time is represented in m_BitrateTimeInterval.
	 * The bitrate is calculated in software as:<br>
	 * 8 * m_BitrateBytesCount * 27000000/m_BitrateTimeInterval.<br>
	 * If the source stream does not contain PCR's, this measurements is done
	 * using an internal time measurement.\n
	 * (Read-only)
	 * \see m_BitrateTimeInterval
	 */
	DWORD	m_BitrateBytesCount;	///< Bytes received during the measurement time.

	/**
	 * Bitrate measurement is done by counting the bytes (8 bits) in a
	 * PCR interval. The interval time is represented in m_BitrateTimeInterval.
	 * The bitrate is calculated in software as:<br>
	 * 8 * m_BitrateBytesCount * 27000000/m_BitrateTimeInterval.<br>
	 * If the source stream does not contain PCR's, this measurements is done
	 * using an internal time measurement.\n
	 * (Read-only)
	 * \see m_BitrateBytesCount
	 */
	DWORD	m_BitrateTimeInterval;	///< Measurement time for receiving bytes.

	DWORD	m_DataErrorCnt;			///< Cumulative data errors	(Read-only)
	DWORD	m_SyncErrorCnt;			///< Cumulative sync errors	(Read-only)

	/**
	 * Recording FIFO content in Bytes (16-bit data) (0..\ref SDRAMFIFO_SZ)	(Read-only)\n
	 * The USB driver has an extra buffer, that is not included in this amount. See \ref CATBoard::GetFifoStatus.\n
	 * Example:
	 * \code
	// Now read the SDRAM FIFO counter and set it in the Progress Control.
	// The indication here is inverted to show the FREE FIFO space.
	m_SdramFillCtrl.SetPos(100 * (SDRAMFIFO_SZ - Registers.m_RecFifoContent) / SDRAMFIFO_SZ);
	 * \endcode
	 */
	DWORD	m_RecFifoContent;		///< Recording FIFO content in Bytes (0..SDRAMFIFO_SZ)
	/**
	 * Play FIFO content in Bytes (16-bit data) (0..\ref SDRAMFIFO_SZ)	(Read-only)
	 * The USB driver has an extra buffer, that is not included in this amount. See \ref CATBoard::GetFifoStatus.\n
	 * Example:
	 * \code
	// Now read the SDRAM FIFO counter into the Progress Control.
	m_SdramFillCtrl.SetPos(100 * Registers.m_PlayFifoContent / SDRAMFIFO_SZ);
	 * \endcode
	 */
	DWORD	m_PlayFifoContent;		///< Play FIFO content in Bytes (0..SDRAMFIFO_SZ)

	DWORD	m_AudioChanAvailBits;	///< Available audio channels per bits (Read-only)

	DWORD	m_PidTblSz;				///< Read only register with the max size of the PID table (Read-only).

	/* ------------- END OF REGISTER SET ----------------- */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	BYTE	m_RegEnd;				///< Dummy to mark the end of the register sets 
#endif

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	int		m_DvbSmpte;				///< Temp storage of Dvb/Smpte/Raw for Record/Monitor
#endif

//@internal
//@{
protected:
	TSysMutex m_mutex;		///< mutex to lock the structure.
	friend class CATBoard;
	friend class CAtRegisters;
//@}
};
typedef struct _ATREGISTRY ATREGISTRY;

/*@}*/

#define ATREGRWSIZE (7*sizeof(DWORD))

#ifdef _WINDOWS
#include <POPPACK.H>
#endif

/**@ingroup public_api*/
/*@{*/

/** 
 *	CAtRegisters is the ATBoard register access regulator.\n
 *	To access the registers of an ATBoard, instantiate a local CAtRegisters using
 *	the ATBoard that contains it.\n
 *	For as long as the object exists, the registers
 *	of the specified ATBoard will remain locked to the thread.\n
 *	The application should create an auto variable (within the function)
 *	to create a locked access to the registers.
 \code
	CAtRegisters RegAcc(pBoard);

	RegAcc.m_RecordConfig &= ~(AT_RCONF_SMP);	// SMPTE bit off
	RegAcc.m_RecordConfig |= AT_RCONF_DVB;		// DVB bit on

	pBoard->UpdateRegisters;
 \endcode
 */

class ATDV_API_API CAtRegisters  
{
//@{
//@internal
protected:
	CATBoard * const m_pOwner; ///< The board object of which to access the registers.
//@}
public:
	/** Constructor
	 *	@param pOwner The board object of which to access the registers.
	 */
	CAtRegisters(CATBoard *pOwner);
	virtual ~CAtRegisters();
	void Reset();

public:
	operator ATREGISTRY&();
	ATREGISTRY &m_Registers;		///< The registers of the assigned board.

};
/*@}*/

#endif // !defined(AFX_ATREGISTERS_H__A71704CB_5507_4A48_A386_F9A9A9D9DB1D__INCLUDED_)
