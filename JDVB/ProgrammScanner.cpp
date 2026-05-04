#include "ProgrammScanner.h"
#include "TSSource.h"
#include "TSPidFilter.h"
#include "PATPidFilter.h"
#include "PMTPidFilter.h"
#include "SDTPidFilter.h"
#include "Programm.h"

#include <stdlib.h>

#include <winerror.h>

//-------------------------------------------------------------------
ProgrammScanner::ProgrammScanner ()
{
	SDTFilter_ = 0;

}

//-------------------------------------------------------------------
ProgrammScanner::~ProgrammScanner ()
{
#ifdef DEBUG
	ASSERT(SDTFilter_ == 0);
#endif
}
//-------------------------------------------------------------------
HRESULT ProgrammScanner::scanTSSource (TSSource *Source, Programm **Prog)
{
	if(Source == 0 || Prog == 0)
		return E_FAIL;
	
#ifdef DEBUG
	ASSERT(SDTFilter_ == 0);
#endif

	Programm *HeadProgramm = 0;
	Programm *TailProgramm = 0;

	SDTFilter_ = new SDTPidFilter(PID_SDT,"DVB Source SDT Filter");
	if(FAILED(Source->registerPid (SDTFilter_)))
	{
		delete SDTFilter_;
		SDTFilter_ = 0;
		return E_FAIL;
	}
	while(SDTFilter_->isProcessingDone () == false)
		_sleep(7);

	Source->unregisterPid (SDTFilter_);


	TSPidFilter *PATFilter = new PATPidFilter(PID_PAT,"DVB Source PAT Pid");
	
	if(FAILED(Source->registerPid (PATFilter)))
	{
		delete SDTFilter_;
		SDTFilter_ = 0;
		delete PATFilter;	
		return E_FAIL;
	}

	while(PATFilter->isProcessingDone () == false)
		_sleep(7);

	Source->unregisterPid (PATFilter);

	const PATInfo * pat_head = static_cast<PATPidFilter *>(PATFilter)->getPATInfo ();

	const PATInfo * pat_current = pat_head;

	Programm * newProgramm = 0;

	while(pat_current)
	{
		newProgramm = new Programm(Source);
		newProgramm->setFrequency (Source->getFrequency ());
		newProgramm->setPATPid (0);
		newProgramm->setSymbolRate (Source->getSymbolRate ());
		newProgramm->setPolarization (Source->isVerticalPolarization());
		newProgramm->setSIDPid (pat_current->SIDPid );
		newProgramm->setPMTPid (pat_current->PMTPid );

		if(FAILED(scanProgramm(Source,newProgramm)))
		{
			delete newProgramm;
			pat_current = pat_current->Next;
			continue;
		}
		if(HeadProgramm == 0)
			HeadProgramm = TailProgramm = newProgramm;
		else
		{
			TailProgramm->setNextProgramm (newProgramm);
			newProgramm->setLastProgramm (TailProgramm);
			TailProgramm = newProgramm;
		}
		pat_current = pat_current->Next ;
	}


	delete PATFilter;
	PATFilter = 0;
	delete SDTFilter_;
	SDTFilter_ = 0;
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT ProgrammScanner::scanProgramm (TSSource *Source, Programm *Prog)
{
	if(Source == 0 || Prog == 0)
		return E_FAIL;

	if(Source->getFrequency () != Prog->getFrequency () ||
	   Source->getSymbolRate () != Prog->getSymbolRate () ||
	   Source->isVerticalPolarization() != Prog->isVerticalPolarization ())
	{
#ifdef DEBUG
		ASSERT(SDTFilter_ == 0);
#endif
		if(FAILED(Source->setTransponder(Prog->getFrequency (),
			                             Prog->getSymbolRate (),
										 Prog->isVerticalPolarization () )))
		{
			return E_FAIL;
		}
	}

	TSPidFilter *PMTFilter = new PMTPidFilter(Prog->getPMTPid (),"DVB Source PMT Pid Filter");
	
	if(FAILED(Source->registerPid (PMTFilter)))
	{
		delete PMTFilter;
		return E_FAIL;
	}

	while(PMTFilter->isProcessingDone () == false)
		_sleep(7);

	const PMT * pmt_head = static_cast<PMTPidFilter *>(PMTFilter)->getPMTInfo ();

	Prog->setAudioPid (pmt_head->AudioPid );
	Prog->setVideoPid (pmt_head->VideoPid );
	Prog->setTeleTextPid (pmt_head->TeleTextPid );
	Prog->setPCRPid (pmt_head->PCRPid );
	Prog->setECMPid(pmt_head->ECMPid );
	Source->unregisterPid (PMTFilter);
	delete PMTFilter;
	PMTFilter = 0;

	if(SDTFilter_ == 0)
	{
		SDTFilter_ = new SDTPidFilter(PID_SDT,"Programm Scanner SDT Pid");
		if(FAILED(Source->registerPid (SDTFilter_)))
		{
			delete SDTFilter_;
			SDTFilter_ = 0;
			return E_FAIL;
		}
		while(SDTFilter_->isProcessingDone () == false)
			_sleep(7);
	}

	const SDT * sdt_head = static_cast<SDTPidFilter *>(SDTFilter_)->getSDTInfo ();

	const SDT * sdt_current = sdt_head;

	while(sdt_current)
	{
		if(sdt_current->SIDPid == Prog->getSIDPid ())
		{
			Prog->setName (sdt_current->Name );
			Prog->setProvider (sdt_current->Provider );
			return S_OK;
		}
		else
			sdt_current = sdt_current->Next;

	}

	return S_OK; //no SDT has been found, but thats ok
}