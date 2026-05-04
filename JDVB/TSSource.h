
#ifndef TSSOURCE_H___
#define TSSOURCE_H___

/*********************************************************************************
 *                                                                               *
 * TSSource.h  Tranponder Stream Source Base Class                               *
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
 *     Version: 0.0.2
 *
 *********************************************************************************/ 

/****************************************************************************************
 *	Class TSSource
 *
 *	This class is an base for MPEG-2 Transponder Source Streams. Any
 *	class which implements the properites of an MPEG-2 Transponder Stream
 *	class must provide the following 2 functions:
 *	1.) registerTSPid
 *		This function registers a TSPidFilter. The class is supposed to store
 *		the TSPidFilter internaly and to call the TSPidFilter's dispatch function
 *		when data arrives for that function
 *	2.) unregisterTSPid
 *		This function deregisters a TSPidFilter. The class is supposed to remove
 *		the TSPidFilter internaly but not DELETING the object itself. The TSPidFilter
 *		should then no longer receive any Data from that TSSource class
 *
 *
 ***************************************************************************************/

#include <string.h> /* strdup */

class TSPidFilter;
class Programm;

typedef long HRESULT;
#define UNREFERENCED_PARAMETER(P)	(P)

#include <winerror.h>

class TSSource
{
protected:
	char *Name_;
public:
	//------------------------------------------------------------------
	TSSource(char *Name)
	{
		if(Name)
			Name_ = _strdup(Name);
		else
			Name_ = 0;
	}
	//------------------------------------------------------------------
	TSSource()
	{
		Name_ = 0;
	}
	//------------------------------------------------------------------
	virtual ~TSSource()
	{
		delete [] Name_;
	}
	//------------------------------------------------------------------
	virtual HRESULT registerPid(TSPidFilter *PidFilter) = 0;
	//------------------------------------------------------------------
	virtual HRESULT unregisterPid(TSPidFilter *PidFilter) = 0;
	//------------------------------------------------------------------
	virtual const unsigned long getNumOfProcessedPackets() const
	{
		return 0L;
	}
	//------------------------------------------------------------------
	virtual const unsigned long getNumOfDiscardedPackets() const
	{
		return 0L;
	}
	//------------------------------------------------------------------
	virtual const char * getName() const
	{
		return Name_;
	}
	//------------------------------------------------------------------
	virtual const unsigned long getFrequency() const
	{
		return 0l;
	}
	//------------------------------------------------------------------
	virtual const unsigned int getSymbolRate () const
	{
		return 0;
	}
	//------------------------------------------------------------------
	virtual const unsigned char getDiSEqC() const
	{
		return 0;
	}
	//------------------------------------------------------------------
	virtual const bool isVerticalPolarization() const
	{
		return true;
	}
	virtual HRESULT setTransponder(const unsigned long Frequency,
		                           const unsigned long SymbolRate,
								   const bool Vertical)
	{
		UNREFERENCED_PARAMETER(Frequency);
		UNREFERENCED_PARAMETER(SymbolRate);
		UNREFERENCED_PARAMETER(Vertical);
		return E_NOTIMPL;

	}
};

#endif /* TSSource.h */