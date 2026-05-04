
#ifndef TSTARGET_H___
#define TSTARGET_H___
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
 *     Version: 0.0.1
 *
 *********************************************************************************/ 


/********************************************************************
 * Class TSTargetFilter
 *
 * This class represents an abstract playback class. This means that
 * this class is responsible for playing back the received video / audio
 * In order to support multiple implementations(eg DirectShow enabled playback /
 * DirectDraw rendered with DirectSound... or viewing over LAN), this class has to create
 * the necessary TSPidFilter that will transform the TSPackets to the appropiate
 * location to perform playback
 *
 *
 *******************************************************************/


#include <windows.h>

class TSPidFilter;

class TSTarget
{
protected:
char *Name_;
unsigned long ProcessedPackets_;
unsigned long DiscardedPackets_;
public:
	//------------------------------------------------------------------
	TSTarget(const char *Name)
	{
		if(Name)
			Name_ = _strdup(Name);
		else
			Name_ = 0;
		ProcessedPackets_ = DiscardedPackets_ = 0;
	}
	//------------------------------------------------------------------
	virtual ~TSTarget()
	{
		delete Name_;
	}
	//------------------------------------------------------------------
	virtual unsigned long getNumOfProcessedPackets()
	{
		return ProcessedPackets_;
	}
	//------------------------------------------------------------------
	virtual unsigned long getNumOfDiscardedPackets()
	{
		return DiscardedPackets_;
	}
	//------------------------------------------------------------------
	//creates TSPidFilter
	//does not store it internaly
	virtual HRESULT createTSTargetFilter(const unsigned short Pid, const unsigned int PidType, const char *PidName, TSPidFilter **Filter) = 0;
	virtual HRESULT deleteTSTargetFilter( const unsigned int PidType) = 0;
	//------------------------------------------------------------------
	//starts / stops playback
	virtual HRESULT togglePlayback()
	{
		return E_NOTIMPL;
	}
	//------------------------------------------------------------------
	//sets to mute or volume on
	virtual HRESULT toggleVolume()
	{
		return E_NOTIMPL;
	}
	//------------------------------------------------------------------
	//sets either to full screen mode or to normal mode
	virtual HRESULT toggleFullScreen(HWND hWnd)
	{
		return E_NOTIMPL;
	}
	//------------------------------------------------------------------
	//returns the property page of this module when available
	virtual HRESULT getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND * Window, unsigned int * CommandId)
	{
		return E_NOTIMPL;
	}
	//------------------------------------------------------------------
	//sets the video target window & position
	virtual HRESULT setVideoProperties(HWND hWnd, RECT rect)
	{
		return E_NOTIMPL;
	}
	//------------------------------------------------------------------
	//used for dshow implementation to load an external graph
	//for file write implementation to set the target file name
	virtual HRESULT setTargetFileName(const char * Name)
	{
		return E_NOTIMPL;
	}
	


};

#endif