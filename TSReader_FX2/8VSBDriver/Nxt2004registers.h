/**********************************************************************
 * NXT2004REGISTERS.H
 * Defines all NXT2004 registers used in this software.
 *
 * Copyright (C) 2002 NxtWave Communications, Inc.
 *
 * $Log:   //panxtbdc01/AppsEng/PVCS/Application Eval/archives/EVAL2004/Include/Nxt2004registers.h-arc  $
 * 
 *    Rev 1.14   Apr 15 2003 11:25:52   raggarwa
 * Arranged registers in order of their offsets
 * 
 *    Rev 1.13   Mar 06 2003 09:37:18   raggarwa
 * Changed power control mask value; PR 27/2005
 * 
 *    Rev 1.12   Jan 21 2003 17:22:04   raggarwa
 * Removed RDC bits; Renamed UC_I2C_ to UC_IIC_
 * 
 *    Rev 1.11   Oct 01 2002 14:48:32   raggarwa
 * Added Nxtenna registers
 * 
 *    Rev 1.10   Jun 18 2002 09:34:04   raggarwal
 * Alpha3 Release
 * 
 *    Rev 1.9   May 17 2002 16:31:58   raggarwal
 * Alpha2 Release
 * 
 *    Rev 1.3   Mar 08 2002 16:25:38   raggarwal
 * Alpha Release
 * 
 **********************************************************************/

#ifndef NXT2004REG_H
#define NXT2004REG_H

/*
******************************************************************************
Defines
******************************************************************************
*/
/* Register Addresses and Masks/Values */
#define	MISC_DEV_ID				0x0000
#define		DEV_ID_LENGTH		5		/* dev, fab, month, yearMSB, yearLSB */
#define		DEV_ID_DEVICE		0
#define		DEV_ID_FAB			1
#define		DEV_ID_MONTH		2
#define		DEV_ID_YEAR_MSB		3
#define		DEV_ID_YEAR_LSB		4

#define	MISC_ROM_MASK_VERSION	0x0005
#define		CODE_VERSION_LENGTH	3
#define		CODE_VERSION_MAJOR	0
#define		CODE_VERSION_CUSTOM	1
#define		CODE_VERSION_MINOR	2

#define	MISC_RESET_CONTROL		0x0008
#define		SOFT_RESET_ALL		0xFF
#define		SOFT_RESET_ADC_FIFO	0x80
#define		SOFT_RESET_BERT		0x40
#define		SOFT_RESET_SMOOTHER	0x10
#define		SOFT_RESET_FAT_AGC	0x08	/* soft-reset AGC core - cleared by driver */
#define		SOFT_RESET_FAT_FE	0x04	/* soft-reset front-end - cleared by driver */
#define		SOFT_RESET_FAT_EQ	0x02	/* soft-reset equalizer - cleared by driver */
#define		SOFT_RESET_FAT_FEC	0x01	/* soft-reset FEC - cleared by driver */
#define		SOFT_RESET_FAT_ALL	0x1F

#define	MISC_RESET_CONTROL_2	0x0009
#define		SOFT_RESET_2_ALL	0x05

#define MISC_MOD_CTRL_OUT_FMT	0x000A
#define		MOD_FMT_MASK		0xC0	/* masks thru modulation format */
#define		MOD_FMT_8VSB		0x00
#define		MOD_FMT_16VSB		0x40
#define		MOD_FMT_64QAM		0x80
#define		MOD_FMT_256QAM		0xC0
#define		OUT_FMT_POL_MASK	0x3C	/* masks thru polarity bits */
#define		OUT_FMT_POL_DAT_EN	0x04
#define		OUT_FMT_POL_PKT_SY	0x08
#define		OUT_FMT_POL_ERROR	0x10
#define		OUT_FMT_POL_CLOCK	0x20
#define		OUT_FMT_DATA_MASK	0x03	/* masks thru output format bits */
#define		OUT_FMT_DATA_SERIAL 0x02
#define		OUT_FMT_DATA_GATED	0x01


#define	MISC_ASIC_HW_STATUS_1		0x0010
#define		MPEG_OUT_SMOOTHER_EN	0x10
#define		INT_ROM_DISABLED		0x08
#define		MISC_IIC_BYPASS			0x02

#define	MISC_ASIC_HW_STATUS_0		0x0011

#define MISC_POWER_CONTROL			0x0012
#define		FAT_AGC_OUT_ENABLE		0x20
#define		MPEG_OUT_ENABLE			0x10
#define		MISC_POWER_CTL_MASK		0x04
#define		MISC_POWER_DN_FAT		0x04

#define	MISC_GPIO_ACCESS_SELECT		0x0013

#define	MISC_GPIO_OEN				0x0014

#define	MISC_GPIO_MONITOR			0x0015
#define		NXT2004_GPIO_MASK		0xFF00
#define		NXT_GPIO_ASSIGN_MASK	0xFF00	/* only lower 8 pins are assignable */

#define	MISC_TEST_IO_OEN			0x0016

#define	MISC_TEST_IO_DATA			0x0017

#define MISC_FIRMWARE_CONTROL		0x0019
#define		MISC_FW_ENABLE_FDC		0x04
#define		MISC_FW_ENABLE_RAM_DNLD	0x01

#define	MISC_ELARA_SERIAL_CMD_3		0x001A
#define		MISC_FDC_ADC_GAIN_1V	0x80
#define		MISC_FAT_ADC_GAIN_1V	0x40

#define	MISC_ELARA_SERIAL_CMD_2		0x001B
#define		MISC_ELARA_PDWN_FS		0x80
#define		MISC_ELARA_PDWN_ADC1	0x40
#define		MISC_ELARA_PDWN_ADC0	0x20

#define MISC_ELARA_SERIAL_CMD_1		0x001C
#define		MISC_ELARA_1_MASK		0x3F
#define		FS_CP_CTL_R_8			0x30	/* charge pump value for ref div = 8 */
#define		FS_CP_CTL_R_16			0x20	/* charge pump value for ref div = 16 */
#define		FS_CP_CTL_R_32			0x10	/* charge pump value for ref div = 32 */
#define		FS_CP_CTL_R_64			0x00	/* charge pump value for ref div = 64 */
#define		FS_REF_DIVIDER_R_8		0x00	/* input ref divider = 8 */
#define		FS_REF_DIVIDER_R_16		0x04	/* input ref divider = 16 */
#define		FS_REF_DIVIDER_R_32		0x08	/* input ref divider = 32 */
#define		FS_REF_DIVIDER_R_64		0x0C	/* input ref divider = 64 */

#define	MISC_ELARA_SERIAL_CMD_0		0x001D
#define		MISC_ELARA_0_MASK		0xFF

#define MISC_IIC_SLAVE_ADDR_1		0x001E	/* IIC slave address 1 */




#define	UC_CONTROL					0x0020		/* IIC Xfer Speed */
#define		UC_CTRL_SPEED_MASK		0x7F

#define UC_SERVICES						0x0021
#define		UC_SERVICES_DATA_XFER		0x80
#define		UC_SERVICES_NTSC_DET_SVC	0x40
#define		UC_SERVICES_SPECTRUM		0x20
#define		UC_SERVICES_FAT_TAP_READ	0x10
#define		UC_SERVICES_DATA_XFER_ERR	0x08
#define		UC_SERVICES_NTSC_STATUS		0x04

#define	UC_ACQ_CONTROL					0x0022
#define		UC_ACQ_FAT_STOP				0x80	/* 0=Start; 1=Immediate Stop */
#define		UC_ACQ_FAT_PAUSE_INIT		0x40	/* Pause before pre-acquisition initialization */
#define		UC_ACQ_FAT_PAUSE_TRACK		0x20	/* Pause before tracking */
#define		UC_ACQ_FAT_REACQ_DISABLE	0x10	/* 1=Disable reacquisition */

#define	UC_CMD					0x0023
#define		UC_CMD_GPIO_MASK	0x80	/* 1=Mask; 0=R/W */
#define		UC_CMD_GPIO_ASG_WR	0x40	/* 1=Assign/Write; 0=I0/Read */
#define		UC_CMD_GPIO_DATA	0x3F

#define	UC_IRQ_MASK				0x0025

#define	UC_IRQ_SOURCE			0x0026

#define	UC_AGC_START_ADDR_MSB	0x0029

#define	UC_AGC_PGM_DNLD_CTL		0x002B
#define		UC_AGC_PDC_RESET	0x80	/* holds micro in reset */
#define		UC_AGC_PDC_NO_CRC	0x40	/* disable CRC calculation */
#define		UC_AGC_PDC_CRC_FAIL	0x20	/* read to check CRC result */
#define		UC_AGC_PDC_UC_LOAD	0x10	/* enables micro to load RAM */
#define		UC_AGC_PDC_AGC_LOAD	0x08	/* directs read/write to AGC RAM */
#define		UC_AGC_PDC_ADDR_DEC	0x04	/* if TRUE then autodecrement */
#define		UC_AGC_PDC_READ_EN	0x02	/* if TRUE then read from RAM */
#define		UC_AGC_PDC_XFER_EN	0x01	/* set TRUE to enable transfer */

#define	UC_AGC_DATA_TRANSFER	0x002C

#define UC_GP_0					0x0030	/* UC_ACQ_MODE_SELECT */
#define		UC_FDC_SYMBOL_MASK	0xC0	/* mask to set FDC symbol rate */
#define		UC_GP_0_NO_CO		0x04	/* disable cochannel */
#define		UC_GP_0_MOD_FMT		0x03	/* mask to read mod format */
#define		UC_GP_0_8VSB		0x00	/* mod format = 8VSB maybe cochannel */
#define		UC_GP_0_COCHANNEL	0x01	/* mod format = 8VSB with cochannel */
#define		UC_GP_0_64QAM		0x02	/* mod format = 64QAM */
#define		UC_GP_0_256QAM		0x03	/* mod format = 256QAM */

#define UC_GP_1					0x0031	/* UC_ACQ_MODE_STATUS */
#define		UC_GP_1_FAT_LOCK	0x20	/* FAT lock condition */
#define		UC_GP_1_FAT_PAUSED	0x10	/* FAT stopped/paused by UC_ACQ_CONTROL */
#define		UC_GP_1_ADJ_REJ		0x08	/* adj rej in use */
#define		UC_GP_1_MOD_FMT		0x03	/* mask to read mod format */
#define		UC_GP_1_8VSB		0x00	/* selected 8VSB no cochannel */
#define		UC_GP_1_COCHANNEL	0x01	/* selected 8VSB with cochannel */
#define		UC_GP_1_64QAM		0x02	/* selected 64QAM */
#define		UC_GP_1_256QAM		0x03	/* selected 256QAM */

#define	UC_GP_2					0x0032

#define UC_GP_3					0x0033
#define FEC_RS_ERROR_MIRROR		0x0033

#define UC_GP_4					0x0034	/* Data Xfer Control - type and byte count */
#define		UC_IIC_DATA_XFER	0x00
#define		UC_DATA_XFER_DEV1	0x40
#define		UC_DATA_XFER_DEV0	0x20
#define		UC_MULTI_WRITE_SVC	0x10
#define		UC_XFER_SIZE_MASK	0x0F
#define		UC_NXTENNA_READ		0x60
#define		UC_NXTENNA_WRITE	0x70

#define UC_GP_5					0x0035	/* IIC Xfer device address */
#define		IIC_XFER_ADDR		UC_GP_5
#define		NXTENNA_DRIVER_STATE		0x00
#define		NXTENNA_ENABLE_DRIVER		0x00
#define		NXTENNA_STATIC_SETTING		0x01
#define		NXTENNA_TRACKING_REQUEST	0x02
#define		NXTENNA_ANT_SETTING			0x03
#define		NXTENNA_ANT_CONFIG			0x04
#define		NXTENNA_ADJ_DETECT_THOLD	0x05
#define		NXTENNA_SIGNAL_METRIC		0x06
#define		NXTENNA_CHANNEL_METRIC		0x07

#define UC_GP_6					0x0036	/* IIC Xfer data(0) */
#define		IIC_XFER_DATA_0		UC_GP_6
#define		NXTENNA_ENABLE		0x01
#define		NXTENNA_DISABLE		0x00

#define UC_GP_14				0x003E	/* IIC Xfer data(8) */
#define		IIC_XFER_DATA_MAX	UC_GP_14

#define UC_GP_15				0x003F


#define	AGC_CONTROL				0x0041
#define		AGC_CONTROL_DUAL	0x01
#define		AGC_CONTROL_CLEAR	0x02
#define		AGC_CONTROL_GO		0x04

#define AGC_ADC_TARGET_POWER_LEVEL					0x0042

#define AGC_ADC_POWER_LPF_FC						0x0043

#define AGC_ADC_POWER_DETECT_MSB					0x0044

#define AGC_GAIN_LOOP_BANDWIDTH						0x0046

#define AGC_GAIN_DISTRIBUTION_LOOP_BANDWIDTH		0x0047

#define AGC_LOOP_DAMPING_RATIOS						0x0048

#define AGC_ACCUMULATOR1_MSB						0x0049

#define AGC_ACCUMULATOR2_MSB						0x004B

#define AGC_KG1										0x004D

#define AGC_PDET_TARGET_POWER_LEVEL					0x0051

#define AGC_PDET_POWER_MSB							0x0053

#define AGC_SDM12_LPF_FC							0x0055

#define AGC_SDM_CONFIGURE							0x0057
#define		AGC_SDM_POL_MASK						0xF0
#define		AGC_SDM1_INVERT							0x80
#define		AGC_SDM2_INVERT							0x40
#define		AGC_SDMX_INVERT							0x20
#define		AGC_SDMA_INVERT							0x10

#define AGC_SDM1_INPUT_MSB							0x0058

#define AGC_SDM2_INPUT_MSB							0x005A

#define AGC_SDMX_INPUT_MSB							0x005C



#define	FE_TIMING_PILOT_CONTROL			0x0061
#define		FE_TIMING_FEEDBACK_GAIN		0x0C
#define		FE_TIMING_FEEDBACK_1X		0x00
#define		FE_TIMING_FEEDBACK_4X		0x04
#define		FE_TIMING_FEEDBACK_16D		0x08
#define		FE_TIMING_FEEDBACK_4D		0x0C

#define	FE_TIMING_RATE_NOM_4			0x0066

#define FE_TIMING_BAUD_RATE_OFFSET_3	0x006B

#define FE_PILOT_FREQ_OFFSET_3			0x0073



#define EQ_DFS_STATE			0x009F
#define		EQ_DFS_MASK			0x0F
#define		EQ_DFS_LOCK			0x08
#define		EQ_DFS_ACQ			0x01

#define EQ_SNR					0x00A0

#define EQ_CL_CONTROL			0x00A1
#define		CL_STAT_PTR_MASK	0x03
#define		CL_STAT_CV			0x00	/* cluster variance */
#define		CL_STAT_CL			0x01	/* carrier loop */
#define		CL_STAT_LF			0x02	/* loop filter */
#define		CL_STAT_I_Q			0x03	/* i, q error etc. */

#define EQ_CL_STAT_3			0x00A6

#define EQ_CL_STAT_2			0x00A7

#define EQ_CL_STAT_1			0x00A8

#define EQ_CL_STAT_0			0x00A9

#define EQ_TEST_MUX_SELECT		0x00CC



#define	FEC_TD_MODE_CTRL		0x00D0
#define		FEC_TD_LOCK			0x08

#define FEC_QAMFSD_STATUS		0x00DB
#define		FEC_QAM_DET_STATE	0x03
#define		FEC_QAM_LOCK		0x02
#define		FEC_QAM_VERIFY		0x01

#define FEC_RS_MODE				0x00E4
#define		FEC_RS_MD_FLAG		0x08	/* nframes completed */
#define		FEC_RS_MD_MODE_GATE	0x04	/* gated over nframes */
#define		FEC_RS_MD_UNCOR		0x02	/* include uncorrectables */

#define FEC_RS_NFRAME			0x00E5

#define FEC_RS_ERROR_COUNT_MSB	0x00E6
#define		FEC_RS_ERR_CNT_MASK	0x0F	/* msb has only 4 bits */

#define FEC_MPEG_MODE_0			0x00E9
#define		FEC_MPEG_MODE_0_STD	0x7E	/* standard operation */
#define		FEC_MPEG_MODE_0_BER	0x02	/* bert mode */

#define FEC_MPEG_MODE_1			0x00EA
#define		FEC_MPEG_MODE_1_STD	0x00	/* standard operation */
#define		FEC_MPEG_MODE_1_BER	0x02	/* bert mode */




/* Dev 1 registers start here */

/* BERT offset is 0x30 */
#define BERT_CTL_A						0x0130
#define		BERT_CTL_A_MASK				0x7F
#define		BERT_CTL_A_ON				0x01

#define BERT_CTL_B						0x0131
#define		BERT_CTL_LSS_LCK			0x01
#define		BERT_CTL_NEW_ERR			0x02

#define BERT_STATUS						0x0132
#define		BERT_STATUS_LOCKED			0x01

#define BERT_WIN_SIZE					0x0133

#define BERT_ERR_CNTR_3					0x0134

#define BERT_SYNC_THOLD					0x0138

#define BERT_SYNC_ACQ_LOSS				0x0139



/* Smoother offset is 0x80 */
#define SMOOTHER_CONTROL				0x0180
#define		SMOOTHER_CTL_MASK			0x0C
#define		SMOOTHER_CTL_GO_AGC			0x40
#define		SMOOTHER_CTL_DIS_FILTER		0x08
#define		SMOOTHER_CTL_RESUME_DATA_OP	0x04
#define		SMOOTHER_CTL_ENABLE_LOOP	0x01

#define SMOOTHER_TARGET					0x0181

#define	SMOOTHER_NOMINAL_NCO_2			0x0182
#define SMOOTHER_NCO2_MAX				0x800000
#define	SMOOTHER_NCO2_256QAM_SYNC_DIS	0x62CD56
#define	SMOOTHER_NCO2_256QAM_SYNC_EN	0x635499
#define	SMOOTHER_NCO2_64QAM_SYNC_DIS	0x44A8DC
#define	SMOOTHER_NCO2_64QAM_SYNC_EN		0x4506DA
#define	SMOOTHER_NCO2_8VSB_SYNC_DIS		0x311B2C
#define	SMOOTHER_NCO2_8VSB_SYNC_EN		0x315E66

#define SMOOTHER_NCO_2					0x0185

#define SMOOTHER_BANDWIDTH				0x0188
#define		SMOOTHER_BW_MASK			0x7F

#define SMOOTHER_MPEG_ERR_CNTR_1		0x0189



#endif /* NXT2004REG */

