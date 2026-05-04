#include "Programm.h"
#include "AVPidFilter.h"
#include "TSPidFilter.h"
#include "TSTarget.h"
#include "TSSource.h"
#include "Resource.h"
#include <winerror.h>

#include "Global.h"


HWND g_cProgramm = 0;
//--------------------------------------------------------------------
Programm::Programm ()
{
	ECMPid_ = 0;
	SIDPid_ = 0;
	PATPid_ = 0;
	PMTPid_ = 0;
	PCRPid_ = 0;
	AudioPid_ = 0;
	VideoPid_ = 0;
	TeleTextPid_ = 0;
	Frequency_;
	SymbolRate_;
	Vertical_;
	hWnd_ = 0;
	AC3_ = false;
	Name_ = 0;
	Provider_ = 0;
	Active_ = false;
	Next_ = 0;
	Last_ = 0;
	Target_ = 0;
	Source_ = 0;

}
Programm::Programm(TSSource *Source)
{
	ECMPid_ = 0;
	SIDPid_ = 0;
	PATPid_ = 0;
	PMTPid_ = 0;
	PCRPid_ = 0;
	AudioPid_ = 0;
	VideoPid_ = 0;
	TeleTextPid_ = 0;
	Frequency_;
	SymbolRate_;
	Vertical_;
	hWnd_ = 0;
	AC3_ = false;
	Name_ = 0;
	Provider_ = 0;
	Active_ = false;
	Next_ = 0;
	Last_ = 0;
	Target_ = 0;
	Source_ = Source;
}
//--------------------------------------------------------------------
Programm::~Programm()
{
}
//--------------------------------------------------------------------
void Programm::setPATPid(const unsigned short Pid)
{
	PATPid_ = Pid;
}
//--------------------------------------------------------------------
void Programm::setPMTPid(const unsigned short Pid)
{
	PMTPid_ = Pid;
}
//--------------------------------------------------------------------
void Programm::setPCRPid(const unsigned short Pid)
{
	PCRPid_ = Pid;
}
//--------------------------------------------------------------------
void Programm::setAudioPid(const unsigned short Pid, const bool AC3)
{
	AudioPid_ = Pid;
	AC3_ = AC3;
}
//--------------------------------------------------------------------
void Programm::setVideoPid(const unsigned short Pid)
{
	VideoPid_ = Pid;
}
//--------------------------------------------------------------------
void Programm::setFrequency(const unsigned long Frequency)
{
	Frequency_ = Frequency;
}	
//--------------------------------------------------------------------
void Programm::setSymbolRate(const unsigned long SymbolRate)
{
	SymbolRate_ = SymbolRate;
}
//--------------------------------------------------------------------
void Programm::setPolarization(const bool Vertical)
{
	Vertical_ = Vertical;
}
//--------------------------------------------------------------------
void Programm::setTeleTextPid(const unsigned short Pid)
{
	TeleTextPid_ = Pid;
}	
//--------------------------------------------------------------------
void Programm::setLastProgramm (Programm *Prg){Last_ = Prg;}
//--------------------------------------------------------------------
void Programm::setNextProgramm (Programm *Prg){Next_ = Prg;}
//--------------------------------------------------------------------
void Programm::setSIDPid (const unsigned short Pid){SIDPid_ = Pid;}
//--------------------------------------------------------------------
const unsigned short Programm::getSIDPid () const {return SIDPid_;}
//--------------------------------------------------------------------
void Programm::setECMPid (const unsigned short Pid){ECMPid_ = Pid;}
const unsigned short Programm::getECMPid () const {return ECMPid_;}
//--------------------------------------------------------------------
void Programm::setProvider (const char *Provider)
{
	if(Provider == 0)
		return;
	delete Provider_;
	Provider_ = _strdup(Provider);
}
//--------------------------------------------------------------------
void Programm::setName (const char *Name)
{
	if(Name == 0)
		return;
	delete Name_;
	Name_ = _strdup(Name);
}
void Programm::setTSSource(TSSource *Source) {Source_ = Source;}
const unsigned short Programm::getPATPid() const {return PATPid_;}
//--------------------------------------------------------------------
const unsigned short Programm::getPMTPid() const{return PMTPid_;}
//--------------------------------------------------------------------
const unsigned short Programm::getPCRPid() const{return PCRPid_;}
//--------------------------------------------------------------------
const unsigned short Programm::getAudioPid() const{return AudioPid_;}
//--------------------------------------------------------------------
const unsigned short Programm::getVideoPid() const{return VideoPid_;}
//--------------------------------------------------------------------
const unsigned short Programm::getTeleTextPid() const{return TeleTextPid_;}
//--------------------------------------------------------------------
const unsigned long Programm::getFrequency() const{return Frequency_;}
//--------------------------------------------------------------------
const unsigned long Programm::getSymbolRate() const{return SymbolRate_;}
//--------------------------------------------------------------------
const char *Programm::getName() const{return Name_;}
//--------------------------------------------------------------------
const bool Programm::isVerticalPolarization() const{return Vertical_;}
//--------------------------------------------------------------------
const bool Programm::isActive() const{return Active_;}
//--------------------------------------------------------------------
Programm * Programm::getNextProgramm () const {return Next_;}
//--------------------------------------------------------------------
Programm * Programm::getLastProgramm () const {return Last_;}
//--------------------------------------------------------------------
HRESULT Programm::insert(TSTarget *Target)
{
	if(Source_ == 0)
		return E_FAIL;
	

	TSPidFilter *Filter = 0;

	if(FAILED(Target->createTSTargetFilter (PMTPid_,PID_TYPE_PMT,"Programm X PMT Filter",&Filter)))
		return E_FAIL;
	if(FAILED(Source_->registerPid (Filter)))
		return E_FAIL;
	if(FAILED(Target->createTSTargetFilter (PATPid_,PID_TYPE_PAT,"Programm X PAT Filter",&Filter)))
		return E_FAIL;
	if(FAILED(Source_->registerPid (Filter)))
		return E_FAIL;
	
	if(FAILED(Target->createTSTargetFilter (VideoPid_,PID_TYPE_VIDEO_MPEG2,"Programm X Video Filter",&Filter)))
		return E_FAIL;
	if(FAILED(Source_->registerPid (Filter)))
		return E_FAIL;
	

	if(AC3_)
	{
		if(FAILED(Target->createTSTargetFilter (AudioPid_,PID_TYPE_AUDIO_AC3, "Programm X Dolby AC3 Filter",&Filter)))
			return E_FAIL;
	}
	else
		if(FAILED(Target->createTSTargetFilter (AudioPid_,PID_TYPE_AUDIO_MPEG2,"Programm X Audio Filter",&Filter)))
			return E_FAIL;
	
	if(FAILED(Source_->registerPid (Filter)))
		return E_FAIL;
	

	if(PCRPid_ != AudioPid_ && PCRPid_ != VideoPid_)
	{
		if(FAILED(Target->createTSTargetFilter (PCRPid_,PID_TYPE_PCR,"Programm X PCR Pid",&Filter)))
			return E_FAIL;
		if(FAILED(Source_->registerPid (Filter)))
			return E_FAIL;
	}
	
	return S_OK;




}
//--------------------------------------------------------------------
const bool Programm::isEqual (const Programm *Prog)
{
	return ((Frequency_ == Prog->Frequency_ ) && (SymbolRate_ == Prog->SymbolRate_ ) &&
		   (Vertical_ == Prog->Vertical_ ) && (AC3_ == Prog->AC3_) &&  (AudioPid_ == Prog->AudioPid_ ) &&
		   (ECMPid_ == Prog->ECMPid_ ) && (PATPid_ == Prog->PATPid_ ) && (PCRPid_ == Prog->PCRPid_ ) &&
		   (PMTPid_ == Prog->PMTPid_ ) && (SIDPid_ == Prog->SIDPid_ ) && (TeleTextPid_ == Prog->TeleTextPid_ ) &&
		   (VideoPid_ == Prog->VideoPid_ ));
}
//--------------------------------------------------------------------
HRESULT Programm::remove(TSTarget *Target)
{
	if(Target == 0 || Source_ == 0)
		return E_FAIL;
	
	TSPidFilter *Filter = 0;

	if(SUCCEEDED(Target->createTSTargetFilter (PATPid_,PID_TYPE_PAT,"A",&Filter)))
		Source_->unregisterPid (Filter);
	Target->deleteTSTargetFilter (PID_TYPE_PAT);
	
	if(SUCCEEDED(Target->createTSTargetFilter (PMTPid_,PID_TYPE_PMT,"A",&Filter)))
		Source_->unregisterPid (Filter);
	Target->deleteTSTargetFilter (PID_TYPE_PMT);
	
	if(PCRPid_ != AudioPid_ && PCRPid_ != VideoPid_)
	{
		if(SUCCEEDED(Target->createTSTargetFilter (PCRPid_,PID_TYPE_PCR,"A",&Filter)))
			Source_->unregisterPid (Filter);
		Target->deleteTSTargetFilter (PID_TYPE_PCR);
	}

	if(SUCCEEDED(Target->createTSTargetFilter (VideoPid_,PID_TYPE_VIDEO,"X",&Filter)))
		Source_->unregisterPid (Filter);
	Target->deleteTSTargetFilter (PID_TYPE_VIDEO);
	
	if(SUCCEEDED(Target->createTSTargetFilter (AudioPid_,PID_TYPE_AUDIO,"",&Filter)))
		Source_->unregisterPid (Filter);
	Target->deleteTSTargetFilter (PID_TYPE_AUDIO);
	
	return S_OK;
}
//--------------------------------------------------------------------
HRESULT Programm::configureVideoWindow(HWND hWnd, RECT rect)
{

	return E_NOTIMPL;
}
//--------------------------------------------------------------------
HRESULT Programm::toggleVolume()
{

	return E_NOTIMPL;
}
//--------------------------------------------------------------------
HRESULT Programm::togglePlayback()
{
	return E_NOTIMPL;
}
//--------------------------------------------------------------------
HRESULT Programm::toggleFullScreen()
{

	return E_NOTIMPL;
}
//--------------------------------------------------------------------
HRESULT Programm::getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND * Window, unsigned int * CommandId)
{

	return E_NOTIMPL;
}
//--------------------------------------------------------------------
LRESULT CALLBACK ProgrammDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			return true;
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, LOWORD(wParam));
				g_cProgramm = 0;
//				g_defaultVideoOutDevice->setVideoOwner (g_cProgrammIndex,hWnd){
				//KillTimer(hWnd,IDT_DVB_SOURCE_TIMER){
				return TRUE;
			}
			if(LOWORD(wParam) == IDC_BUTTON_NEXT)
			{
				//g_defaultVideoOutDevice->togglePlayback (g_cProgrammIndex){
				
				//g_defaultVideoOutDevice->setVideoOwner (++g_cProgrammIndex,
				//g_defaultVideoOutDevice->togglePlayback (++g_cProgrammIndex){
				
				//FIXME select the next available programm in list
				return true;
			}
			if(LOWORD(wParam) == IDC_BUTTON_BACK)
			{
				//FIXME select the last availabe programm in list
				return true;
			}
			break;
		}
	}




	return FALSE;
}

