/*! Time-stamp: <@(#)ATBoardTunDVBCAnnexB.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardTunDVBCAnnexB.h
*
*          : Device supported: AT72, AT720
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

#ifndef __ATBOARD_TUN_DVBC_ANNEXB_H__
#define __ATBOARD_TUN_DVBC_ANNEXB_H__

/**********************************************************************************************************/
// includes
#include "ATBoardPlayRec.h"

class CATBoardTunDVBCAnnexB : public CATBoardPlayRec
{
public:
	CATBoardTunDVBCAnnexB(void);
public:
	virtual ~CATBoardTunDVBCAnnexB(void);

public:

    /*!
     * Do the one time initialize and reset of the tuner. Is done once at startup.
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Init();

    /*!
     * Set the tuner.
	 * Valid settings:
	 *	nFreq:			54-864MHz in 62.5kHz steps
	 *	Modulation:		QAM_64 or QAM_256
     *
     * @param nFreq : Cable frequency (in Hz)
     * @param Modulation : Set modulation type
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Set(u32 nFreq, src_modulation_t Modulation);

    /*!
     * Get status from the tuner
     *
	 * @param bInSync : TRUE: Tuner is in sync, FALSE: Tuner is not in sync
	 * @param bCarrierDetect : TRUE: Tuner has detected a carrier, FALSE: Tuner has not detected a carrier
	 * @param bTsLock : TRUE: Tuner has detected a valid transport stream, FALSE: Tuner has not detected a valid transport stream
     * @param nSnr : Get the tuner signal to noise ratio. 
     * @param nCorBer : Get the tuner bit error rate that which the tuner was able to correct
     * @param nUnCorBer : Get the tuner bit error rate that which the tuner was not able to correct
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Get(BOOL &bInSync, BOOL &bCarrierDetect, BOOL &bTsLock, u16 &nSnr, u32 &nCorBer, u32 &nUnCorBer);
};

#endif //__ATBOARD_TUN_DVBC_ANNEXB_H__
