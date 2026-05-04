#ifndef	__TSP100_HLD_FCN_DEF_	
#define	__TSP100_HLD_FCN_DEF_

#ifdef __cplusplus		// 2002.12 yuri 
extern "C" {
#endif
int		_stdcall	TSPH_START_PLAY(char *szfnNewPlayFile,
								long	PlayFreq,
								double	DblStartOffset, 
								long	nOutputClockSource, // 0 for play rate 1 for 19.392 Mbps
								long	fRepeatMode);
int		_stdcall	TSPH_START_MONITOR(int nPort);
int		_stdcall	TSPH_START_RECORD(char *CurFile, int nPort);
int		_stdcall    TSPH_START_DELAY(int nPort);
int		_stdcall   	TSPH_SET_REPEAT_PLAY_MODE(int fRepeatMode);

int 	_stdcall 	TSPH_GET_CURRENT_THREAD_STATE(void);
double	_stdcall	TSPH_GET_CURRENT_RECORD_POINT(void);
int		_stdcall	TSPH_SET_FILENAME_NEXT_PLAY(char *szFile);
int		_stdcall	TSPH_SET_SDRAM_BANK_CONFIG(int nBankConfig);
int		_stdcall	TSPH_SET_SDRAM_BANK_OFFSET_CONFIG(int nBankConfig);

int 	_stdcall 	TSPH_GET_PLAY_BUFFER_STATUS(void);
int		_stdcall	TSPH_CAL_PLAY_RATE(char *szFile, int Type);	// TYPE: 0=bitrate, 1:packet size

int 	_stdcall   	TSPH_GET_PAT_SECTION(char *szStr);
int 	_stdcall   	TSPH_GET_PMT_SECTION(char *szStr);
int 	_stdcall 	TSPH_SET_PCR_OFFSET(long dwOffset);
int 	_stdcall    TSPH_SET_PID_TO_SEARCH( int table_id,int pid_to_search);

#ifdef __cplusplus
}
#endif
 
#endif	//__TSP100_HLD_FCN_DEF_	
