//-----------------------------------------------------------------------------
//   File:      8psk_VendorCommandCodes.h
//
//   8PSK Vendor Request commands
//
//   Copyright (c) 2005 GENPIX, All rights reserved
//-----------------------------------------------------------------------------

// vendor command codes
#define	GET_8PSK_CONFIG			0x80	// in
#define	SET_8PSK_CONFIG			0x81
#define	SPI_WRITE				0x82	// in
#define	I2C_WRITE				0x83
#define	I2C_READ				0x84	// in
#define	ARM_TRANSFER			0x85
#define	TUNE_8PSK				0x86
#define	GET_SIGNAL_STRENGTH		0x87	// in
#define	LOAD_BCM4500			0x88	
#define	BOOT_8PSK				0x89	// in
#define	START_INTERSIL			0x8A	// in
#define SET_LNB_VOLTAGE			0x8B
#define	SET_22KHZ_TONE			0x8C
#define SEND_DISEQC_COMMAND		0x8D
#define SET_DVB_MODE			0x8E
#define SET_DN_SWITCH			0x8F

// PSK_configuration bits
#define	bm8pskStarted	0x01
#define bm8pskFW_Loaded	0x02
#define bmIntersilOn	0x04
#define bmDVBmode		0x08
#define bm22kHz 		0x10
#define bmSEL18V		0x20
#define bmTuned			0x40
#define bmArmed			0x80

// FX2 I/O Port bits
#define bmSPI_CS		bmBIT0
#define bmSPI_CLK		bmBIT1
#define bmSPI_WR		bmBIT2
#define bmSPI_RD		bmBIT3
#define bmSequrity		bmBIT4
#define bmRED_LED		bmBIT5
#define bmYELLOW_LED	bmBIT6
#define bmGREEN_LED		bmBIT7
#define bmRESET			bmBIT1
#define bmWR_Protect	bmBIT2
#define bmDISEqC		bmBIT3
#define bm18V			bmBIT4
#define bmData_EN		bmBIT7

#define PortD_Config	(bmSPI_CS | bmSPI_CLK | bmSPI_WR | bmSequrity |	bmRED_LED | bmYELLOW_LED | bmGREEN_LED) 
#define PortA_Config	(bmRESET | bmWR_Protect | bmDISEqC | bm18V | bmData_EN)
	
#define	SET_SPI_CS			(IOD |= bmSPI_CS)
#define CLR_SPI_CS  		(IOD &= ~bmSPI_CS)
#define	SET_SPI_CLK			(IOD |= bmSPI_CLK)
#define CLR_SPI_CLK  		(IOD &= ~bmSPI_CLK)
#define SET_SPI_bit			(IOD |= bmSPI_WR)
#define CLR_SPI_bit 		(IOD &= ~bmSPI_WR)
#define SPI_DATA_in			(IOD & bmSPI_RD)
#define SET_Sequrity_bit	(IOD |= bmSequrity)
#define CLR_Sequrity_bit 	(IOD &= ~bmSequrity)
#define SET_RED_LED			(IOD &=	~bmRED_LED)
#define CLR_RED_LED			(IOD |=	bmRED_LED)
#define SET_YELLOW_LED		(IOD &=	~bmYELLOW_LED)
#define CLR_YELLOW_LED		(IOD |=	bmYELLOW_LED)
#define SET_GREEN_LED		(IOD &=	~bmGREEN_LED)
#define CLR_GREEN_LED		(IOD |=	bmGREEN_LED)
#define CLR_ALL_LED			(IOD |=	(bmRED_LED | bmYELLOW_LED | bmGREEN_LED))
#define SET_RESET_bit		(IOA |= bmRESET)
#define CLR_RESET_bit	 	(IOA &= ~bmRESET)
#define SET_WPR_bit			(IOA |= bmWR_Protect)
#define CLR_WPR_bit	 		(IOA &= ~bmWR_Protect)
#define SET_DISEqC_bit		(IOA |= bmDISEqC)
#define CLR_DISEqC_bit	 	(IOA &= ~bmDISEqC)
#define SET_18V				(IOA |= bm18V)
#define SET_13V				(IOA &= ~bm18V)
#define SET_Data_EN_bit		(IOA |= bmData_EN)
#define CLR_Data_EN_bit 	(IOA &= ~bmData_EN)