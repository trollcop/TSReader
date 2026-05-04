

#ifndef DSHOWTARGET_H___
#define DSHOWTARGET_H___


/********************************************************************
 * Class DShowTarget
 *
 * This class implements a playback class. 
 * Note: this class only supports playback of one audio and/or video
 * stream. 
 *
 *
 *
 *******************************************************************/


#include "TSTarget.h"
#include "TSPidFilter.h"
#include "InterfaceSourceFilter.h"

class Programm;


class DShowTarget : public TSTarget
{
protected:
Programm **programm_;
int NumOfProgramm_;
int MaxOfProgramm_;

public:
	DShowTarget(const char *Name);

	virtual ~DShowTarget();
	
	unsigned long getNumOfProcessedPackets(const int ProgrammIndex, const int PidIndex);
	
	//------------------------------------------------------------------
	unsigned long getNumOfDiscardedPackets(const int ProgrammIndex, const int PidIndex);
	
	HRESULT createProgramm(int *ProgrammNumber, const char *ProgrammName);

	HRESULT deleteProgramm(const int ProgrammNumber);


	HRESULT createTSTargetFilter(const int ProgrammNumber, const unsigned short Pid,
		                         const unsigned int PidType, const char *PidName, TSPidFilter **Filter);

	HRESULT getProgrammProperties(const int ProgrammNumber, HINSTANCE hInstOwner, HWND hWndOwner, HWND *Window, unsigned int *CommandId);
	HRESULT togglePlayback(const int ProgrammNumber);

	HRESULT toggleVolume(const int ProgrammNumber);

	HRESULT toggleFullScreen(const int ProgrammNumber, HWND hWnd);

	HRESULT getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND ** Window, unsigned int * CommandId);

	HRESULT setVideoOwner(const int ProgrammNumber, HWND hWnd);

};


LRESULT CALLBACK DSTargetDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);




#endif