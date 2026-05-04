/*! Time-stamp: <@(#)ATBoardTunDVBCAnnexA.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBCAnnexA.h
*
*          : Device supported: AT70, AT730, AT700, AT730
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

#ifndef __ATBOARD_TUN_DVBC_ANNEXA_H__
#define __ATBOARD_TUN_DVBC_ANNEXA_H__

/**********************************************************************************************************/
// includes
#include "ATBoardPlayRec.h"

class CATBoardTunDVBCAnnexA : public CATBoardPlayRec
{
public:
	CATBoardTunDVBCAnnexA(void);
public:
	virtual ~CATBoardTunDVBCAnnexA(void);

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
	 *	nFreq:		47-862MHz in 62.5kHz steps
	 *	nSymRate:	in Symbols/s
	 *	Invert:		INVERSION_OFF, INVERSION_ON or INVERSION_AUTO
	 *	Modulation:	QAM_16, QAM_32, QAM_64, QAM_128, QAM_256 or QAM_AUTO
	 *
     * @param nFreq : Cable frequency (in Hz)
     * @param nSymRate : Cable symbol rate (in Symbols/s)
     * @param Invert : Set modulation inversion
     * @param Modulation : Set modulation type
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Set(u32 nFreq, u32 nSymRate, src_spectral_inversion_t Invert, src_modulation_t Modulation);


    /*!
     * Get status from the tuner
     *
     * @param bInSync : TRUE: Tuner is in sync, FALSE: Tuner is not in sync
     * @param bCarrierDetect : TRUE: Tuner has detected a carrier, FALSE: Tuner has not detected a carrier
     * @param nSignalStrength : Get the tuner signal strength. 
     * @param nBer : Get the tuner bit error rate
     * @param Invert : Get the modulation inversion (can be used if the INVERSION_AUTO is used at setting the tuner)
     * @param Modulation : Get the modulation type. (can be used if the QAM_AUTO is used at setting the tuner)
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Get(BOOL &bInSync, BOOL &bCarrierDetect, u16 &nSignalStrength, u32 &nBer, src_spectral_inversion_t &Invert, src_modulation_t &Modulation);
};

#endif //__ATBOARD_TUN_DVBC_ANNEXA_H__
