/*! Time-stamp: <@(#)ATDemo.h   07/05/2007 - 15:46:04   P.Hoogervorst>
 *********************************************************************
 *  @file   : ATDemo.h
 *
 *  Project : ATTestApp. Testappliction for Alitronika devices.
 *			  Supports Linux and windows operating System.
 *
 *			: Devices supported (USB and PCI)
 *				-ATDEMO
 *				-AT20, AT200
 *				-AT30, AT30R1, AT300
 *				-AT4, AT40, AT40X, AT40R1, AT400
 *				-AT70, AT72, AT73, AT700, AT720, AT730
 *				-AT8, AT80, AT82, AT800, AT820, AT1800
 *				-AT2700, AT2800, AT3800, AT2780
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

#ifndef __AT_TESTAPP_H__
#define __AT_TESTAPP_H__

/**********************************************************************************************************/
// includes
#include <common.h>

#include "../Src/Support/msg.h"
#include "../Src/Support/tsrate.h"

#include "../Src/Sys/SysFunc.h"

#include "../Src/ATBoards/ATBoardPlayRec.h"
#include "../Src/ATBoards/ATBoardTunDVBT.h"
#include "../Src/ATBoards/ATBoardTunDVBCAnnexA.h"
#include "../Src/ATBoards/ATBoardTunDVBCAnnexB.h"
#include "../Src/ATBoards/ATBoardModRfIf.h"
#include "../Src/ATBoards/ATBoardModDVBT.h"
#include "../Src/ATBoards/ATBoardModDVBC.h"

/**********************************************************************************************************/
// defines
#define	MAX_TMP_STR_LEN		256

/**********************************************************************************************************/
// typedefs

/** Play options array
*	Holds the options specified in the argument list
*/
typedef struct _SPlayOpt
{
	BOOL	m_bReMux;				///< Play re-multiplexing enable
	u32		m_dFileBitrate;			///< Play file bit rate
	u32		m_dOutputBitrate;		///< Play output bit rate
} SPlayOpt;

/** Record options array
*	Holds the options specified in the argument list
*/
typedef struct _SRecOpt
{
	u32		m_nFileSize;			///< Requested Record file size
} SRecOpt;

/** DVB-C AnnexA tuner options array
*	Holds the options specified in the argument list
*/
typedef struct _STunDVBCAnnexAOpt
{
	u32							m_nFrequency;		///< Requested tuning frequency (in Hz)
	u32							m_nSymbolRate;		///< Requested symbol rate (in Symbols/s)
	src_spectral_inversion_t	m_Invert;			///< Requested inversion setting
	src_modulation_t			m_Modulation;		///< Requested modulation setting
} STunDVBCAnnexAOpt;

/** DVB-C AnnexB tuner options array
*	Holds the options specified in the argument list
*/
typedef struct _STunDVBCAnnexBOpt
{
	u32							m_nFrequency;		///< Requested tuning frequency (in Hz)
	src_modulation_t			m_Modulation;		///< Requested modulation setting
} STunDVBCAnnexBOpt;

/** DVB-T tuner options array
*	Holds the options specified in the argument list
*/
typedef struct _STunDVBTOpt
{
	u32							m_nFrequency;		///< Requested tuning frequency (in Hz)
	src_bandwidth_t				m_BandWidth;		///< Requested modulation bandwidth
	BOOL						m_bBandWidthGroup;	///< FALSE for AT8, AT80, AT800, TRUE for AT82 and AT820
} STunDVBTOpt;

/** DVB-T modulator options array
*	Holds the options specified in the argument list
*/
typedef struct _SModDVBTOpt
{
	BOOL						m_bRfIfSel;				///< selects if RF output is selected or IF output. 1: RF is selected, 0: IF is selected
	s32							m_iIfFrequency;			///< (absolute) IF frequency in Hz for. (ranges: 35-37MHz, 69-71MHz)
	u32							m_nRfFrequency;			///< (absolute) RF frequency in Hz for. (range: 50-1000MHz)
	s32							m_iRfoutputLevel;		///< (absolute) output level for the modulators in 0.1dBm units. (range: -35 to +2dBm in 0.5dBm stepsize)
	src_modulation_t			m_Modulation;			///< modulation type
	src_transmit_mode_t			m_TranMode;				///< transmission mode selection
	src_guard_interval_t		m_GuardInt;				///< guard interval selection
	src_code_rate_t				m_FECCodeRate;			///< high priority stream code rate
	src_bandwidth_t				m_Bandwidth;			///< channel bandwidth selection
	src_spectral_inversion_t	m_Inversion;			///< spectral inversion selection
	BOOL						m_bDvbHEn;				///< enable dvb-h mode in ofdm fpga core
	BOOL						m_bDvbHTimeSlice;		///< enable high priority time slicing for dvb-h
	BOOL						m_bDvbH_MPE_FEC;		///< enable high priority MPE FEC for dvb-h
	BOOL						m_bDvbHIndepthInt;		///< enable indepth interleaver for dvb-h
	u16							m_nDvbHCellId;			///< cell id number for dvb-h
} SModDVBTOpt;

/** DVB-C modulator options array
*	Holds the options specified in the argument list
*/
typedef struct _SModDVBCOpt
{
	BOOL						m_bRfIfSel;				///< selects if RF output is selected or IF output. 1: RF is selected, 0: IF is selected
	s32							m_iIfFrequency;			///< (absolute) IF frequency in Hz for. (ranges: 35-37MHz, 69-71MHz)
	u32							m_nRfFrequency;			///< (absolute) RF frequency in Hz for. (range: 50-1000MHz)
	s32							m_iRfoutputLevel;		///< (absolute) output level for the modulators in 0.1dBm units. (range: -35 to +2dBm in 0.5dBm stepsize)
	src_modulation_t			m_Modulation;			///< modulation type
	src_filter_rolloff_t		m_FltRollOff;			///< set filter roll off factor
	src_interleaver_t			m_Interleaver;			///< set interleaver
	src_spectral_inversion_t	m_Inversion;			///< set spectral inversion
	u32							m_nSymbolRate;			///< symbol rate in Symbols per second
	BOOL						m_bAnnexBEn;			///< 1: enable annexB, 0: disable annexB (annexA/C mode enabled)
} SModDVBCOpt;

/**********************************************************************************************************/
// functions

/*!
 * Play a file. This function returns when the complete file is played, or on error
 *
 * @param pATBoardCtrl : Pointer to the class that controls the ATBoard
 * @param pFilename : FileName of file to play
 * @param Options : Play options array
 *
 * @return BOOL  : TRUE on success, FALSE on error
 */
BOOL PlayFile(CATBoardPlayRec *pATBoardCtrl, char *pFilename, SPlayOpt Options);

/*!
 * Record a file. This function returns when the complete file is recorded, or on error
 *
 * @param pATBoardCtrl : Pointer to the class that controls the ATBoard
 * @param pFilename : FileName of file to record the data to
 * @param Options : Record options array
 *
 * @return BOOL  : TRUE on success, FALSE on error
 */
BOOL RecordFile(CATBoardPlayRec *pATBoardCtrl, char *pFilename, SRecOpt Options);

/*!
* Set the DVB-T tuner
*
* @param pATBoardCtrl : Pointer to the class that controls the ATBoard
* @param Options :  DVB-T tuner options array
*
* @return BOOL  : TRUE on success, FALSE on error
*/
BOOL SetTunDVBT(CATBoardTunDVBT *pATBoardCtrl, STunDVBTOpt Options);

/*!
* Set the DVB-C AnnexA tuner
*
* @param pATBoardCtrl : Pointer to the class that controls the ATBoard
* @param Options :  DVB-C AnnexA tuner options array
*
* @return BOOL  : TRUE on success, FALSE on error
*/
BOOL SetTunDVBCAnnexA(CATBoardTunDVBCAnnexA * pATBoardCtrl, STunDVBCAnnexAOpt Options);

/*!
* Set the DVB-C AnnexB tuner
*
* @param pATBoardCtrl : Pointer to the class that controls the ATBoard
* @param Options :  DVB-C AnnexB tuner options array
*
* @return BOOL  : TRUE on success, FALSE on error
*/
BOOL SetTunDVBCAnnexB(CATBoardTunDVBCAnnexB *pATBoardCtrl, STunDVBCAnnexBOpt Options);

/*!
* Set the DVB-T modulator
*
* @param pATBoardCtrl : Pointer to the class that controls the ATBoard
* @param Options :  DVB-T modulator options array
*
* @return BOOL  : TRUE on success, FALSE on error
*/
BOOL SetModDVBT(CATBoardModDVBT *pATBoardCtrl, SModDVBTOpt Options);

/*!
* Set the DVB-C modulator
*
* @param pATBoardCtrl : Pointer to the class that controls the ATBoard
* @param Options :  DVB-C modulator options array
*
* @return BOOL  : TRUE on success, FALSE on error
*/
BOOL SetModDVBC(CATBoardModDVBC *pATBoardCtrl, SModDVBCOpt Options);

#endif // __AT_TESTAPP_H__