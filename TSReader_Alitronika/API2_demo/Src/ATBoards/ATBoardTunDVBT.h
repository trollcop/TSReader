/*! Time-stamp: <@(#)ATBoardTunDVBT.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBT.h
*
*          : Device supported: AT8, AT80, AT82, AT800, AT820, AT1800
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

#ifndef __ATBOARD_TUN_DVBT_H__
#define __ATBOARD_TUN_DVBT_H__

/**********************************************************************************************************/
// includes
#include "ATBoardPlayRec.h"

class CATBoardTunDVBT : public CATBoardPlayRec
{
public:
	CATBoardTunDVBT(void);
public:
	virtual ~CATBoardTunDVBT(void);

public:

    /*!
     * Do the one time initialize and reset of the tuner. Is done once at startup.
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Init();


    /*!
     * Set the tuner.
     *
	 * Valid settings:
	 *	nFreq:					174-230MHz and 470-862MHz in 62.5kHz steps
	 *	AT8, AT80 and AT800:			
	 *		BandWidth:			BANDWIDTH_8_7_MHZ, BANDWIDTH_7_6_MHZ and BANDWIDTH_6_MHZ (8,7,6MHz)
	 *		bBandWidthGroup:	FALSE
	 *	AT82 and AT820:			
	 *		BandWidth:			BANDWIDTH_7_6_MHZ (6MHz)
	 *		bBandWidthGroup:	TRUE
	 *
     * @param nFreq : Tuning frequency (in Hz)
     * @param BandWidth : Requested modulation bandwidth
     * @param bBandWidthGroup : Set bandwidth group. FALSE for AT8, AT80, AT800, TRUE for AT82 and AT820
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Set(u32 nFreq, src_bandwidth_t BandWidth, BOOL bBandWidthGroup);

    /*!
     * Get status from the tuner.
	 * If bInSync, bCarrierDetect, bSigDet and bViterbiDet are high: then the tuner has properly locked
	 * to the input signal.
     *
     * @param bInSync : TRUE: Tuner is in sync, FALSE: Tuner is not in sync
     * @param bCarrierDet : TRUE: Tuner has detected a carrier, FALSE: Tuner has not detected a carrier
	 * @param bSigDet : TRUE: Tuner has detected a signal, FALSE: Tuner has not detected a signal
	 * @param bViterbiDet : TRUE: Tuner has detected a Viterbi (a COFDM modulated signal is detected), FALSE: Tuner has not detected a Viterbi
     * @param nSignalStrength : Get the tuner signal strength. 
	 * @param nSnr : Get the tuner signal to noise ratio. 
     * @param nBer : Get the tuner bit error rate
     * @param Modulation : Get the modulation type. (QPSK, QAM16 or QAM64)
	 * @param ModTransmitMode : Get the modulation transmit mode (2k or 8k)
	 * @param ModGuard : Get the  modulation guard interval.
	 * @param ModHierarch : Get the modulation hierarchical mode.
	 * @param ModHPFecCodeRate : Get the High priority stream FEC code rate
	 * @param ModLPFecCodeRate : Get the Low priority stream FEC code rate
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Get(	BOOL &bInSync,
				BOOL &bCarrierDet,
				BOOL &bSigDet,
				BOOL &bViterbiDet,
				u16 &nSignalStrength,
				u16 &nSnr,
				u32 &nBer,
				src_modulation_t &Modulation,
				src_transmit_mode_t &ModTransmitMode,
				src_guard_interval_t &ModGuard,
				src_hierarchy_t &ModHierarch,
				src_code_rate_t &ModHPFecCodeRate,
				src_code_rate_t &ModLPFecCodeRate); 
};

#endif //__ATBOARD_TUN_DVBT_H__
