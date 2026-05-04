#ifndef	__TSP_TUNE_FCN_DEF_	
#define	__TSP_TUNE_FCN_DEF_

#ifdef __cplusplus		// 2002.12 yuri 
extern "C" {
#endif

//--- for TVB140
int		_stdcall	TVB_TUNE(long dwfreq);
int		_stdcall	TLV_Read_SNRBER_fromChip(void);
double	_stdcall	TLV_Get_BER(void);
double 	_stdcall	TLV_Get_SNR(void);
int		_stdcall	TLV_Set_DeMod_Conf(unsigned char mode, int if_freq_idx, int symbol_rate_idx,
				   			int bAltBw, int bFastAcquire, int bEnableOffsets, int bEnableNtscSweep,
				   			int if_offset_10hz, int sym_offset, int ntsc_offset_10hz);
int		_stdcall	TLV_Get_ConsData(int Index);
int		_stdcall	TLV_Read_ConsData_fromChip(void);
int		_stdcall	TLV_SetSymRate(UINT hz);
int		_stdcall	TLV_SetIfFreq(UINT hz);

//--- for TQK150
int		_stdcall	TQK_GET_SNR(void);
int		_stdcall	TQK_GET_SWEEP_INDEX(void);
int		_stdcall	TQK_TUNE(long dwfreqkHz, long dwsymbolrate,long dwLNBfreqkHz, int lnb_vtg,
							 int lnb_on_off, int lnb_tone, int spectrum, int sweep_offset);


//--- for TOM160
double	_stdcall	TOM_GET_BER(void);
int		_stdcall	TOM_GET_AGC_STATUS(void);
int		_stdcall	TOM_GET_TSP_STATUS(void);
int		_stdcall	TOM_GET_FEC_STATUS(void);
int		_stdcall	TOM_TUNE(long dwRfFreq, long dwIfFreq);
#ifdef __cplusplus
}
#endif
 
#endif	//__TSP_TUNE_FCN_DEF_	
