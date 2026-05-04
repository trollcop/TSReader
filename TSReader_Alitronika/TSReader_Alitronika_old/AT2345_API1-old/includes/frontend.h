
#ifndef _DVBfrontend_H_
#define _DVBfrontend_H_

#include "types.h"

/** @defgroup dvbsource_cmd DVB-Source command codes
 *	These commands can be executed using the DvbSource(u32 nCmd, void* arg)
 *	method of the CATBoard class. It enables the user to communicate with the 
 *	source device of the board, in most cases this is a tuner.
 *	\see	\ref Satellite (DVB-S)
 *	\see	\ref Terrestrial (DVB-T)
 *	\see	\ref Cable (DVB-C)
 */
/*@{*/

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

/** DVB Source information. 
 *	Information about the installed dvb source device.
 */
struct dvb_frontend_info {
	char       name[128];
    src_type_t  type;
    u32      frequency_min;
    u32      frequency_max;
	u32      frequency_stepsize;
	u32      frequency_tolerance;
	u32      symbol_rate_min;
    u32      symbol_rate_max;
	u32      symbol_rate_tolerance;     /* ppm */
	u32      notifier_delay;            /* ms */
	src_caps_t  caps;
};


/** Diseqc master command.
 *	A message sent from the DVB Source to DiSEqC capable equipment.
 *  Check out the DiSEqC bus spec available on http://www.eutelsat.org/ for
 *  the meaning of this struct...
 *	@remarks DiSEqC support is under developement.
 */
struct dvb_diseqc_master_cmd {
        u8 msg [6];///<  {framing, address, command, data [3]}  
        u8 msg_len;///<  valid values are 3...6  
};


/** Diseqc slave reply. 
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


/** SEC continuous tone. 
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

/** DVB Source status.
 *	The status of the dvb source.
 */
typedef enum src_status {
	SRC_HAS_SIGNAL     = 0x01,///<  found something above the noise level (AT6xx, AT8xx)
	SRC_HAS_CARRIER    = 0x02,///<  found a DVB signal (AT6xx, AT7xx, AT8xx)
	SRC_HAS_VITERBI    = 0x04,///<  FEC is stable (AT6xx, AT8xx)
	SRC_HAS_SYNC       = 0x08,///<  found sync bytes (AT6xx, AT7xx, AT8xx)
	SRC_HAS_LOCK       = 0x10,///<  everything's working... (AT6xx) 
	SRC_TIMEDOUT       = 0x20,///<  no lock within the last ~2 seconds (reserved)
	SRC_REINIT         = 0x40 ///<  DVB Source was reinitialized (reserved)
} src_status_t;                      

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
	QAM_AUTO
} src_modulation_t;



/**	
 *	Used for OFDM (Terrestrial). Normally set to AUTO.
 */
typedef enum src_transmit_mode {
	TRANSMISSION_MODE_2K,
	TRANSMISSION_MODE_8K,
	TRANSMISSION_MODE_AUTO
} src_transmit_mode_t;


/**
 *	Used for OFDM (Terrestrial). Depends on the country/area.
 */
typedef enum src_bandwidth {
	BANDWIDTH_8_MHZ,
	BANDWIDTH_7_MHZ,
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

/** QPSK parameters.
 *	For satellite QPSK DVB Sources	
 */
struct dvb_qpsk_parameters {
        u32           symbol_rate;///< symbol rate in Symbols per second
        src_code_rate_t  fec_inner;///< forward error correction
};


/** QAM parameters.
 *	For cable QAM DVB Sources
 */
struct dvb_qam_parameters {
        u32            symbol_rate;	///< symbol rate in Symbols per second
        src_code_rate_t   fec_inner; ///< forward error correction
        src_modulation_t  modulation;///< modulation type
};
/** OFDM parameters.
 *	For terrestrial OFDM DVB Sources 
 */
struct dvb_ofdm_parameters {
        src_bandwidth_t      bandwidth;
        src_code_rate_t      code_rate_HP;  ///< high priority stream code rate
        src_code_rate_t      code_rate_LP;  ///< low priority stream code rate
        src_modulation_t     constellation; ///< modulation type
        src_transmit_mode_t  transmission_mode;
        src_guard_interval_t guard_interval;
        src_hierarchy_t      hierarchy_information;
};
/** DVB Source parameters.
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
/*@}*/

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
 *	If arg != 0 enables a 1Volt higher voltages instead of 13/18V (to compensate for long cables). 
 *	Not all DVB adapters support this ioctl.
 *	Application: AT6xx.
 *	@param arg int.
 *
 *	@code
 int LnbHighVoltage;
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
 *	This ioctl call returns the bit error rate for the signal currently received/demodulated 
 *	by the dvb source.
 *	Application: AT6xx, AT8xx.
 *	@param arg uint32_t *.
 *
 *	@code
uint32_t bitRateError;	// errors per second
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
 *	the dvb source. It can be used to display a 'bar'. Note that no attempt has been made
 *	to make it a linear scale, nor is it guaranteed that the extends are reached.
 *	Application: AT6xx.
 *	@param arg int16_t *.
 *
 *	@code
int16_t sigNoiseRatio;	// 0..65535
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
#define SRC_RESET				   0x0012

/** 	
 *	This ioctl call initializes the dvb source.
 *	@remarks Reserved for future use.
 */
#define SRC_INIT					   0x0013
//
//Control codes added to original code...
/** @internal @def IOCTL_GET_REGS
 */
#define IOCTL_GET_REGS		   0x1000

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
#endif /*_DVBfrontend_H_*/





