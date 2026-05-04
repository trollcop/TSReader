
#ifndef _DVBfrontend_H_
#define _DVBfrontend_H_

#ifndef LINUX
	#ifdef BUILDING_API
	#include "..\common.h"
	#else
	#include "common.h"
	#endif
#else
#include "common.h"
#endif

/** @defgroup dvbsource_types Data types
 *	Data types used as parameters by the command codes.
 */
/*@{*/

/** DVB Source type.
 *	What type of modulation is used.
 */
typedef enum src_type {
    SRC_QPSK,	///< ex. Satellite
    SRC_QAM,	///< ex. Cable
    SRC_OFDM	///< ex. Terrestrial
} src_type_t;


/** DVB Source capabilities.
 *	What are the capabilities of the dvb source.
 */
typedef enum src_caps {
	SRC_IS_STUPID                  = 0,
	SRC_CAN_INVERSION_AUTO         = 0x1,
	SRC_CAN_FEC_1_2                = 0x2,
	SRC_CAN_FEC_2_3                = 0x4,
	SRC_CAN_FEC_3_4                = 0x8,
	SRC_CAN_FEC_4_5                = 0x10,
	SRC_CAN_FEC_5_6                = 0x20,
	SRC_CAN_FEC_6_7                = 0x40,
	SRC_CAN_FEC_7_8                = 0x80,
	SRC_CAN_FEC_8_9                = 0x100,
	SRC_CAN_FEC_AUTO               = 0x200,
	SRC_CAN_QPSK                   = 0x400,
	SRC_CAN_QAM_16                 = 0x800,
	SRC_CAN_QAM_32                 = 0x1000,
	SRC_CAN_QAM_64                 = 0x2000,
	SRC_CAN_QAM_128                = 0x4000,
	SRC_CAN_QAM_256                = 0x8000,
	SRC_CAN_QAM_AUTO               = 0x10000,
	SRC_CAN_TRANSMISSION_MODE_AUTO = 0x20000,
	SRC_CAN_BANDWIDTH_AUTO         = 0x40000,
	SRC_CAN_GUARD_INTERVAL_AUTO    = 0x80000,
	SRC_CAN_HIERARCHY_AUTO         = 0x100000,
	SRC_CAN_RECOVER                = 0x20000000,
	SRC_CAN_CLEAN_SETUP            = 0x40000000,
	SRC_CAN_MUTE_TS                = 0x80000000
} src_caps_t;

/** \brief DVB Source information.
 *
 *	Information about the installed dvb source device.
 */
struct dvb_frontend_info {
	char		name[128];
    src_type_t	type;
    u32			frequency_min;
    u32			frequency_max;
	u32			frequency_stepsize;
	u32			frequency_tolerance;
	u32			symbol_rate_min;
    u32			symbol_rate_max;
	u32			symbol_rate_tolerance;     /* ppm */
	u32			notifier_delay;            /* ms */
	u16			nrConstels;					///< Number of constellations read in one go (read-only for the application)
	s16			*constels;					///< Constellation I (real part) / Q (imaginary part) in sequence.
	s16			ConstelIBound;				///< Upper value of the highest I point
	s16			ConstelQBound;				///< Upper value of the highest Q point
    s32			rflevel_min;				///< minimum RF output level (in 0.1dBm steps)
    s32			rflevel_max;				///< maximum RF output level (in 0.1dBm steps)
	s32			rflevel_stepsize;			///< RF output level step size (in 0.1dBm steps)
	src_caps_t	caps;						///< OBSOLETE
	u16			nrIffreqs;					///< number of IF frequencies
	const u32	*iffreqs;					///< IF frequency list
	u32			ifrange;					///< IF shift range
	s16			ifgain_min;					///< IF gain low range (for modulators AD9778 only)
	s16			ifgain_max;					///< IF gain high range (for modulators AD9778 only)
	s16			ifoffset_min;				///< IF offset low range (for modulators AD9778 only)
	s16			ifoffset_max;				///< IF offset high range (for modulators AD9778 only)
};


/** \brief struct for a Diseqc master command.
 *
 *	Diseqc master command.
 *	A message sent from the DVB Source to DiSEqC capable equipment.
 *  Check out the DiSEqC bus spec available on http://www.eutelsat.org/ for
 *  the meaning of this struct...
 *	@remarks DiSEqC support is under development.
 */
struct dvb_diseqc_master_cmd {
        u8 msg [6];///<  {framing, address, command, data [3]}  
        u8 msg_len;///<  valid values are 3...6  
};


/** \brief struct for a Diseqc slave reply.
 *
 *	Diseqc slave reply. 
 *	A reply to the DVB Source from DiSEqC 2.0 capable equipment. 
 *	@remarks DiSEqC support is under developement.
 *	@see dvb_diseqc_master_cmd
 */
struct dvb_diseqc_slave_reply {
	u8 msg [4];///<   { framing, data [3] }
	u8 msg_len;///<  valid values are 0...4, 0 means no msg  
	int  timeout;///<  return from ioctl after timeout ms with error code when no message was received
};

/** SEC voltage.
 *
 *	SEC voltage.\n
 *	The voltage is usually used with non-DiSEqC capable LNB's to switch
 *	the polarization: Vertical (13V), Horizontal (18V).
 *	When using DiSEqC equipment this voltage has to be switched
 *	according to the DiSEqC spec. 
 */
typedef enum src_sec_voltage {
        SEC_VOLTAGE_13,		///< Sets the LNB to vertical polarization
        SEC_VOLTAGE_18,		///< Sets the LNB to horizontal polarization
		SEC_VOLTAGE_OFF		///< Switches off the LNB power
} src_sec_voltage_t;


/** \brief SEC continuous tone.
 *
 *	SEC continuous tone.\n
 *	The continuous 22KHz tone is usually used with non-DiSEqC
 *	capable LNB's to switch the high/low band of a dual-band LNB.
 *	When using DiSEqC equipment, this has to be switched
 *	according to the DiSEqC spec. 
 */
typedef enum src_sec_tone_mode {
        SEC_TONE_ON,	///< non-DiSEqC: High band
        SEC_TONE_OFF	///< non-DiSEqC: Low band
} src_sec_tone_mode_t;

typedef enum ex_src_search_mode {
		AUTO_SEARCH,
		MANUAL_SEARCH
} ex_src_search_mode_t;

/** \brief Demodulator status.
 *
 *	DVB Source status.\n
 *	The status of the dvb source.
 */
typedef enum src_status {
	SRC_HAS_SIGNAL     = 0x1,	///<  found something above the noise level (AT6xx, AT8xx)
	SRC_HAS_CARRIER    = 0x2,	///<  found a DVB signal (AT6xx, AT7xx, AT8xx)
	SRC_HAS_VITERBI    = 0x4,	///<  FEC is stable (AT6xx, AT8xx)
	SRC_HAS_SYNC       = 0x8,	///<  found sync bytes (AT6xx, AT7xx, AT8xx)
	SRC_HAS_LOCK       = 0x10,	///<  everything's working... (AT6xx) 
	SRC_TIMEDOUT       = 0x20,	///<  no lock within the last ~2 seconds (reserved)
	SRC_REINIT         = 0x40	///<  DVB Source was reinitialized (reserved)
} src_status_t;                      

/** \brief DVB lock status.
 *
 *	The LOCK status of the dvb source.
 */
typedef enum src_status_dvbcb {
	SRC_HAS_FAT_LOCK     = 0x01,  ///<  has valid input transport stream
	SRC_HAS_QAMSYNC_LOCK = 0x02,  ///<  has qam synchronisation lock
	SRC_HAS_MPEG_LOCK  = 0x04,    ///<  has a MPEG lock
	SRC_HAS_BERT_LOCK  = 0x08,    ///<  has a BERT lock
} src_status_dvbcb_t;                      

/**	
 */
typedef enum src_spectral_inversion {
        INVERSION_OFF,
        INVERSION_ON,
        INVERSION_AUTO		///< The satellite tuner is normally set to Automatic inversion detection.
} src_spectral_inversion_t;

/**	
 */
typedef enum src_sec_mini_cmd {
        SEC_MINI_A,
        SEC_MINI_B
} src_sec_mini_cmd_t;

/**	
 */
typedef enum src_code_rate {
        FEC_NONE = 0,
        FEC_1_2,
        FEC_2_3,
        FEC_3_4,
        FEC_4_5,
        FEC_5_6,
        FEC_6_7,
        FEC_7_8,
        FEC_8_9,
        FEC_AUTO
} src_code_rate_t;

/**	
 */
typedef enum src_modulation {
        QPSK,
        QAM_16,
        QAM_32,
        QAM_64,
        QAM_128,
        QAM_256,
		VSB_8,		///< Annex-B (or D)
		VSB_16,		///< not supported on any device
		QAM_AUTO
} src_modulation_t;

/**	
 *	Used for OFDM (Terrestrial). Normally set to AUTO.
 */
typedef enum src_transmit_mode {
	TRANSMISSION_MODE_2K,
	TRANSMISSION_MODE_4K,
	TRANSMISSION_MODE_8K,
	TRANSMISSION_MODE_AUTO
} src_transmit_mode_t;

/**
 *	Used for OFDM (Terrestrial). Depends on the country/area.
 *
 *  BANDWIDTH_8_7_MHZ = 8Mhz for AT80/800, 7Mhz for AT81/801
 *	BANDWIDTH_7_6_MHZ = 7Mhz for AT80/800, 6Mhz for AT81/801
 */
typedef enum src_bandwidth {
	BANDWIDTH_8_7_MHZ,			
	BANDWIDTH_7_6_MHZ,
	BANDWIDTH_6_MHZ,
	BANDWIDTH_5_MHZ,
	BANDWIDTH_ENUM_END
} src_bandwidth_t;

/**	
 */
typedef enum src_guard_interval {
	GUARD_INTERVAL_1_32,
	GUARD_INTERVAL_1_16,
	GUARD_INTERVAL_1_8,
	GUARD_INTERVAL_1_4,
	GUARD_INTERVAL_AUTO
} src_guard_interval_t;


/**	
 */
typedef enum src_hierarchy {
	HIERARCHY_NONE,
	HIERARCHY_1,
	HIERARCHY_2,
	HIERARCHY_4,
	HIERARCHY_AUTO
} src_hierarchy_t;

/**	
 */
typedef enum src_filter_rolloff {
	FLT_ALPHA_12,
	FLT_ALPHA_13,
	FLT_ALPHA_15,
	FLT_ALPHA_18
} src_filter_rolloff_t;

/**	
 */
typedef enum src_interleaver {
	INTLEAV_I128_J1,	///< interleaver mode I-128, J-1	annexB
	INTLEAV_I128_J2,	///< interleaver mode I-128, J-2	annexB
	INTLEAV_I128_J3,	///< interleaver mode I-128, J-3	annexB
	INTLEAV_I128_J4,	///< interleaver mode I-128, J-4	annexB
	INTLEAV_I128_J5,	///< interleaver mode I-128, J-5	annexB
	INTLEAV_I128_J6,	///< interleaver mode I-128, J-6	annexB
	INTLEAV_I128_J7,	///< interleaver mode I-128, J-7	annexB
	INTLEAV_I128_J8,	///< interleaver mode I-128, J-8	annexB
	INTLEAV_I64_J2,		///< interleaver mode I-64,  J-2	annexB
	INTLEAV_I32_J4,		///< interleaver mode I-32,  J-4	annexB
	INTLEAV_I16_J8,		///< interleaver mode I-16,  J-8	annexB
	INTLEAV_I8_J16,		///< interleaver mode I-8,   J-16	annexB
	INTLEAV_I12_J17		///< interleaver mode I-12,  J-17	annexA/C
} src_interleaver_t;

/** \brief QPSK parameters (Satellite).
 *
 *	QPSK parameters for satellite QPSK DVB receivers
 */
struct dvb_qpsk_parameters {
        u32           symbol_rate;///< symbol rate in Symbols per second
        src_code_rate_t  fec_inner;///< forward error correction
};


/** \brief QAM parameters (Cable).
 *
 *	QAM parameters for cable QAM DVB receivers
 */
struct dvb_qam_parameters {
        u32							symbol_rate;	///< symbol rate in Symbols per second
        src_code_rate_t				fec_inner;		///< forward error correction
        src_modulation_t			modulation;		///< modulation type
};

/** \brief OFDM parameters (Terrestrial).
 *
 *	OFDM parameters for terrestrial OFDM DVB receivers. 
 */
struct dvb_ofdm_parameters {
        src_bandwidth_t				bandwidth;
        src_code_rate_t				code_rate_HP;  ///< high priority stream code rate
        src_code_rate_t				code_rate_LP;  ///< low priority stream code rate
        src_modulation_t			constellation; ///< modulation type
        src_transmit_mode_t			transmission_mode;
        src_guard_interval_t		guard_interval;
        src_hierarchy_t				hierarchy_information;
		u16							bwMode67;		///< Type of IF Bandwidth filter 0 => 7&8MHz; 1 => 6&7MHz
};

/** \brief DVB Source parameters.
 *
 *	DVB Source parameters.\n
 *	The kind of parameters passed to the DVB Source device for tuning depend on the kind of hardware 
 *	you are using. All kinds of parameters are combined as a union in the DVB SourceParameters structure: 
 */
struct dvb_frontend_parameters {
    u32 frequency;     ///< (absolute) frequency in Hz for QAM/OFDM. Intermediate (output of the LNB) frequency in kHz for QPSK.
	src_spectral_inversion_t inversion;
	union {
		struct dvb_qpsk_parameters qpsk;
		struct dvb_qam_parameters  qam;
		struct dvb_ofdm_parameters ofdm;
	} u;
};

/*************************************************************************************************************************
// modulation specific definitions

/** \brief DVB Destination status. (at2700 DVB-C modulator)
 *
 *	DVB Destination status. (at2700 DVB-C modulator)
 */
typedef enum src_status_at2700 {
	DST_HAS_PCR_TS_INSYNC_CH0	= 0x1,
	DST_HAS_PCR_TS_SYNC_ERR_CH0	= 0x2,
	DST_HAS_PCR_TS_OFLOW_CH0	= 0x4,
	DST_HAS_PCR_TS_204PKT_CH0	= 0x8,
	DST_HAS_PCR_TS_INSYNC_CH1	= 0x10,
	DST_HAS_PCR_TS_SYNC_ERR_CH1	= 0x20,
	DST_HAS_PCR_TS_OFLOW_CH1	= 0x40,
	DST_HAS_PCR_TS_204PKT_CH1	= 0x80,
	DST_HAS_PCR_TS_INSYNC_CH2	= 0x100,
	DST_HAS_PCR_TS_SYNC_ERR_CH2	= 0x200,
	DST_HAS_PCR_TS_OFLOW_CH2	= 0x400,
	DST_HAS_PCR_TS_204PKT_CH2	= 0x800,
	DST_HAS_PCR_TS_INSYNC_CH3	= 0x1000,
	DST_HAS_PCR_TS_SYNC_ERR_CH3	= 0x2000,
	DST_HAS_PCR_TS_OFLOW_CH3	= 0x4000,
	DST_HAS_PCR_TS_204PKT_CH3	= 0x8000,
} src_status_at2700_t;

/** \brief DVB Destination status. (at2800 DVB-T modulator)
 *
 *	DVB Destination status. (at2800 DVB-T modulator)
 */
typedef enum src_status_at2800 {
	DST_HAS_HP_PCR_TS_INSYNC	= 0x1,
	DST_HAS_HP_PCR_TS_SYNC_ERR	= 0x2,
	DST_HAS_HP_PCR_TS_OFLOW		= 0x4,
	DST_HAS_HP_PCR_TS_204PKT	= 0x8,
	DST_HAS_LP_PCR_TS_INSYNC	= 0x10,
	DST_HAS_LP_PCR_TS_SYNC_ERR	= 0x20,
	DST_HAS_LP_PCR_TS_OFLOW		= 0x40,
	DST_HAS_LP_PCR_TS_204PKT	= 0x80,
	DST_HAS_MOD_UFLOW			= 0x100,
	DST_HAS_FIFO_OFLOW			= 0x200,
	DST_HAS_HP_TS_OFLOW			= 0x400,
	DST_HAS_HP_TS_UFLOW			= 0x800,
	DST_HAS_HP_TS_SYNC_ERR		= 0x1000,
	DST_HAS_LP_TS_OFLOW			= 0x2000,
	DST_HAS_LP_TS_UFLOW			= 0x4000,
	DST_HAS_LP_TS_SYNC_ERR		= 0x8000
} src_status_at2800_t;

/** \brief AT2700 DVB-C parameters (Cable).
 *
 *	AT2700 DVB-C parameters for cable modulators
 */
struct dvb_at2700_parameters {
		bool						annexb_enable;			///< 1: enable annexB, 0: disable annexB (annexA/C mode enabled)
        u32							symbol_rate;			///< symbol rate in Symbols per second
		src_spectral_inversion_t	inversion;				///< set spectral inversion
        src_modulation_t			modulation;				///< modulation type
		src_filter_rolloff_t		filter_rolloff;			///< set filter roll off factor
		src_interleaver_t			interleaver;			///< set interleaver
};

/** \brief AT2800 DVB-T/H parameters (Terrestrial).
 *
 *	AT2800 DVB-T/H parameters for terrestrial modulators
 */
struct dvb_at2800_parameters {
        src_bandwidth_t				bandwidth;				///< channel bandwidth selection
        src_code_rate_t				code_rate_HP;			///< high priority stream code rate
        src_code_rate_t				code_rate_LP;			///< low priority stream code rate
        src_modulation_t			constellation;			///< modulation type
        src_transmit_mode_t			transmission_mode;		///< transmission mode selection
        src_guard_interval_t		guard_interval;			///< quart interval selection
        src_hierarchy_t				hierarchy_information;	///< hierarchical mode selection
		src_spectral_inversion_t	inversion;				///< spectral inversion slection
		bool						dvb_h_en;				///< enable dvb-h mode in ofdm fpga core
		u16							dvb_h_cell_id;			///< cell id number for dvb-h
		bool						dvb_h_indepth_int;		///< enable indepth interleaver for dvb-h
		bool						dvb_h_HP_time_slice;	///< enable high priority time slicing for dvb-h
		bool						dvb_h_HP_MPE_FEC;		///< enable high priority MPE FEC for dvb-h
		bool						dvb_h_LP_time_slice;	///< enable low priority time slicing for dvb-h
		bool						dvb_h_LP_MPE_FEC;		///< enable low priority MPE FEC for dvb-h
};

/** Modulator RF anf IF parameters
 *  Used for AT2600, AT2700, and AT2800 modulators
 */
struct dvb_if_rf_parameters {
	bool RfLocked;		///< status: 1: RF pll has locked, 0: RF pll is not locked
	bool RfIfSel;		///< selects if RF output is selected or IF output. 1: RF is selected, 0: IF is selected
	u32 RfFrequency;	///< (absolute) RF frequency in Hz for QAM/OFDM. 
	s32 IfFrequency;	///< (absolute) IF frequency in Hz for QAM/OFDM.
	s32 RfoutputLevel;	///< (absolute) output level for the modulators. (in 0.1dBm)

};

/** Modulator IF calibration parameters
 *  Used for AT2600, AT2700, and AT2800 modulators
 */
struct dvb_if_cal_parameters {
	s16 IfGainI;		///< IF I channel gain setting
	s16 IfGainQ;		///< IF Q channel gain setting
	s16 IfOffsetI;		///< IF I channel offset setting
	s16 IfOffsetQ;		///< IF Q channel offset setting
};

struct status_sigquality
{
	BOOL	m_IsValid;	///< Must be non-zero to carry a valid SNR and EVM
	double	m_Snr;		///< The signal to noise ration in [dB]
	double	m_Evm;		///< The Error vector magnitude in [%]
};

/*************************************************************************************************************************
/*@}*/

/** @defgroup dvbsource_cmd DVB-Source command codes
*	These commands can be executed using the DvbSource(u32 nCmd, void* arg)
*	method of the CATBoard class. It enables the user to communicate with the 
*	source device of the board, in most cases this is a tuner.
*	\see	\ref Satellite (DVB-S)
*	\see	\ref Terrestrial (DVB-T)
*	\see	\ref Cable_Annex_A (DVB-C)
*	\see	\ref Cable_Annex_B (DVB-C)
*	\see	\ref ModulatorT (DVB-T/H Modulator)
*	\see	\ref ModulatorC (DVB-C Modulator)
*/
/*@{*/

/** 
 *	Get information about the DVB source.
 *	@remarks Application: AT6xx, AT7xx, AT8xx.
 *	@param arg dvb_frontend_info *.
 *
 *	@code
struct dvb_frontend_info info;
pBoard->DvbSource(SRC_GET_INFO, &info);
...
...  info contains the device information of the DVB source
...	
	@endcode
 */
#define SRC_GET_INFO                0x0001

/** 
 *	If the LNB has been automatically powered off due to power overload, 
 *	this ioctl call restores the power to the bus. 
 *	This call has no effect if the device is manually powered off. 
 *	@remarks Application: AT6xx.
 *	@remarks DiSEqC support is under developement.
 */
#define SRC_DISEQC_RESET_OVERLOAD   0x0002

/** 
 *	This ioctl call is used to send a DiSEqC command. 
 *	@remarks Application: AT6xx.
 *	@param arg dvb_diseqc_master_cmd *.
 *	@remarks DiSEqC support is under developement.
 */
#define SRC_DISEQC_SEND_MASTER_CMD  0x0003

/** 
 *	This ioctl call is used to receive reply to a DiSEqC 2.0 command.
 *	@remarks Application: AT6xx.
 *	@param arg dvb_diseqc_slave_reply *.
 *	@remarks DiSEqC support is under developement.
 */
#define SRC_DISEQC_RECV_SLAVE_REPLY 0x0004

/**  
 *	This ioctl call is used to send a 22KHz tone burst. 
 *	@remarks Application: AT6xx.
 *	@param arg src_sec_mini_cmd_t.
 *	@remarks DiSEqC support is under developement.
 */
#define SRC_DISEQC_SEND_BURST       0x0005

/** 
 *	This call is used to set the generation of the continuous 22kHz tone.\n
 *	The tone is put on the LNB's power supply and makes it switch to the
 *	local oscillator for the High frequency band.
 *	@remarks Application: AT6xx.
 *	@param arg src_sec_tone_mode_t.
 *
 *	@code
src_sec_tone_mode_t ToneMode;
ToneMode = SEC_TONE_ON;	// or SEC_TONE_OFF
pBoard->DvbSource(SRC_SET_TONE, (void*)ToneMode);
	@endcode
 */
#define SRC_SET_TONE                0x0006

/** 
 *	This call is used to set the LNB voltage, to set the polarity.\n
 *	SEC_VOLTAGE_13 sets the LNB to Vertical polarity\n
 *	SEC_VOLTAGE_18 sets the LNB to Horizontal polarity.\n
 *	Application: AT6xx.
 *	@param arg src_sec_voltage_t.
 *
 *	@code
src_sec_voltage_t LnbVoltage;
LnbVoltage = SEC_VOLTAGE_13;	// or SEC_VOLTAGE_18 or SEC_VOLTAGE_OFF
pBoard->DvbSource(SRC_SET_VOLTAGE, (void*)LnbVoltage);
	@endcode
 */
#define SRC_SET_VOLTAGE             0x0007

/** 
 *	If arg != 0 enables a 1Volt higher voltages instead of 13/18V (to compensate for long cables).\n
 *	Warning: The 'long' parameter is passes directly, not by address.
 *	Not all DVB adapters support this ioctl.
 *	Application: AT6xx.
 *	@param arg long.
 *
 *	@code
 long LnbHighVoltage;
 LnbHighVoltage = TRUE;	// or FALSE
 pBoard->DvbSource(SRC_ENABLE_HIGH_LNB_VOLTAGE, (void*)LnbHighVoltage);
	@endcode
 */
#define SRC_ENABLE_HIGH_LNB_VOLTAGE 0x0008

 /** 
 *	This ioctl call returns status information about the dvb source. 
 *	Application: AT6xx, AT7xx, AT8xx.
 *	@param arg src_status_t *.
 *
 *	@code
src_status_t status;
pBoard->DvbSource(SRC_READ_STATUS, &status);
 ...
 ...  status contains the current status information of the DVB source
 ...	
	@endcode
 */
#define SRC_READ_STATUS             0x0009

/**  
 *	This ioctl call returns the bit error rate \b after the FEC (forware error correction) for the
 *	signal currently received/demodulated by the dvb source.
 *	Application: AT6xx, AT8xx.
 *	@param arg uint32_t *.
 *
 *	@code
uint32_t bitErrorRate;	// errors per second
pBoard->DvbSource(SRC_READ_BER, &bitRateError);
	@endcode
 */
#define SRC_READ_BER                0x000a

/** 
 *	This ioctl call returns the signal strength value for the signal currently received by 
 *	the dvb source. Note that no attempt has been made
 *	to make it a linear scale, nor is it guaranteed that the extends are reached.
 *	Application: AT6xx, AT7xx, AT8xx.
 *	@param arg int16_t *.
 *
 *	@code
int16_t sigStrength;	// 0..65535
pBoard->DvbSource(SRC_READ_SIGNAL_STRENGTH, &sigStrength);
	@endcode
 */
#define SRC_READ_SIGNAL_STRENGTH    0x000b

/** 
 *	This ioctl call returns the signal-to-noise ratio for the signal currently received by
 *	the dvb source. It can be used to display a 'bar'.\n
 *	Unit of measure is 1/32th of a dB for:
 *	\li AT60/600
 *	\li AT72/720
 *	\li AT80/800
 *
 *	Note that some attempt has been made to make it usable scale in dB, although the accuracy is limited\n
 *	Application: AT6xx, AT72x and AT8xx.
 *	@note: The AT700 does not output a SNR.
 *	@param arg int16_t *.
 *
 *	@code
int16_t sigNoiseRatio;	// in units of 1/32th dB
pBoard->DvbSource(SRC_READ_SNR, &sigNoiseRatio);
	@endcode
 */
#define SRC_READ_SNR                0x000c

/** SRC_READ_UNCORRECTED_BLOCKS.
 *	@remarks Reserved for future use.

 */
#define SRC_READ_UNCORRECTED_BLOCKS 0x000d

/** 
 *	This ioctl call starts a tuning operation using specified parameters.
 *	The dvb_frontend_parameters in 'parms' are saved in CATBoard to allow
 *	a restore using SRC_RESTORE_FRONTEND.
 *	@param arg dvb_frontend_parameters *.
 *
 *	@code
dvb_frontend_parameters parms;

parms.frequency = nFreq;
parms.inversion = INVERSION_AUTO;

parms.u.qpsk.fec_inner = FEC_AUTO;
parms.u.qpsk.symbol_rate = nSymbRate;

pBoard->DvbSource(SRC_SET_FRONTEND, &parms);
	@endcode
 */
#define SRC_SET_FRONTEND            0x000e

/** 
 *	This ioctl call queries the currently effective dvb source parameters.
 *	@param arg dvb_frontend_parameters *.
 *
 *	@code
pBoard->DvbSource(SRC_GET_FRONTEND, &info);
sTxt.Format("%lf", info.frequency/1000000.0+(m_Freq >= iLnb.dBandSwGhz ? 
											iLnb.dLoHiGHz : iLnb.dLoLoGHz));
	@endcode
 */
#define SRC_GET_FRONTEND            0x000f



/** 
 *	This ioctl call restores the last front_end parameters to the dvb source (device).
 *	@remarks Added: june 17, 2005
 */
#define SRC_RESTORE_FRONTEND		0x0010

/** 
 *	This ioctl call resets the dvb source.
 *	@remarks Reserved for future use.
 */
#define SRC_RESET					0x0012

/** 	
 *	This ioctl call initializes the dvb source.
 */
#define SRC_INIT					0x0013

/** 	
 *	This ioctl call sets the modulators modulation parameters.\n
 *	@param arg dvb_at2700_parameters * or dvb_at2800_parameters *.
 *	@code
 // For the DVB-C Modulator
 dvb_at2700_parameters params;

 memset(params, 0, sizeof(params));

 params.modulation		= QAM_16;			// set modulation. (caps: QAM_16, QAM_32, QAM_64, QAM_128, QAM_256)
 params.filter_rolloff	= FLT_ALPHA_15;		// set filter roll-off. (caps: FLT_ALPHA_12, FLT_ALPHA_13, FLT_ALPHA_15, FLT_ALPHA_18)
 params.interleaver		= INTLEAV_I12_J17;	// set interleaver mode. (caps: INTLEAV_I128_J1, INTLEAV_I128_J2, INTLEAV_I128_J3, INTLEAV_I128_J4, INTLEAV_I128_J5, INTLEAV_I128_J6, INTLEAV_I128_J7, INTLEAV_I128_J8, INTLEAV_I64_J2, INTLEAV_I32_J4, INTLEAV_I16_J8, INTLEAV_I8_J16, INTLEAV_I12_J17)
 params.inversion		= INVERSION_OFF;	// set inversion.
 params.symbol_rate		= 6000000;			// set symbol rate. (in MSymbols/s)
 params.annexb_enable	= FALSE;			// enable annexB mode.
 pBoard->DvbSource(SRC_SET_MOD_PARAMS, &params);

 // For the DVB-T/H Modulator
 dvb_at2800_parameters DvbTOfdmParams;

 DvbTOfdmParams.constellation		= (src_modulation_t)m_iModulation;
 DvbTOfdmParams.transmission_mode	= (src_transmit_mode_t)m_iTranMode;
 DvbTOfdmParams.guard_interval		= (src_guard_interval_t)m_iGuardInt;
 DvbTOfdmParams.code_rate_HP		= (src_code_rate_t)m_iHPFecCodeRate;
 DvbTOfdmParams.code_rate_LP		= FEC_7_8;
 DvbTOfdmParams.bandwidth			= (src_bandwidth_t)m_iBandWidth;
 DvbTOfdmParams.hierarchy_information= HIERARCHY_NONE;
 DvbTOfdmParams.inversion			= (src_spectral_inversion_t)m_iSpecInv;
 DvbTOfdmParams.dvb_h_cell_id		= (u16)m_nCellID;
 DvbTOfdmParams.dvb_h_LP_time_slice	= false;
 DvbTOfdmParams.dvb_h_LP_MPE_FEC	= false;

 DvbTOfdmParams.dvb_h_HP_time_slice	= m_bMpeFec;
 DvbTOfdmParams.dvb_h_HP_MPE_FEC	= m_bTimeSlice;
 DvbTOfdmParams.dvb_h_en			= m_bDvbHEn;
 DvbTOfdmParams.dvb_h_indepth_int	= m_bInDepthInt;

 TheBoard.DvbSource(SRC_SET_MOD_PARAMS, &DvbTOfdmParams);

 @endcode
 */
#define SRC_SET_MOD_PARAMS			0x0014

/** 	
 *	This ioctl call sets the modulators modulation parameters.\n
 *	@param arg src_status_at2700_t * or src_status_at2800_t *.
 *	@code
 // For the DVB-C Modulator
 src_status_at2700_t eStatus;
 TheBoard.DvbSource(SRC_GET_MOD_STAT, &eStatus);

 // For the DVB-T/H Modulator
 src_status_at2800 eStatus;
 TheBoard.DvbSource(SRC_GET_MOD_STAT, &eStatus);

 @endcode
 */
#define SRC_GET_MOD_STAT			0x0015

 /** 	
 *	This ioctl call sets the modulators IF and RF parameters.\n
 *	@param arg dvb_if_rf_parameters *.
 *	@code
 dvb_if_rf_parameters IfRfParams;

 // set RF and IF parameters
 if (m_iRfIfSel)
	 IfRfParams.RfIfSel	 = TRUE;
 else
	IfRfParams.RfIfSel	 = FALSE;

 IfRfParams.IfFrequency	 = (s32)(m_dFreqIF*1000000.0);
 IfRfParams.RfFrequency	 = (u32)(m_dFreqRF*1000000.0);
 IfRfParams.RfoutputLevel = (s32)(m_dRFLevel*10);

 // send parameter list
 TheBoard.DvbSource(SRC_SET_MOD_IF_RF_PARAMS, &IfRfParams);

 @endcode
 */
#define SRC_SET_MOD_IF_RF_PARAMS	0x0016

/** 	
 *	This ioctl call gets the modulators IF and RF status.\n
 *	@param arg dvb_if_rf_parameters *.
 *	@code
 dvb_if_rf_parameters IfRfParams;
 TheBoard.DvbSource(SRC_GET_MOD_IF_RF_STAT, &IfRfParams);

 @endcode
 */
#define SRC_GET_MOD_IF_RF_STAT		0x0017

/** 	
*	This ioctl call sets the modulators calibration parameters.\n
*	\internal This function must not be used by application programmers. It
*	will damage calibration settings of boards.
*/
#define SRC_SET_MOD_CAL				0x0018

/** 	
*	This ioctl call stores the modulators calibration parameters in EEPROM.
*	\internal This function must not be used by application programmers. It
*	will damage calibration settings of boards.
*/
#define SRC_STORE_MOD_CAL			0x0019

/** 	
*	This ioctl call restores the modulators calibration parameters in EEPROM.
*	\internal This function must not be used by application programmers. It
*	will damage calibration settings of boards.
*/
#define SRC_RESTORE_MOD_CAL			0x001A

 /**
 *	This ioctl call retrieves the I/Q constellation as numbers.
 *	Only available on some tuner/decoders.
 *	@note Reading the constellation info is 'statistical' some current values
 *			are read at a given time. Also, the process in rather slow. You may want
 *			to turn it off when the display is not visible.
 *	@remarks Added in V2.93 (mar 20, 2006 SK)
 *	@param arg dvb_frontend_info *.
 *
 *	@code
struct dvb_frontend_info info;
pBoard->DvbSource(SRC_GET_CONSTELLATION, &info);
for (int i=0; i<info.nrConstels; i++)
	sTxt.Format("%d:%d", info.constels[2*i], info.constels[2*i+1]);
	@endcode
 */
#define SRC_GET_CONSTELLATION		0x0020

 /**
 *	This ioctl call retrieves the signal quality metrics.\n
 *	The metric can only be computed if the constellation data is available from
 *	the receiver/decoder. Calls to SRC_GET_CONSTELLATION must be made periodically
 *	to provide the signal quality system with constellation points.\n
 *	The SNR and EVM are computed from the constellation point information.
 * (see \ref SRC_GET_CONSTELLATION).
 *	@remarks Added in V3.14 (jan 10, 2007 SK)
 *	@param arg status_sigquality *.
 *
 *	@code
struct status_sigquality sigquality;
pBoard->DvbSource(SRC_GET_SIGNALQUALITY, &sigquality);
	@endcode
 */
#define SRC_GET_SIGNALQUALITY		0x0021

/**  
 *	This ioctl call returns the bit error rate \b before the FEC (forware error correction) for the
 *	signal currently received/demodulated by the dvb source.
 *	Application: AT72x.
 *	@param arg uint32_t *.
 *
 *	@code
uint32_t UnCorrBitErrorRate;	// errors per second
pBoard->DvbSource(SRC_READ_BER, &UnCorrBitErrorRate);
	@endcode
 */
#define SRC_READ_UNCORECTED_BER		0x0022


//
//Control codes added to original code...
/** @internal @def IOCTL_GET_REGS
 */
#define IOCTL_GET_REGS		   0x1000

//
//Put a byte to the tuner registers..

/** @internal @def IOCTL_PUT_BYTE
 */
#define IOCTL_PUT_BYTE			0x1002

//
//Get a byte from the tuner registers..
/** @internal @def IOCTL_GET_BYTE
 */
#define IOCTL_GET_BYTE			0x1003

/**
 *	This ioctl call queries the currently effective LNB voltage.
 *	@param arg dvb_frontend_parameters *.
 *
 *	@code
src_sec_voltage_t Volt = (src_sec_voltage_t)pBoard->DvbSource(IOCTL_READ_VOLTAGE, NULL);
	@endcode
 */
#define IOCTL_READ_VOLTAGE		   0x1001


/*@}*/





#define FE_GET_INFO                0x0001
//_IOR('o', 61, struct dvb_frontend_info)

#define FE_DISEQC_RESET_OVERLOAD   0x0002
//_IO('o', 62)
#define FE_DISEQC_SEND_MASTER_CMD  0x0003
//_IOW('o', 63, struct dvb_diseqc_master_cmd)
#define FE_DISEQC_RECV_SLAVE_REPLY 0x0004
//_IOR('o', 64, struct dvb_diseqc_slave_reply)
#define FE_DISEQC_SEND_BURST       0x0005
//_IO('o', 65)  /* fe_sec_mini_cmd_t */

#define FE_SET_TONE                0x0006
//_IO('o', 66)  /* fe_sec_tone_mode_t */
#define FE_SET_VOLTAGE             0x0007
//_IO('o', 67)  /* fe_sec_voltage_t */
#define FE_ENABLE_HIGH_LNB_VOLTAGE 0x0008
//_IO('o', 68)  /* int */

#define FE_READ_STATUS             0x0009
//_IOR('o', 69, fe_status_t)
#define FE_READ_BER                0x000a
//_IOR('o', 70, u32)
#define FE_READ_SIGNAL_STRENGTH    0x000b
//_IOR('o', 71, u16)
#define FE_READ_SNR                0x000c
//_IOR('o', 72, u16)
#define FE_READ_UNCORRECTED_BLOCKS 0x000d
//_IOR('o', 73, u32)

#define FE_SET_FRONTEND            0x000e
//_IOW('o', 76, struct dvb_frontend_parameters)
#define FE_GET_FRONTEND            0x000f
//_IOR('o', 77, struct dvb_frontend_parameters)
#define FE_GET_EVENT               0x0010
//_IOR('o', 78, struct dvb_frontend_event)
#define FE_SLEEP				   0x0011
#define FE_RESET				   0x0012
#define FE_INIT					   0x0013



#endif /*_DVBfrontend_H_*/
