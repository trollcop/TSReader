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

#define SDRAMFIFO_SZ	(4*1024*1024)	///< in FIFO Words

#ifdef _WINDOWS
#include <PSHPACK1.H>
#endif

/** @defgroup public_api_regs	The AT device registers.
 */
/*@{*/

/**
 *	The (accessible) registers of an AT Device.\n
 *	see: \ref _ATREGISTRY
 */
struct	ATDV_API_API _ATREGISTRY
{

	DWORD	m_RecordConfig;			///< Recording settings 
	DWORD	m_PlayConfig;			///< Playback / Hardwareware generation settings
	DWORD	m_Bitrate;				///< Bitrate for Play or Hardwareware generation (bits per second)
	DWORD	m_Status;				///< Status during Recordings

	DWORD	m_DataErrorCnt;			///< Cumulative data errors
	DWORD	m_SyncErrorCnt;			///< Cumulative sync errors
	/**
	 * Time between 10 sync's; see \ref ExInpBitrate
	 */
	DWORD	m_PckPulseCnt;			///< Time between 10 sync's

	DWORD	m_AudioChanAvailBits;	///< Available audio channels per bits 
	DWORD	m_AudioChanSelectBits0;	///< Selected audio channels per bits 
	DWORD	m_AudioChanSelectBits1;	///< Selected audio channels per bits

	/**
	 * Recording FIFO content in WORDs (16-bit data) (0..\ref SDRAMFIFO_SZ)\n
	 * The USB driver has an extra buffer, that is not included in this amount.\n
	 * Example:
	 * \code
	// Now read the SDRAM FIFO counter in the Progress Control.
	// The indication here is inverted to show the FREE FIFO space.
	m_SdramFillCtrl.SetPos(100 * (SDRAMFIFO_SZ - Registers.m_RecFifoContent) / SDRAMFIFO_SZ);
	 * \endcode
	 */
	DWORD	m_RecFifoContent;		///< Recording FIFO content in bytes (0..SDRAMFIFO_SZ)
	/**
	 * Play FIFO content in WORDs (16-bit data) (0..\ref SDRAMFIFO_SZ)
	 * The USB driver has an extra buffer, that is not included in this amount.\n
	 * Example:
	 * \code
	// Now read the SDRAM FIFO counter into the Progress Control.
	m_SdramFillCtrl.SetPos(100 * Registers.m_PlayFifoContent / SDRAMFIFO_SZ);
	 * \endcode
	 */
	DWORD	m_PlayFifoContent;		///< Play FIFO content in bytes (0..SDRAMFIFO_SZ)

#ifndef DOXYGEN_SHOULD_SKIP_THIS
	BYTE	m_RegEnd;				///< Dummy to mark the end of the register sets 
#endif

	WORD	m_PidTbl[AT_REG_PIDSZ];	///< The pid table in the FPGA

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
