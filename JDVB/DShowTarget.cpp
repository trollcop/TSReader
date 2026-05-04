#include "DShowTarget.h"
#include "Programm.h"

//-------------------------------------------------------------------
DShowTarget::DShowTarget (const char *Name)
: TSTarget(Name)
{
	MaxOfProgramm_ = 5; //FIXME
	programm_ = new Programm *[5];
	NumOfProgramm_ = 0;
	
	for(int index = 0; index < MaxOfProgramm_; index++)
		programm_ [index] = 0;

}
//-------------------------------------------------------------------
DShowTarget::~DShowTarget ()
{
	for(int index = 0; index < MaxOfProgramm_; index++)
		if(programm_[index])
			delete programm_[index];

	delete [] programm_;
}
//-------------------------------------------------------------------
HRESULT DShowTarget::createProgramm(int *ProgrammNumber, const char *ProgrammName)
{
	int position;

	for(position = 0; position < MaxOfProgramm_; position++)
		if(programm_[position] == 0)
			break;

	if(programm_[position])
		return E_FAIL; //FIXME implement array resizing

	programm_[position] = new Programm(ProgrammName);

	*ProgrammNumber = position;

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT DShowTarget::deleteProgramm(const int ProgrammNumber)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;

	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;

	delete programm_[ProgrammNumber];
	programm_[ProgrammNumber] = 0;

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT DShowTarget::togglePlayback(const int ProgrammNumber)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;
	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;

	return programm_[ProgrammNumber]->togglePlayback ();

}	
//-------------------------------------------------------------------
HRESULT DShowTarget::createTSTargetFilter(const int ProgrammNumber, const unsigned short Pid,
		                         const unsigned int PidType, const char *PidName, TSPidFilter **Filter)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;
	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;
	
	return programm_[ProgrammNumber]->createTSPidFilter (Pid,PidName,PidType,Filter);
	
}
//-------------------------------------------------------------------
HRESULT DShowTarget::toggleVolume(const int ProgrammNumber)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;

	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;

	return programm_[ProgrammNumber]->toggleVolume ();
}
//-------------------------------------------------------------------
unsigned long DShowTarget::getNumOfProcessedPackets(const int ProgrammIndex, const int PidIndex)
{
	return 0; //FIXME

}	
//------------------------------------------------------------------
unsigned long DShowTarget::getNumOfDiscardedPackets(const int ProgrammIndex, const int PidIndex)
{
	return 0; //FIXME

}
//-------------------------------------------------------------------
HRESULT DShowTarget::setVideoOwner(const int ProgrammNumber, HWND hWnd)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;

	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;
	
	return programm_[ProgrammNumber]->setVideoWindow (hWnd);
}

//-------------------------------------------------------------------
HRESULT DShowTarget::toggleFullScreen(const int ProgrammNumber, HWND hWnd)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;
	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;

	return programm_[ProgrammNumber]->toggleFullScreen (hWnd);
}
//-------------------------------------------------------------------
HRESULT DShowTarget::getProgrammProperties(const int ProgrammNumber, HINSTANCE hInstOwner, HWND hWndOwner, HWND *Window, unsigned int *CommandId)
{
	if(ProgrammNumber >= MaxOfProgramm_)
		return E_FAIL;
	if(programm_[ProgrammNumber] == 0)
		return E_FAIL;

	return programm_[ProgrammNumber]->getProperties (hInstOwner,hWndOwner,Window,CommandId);
}

//-------------------------------------------------------------------
HRESULT DShowTarget::getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND ** Window, unsigned int * CommandId)
{

	return E_NOTIMPL;
}

//-------------------------------------------------------------------
LRESULT CALLBACK DSTargetDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{



	return E_NOTIMPL;
}
