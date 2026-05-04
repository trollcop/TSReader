
#ifndef DVBSOURCE_H___
#define DVBSOURCE_H___

/*********************************************************************************
 *                                                                               *
 * DVBSource.h		TSSource implementation for TwinHan                          *
 *                                                                               *
 * Copyright (C) 2003      Ware4z                                                *                                                                               *
 *                                                                               *
 * This program is free software; you can redistribute it and/or                 *
 * modify it under the terms of the GNU General Public License                   *
 * as published by the Free Software Foundation; either version 2                *
 * of the License, or (at your option) any later version.                        *
 *                                                                               *
 *                                                                               *          
 * This program is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
 * GNU General Public License for more details.                                  *
 *                                                                               *
 *                                                                               *
 * You should have received a copy of the GNU General Public License             *
 * along with this program; if not, write to the Free Software                   *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.    *
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html                *
 *                                                                               *
 *                                                                               *
 * The author can be reached at j_anderw@sbox.tugraz.at                          *
 *
 * Version 0.0.2
 *
 *********************************************************************************/ 


/********************************************************************
 * Class DVBSource
 *
 * This class is a concreate implementation of the abstract TSSource 
 * class. This source class uses the DirectShow Filter of the TwinHan
 * SDK (DVB Source). Actually it creates an IGraphBuilder object, adds
 * the TwinHan DVB Source filter to it, and adds callback functions
 * to the DVBSource Filter. When data arrives via one of the 32 
 * HARDCODED callback function, it just submits the data to one 
 * appropiate TSPidFilterQueue
 *
 *******************************************************************/


#include "TSSource.h"
#include "Interface.h"

class TSPacketQueue;
class Transponder;

#include <streams.h>

class DVBSource : public TSSource
{
protected:
unsigned short ActiveFilters_[33];
int MaxFilters_;
int NumOfFilters_;
TSPacketQueue *PacketQueue_;
IBaseFilter *DVBSourceBase_;
IDVBSource  *DVBSource_;
Transponder *HeadTransponder_;
Transponder *TailTransponder_;
Transponder *ActiveTransponder_;

public:

	DVBSource();

	virtual ~DVBSource();

	

	HRESULT registerPid(TSPidFilter *PidFilter);
	HRESULT unregisterPid(TSPidFilter *PidFilter);
	HRESULT registerCallback (const unsigned short Pid, TSPidFilter * Filter);
	HRESULT unregisterCallback (const unsigned short Pid);

	void insertRawStream(unsigned char *Data);

	const int getNumOfFilters() const;

	HRESULT setTransponder(const unsigned long Frequency,
						   const unsigned long SymbolRate,
						   const bool VerticalPolarization);
	HRESULT DVBSource::scanProgramm(Programm *Prog);

	const int getMaxFilters() const;
	const bool isVerticalPolarization() const;
	const unsigned int getSymbolRate() const;
	const unsigned long getFrequency() const;
	const unsigned long DVBSource::getNumOfDiscardedPackets () const;
	const unsigned long DVBSource::getNumOfProcessedPackets () const;
	HRESULT findTransponder(const unsigned long Frequency,
		                    const unsigned long SymbolRate,
							const bool VerticalPolarization,
							Transponder ** Trans);

	HRESULT scanTransponder();
	HRESULT findProgrammBySID(const unsigned long Frequency,
  							  const unsigned long SymbolRate,
							  const bool VerticalPolarization,
							  const unsigned short PMTPid,
							  Programm ** Prog);
	HRESULT findProgrammByPMT(const unsigned long Frequency,
							  const unsigned long SymbolRate,
							  const bool VerticalPolarization,
							  const unsigned short PMTPid,
							  Programm ** Prog);
	HRESULT findProgrammByName(const unsigned long Frequency,
							   const unsigned long SymbolRate,
							   const bool VerticalPolarization,
							   const char *Name,
							   Programm ** Prog);

	HRESULT importProgramm(Programm *Prog);

};

//-------------------------------------------------------------------
LRESULT CALLBACK DVBSourceDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//-------------------------------------------------------------------
void __stdcall Filter00(unsigned char *Data);
void __stdcall Filter01(unsigned char *Data);
void __stdcall Filter02(unsigned char *Data);
void __stdcall Filter03(unsigned char *Data);
void __stdcall Filter04(unsigned char *Data);
void __stdcall Filter05(unsigned char *Data);
void __stdcall Filter06(unsigned char *Data);
void __stdcall Filter07(unsigned char *Data);
void __stdcall Filter08(unsigned char *Data);
void __stdcall Filter09(unsigned char *Data);
void __stdcall Filter00(unsigned char *Data);
void __stdcall Filter0A(unsigned char *Data);
void __stdcall Filter0B(unsigned char *Data);
void __stdcall Filter0C(unsigned char *Data);
void __stdcall Filter0D(unsigned char *Data);
void __stdcall Filter0E(unsigned char *Data);
void __stdcall Filter0F(unsigned char *Data);
void __stdcall Filter10(unsigned char *Data);
void __stdcall Filter11(unsigned char *Data);
void __stdcall Filter12(unsigned char *Data);
void __stdcall Filter13(unsigned char *Data);
void __stdcall Filter14(unsigned char *Data);
void __stdcall Filter15(unsigned char *Data);
void __stdcall Filter16(unsigned char *Data);
void __stdcall Filter17(unsigned char *Data);
void __stdcall Filter18(unsigned char *Data);
void __stdcall Filter19(unsigned char *Data);
void __stdcall Filter1A(unsigned char *Data);
void __stdcall Filter1B(unsigned char *Data);
void __stdcall Filter1C(unsigned char *Data);
void __stdcall Filter1D(unsigned char *Data);
void __stdcall Filter1E(unsigned char *Data);
void __stdcall Filter1F(unsigned char *Data);



#endif