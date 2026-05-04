/*! Time-stamp: <@(#)ATBoardModDVBC.h   07/05/2007 - 15:46:04   P.Hoogervorst>
*********************************************************************
*  @file   : ATBoardModDVBC.h
*
*          : Device supported: AT2700, AT2780
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

#ifndef __ATBOARD_MOD_DVBC_H__
#define __ATBOARD_MOD_DVBC_H__

#define ANNEXB_QAM64_SYMRATE	((u32)5056941)
#define ANNEXB_QAM256_SYMRATE	((u32)5360537)

/**********************************************************************************************************/
// includes
#include "ATBoardModRfIf.h"

class CATBoardModDVBC : public CAtBoardModRfIf
{
public:
	CATBoardModDVBC(void);
public:
	virtual ~CATBoardModDVBC(void);

public:

    /*!
     * Do the one time initialize of the modulator. Is done once at startup.
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Init();

	/*!
     * Set the Modulator.
     * \htmlonly
Valid settings:<br>
The Cable modulator supports four different modes: J83-annexA, J83-annexB, J83-annexC and DVB-C<br>
Every mode has its own capabilities:
<br>
J83-annexA:
<ul>
<li>	modulation:		QAM16, QAM32, QAM64
<li>	filter_rolloff:	FLT_ALPHA_15,
<li>	interleaver:	INTLEAV_I12_J17,
<li>	inversion:		INVERSION_OFF, INVERSION_ON,
<li>	symbol_rate:	0-6950000 MSymbols/s
<li>	annexb_enable:	FALSE.
</ul>
J83-annexB:
<ul>
<li>	modulation:		QAM64, QAM256
<li>	filter_rolloff:	FLT_ALPHA_18 for QAM64, FLT_ALPHA_12 for QAM256
<li>	interleaver:	INTLEAV_I128_J1, INTLEAV_I128_J2, INTLEAV_I128_J3, INTLEAV_I128_J4, INTLEAV_I128_J5, INTLEAV_I128_J6, INTLEAV_I128_J7, INTLEAV_I128_J8, INTLEAV_I64_J2, INTLEAV_I32_J4, INTLEAV_I16_J8, INTLEAV_I8_J16
<li>	inversion:		INVERSION_OFF, INVERSION_ON,
<li>	symbol_rate:	5056941 MSymbols/s for QAM64, 5360537 MSymbols/s for QAM256
<li>	annexb_enable:	TRUE.
</ul>
J83-annexC:
<ul>
<li>	modulation:		QAM64
<li>	filter_rolloff:	FLT_ALPHA_13,
<li>	interleaver:	INTLEAV_I12_J17,
<li>	inversion:		INVERSION_OFF, INVERSION_ON,
<li>	symbol_rate:	0-5300000 MSymbols/s
<li>	annexb_enable:	FALSE.
</ul>
DVB-C:
<ul>
<li>	modulation:		QAM16, QAM32, QAM64, QAM128, QAM256
<li>	filter_rolloff:	FLT_ALPHA_15,
<li>	interleaver:	INTLEAV_I12_J17,
<li>	inversion:		INVERSION_OFF, INVERSION_ON,
<li>	symbol_rate:	0-6950000 MSymbols/s
<li>	annexb_enable:	FALSE.
</ul>
\endhtmlonly
	 *
	 * @param Modulation :			modulation type
	 * @param FltRollOff :			set filter roll off factor
	 * @param Interleaver :			set interleaver
	 * @param Inversion :			spectral inversion selection
	 * @param nSymbolRate :			symbol rate in Symbols per second
	 * @param bAnnexBEn :			1: enable annexB, 0: disable annexB (annexA/C mode enabled)
 	 *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Set(	src_modulation_t			Modulation,
				src_filter_rolloff_t		FltRollOff,
				src_interleaver_t			Interleaver,
				src_spectral_inversion_t	Inversion,
				u32							nSymbolRate,
				BOOL						bAnnexBEn);

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

	/*!
	 * Get the modulation status of the modulator
     *
	 * @param bInSync :				TRUE: modulator core is in sync, FALSE: not in sync
	 * @param b204PacketSize :		TRUE: modulator core has detected a transport stream packet size of 204, FALSE: a transport stream packet size of 188
	 * @param bModOFlow :			TRUE: modulator core has overflowed (error), FALSE: modulator is working correctly
     *
	 * @return BOOL  : TRUE on success, else FALSE
     */
	BOOL Get(BOOL &bInSync, BOOL &b204PacketSize, BOOL &bModOFlow); 
};

#endif //__ATBOARD_MOD_DVBC_H__
