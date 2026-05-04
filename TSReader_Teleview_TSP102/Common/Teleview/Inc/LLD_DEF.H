#ifndef	__TSP100_LLD_FCN_DEF_	
#define	__TSP100_LLD_FCN_DEF_

#ifdef __cplusplus		// 2002.12 yuri 
extern "C" {
#endif

//--- General 
int 	_stdcall TSPL_GET_BOARD_ID(void);
int 	_stdcall TSPL_GET_AUTHORIZATION(void);		// Not Used in Application

void *	_stdcall TSPL_READ_BLOCK(void);
int		_stdcall TSPL_WRITE_BLOCK(unsigned long *pdwSrcBuff, 
			  			unsigned long dwBuffSize, unsigned long *dest);
int		_stdcall TSPL_SET_SDRAM_BANK_CONFIG(int nBankConfig);
int		_stdcall TSPL_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig);
int		_stdcall TSPL_SET_PLAY_RATE(long play_freq, long nOutputClockSourc);

int 	_stdcall TSPL_GET_TSIO_DIRECTION(void);
int 	_stdcall TSPL_SET_TSIO_DIRECTION(int nDirection);
int		_stdcall TSPL_GET_CUR_BANK_GROUP(void);
int 	_stdcall TSPL_SET_BANK_COUNTER(int nBank);
int 	_stdcall TSPL_SET_SDCON_MODE(int nMode);

int 	_stdcall TSPL_RESET_SDCON(void);
int		_stdcall TSPL_PUT_CHAR(long dwCommand);
unsigned long _stdcall	TSPL_GET_CHAR(void);

//-- Special Purpose
int 	_stdcall TSPL_SET_PCR_PID_FOR_STC(int PID);
long 	_stdcall TSPL_GET_CUR_STC_LSB(void);
long 	_stdcall TSPL_GET_CUR_STC_MSB(void);
long 	_stdcall TSPL_GET_CUR_IN_PTS(void);
long 	_stdcall TSPL_GET_CUR_IN_DTS(void);
int 	_stdcall TSPL_RESET_NULL_PACKET_CNT(void);
long 	_stdcall TSPL_NUM_NULL_PACKET(long ResetCounter);
int 	_stdcall TSPL_SET_INSERT_PID_TABLE(int table_idx, int PID);
int 	_stdcall TSPL_SET_PTSDTS_OFFSET(long dwOffset);

//--- I2C for TUNER
int		_stdcall TSPL_I2C_WRITE( unsigned char address, unsigned char *tx_buffer, int txlen, int* actlen);
int		_stdcall TSPL_I2C_READ( unsigned char address, unsigned char *rx_buffer, int maxlen, int* actlen);
int		_stdcall TSPL_II2C_READ( unsigned char address,
						       unsigned char *rx_buffer, 
						       int maxlen, 
						       int* actlen);
int		_stdcall TSPL_II2C_WRITE(unsigned char address,  unsigned char *tx_buffer, int nbytes, int* actlen);
int		_stdcall TSPL_I2C_WRITEREAD( unsigned char dev_addr, int reg_addr, unsigned char *data, int count, int *actlen);
int 	_stdcall TSPL_SET_NIM(int reset,  int lnb_vtg,  int lnb_on_off,  int	lnb_tone);

//--- Modulator
int 	_stdcall TSPL_CONFIG_TVB350(char *szfnTVB350);
int 	_stdcall TSPL_INIT_MODULATOR(int modulation_mode, 	// 1(VSB),2(QAM),3(QPSK)
			    				int InputSelection,		// SMPTE310 or DVB-ASI
							    int CodingMode,			// 1/2, 2/3, 3/4, 5/6, 7/8
			    				long dwfreq,
							    long symbol_freq);		
int 	_stdcall TSPL_SET_MODULATOR_FREQ(int modulation_mode, long dwfreq, long symbol_freq);
int 	_stdcall TSPL_SET_MODULATOR_CODERATE(int CodingMode);
int 	_stdcall TSPL_SET_MODULATOR_SYMRATE(int modulation_mode, long dwfreq, long symbol_freq);
int 	_stdcall TSPL_GET_MODSYNCLOCK(void);				// 2003.2.24 SWPARK/YURICHO

int		_stdcall TSPL_WRITE_BLOCK_DMA(DWORD dwBuffSize,DWORD *dest);
void *	_stdcall TSPL_GET_DMA_ADDR(void);

#ifdef __cplusplus
}
#endif

#define MAJOR_BANK_WORD_SIZE	0x200000	// 8Mbyt is 2M x 32bit 
#define	SUB_BANK_MAX_BYTE_SIZE	0x100000	// 1 MBYTE

#define	HW_BANK_NUMBER			7			// Use 1 bank			// 0~7 for 1MB - 8MB
#define	SUB_BANK_NUMBER			(HW_BANK_NUMBER+1)
#define SUB_BANK_OFFSET_SIZE	0x400		// max 0x400
#define	SUB_BANK_BYTE_SIZE		0x100000	// 1MBYTE
#define	SUB_BANK_LWORD_SIZE		0x40000		// 1 Mbyte / 4

//
//	argument for TSPL_SET_SDCON_MODE
//
#define	TSPL_SDCON_PLAY_MODE		0
#define	TSPL_SDCON_CAPTURE_MODE		1
#define	TSPL_SDCON_DELAY_MODE		2
#define	TSPL_SDCON_LOOP_THRU_MODE	3

#define	TSIO_PLAY_WITH_310INPUT		0
#define	TSIO_PLAY_WITH_ASIINPUT		1
#define	TSIO_310_LOOPTHRU			2
#define	TSIO_ASI_LOOPTHRU			3
  
#endif	//__TSP100_LLD_FCN_DEF_	
