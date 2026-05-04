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
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW,  Bit 2= HIGH
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH, Bit 2= LOW
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW,	 Bit 2= HIGH
 *	\par
 *	Applicable in all modes.
 *	\see AT_RCONF_SMP
 */
#define AT_RCONF_DVB	BIT(0)

/**
 *	Set the data input in SMPTE mode.\n
 *	Works in conjunction with Bit 0 (DVB), configure the GS9060 to operate in SMPTE
 *	DVB  or Data through (RAW DATA) mode.
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW,  Bit 2= HIGH
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH, Bit 2= LOW
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW,	 Bit 2= HIGH
 *	\par
 *	Applicable in all modes.
 *	\see AT_RCONF_DVB
 */
#define AT_RCONF_SMP	BIT(1)

/**
 *	Set the data input in RAW mode.\n
 *	Works in conjunction with Bit 0 (DVB), configure the GS9060 to operate in SMPTE
 *	DVB  or Data through (RAW DATA) mode.
 *	\li DVB			mode - Bit0 = HIGH,  Bit1= LOW,  Bit 2= HIGH
 *	\li SMPTE		mode - Bit0 = LOW,   Bit1= HIGH, Bit 2= LOW
 *	\li RAW Data	mode - Bit0 = LOW,   Bit1= LOW,	 Bit 2= HIGH
 *	\par
 *	Applicable in all modes.
 *	\see AT_RCONF_DVB
 */
#define AT_RCONF_RAW	BIT(2)

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
 *	Input source select (bit 0).\n
 *	Select the input in combination with ISEL1.\n
 *	ISEL1	ISEL0	Source
 *	  0		  0		Disconnected
 *	  0		  1		USB / PCI
 *	  1		  0		SPI
 *	  1		  1		ASI / Tuner
 */
#define AT_RCONF_ISEL0	BIT(13)
#define AT_RCONF_ISEL_POS	(13)

/**
 *	Input source select (bit 0).\n
 *	Select the input in combination with ISEL0.\n
 *	ISEL1	ISEL0	Source
 *	  0		  0		Disconnected
 *	  0		  1		USB / PCI
 *	  1		  0		SPI
 *	  1		  1		ASI / Tuner
 */
#define AT_RCONF_ISEL1	BIT(14)


/**
 *	Clear PID table content.\n
 *	If written HIGH, the PID table content is cleared.
 *	To write new PID values, keep this bit LOW.
 */
#define AT_RCONF_PIDCLR	BIT(15)

/**
 *	SPI mode.\n
 *	0 = constant clock.
 *	1 = variable clok mode.
 */
#define AT_RCONF_SPI	BIT(16)


/**
 *	Reset Tuner.\n
 *	Set high to place tuner in reset.
 *	Use this only once when the tuner needs to be initialized.
 */
#define AT_RCONF_TUNRST	BIT(29)
 
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
 *	Select output packet.\n
 *	The bits TPS0 and TPS1 for the output packet size selection.\n
 *	\li Size		TPS1	TPS0
 *	\li	188			 0		 0
 *	\li	204			 0		 1
 *	\li	127			 1		 0
 *	\li	Reserved	 1		 1
 *	\see AT_PCONF_TPS1
 */
#define AT_PCONF_TPS0	BIT(4)

/**
 *	Select output packet.\n
 *	The bits TPS0 and TPS1 for the output packet size selection.\n
 *	\li Size		TPS1	TPS0
 *	\li	188			 0		 0
 *	\li	204			 0		 1
 *	\li	127			 1		 0
 *	\li	Reserved	 1		 1
 *	\see AT_PCONF_TPS0
 */
#define AT_PCONF_TPS1	BIT(5)

/**
 *	Enable PCR Restamping.\n
 *	PCR Restamping adapts the existing PCR value to account for time-shifts
 *	due to NIL-packet insertion. Both NIL-packet insertion and the PCR
 *	restamping are performed by hardware.\n
 *	\note This function is only relevant when the Output bitrate is set higher than
 *	the bitrate of the input, and NIL-packet insertion is enabled.\n
 */
#define AT_PCONF_PCRREST	BIT(6)

/**
 *	Select output packet size 188+16.\n
 *	When this bit and bit4 are both HIGH, 16 bytes are added to the end of the 
 *	188 byte transport packet pay load hence making a 204 byte transport stream.\n
 *	This function is useful for the designers of receiver equipments to test
 *	if the system under development could handle both 188 and 204 packet sizes.
 *	\see AT_PCONF_188
 *	\see AT_PCONF_204
 */
#define AT_PCONF_188_16	BIT(7)

 /**
 *	Output data in Raw Data Mode.\n
 *	When HIGH the tranmitted data is regarded as NONE_DVB data, as such there
 *	are no attempts made  to comply to DVB standards.\n
 *	This mode is of interest to the designer of receiver equipments to test their
 *	systems for detection of streams which are not DVB_ASI complient as well as 
 *	those applications which only requires a high speed serial data link.
 */
#define AT_PCONF_RAW	BIT(8)

/**
 *	Select 27MHz Clock for output.\n
 *	By default the on board 27MHz clock is the transmission clock.
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_CLK27	BIT(9)

/**
 *	Select External Clock for output.\n
 *	In applications where there is a need for using an external clock for data
 *	transmission, Alitronika boards provide an external input clock.\n
 *	Setting this bit HIGH will select the external clock instead of the on board clock.
 *	\par
 *	Applicable to all modes.
 */
#define AT_PCONF_EXCLK	BIT(10)

/**
 *	SPI (Parallel) output mode bit0.\n
 *	\li SPI1	SPI0	Mode				
 *	\li	 0		 0		constant (27MHz) clock, Data valid indicates data.
 *	\li	 0		 1		variable clock. Data valid is HIGH.
 *	\li	 1		 0		Reserved			
 *	\li	 1		 1		Reserved			
 *	\see AT_PCONF_SPI1
 */
#define AT_PCONF_SPI0	BIT(11)

/**
 *	SPI (Parallel) output mode bit0.\n
 *	\li SPI1	SPI0	Mode				
 *	\li	 0		 0		constant (27MHz) clock, Data valid indicates data.
 *	\li	 0		 1		variable clock. Data valid is HIGH.
 *	\li	 1		 0		Reserved			
 *	\li	 1		 1		Reserved			
 *	\see AT_PCONF_SPI0
 */
#define AT_PCONF_SPI1	BIT(12)

/**
 *	Enable Hardware generation of NIL packets or Counter packets in hardware.\n
 *	When set HIGH, hardware generation of Null packet (or counter packets) is enabled.
 *	This may be used as stand-alone NIL/counter packet (\see AT_PCONF_CTP) generator,
 *	when input data is disabled (\see AT_RCONF_ISEL0).\n
 *	When the output bitrate is higher than the input/play bitrate, this function
 *	automatically inserts NIL packets (AT_PCONF_CTP should be disabled), to fill
 *	up the gaps in the transport stream. In that case, enable \ref AT_PCONF_PCRREST to
 *	have the system adapt the PCR to the correct time.
 *	\par
 *	Applicable to DVB_ASI mode only.
 *	\see AT_PCONF_CTP
 */
#define AT_PCONF_HTP	BIT(13)



 /**
 *	Enable bitrate remultiplexing\n
 *	If the Output bitrate is higher than the Play bitrate and AT_PCONF_BRREMUX bit is set to 1,
 *  Null packets are inserted where needed to achieve the requested output bitrate.\n
 *	Note that the transportstream is only correct when the \ref AT_PCONF_PCRREST is activated as well.
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_BRREMUX	BIT(14)
 
 /**
 *	Generate Counter packets by hardware.\n
 *	Although very usefull for many tests, the NIL packet is not sufficient to test
 *	the integrity of a DVB-ASI transmission link.\n
 *	In the other hand if a transport packet of say 188 byte size contains bytes
 *	which count from 0 to 188, with the byte H"47" replaced by the content of another
 *	free running counter, the integrity of the transmission link can easily be verified.
 *	When set HIGH, this bit in conjuction with \ref AT_PCONF_HTP will enable the
 *	hardware generation of a Counter packet transport stream. Disable the input using \ref AT_RCONF_ISEL0.
 *	\par
 *	Applicable to DVB_ASI mode only.
 *	\see AT_PCONF_NTP
 */
#define AT_PCONF_CTP	BIT(15)

/**
 *	Generate Blanking by hardware.\n
 *	Used to enable or disable the input data blanking of the GS9062 serializer device
 *	in SMPTE mode of operation.\n
 *	When HIGH, in conjuction with Bit13 (\ref AT_PCONF_HTP), the LUMA and CHROMA input data is set to appropriate blanking level.
 *	Horizontal and vertical ancillary spaces will also be set to blanking levels.\n
 *	When set LOW, the LUMA and CHROMA input data pass through the device unaltered.
 *	\par
 *	Applicable to DVB_ASI mode only.
 */
#define AT_PCONF_BLK	BIT(16)

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
#define AT_PCONF_IOP	BIT(17)

/**
 *	Detect TRS.\n
 *	Used to select the timing mode of GS9062 serializer device in SMPTE mode of
 *	operation.When set HIGH, the device will lock the internal flywheel to the 
 *	embedded TRS timing signals in the parallel input data.
 *	\par
 *	Applicable to SMPTE mode only
 */
#define AT_PCONF_TRS	BIT(18)

/**
 *	Enable Serial Pass-Through output.\n
 *	Setting this bit HIGH would pass through the serial input to the serial output.
 *	This function could be used to generate more signal from a single signal.
 *	\par
 *	Applicable in all modes
 */
//#define AT_PCONF_SPT	BIT(19)

/**
 *	Enable Parallel Pass-Through output.\n
 *	Setting this bit HIGH would pass through the parallel input to the parallel output.
 *	This function is used in case the parallel data is needed simultaously to be
 *	used by another equipment.
 *	\par
 *	Applicable in all modes
 */
//#define AT_PCONF_PPT	BIT(20)

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
//#define AT_PCONF_CSP	BIT(21)

/**
 *	Parallel to Serial Conversion.\n
 *	When HIGH the parallel input data is converted to serial data.
 *	\par
 *	Applicable in all modes
 *	\see AT_PCONF_CSP
 */
//#define AT_PCONF_CPS	BIT(22)

/**
 *	Play Reset.\n
 *	When HIGH, resets the play modules and clears all play buffers.
 *	\par
 *	Must be done immediately before the play transfers (USB or PCI) are activated.
 *	\see AT_PCONF_PENA
 */
#define AT_PCONF_PRST	BIT(30)

/**
 *	Enable Playing.\n
 *	Must be HIGH to enable data into the buffers.\n
 *	This may be used to fill the play buffers before allowing the AT-device to play.
 *	This prevents buffer under-runs at start-up.
 *	The normal procedure before playing is:
 *	\li AT_PCONF_PENA Low
 *	\li AT_PCONF_PRST High
 *	\li AT_PCONF_PRST Low
 *	\li Start USB/PCI data transfer and fill the SDRAM buffer.
 *	\li AT_PCONF_PENA High to release the data to the output.
 */
#define AT_PCONF_PENA	BIT(31)

/**
 *	Play Status: Input packet Size bit 0.\n
 *	The packet size of the input transport stream is determined by the hardware
 *	synchronization mechanism and reported in AT_STAT_RTPS0 and AT_STAT_RTPS1.
 *	\li RTPS1	RTPS0	Input packet size
 *	\li	 0		 0		188
 *	\li	 0		 1		204
 *	\li	 1		 0		127
 *	\li	 1		 1		other
 *	\par
 *	Applicable to DVB mode, Ignored by other modes.
 */
#define AT_STAT_RTPS0	BIT(0)

/**
 *	Play Status: Input packet Size bit 1.\n
 *	The packet size of the input transport stream is determined by the hardware
 *	synchronization mechanism and reported in AT_STAT_RTPS0 and AT_STAT_RTPS1.
 *	\par
 *	Applicable to DVB mode, Ignored by other modes.
 */
#define AT_STAT_RTPS1	BIT(1)

/**
 *	Record Status: Record Sync to input stream packets (High = in sync).\n
 *	In DVB-ASI mode the synchronisation to the incoming data stream is achieved
 *	by the main controller. The synchroniser has to detect the transport stream
 *	sync byte, H"47", for every packet, 188, 204 or 127 bytes apart, depending on the
 *	packet size.\n
 *	When Synchronisation is lost this bit is set LOW.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RSYNC	BIT(2)

/**
 *	Record Status: Record Carrier Detected (High = carrier available).\n
 *	This bit is generated by the cable equaliser. As long as this bit is
 *	set HIGH a valid carrier is detected by the input circuitry.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RCD	BIT(3)

/**
 *	Record Status: Record Lock to input stream (High = locked to stream).\n
 *	This flag is generated by the GS9060 input device. This bit will be HIGH whenever
 *	the device has correctly received and locked to SMPTE compliant data in SMPTE mode
 *	or DVB-ASI compliant data in DVB-ASI mode or when the re-clocker
 *	has achieved lock in RAW data (Data Through) mode.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RLOCK	BIT(4)

/**
 *	Record Status: Indicates that the bitrate measurement is done based on data byte
 *	counting, over a fixed time interval instead of PCR time interval (No PCR was found).\n
 *	When HIGH, the register pair m_BitrateBytesCount and m_BitrateTimeInterval may be
 *	read to compute the bitrate.
 *	\see m_BitrateBytesCount
 *	\see m_BitrateTimeInterval
 *	\par
 *	Applicable in DVB mode.
 */
#define AT_STAT_RBRTIMELOCK	BIT(5)

/**
 *	Record Status: Indicates that the bitrate measurement is done based on data byte
 *	counting, over a PCR time interval.\n
 *	When HIGH, the register pair m_BitrateBytesCount and m_BitrateTimeInterval may be
 *	read to compute the bitrate.
 *	\see m_BitrateBytesCount
 *	\see m_BitrateTimeInterval
 *	\par
 *	Applicable in DVB mode.
 */
#define AT_STAT_RBRPCRLOCK	BIT(6)

// Play status

/**
 *	Play Status: Play lock (High = play lock).\n
 *	This flag is generated by the GS9062 output device. This bit will be LOW whenever
 *	the device has correctly received and locked to SMPTE compliant data in SMPTE mode
 *	or DVB-ASI compliant data in DVB-ASI mode or when the re-clocker has achieved
 *	lock in RAW data (Data Through) mode.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PLOCK	BIT(8)

/**
 *	Record Error: Record SDram fifo Empty (High = Empty).\n
 *	The SDRAM used as Fifo will set the R_SDFE flag HIGH whenever the FIFO is empty.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RSDFE	BIT(22)

/**
 *	Record Error: Record SDram fifo Full (High = Full).\n
 *	The SDRAM used as Fifo will set the R_SEFF error flag HIGH whenever an overflow
 *	condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_RSDFF	BIT(23)

/**
 *	Play Error: Play output SDRam fifo Empty.\n
 *	The out going DVB data stream is processed by the main controller.\n
 *	The Fifo used by this section will set the P_SDFE error flag HIGH whenever an
 *	underflow condition has occurred.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PSDFE	BIT(30)

/**
 *	Play Error: Play output SDRam fifo Full.\n
 *	The output Fifo will set the P_SDFF flag HIGH when the Fifo is full.
 *	\par
 *	Applicable in all modes
 */
#define AT_STAT_PSDFF	BIT(31)



//  #define FPGA_I2C_WD0	0x90

/**
 *	Address of the PID (Program ID) table register.\n
 *	The PID table is written as a sequence of PID values, written to
 *	to this register. The maximum number of PID's allowed can be read
 *	from m_PidTblSz.\n
 *	To clear the PID table see AT_RCONF_PIDCLR.
 *	\see m_PidTblSz
 */
#define AT_REG_PIDTBL	0x60


/**
 *	@}
 */

#endif
