#ifndef	__LOG_FILE_DEF_H_
#define	__LOG_FILE_DEF_H_

//
//	Log file declaration
//
//	June 14 2001
//	Teleview Corporation
//

extern	FILE	*fn_Log;

#ifdef	_DEBUG
#define	LPRINT	fprintf
#else
#define	LPRINT
#endif

#endif //__LOG_FILE_DEF_H_ 
