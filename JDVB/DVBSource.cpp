#include "DVBSource.h"
#include "Interface.h"
#include "CLSID.h"
#include "Global.h"
#include "Resource.h"
#include "TSPacket.h"
#include "TSPacketQueue.h"
#include "Transponder.h"
#include "Programm.h"
#include "TSPidFilter.h"

#include <stdio.h>

DVBSource *g_DVBSource = NULL;

HWND g_DVBSourceDlg = 0;

unsigned long PacketPeak = 0;
unsigned long OldProcessedPackets = 0;
void (__stdcall *FilterFunction[33])(unsigned char *Data);
//-------------------------------------------------------------------
void setCallBackFunction()
{
	int index = 0;
	FilterFunction[0] = Filter00;
	FilterFunction[1] = Filter01;
	FilterFunction[2] = Filter02;
	FilterFunction[3] = Filter03;
	FilterFunction[4] = Filter04;
	FilterFunction[5] = Filter05;
	FilterFunction[6] = Filter06;
	FilterFunction[7] = Filter07;
	FilterFunction[8] = Filter08;
	FilterFunction[9] = Filter09;
	FilterFunction[10] = Filter0A;
	FilterFunction[11] = Filter0B;
	FilterFunction[12] = Filter0C;
	FilterFunction[13] = Filter0D;
	FilterFunction[14] = Filter0E;
	FilterFunction[15] = Filter0F;
	FilterFunction[16] = Filter10;
	FilterFunction[17] = Filter11;
	FilterFunction[18] = Filter12;
	FilterFunction[18] = Filter13;
	FilterFunction[19] = Filter14;
	FilterFunction[20] = Filter15;
	FilterFunction[21] = Filter16;
	FilterFunction[22] = Filter17;
	FilterFunction[23] = Filter18;
	FilterFunction[24] = Filter19;
	FilterFunction[25] = Filter1A;
	FilterFunction[26] = Filter1B;
	FilterFunction[27] = Filter1C,
	FilterFunction[28] = Filter1D;
	FilterFunction[29] = Filter1E;
	FilterFunction[30] = Filter1F;


}
//-------------------------------------------------------------------
DVBSource::DVBSource ()
{
	PacketQueue_ = new TSPacketQueue(4000);
	MaxFilters_ = 33;
	for(int index = 0; index < MaxFilters_; index++)
		ActiveFilters_[index] = 0xFFFF;
	
	
	NumOfFilters_ = 0;
	HRESULT hr = CoCreateInstance(CLSID_DVBSource,
								  NULL,
								  CLSCTX_INPROC,
								  IID_IBaseFilter,
								  (void **)&DVBSourceBase_);
	if(FAILED(hr))
	{
		DVBSourceBase_ = 0;
		DVBSource_ = 0;
		return;
	}
	
	hr = DVBSourceBase_->QueryInterface (IID_IDVBSource, (void **)&DVBSource_);

	if(FAILED(hr))
	{
		DVBSourceBase_->Release ();
		DVBSource_ = 0;
		DVBSourceBase_ = 0;
	}

	DVBSource_->set_LNBType (LNBTYPE_UNIVERSAL,0,0);

	ActiveTransponder_ = 0;
	HeadTransponder_ = 0;
	TailTransponder_ = 0;
	setCallBackFunction();
}

//-------------------------------------------------------------------
DVBSource::~DVBSource ()
{
	if(DVBSource_)
		DVBSource_->Release ();

	if(DVBSourceBase_)
		DVBSourceBase_->Release ();
	ActiveTransponder_ = HeadTransponder_;

	while(ActiveTransponder_)
	{
		ActiveTransponder_ = HeadTransponder_->getNextTransponder();
		delete HeadTransponder_;
		HeadTransponder_ = ActiveTransponder_;
	}
	delete PacketQueue_;
}

//-------------------------------------------------------------------
HRESULT DVBSource::findTransponder (const unsigned long Frequency, const unsigned long SymbolRate, const bool VerticalPolarization,Transponder **Trans)
{
	if(HeadTransponder_ == 0 || Trans == 0)
		return E_FAIL;

	Transponder *Current = HeadTransponder_;

	do
	{
		if(Current->getFrequency () == Frequency && Current->getSymbolRate () == SymbolRate && Current->isVerticalPolarization () == VerticalPolarization)
		{
			*Trans = Current;
			return S_OK;
		}

		Current = Current->getNextTransponder ();
	}
	while(Current);

	return E_FAIL;

}
//-------------------------------------------------------------------
HRESULT DVBSource::registerCallback (const unsigned short Pid, TSPidFilter * Filter)
{
	if(Filter == 0)
		return E_FAIL;

	if(DVBSource_ == 0)
		return E_FAIL;

	for(int index = 0; index < MaxFilters_; index++)
	{
		if(ActiveFilters_[index] == Pid)
		{
			Filter->setTSPacketQueue (PacketQueue_);
			return S_OK;
		}

	}
	for(int index = 0; index < MaxFilters_ ; index++)
	{
		if(ActiveFilters_[index] == 0xFFFF)
		{
			Filter->setTSPacketQueue (PacketQueue_);
			ActiveFilters_[index] = Pid;
			NumOfFilters_++;
			return DVBSource_->AddPidFilter (FilterFunction[index],Pid);
		}
	}

	return E_FAIL;
}
//--------------------------------------------------------------------
HRESULT DVBSource::unregisterCallback (const unsigned short Pid)
{
	if(DVBSource_ == 0)
		return E_FAIL;

	for(int index = 0; index < MaxFilters_ ; index++)
	{
		if(ActiveFilters_[index] == Pid)
		{
			ActiveFilters_[index] = 0xFFFF;
			NumOfFilters_--;
			return DVBSource_->DelPidFilter (FilterFunction[index],Pid);
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT DVBSource::registerPid (TSPidFilter *PidFilter)
{
	if(PidFilter == 0 || ActiveTransponder_ == 0)
		return E_FAIL;
	
	for(unsigned char PidIndex = 0; PidIndex < PidFilter->getPidCount (); PidIndex++)
	{
		if(FAILED(registerCallback(PidFilter->getPid (PidIndex),PidFilter)))
			return E_FAIL;
		
		if(FAILED(ActiveTransponder_->registerTSPidFilter (PidFilter->getPid (PidIndex),PidFilter)))
			return E_FAIL;
	}

	return S_OK;
}

//-------------------------------------------------------------------
HRESULT DVBSource::unregisterPid (TSPidFilter *PidFilter)
{
	if(PidFilter == 0 || ActiveTransponder_ == 0)
		return E_FAIL;

	PidFilter->setTSPacketQueue (0);

	return ActiveTransponder_->unregisterTSPidFilter (PidFilter);
}
//-------------------------------------------------------------------
void DVBSource::insertRawStream (unsigned char *Data)
{
	TSPacket *Packet = NULL;

	while(FAILED(PacketQueue_->pull (&Packet)))
		_sleep(10);

	Packet->update ((const char *)Data);

	if(ActiveTransponder_)
		if(ActiveTransponder_->dispatch (Packet) == false)
			unregisterCallback(Packet->getPid ());
}
//-------------------------------------------------------------------
const unsigned long DVBSource::getNumOfDiscardedPackets () const
{
	if(ActiveTransponder_)
		return ActiveTransponder_->getNumOfDiscardedPackets();
	else
		return 0L;
}
//-------------------------------------------------------------------
const unsigned long DVBSource::getNumOfProcessedPackets () const
{
	if(ActiveTransponder_)
		return ActiveTransponder_->getNumOfProcessedPackets();
	else
		return 0L;
}
//-------------------------------------------------------------------
const int DVBSource::getNumOfFilters() const
{
	return NumOfFilters_;
}
//-------------------------------------------------------------------
const int DVBSource::getMaxFilters() const
{
	return MaxFilters_ ;
}
//-------------------------------------------------------------------
HRESULT DVBSource::setTransponder (const unsigned long Frequency,
								   const unsigned long SymbolRate,
								   const bool VerticalPolarization)
{
	if(DVBSource_ == 0)
		return E_FAIL;

	if(ActiveTransponder_ == 0)
		ActiveTransponder_ = HeadTransponder_ = TailTransponder_ = new Transponder(Frequency,SymbolRate,VerticalPolarization);
	else
	{
		if(ActiveTransponder_->getFrequency () != Frequency || ActiveTransponder_->getSymbolRate () != SymbolRate || ActiveTransponder_->isVerticalPolarization () == VerticalPolarization)
		{
			ActiveTransponder_->notifyTransponderChange (false);
			if(FAILED(findTransponder(Frequency,SymbolRate,VerticalPolarization,&ActiveTransponder_)))
			{
				ActiveTransponder_ = new Transponder(Frequency,SymbolRate,VerticalPolarization);
				TailTransponder_->setNextTransponder (ActiveTransponder_);
				TailTransponder_ = ActiveTransponder_;
			}			
		}
	}

	ActiveTransponder_->notifyTransponderChange (true);

	HRESULT hr = DVBSource_->LockChannel (ActiveTransponder_->getFrequency (),
		                                  ActiveTransponder_->getSymbolRate (),
										  (int)ActiveTransponder_->isVerticalPolarization (),
										  0, //Tone disable
										  1); //DiSEqC 1.0

	if(FAILED(hr))
		return E_FAIL;

	return S_OK;
}
//-------------------------------------------------------------------
const unsigned long DVBSource::getFrequency () const
{
	if(ActiveTransponder_)
		return ActiveTransponder_->getFrequency ();
	else
		return 0L;
}
//-------------------------------------------------------------------
const unsigned int DVBSource::getSymbolRate () const
{
	if(ActiveTransponder_)
		return ActiveTransponder_->getFrequency ();
	else
		return 0L;
}

//-------------------------------------------------------------------
const bool DVBSource::isVerticalPolarization () const
{
	if(ActiveTransponder_)
		return ActiveTransponder_->isVerticalPolarization ();
	else
		return false;
}

//-------------------------------------------------------------------
HRESULT DVBSource::findProgrammBySID(const unsigned long Frequency,
									 const unsigned long SymbolRate,
									 const bool VerticalPolarization,
									 const unsigned short SIDPid,			 
									 Programm ** Prog)
{
	if(Prog == 0)
		return E_FAIL;


	if(ActiveTransponder_->getFrequency () == Frequency &&
	   ActiveTransponder_->getSymbolRate () == SymbolRate &&
	   ActiveTransponder_->isVerticalPolarization () == VerticalPolarization)
	{
		return ActiveTransponder_->findProgrammBySID (SIDPid,Prog);
	}

	Transponder *Trans = 0;

	if(FAILED(findTransponder(Frequency,SymbolRate,VerticalPolarization,&Trans)))
		return E_FAIL;

	return Trans->findProgrammBySID (SIDPid,Prog);
}
//-------------------------------------------------------------------
HRESULT DVBSource::findProgrammByPMT(const unsigned long Frequency,
									 const unsigned long SymbolRate,
									 const bool VerticalPolarization,
									 const unsigned short PMTPid,
									 Programm ** Prog)
{
	if(Prog == 0)
		return E_FAIL;
	

	if(ActiveTransponder_->getFrequency () == Frequency &&
	   ActiveTransponder_->getSymbolRate () == SymbolRate &&
	   ActiveTransponder_->isVerticalPolarization () == VerticalPolarization)
	{
		return ActiveTransponder_->findProgrammByPMT (PMTPid,Prog);
	}

	Transponder *Trans = 0;

	if(FAILED(findTransponder(Frequency,SymbolRate,VerticalPolarization,&Trans)))
		return E_FAIL;

	return Trans->findProgrammByPMT(PMTPid,Prog);
	
}
//-------------------------------------------------------------------
HRESULT DVBSource::findProgrammByName(const unsigned long Frequency,
									 const unsigned long SymbolRate,
									 const bool VerticalPolarization,
									 const char *Name,
									 Programm ** Prog)
{
	if(Name == 0 || Prog == 0)
		return E_FAIL;
	
	
	if(ActiveTransponder_->getFrequency () == Frequency &&
	   ActiveTransponder_->getSymbolRate () == SymbolRate &&
	   ActiveTransponder_->isVerticalPolarization () == VerticalPolarization)
	{
		return ActiveTransponder_->findProgrammByName (Name,Prog);
	}

	Transponder *Trans = 0;

	if(FAILED(findTransponder(Frequency,SymbolRate,VerticalPolarization,&Trans)))
		return E_FAIL;

	return Trans->findProgrammByName (Name,Prog);
}
//------------------------------------------------------------------
HRESULT DVBSource::importProgramm (Programm *Prog)
{
	if(Prog == 0)
		return E_FAIL;


	Programm *Current = Prog;
	Transponder *Trans = 0;
	do
	{
		Current->setTSSource (this);	
		if(ActiveTransponder_->getFrequency () == Current->getFrequency () && ActiveTransponder_->getSymbolRate () == Current->getSymbolRate () && ActiveTransponder_->isVerticalPolarization () == Current->isVerticalPolarization ())
			ActiveTransponder_->insertProgramm (Prog);
		
		if(FAILED(findTransponder(Current->getFrequency (), Current->getSymbolRate (), Current->isVerticalPolarization (), &Trans)))
		{
			setTransponder(Current->getFrequency (), Current->getFrequency (), Current->isVerticalPolarization ()); //creates a new transponder
			ActiveTransponder_->insertProgramm (Current);
		}
		else
			Trans->insertProgramm (Current);
		Current = Current->getNextProgramm ();
	}
	while(Current);

	return S_OK;
}
//------------------------------------------------------------------
LRESULT CALLBACK DVBSourceDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR Info[300];
	switch (message)
	{
		case WM_INITDIALOG:
			return true;

		case WM_TIMER:
		{
			switch(wParam)
			{
				case IDT_DVB_SOURCE_TIMER:
				{
					unsigned long CurrentProcessedPackets = g_DVBSource->getNumOfProcessedPackets ();
					if(OldProcessedPackets == 0)
						OldProcessedPackets = CurrentProcessedPackets;
					if(PacketPeak < CurrentProcessedPackets - OldProcessedPackets)
					{
						sprintf(Info,"%u",CurrentProcessedPackets - OldProcessedPackets);
						SetDlgItemText(g_DVBSourceDlg,IDC_PACKET_PEAK,Info);
						PacketPeak = CurrentProcessedPackets - OldProcessedPackets;
					}
						OldProcessedPackets = g_DVBSource->getNumOfProcessedPackets ();
					sprintf(Info,"%uD%u",g_DVBSource->getNumOfProcessedPackets (), g_DVBSource->getNumOfDiscardedPackets ());
					SetDlgItemText(hDlg,IDC_PROCESSED_PACKETS,Info);
					sprintf(Info,"%d",g_DVBSource->getMaxFilters ());	
					SetDlgItemText(g_DVBSourceDlg,IDC_MAX_PID_FILTERS,Info);
					sprintf(Info,"%d",g_DVBSource->getNumOfFilters ());
					SetDlgItemText(g_DVBSourceDlg,IDC_ACTIVE_PID, Info);
					sprintf(Info,"%u",g_DVBSource->getFrequency ());
					SetDlgItemText(g_DVBSourceDlg,IDC_TRANSPONDER_FREQUENCY,Info);
					sprintf(Info,"%u",g_DVBSource->getSymbolRate ());
					SetDlgItemText(g_DVBSourceDlg,IDC_SYMBOL_RATE,Info);
					sprintf(Info,"%d",g_DVBSource->isVerticalPolarization ());
					SetDlgItemText(g_DVBSourceDlg,IDC_POLARIZATION,Info);
					

				}

			}
		break;
		}


	//SetDlgItemText
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			g_UDPPidDlg = 0;
			KillTimer(hWnd,IDT_DVB_SOURCE_TIMER);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
/********************************************************************
 * static Filter Functions
 *******************************************************************/
void __stdcall Filter00(unsigned char *Data)
{
	//if(g_DVBSource == 0)
	//	return; 

	g_DVBSource->insertRawStream (Data);

	//RealReceivedPackets++;
}
//-------------------------------------------------------------------
void __stdcall Filter01(unsigned char *Data)
{
	//if(g_DVBSource == 0)
	//	return; 
	
	//RealReceivedPackets++;
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------

void __stdcall Filter02(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter03(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter04(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter05(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter06(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter07(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter08(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter09(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0A(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0B(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0C(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0D(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0E(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter0F(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter10(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter11(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter12(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter13(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter14(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter15(unsigned char *Data)
{
	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);


}
//-------------------------------------------------------------------
void __stdcall Filter16(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter17(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter18(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter19(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1A(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1B(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1C(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1D(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1E(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
void __stdcall Filter1F(unsigned char *Data)
{

	if(g_DVBSource == 0)
		return; 
	
	g_DVBSource->insertRawStream (Data);

}
//-------------------------------------------------------------------
