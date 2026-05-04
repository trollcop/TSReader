/*! Time-stamp: <@(#)ATBoardModDVBT.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModDVBT.h
*
*          : Device supported: AT2800, AT3800, AT2780
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

#ifndef __ATBOARD_MOD_DVBT_H__
#define __ATBOARD_MOD_DVBT_H__

/**********************************************************************************************************/
// includes
#include "ATBoardModRfIf.h"

class CATBoardModDVBT : public CAtBoardModRfIf
{
public:
	CATBoardModDVBT(void);
public:
	virtual ~CATBoardModDVBT(void);

public:

    /*!
     * Do the one time initialize of the modulator. Is done once at startup.
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Init();

	/*!
     * Set the Modulator.
	 *
	 * \htmlonly
Valid settings:<br>
<br>
<li>Modulation:							QPSK, QAM_16 or QAM_64
<li>TranMode when DVB-H is enabled:		TRANSMISSION_MODE_2K, TRANSMISSION_MODE_4K or TRANSMISSION_MODE_8K
<li>TranMode when DVB-H is disabled:	TRANSMISSION_MODE_2K or TRANSMISSION_MODE_8K
<li>GuardInt:							GUARD_INTERVAL_1_32, GUARD_INTERVAL_1_16, GUARD_INTERVAL_1_8 or GUARD_INTERVAL_1_4
<li>FECCodeRate:						FEC_1_2, FEC_2_3, FEC_3_4, FEC_5_6 or FEC_7_8
<li>Bandwidth:							BANDWIDTH_8_7_MHZ, BANDWIDTH_7_6_MHZ, BANDWIDTH_6_MHZ or BANDWIDTH_5_MHZ
<li>Inversion:							INVERSION_OFF or INVERSION_ON
<li>bDvbHEn:							TRUE or FALSE
<li>bDvbHTimeSlice:						TRUE or FALSE
<li>bDvbH_MPE_FEC:						TRUE or FALSE
<li>bDvbHIndepthInt:					TRUE or FALSE
<li>nDvbHCellId:						0 to 65535
\endhtmlonly
	 *
     * @param Modulation :			modulation type
     * @param TranMode :			transmission mode selection
     * @param GuardInt :			guard interval selection
     * @param FECCodeRate :			high priority stream code rate
     * @param Bandwidth :			channel bandwidth selection
     * @param Inversion :			spectral inversion selection
     * @param bDvbHEn :				enable dvb-h mode in ofdm fpga core
     * @param bDvbHTimeSlice :		enable high priority time slicing for dvb-h
     * @param bDvbH_MPE_FEC :		enable high priority MPE FEC for dvb-h
     * @param bDvbHIndepthInt :		enable indepth interleaver for dvb-h
     * @param nDvbHCellId :			cell id number for dvb-h);
 	 *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Set(	src_modulation_t			Modulation,
				src_transmit_mode_t			TranMode,
				src_guard_interval_t		GuardInt,
				src_code_rate_t				FECCodeRate,
				src_bandwidth_t				Bandwidth,
				src_spectral_inversion_t	Inversion,
				BOOL						bDvbHEn,
				BOOL						bDvbHTimeSlice,
				BOOL						bDvbH_MPE_FEC,
				BOOL						bDvbHIndepthInt,
				u16							nDvbHCellId);

	/*!
	 * Get the modulation status of the modulator
     *
	 * @param bInSync :				TRUE: modulator core is in sync, FALSE: not in sync
	 * @param b204PacketSize :		TRUE: modulator core has detected a transport stream packet size of 204, FALSE: a transport stream packet size of 188
	 * @param bModOFlow :			TRUE: modulator core has overflowed (error), FALSE: modulator is working correctly
	 * @param bModUFlow :			TRUE: modulator core has underflowed (error), FALSE: modulator is working correctly
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Get(BOOL &bInSync, BOOL &b204PacketSize, BOOL &bModOFlow, BOOL &bModUFlow); 

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
	BOOL _InitPlay	   (u32			nFileBitrate,
						BOOL		bORemux,
						u32			nOutputBitrate,
						EIOSel		OSel,
						EIOMode		OMode,
						EIOSpiMode	OSpiMode,
						ETSPSize	OTsPSize,
						BOOL		bOHTP,
						BOOL		bOCTP,
						BYTE		nBurstSize);

};

#endif //__ATBOARD_MOD_DVBT_H__
