#ifndef _ATREGDEFS_H_
#define _ATREGDEFS_H_

#define BIT(a)	(1<<(a))
#define COND_SETBIT(cond,var,pat)	if(cond){(var)|=(pat);}else{(var)&=~(pat);}

/** \defgroup ATRegisterBits ATxxx board Register bits
 *	These bits are used to place into there corresponding registers.
 *	\li \c RCONF Record configuration register
 *	\li \c PCONF Play configuration register
 *	\li	\c STAT Status register
 */

/**
 *	\addtogroup ATRegisterBits
 *	@{
 */

/**
 *	Set the data input in DVB mode.
 *	Works in conjunction with Bit 1 (SMP), configure the GS9060 to operate in SMPTE
 *	DVB  or Data through (RAW DATA) mode.
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW
 *	\par
 *	Applicable in all modes.
 *	\see AT_RCONF_SMP
 */
#define AT_RCONF_DVB	BIT(0)

/**
 *	Set the data input in SMPTE mode.\n
 *	Works in conjunction with Bit 0 (DVB), configure the GS9060 to operate in SMPTE
 *	DVB  or Data through (RAW DATA) mode.
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW
 *	\par
 *	Applicable in all modes.
 *	\see AT_RCONF_DVB
 */
#define AT_RCONF_SMP	BIT(1)

/**
 *	Activate the Serial input (DVB or SMPTE mode).\n
 *	When set HIGH the serial input is selected as the default input for recoding,
 *	loop through or pass through regardless of the status of AT_RCONF_PAR.
 *	\par
 *	Applicable in all modes.
 */
#define AT_RCONF_SER	BIT(2)

/**
 *	Activate the Parallel input (DVB or SMPTE mode).\n
 *	When set HIGH and AT_RCONF_SER is set LOW, the Parallel input is selected as input
 *	for recording, loop through or pass through.	
 *	\par
 *	Applicable in all modes.
 */
#define AT_RCONF_PAR	BIT(3)

/**
 *	Input Loop through Enable.\n
 *	When Set HIGH, the serial loop through is enabled.
 *	\par
 *	Applicable in all modes.
 */
#define AT_RCONF_LEN	BIT(4)

/**
 *	Input Loop through re-clocked Enable.\n
 *	When set HIGH, the serial loopthrough output will be a
 *	reclocked version of the serial input signal regardless of the mode
 *	of operation (SMPTE, DVB-ASI or RAW-Data).\n
 *	When set LOW, the loop through ouput will be a buffered version of the
 *	input signal in all modes.
 *	\par
 *	Applicable in all modes.
 */
#define AT_RCONF_LRC	BIT(5)

/**
 *	Input I/O Processing.\n
 *	Used to enable or disable I/O processing of GS9060 device.
 *	When set HIGH the following I/O processing features of the device are enabled:
 *	\li EDH CRC Error Correction
 *	\li ANC Data Checksum Correction
 *	\li TRS Error Correction
 *	\li Illegal Code Remapping
 *	\par
 *	Applicable in SMPTE mode of operation only. Ignored in other modes. 
 */
#define AT_RCONF_IIO	BIT(6)

/**
 *	Fly wheel enable.\n
 *	Used to enable or disable the noise immune flywheel of the GS9060 device.
 *	The internal flywheel is used in extraction and generation of TRS timing signals.\n
 *	When set LOW, the internal flywheel is disabled and TRS correction and insertion
 *	is unavailable.
 *	\par
 *	Applicable in SMPTE mode of operation only. Ignored in other modes.
 */
#define AT_RCONF_FEN	BIT(7)

/**
 *	Enable Time Stamping.\n
 *	When set high the incoming transport stream is time stamped.
 *	The time stamp is derived from the master clock (48MHz for USB version and 54MHz for PCI version).
 *	On arrival of the 11th byte of the transport stream, the PCR byte,
 *	the content of a 32 bit free running counter is taken as the time stamp.\n
 *	The 32 bit time stamp value is then added to the end of the transport stream.
 *	\note The application then has to be aware of the fact that the last four bytes
 *	of the transport streams are the time stamp value not pay load.
 *	\par
 *	Applicable in DVB mode. Ignored in other modes
 */
#define AT_RCONF_ETS	BIT(8)

/**
 *	Enabling PID filtering.\n
 *	When enabled the hardware will filter out or pass on the selected programs from the
 *	PID table. The PID table consists of 32 registers. As a result  maximum of 32 PIDs
 *	can be entred into the PID table.\n
 *	When the PID table contains a program which is not available in the transport
 *	stream, it is ignored by the PID filtering program.
 *	\par
 *	Applicable in DVB mode. Ignored in other modes
 */
#define AT_RCONF_PID	BIT(9)

/**
 *	PID Table Exclusive.\n
 *	In order to make the PID filtering more efficient, the PID filter can contain
 *	the programs which are to be filtered out or the programs which are to be passed on.
 *	When there are only a few program which the user requires to pass on, then it
 *	is more efficient to enter these programs into the PID table and set TEX bit LOW
 *	indicating the PID table an Inclusive one.\n
 *	When there are more programs to be passed on and only a few to be filtered out, 
 *	(EXCLUDED) then only these programs are entred into the PID table and the
 *	TEX bit is set HIGH.\n
 *	In this way the PID filtering is much faster and does not require a large table.
 *	\par
 *	Applicable in DVB mode. Ignored in other modes
 */
#define AT_RCONF_TEX	BIT(10)

/**
 *	Mute Audio1.\n
 *	The AT500 Audio de-embedder board is capable of de-embedding two pairs of audio channals.
 *	The de-embedded audio channals are avialable on two audio outputs.\n
 *	The Audio1 output could be muted by setting this bit HIGH.
 *	\par
 *	Applicable for AT500 Audio de-embedder otherwise Ignored 
 *	\see AT_RCONF_MUTE2
 */
#define AT_RCONF_MUTE1	BIT(11)

/**
 *	Mute Audio2.\n
 *	The Audio2 output could be muted by setting this bit HIGH.
 *	\par
 *	Applicable for AT500 Audio de-embedder otherwise Ignored 
 *	\see AT_RCONF_MUTE1
 */
#define AT_RCONF_MUTE2	BIT(12)


/**
 *	Enable Recording.\n
 *	Must be LOW before recording and made HIGH to enable the device
 *	to pass data into its buffers.
 */
#define AT_RCONF_RENA	BIT(31)

/**
 *	Output data in DVB Mode.\n
 *	Bit 1 (SMP) in conjunction with Bit 0 (DVB), configure the GS9062 to operate in SMPTE,
 *	DVB  or Data through (RAW DATA) mode. 
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW
 *	\par
 *	Applicable in all modes
 *	\see AT_PCONF_SMP
 */
#define AT_PCONF_DVB	BIT(0)

/**
 *	Output data in SMPTE Mode.\n
 *	Bit 1 (SMP) in conjunction with Bit 0 (DVB), configure the GS9062 to operate in SMPTE,
 *	DVB  or Data through (RAW DATA) mode. 
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW
 *	\par
 *	Applicable in all modes
 *	\see AT_PCONF_DVB
 */
#define AT_PCONF_SMP	BIT(1)

/**
 *	Select Serial output.\n
 *	When set HIGH the serial output is enabled
 *	\par
 *	Applicable in all modes
 */
#define AT_PCONF_SER	BIT(2)

/**
 *	Select Parallel output.\n
 *	When set HIGH the parallel output is enabled
 *	\par
 *	Applicable in all modes
 */
#define AT_PCONF_PAR	BIT(3)

/**
 *	Select output packet size 188.\n
 *	When set HIGH the transmitted transport packet size of 188 is selected.\n
 *	This bit is set by the application when a transport stream file is played back.
 *	The application calculates the bit rate and the packet size from the transport
 *	stream files and sets the correct packet size and bit rate.
 *	\see AT_PCONF_204
 *	\see AT_PCONF_188_16
 */
#define AT_PCONF_188	BIT(4)

/**
 *	Select output packet size 204.\n
 *	When set HIGH the transmitted transport packet size of 204 is selected.\n
 *	This bit set by the application when a transport stream file is played back.
 *	\see AT_PCONF_188
 *	\see AT_PCONF_188_16
 */
#define AT_PCONF_204	BIT(5)

/**
 *	Select output packet size 188+16.\n
 *	When this bit and bit4 are both HIGH, 16 bytes are added to the end of the 
 *	188 byte transport packet pay load hence making a 204 byte transport stream.\n
 *	This function is usefull for the designers of reciver equipments to test
 *	if the system under development could handle both 188 and 204 packet sizes.
 *	\see AT_PCONF_188
 *	\see AT_PCONF_204
 */
#define AT_PCONF_188_16	BIT(6)

/**
 *	Output data in Raw Data Mode.\n
 *	When HIGH the tranmitted data is regarded as NONE_DVB data, as such there
 *	are no attampts made  to comply to DVB standards.\n
 *	This mode is of interset to the designer of reciver equipments to test their
 *	systems for detection of streams which are not DVB_ASI complient as well as 
 *	those applications which only requires a high speed serial data link.
 */
#define AT_PCONF_RAW	BIT(7)

/**
 *	Select 27MHz Clock for output.\n
 *	By default the on board 27MHz clock is the transmission clock.
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_CLK27	BIT(8)

/**
 *	Select External Clock for output.\n
 *	In applications where there is a need for using an external clock for data
 *	transmission, Alitronika boards provide an external input clock.\n
 *	Setting this bit HIGH will select the external clock instead of the on board clock.
 *	\par
 *	Applicable to all modes.
 */
#define AT_PCONF_EXCLK	BIT(9)

/**
 *	Output data in Burst Mode.\n
 *	The DVB-ASI standards specify that data is transmitted at a constant bit rate
 *	of 270 Mbit/s. Considering the 8 bit data bytes are 8b/10b encoded, this correspond
 *	to fixed symbol rate of 27 Msymbol/s.\n
 *	But the transport bit rate could be much less, special character, K28.5 (Comma) is
 *	used as "Stuffing" to make up the difference.
 *	The transport packets may be transmitted as a burst of contiguous bytes or as
 *	individual bytes spread out in time.\n
 *	Normally the later is used, since spreading out the data in time has less demand on
 *	the reciver's input buffer.\n
 *	Alitronika boards use this mode of transmission as default.
 *	By setting the BURST mode HIGH the burst mode could be activated. 
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_BURST	BIT(10)

/**
 *	Active Hardware Generation of packets.\n
 *	In the play mode a transport stream could be played back from a file.
 *	Alitronika boards with output function could also generate simple transport streams
 *	by hardware. This function has the advantage of freeing the board of taking up any
 *	processing power from the system while providing a stream for testing purposes.
 *	Two types of transport streams can be generated by the hardware.\n
 *	Setting this bit HIGH enables the hardware generation of transport stream.
 *	\par
 *	Applicable to DVB_ASI mode only.
 *	\see AT_PCONF_NTP Hardware Null packet generation
 *	\see AT_PCONF_CTP Hardware Counter packet generation
 *	\see AT_PCONF_BLK Hardware Blanking generation
 */
#define AT_PCONF_GTP	BIT(11)

/**
 *	Generate NIL packets by hardware.\n
 *	When set HIGH, this bit in conjuction with Bit11 (\ref AT_PCONF_GTP) will enable the
 *	harware generation of a Null packet transport stream.
 *	\par
 *	Applicable to DVB_ASI mode only.
 *	\see AT_PCONF_CTP
 */
#define AT_PCONF_NTP	BIT(12)

/**
 *	Generate Counter packets by hardware.\n
 *	Although very usefull for many tests, the Null packet is not sufficient to test
 *	the integrity of a DVB-ASI transmission link.\n
 *	In the other hand if a transport packet of say 188 byte size contains bytes
 *	which count from 0 to 188, with the byte H"47" replaced by the content of another
 *	free running counter, the integrity of the transmission link can easily be varified.
 *	When set HIGH, this bit in conjuction with Bit11 (\ref AT_PCONF_GTP) will enable the
 *	harware generation of a Counter packet transport stream.
 *	\par
 *	Applicable to DVB_ASI mode only.
 *	\see AT_PCONF_NTP
 */
#define AT_PCONF_CTP	BIT(13)

/**
 *	Generate Blanking by hardware.\n
 *	Used to enable or disable the input data blanking of the GS9062 serializer device
 *	in SMPTE mode of operation.\n
 *	When HIGH, in conjuction with Bit11 (\ref AT_PCONF_GTP), the LUMA and CHROMA input data is set to appropriate blanking level.
 *	Horizontal and vertical ancillary spaces will also be set to blanking levels.\n
 *	When set LOW, the LUMA and CHROMA input data pass through the device unaltered.
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_BLK	BIT(14)

/**
 *	Select Output I/O Processing.\n
 *	Used to enable or disable I/O processing of GS9062 serializer device in SMPTE mode of
 *	operation.When set HIGH the following I/O processing features of the device are
 *	enabled:
 *	\li EDH Packet Generation and Insertion
 *	\li SMPTE 352M Packet Generation and Insertion
 *	\li ANC Data Checksum Calculation and Insertion
 *	\li TRS Generation and Insertion
 *	\li Illegal Code Remapping
 *	\par
 *	Applicable to SMPTE mode only
 */
#define AT_PCONF_IOP	BIT(15)

/**
 *	Detect TRS.\n
 *	Used to select the timing mode of GS9062 serializer device in SMPTE mode of
 *	operation.When set HIGH, the device will lock the internal flywheel to the 
 *	embedded TRS timing signals in the parallel input data.
 *	\par
 *	Applicable to SMPTE mode only
 */
#define AT_PCONF_TRS	BIT(16)

/**
 *	Enable Serial Pass-Through output.\n
 *	Setting this bit HIGH would pass through the serial input to the serial output.
 *	This function could be used to generate more signal from a single signal.
 *	\par
 *	Applicable in all modes
 */
#define AT_PCONF_SPT	BIT(17)

/**
 *	Enable Parallel Pass-Through output.\n
 *	Setting this bit HIGH would pass through the parallel input to the parallel output.
 *	This function is used in case the parallel data is needed simultaously to be
 *	used by another equipment.
 *	\par
 *	Applicable in all modes
 */
#define AT_PCONF_PPT	BIT(18)

/**
 *	Serial to Parallel Conversion.\n
 *	When HIGH the hardware would convert the input serial data into a parallel
 *	data transmitted via the LVDS/ECL output port provided the bit rate of the
 *	input data does not excced the maximum bit rate allowed by the parallel data
 *	transmission.
 *	\par
 *	Applicable in all modes
 *	\see AT_PCONF_CPS
 */
#define AT_PCONF_CSP	BIT(19)

/**
 *	Parallel to Serial Conversion.\n
 *	When HIGH the parallel input data is converted to serial data.
 *	\par
 *	Applicable in all modes
 *	\see AT_PCONF_CSP
 */
#define AT_PCONF_CPS	BIT(20)


/**
 *	Enable Playing.\n
 *	Must be LOW before playing and made HIGH to enable the device
 *	to pass data into its buffers.
 */
#define AT_PCONF_PENA	BIT(31)

/**
 *	Play Status: Input packet Size: '1' -> 188; '0' -> 204.\n
 *	The packet size of the input transport stream is determined by the hardware
 *	synchronization mechanism.
 *	\par
 *	Applicable to DVB mode, Ignored by othere modes.
 */
#define AT_STAT_RTPS	BIT(0)

/**
 *	Play Status: Packet Size Error (not 188 or 204).\n
 *	The input transport stream should be 188 or 204. If due some errors in the
 *	received input stream, the packet size is not 188 or 204, this bit is set
 *	HIGH by the hardware synchronisation mechanism to indicate a packet
 *	size error condition.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RPSE	BIT(1)

/**
 *	Play Status: Firmware version.\n
 *	This represents a FPGA revision number.
 *	The firmware version is shifted left by \ref AT_STAT_RVERSHIFT bits.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RVER	0x003C

/**
 *	Play Status: Firmware version right shift value.
 */
#define AT_STAT_RVERSHIFT	2

/**
 *	Play Status: Bus type indication (High = PCI, Low = USB)
 */
#define AT_STAT_BUS	BIT(7)

// Record status

/**
 *	Record Status: No Record Carrier Detected (High = no carrier).\n
 *	This bit is generated by the cable equaliser. As long as this bit is
 *	set HIGH there is no carrier detected by the input circuitry.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_NORCD	BIT(16)

/**
 *	Record Status: No Record Lock to input stream (High = no lock).\n
 *	This flag is generated by the GS9060 input device. This bit will be LOW whenever
 *	the device has correctly received and locked to SMPTE compliant data in SMPTE mode
 *	or DVB-ASI compliant data in DVB-ASI mode or when the re-clocker
 *	has achieved lock in RAW data (Data Through) mode.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_NORLOCK	BIT(17)

/**
 *	Record Status: No Record Sync to input stream packets (High = no sync).\n
 *	In DVB-ASI mode the synchronisation to the incoming data stream is achieved
 *	by the main controller. The synchroniser has to detect the transport stream
 *	sync byte, H"47", for every packet, 188 or 204 byte apart, depending on the
 *	packet size.\n
 *	When this is the case Bit2, nR_SYC, is set LOW. 
 *	When Synchronisation is lost this bit is set HIGH.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_NORSYNC	BIT(18)

/**
 *	Record Status: Too many Record Data errors (High = too many errors).\n
 *	In every serial link a degree of errors are tolerated, if the number of errors,
 *	that is the number of corrupted bytes or bytes which can not be decoded by
 *	the DVB-ASI decoder, are too high the link is no longer viable.\n
 *	The synchroniser in the main controller sets the R_ERR to HIGH whenever this
 *	condition is reached. 
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RERR	BIT(19)

/**
 *	Record Status: Record Processing fifo Full (High = Full).\n
 *	The incoming data is processed by the main controller, the input Fifo is
 *	used by this section will set the R_PFF error flag HIGH whenever an
 *	overflow condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RPFF	BIT(20)

/**
 *	Record Status: Record Processing fifo Empty (High = Empty).\n
 *	The incoming data is processed by the main controller, the input Fifo is used
 *	by this section will set the R_PFE error flag HIGH whenever an underflow condition
 *	has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RPFE	BIT(21)

/**
 *	Record Status: Record 8/16 fifo Full (High = Full).\n
 *	The incoming data is processed by the main controller, the output Fifo is
 *	used by this section will set the R_OFF error flag HIGH whenever an overflow
 *	condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_ROFF	BIT(22)

/**
 *	Record Status: Record SDram fifo Full (High = Full).\n
 *	The SDRAM used as Fifo will set the R_SFF error flag HIGH whenever an overflow
 *	condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RSFF	BIT(23)

// Play status

/**
 *	Play Status: No play lock (High = No play lock).\n
 *	This flag is generated by the GS9062 output device. This bit will be LOW whenever
 *	the device has correctly received and locked to SMPTE compliant data in SMPTE mode
 *	or DVB-ASI compliant data in DVB-ASI mode or when the reclocker has achieved
 *	lock in RAW data (Data Through) mode.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_NOPLOCK	BIT(25)

/**
 *	Play Status: Play Burst fifo full.\n
 *	In DVB-ASI mode the out going transport packets could be send in burst mode.\n
 *	The data can enter the main controller in none-burst mode.
 *	A Fifo is therefore needed to hold a full packet before transmission.
 *	When this fifo detects an overflow the P_BFF flag will be set HIGH.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PBFF	BIT(26)

/**
 *	Play Status: Play DVB output fifo Full.\n
 *	The out going DVB data stream is processed by the main controller.\n
 *	The Fifo used by this section will set the P_DFE error flag HIGH whenever an
 *	overflow condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PDFF	BIT(27)

/**
 *	Play Status: Play DVB output fifo Empty.\n
 *	The out going DVB data stream is processed by the main controller.\n
 *	The Fifo used by this section will set the P_DFE error flag HIGH whenever an
 *	underflow condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PDFE	BIT(28)

/**
 *	Play Status: Play 16/8 fifo Full.\n
 *	The output Fifo will set the P_OFF error flag HIGH whenever an overflow
 *	condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_POFF	BIT(29)

/**
 *	Play Status: Play 16/8 fifo Empty.\n
 *	The output Fifo will set the P_OFE error flag HIGH whenever an underflow
 *	condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_POFE	BIT(30)

/**
 *	Play Status: Play SDram fifo Full.\n
 *	The SDRAM used as Fifo will set the P_SFF error flag HIGH whenever
 *	an overflow condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PSFF	BIT(31)



//  #define FPGA_I2C_WD0	0x90

/**
 *	Address of the PID (Program ID) table.
 */
#define AT_REG_PIDTBL	0xC0

/**
 *	Size of the PID table in number of entries.
 */
#define AT_REG_PIDSZ	32

#define ATREGRWSIZE	(3*4)
#define ATREGSIZE	(12*4)

/**
 *	@}
 */

#endif