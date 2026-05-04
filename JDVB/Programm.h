
#ifndef PROGRAMM_H___
#define PROGRAMM_H___

/*********************************************************************************
 *                                                                               *
 * Programm.h Programm Information Class                                         *
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

/********************************************************************
 * Class Programm
 *
 * This class is a simple storage class for Programm Inforamtion 
 * TSTarget classes will create the appropiate information from that information
 * However, the calling class should never delete the gotten object
 * itself, this is done by the class programm
 *
 *******************************************************************/


#include <windows.h>

class TSSource;
class TSTarget;
class TSPidFilter;
class MDPlugin;


class Programm
{
protected:
	unsigned short ECMPid_;
	unsigned short SIDPid_;
	unsigned short PATPid_;
	unsigned short PMTPid_;
	unsigned short PCRPid_;
	unsigned short AudioPid_;
	unsigned short VideoPid_;
	unsigned short TeleTextPid_;
	unsigned long Frequency_;
	unsigned long SymbolRate_;
	bool Vertical_;
	HWND hWnd_;
	bool AC3_;
	char *Name_;
	char *Provider_;
	bool Active_;
	Programm *Next_;
	Programm *Last_;
	TSTarget *Target_;
	TSSource *Source_;
public:
	Programm(TSSource *Source);
	Programm();
	virtual ~Programm();	
	void setName(const char *Name);
	void setTSSource(TSSource *Source);
	void setProvider(const char *Provider);
	void setECMPid(const unsigned short Pid);
	void setPATPid(const unsigned short Pid);
	void setPMTPid(const unsigned short Pid);
	void setPCRPid(const unsigned short Pid);
	void setAudioPid(const unsigned short Pid, const bool AC3 = false);
	void setVideoPid(const unsigned short Pid);
	void setFrequency(const unsigned long Frequency);
	void setSymbolRate(const unsigned long SymbolRate);
	void setPolarization(const bool Vertical);
	void setTeleTextPid(const unsigned short Pid);
	void setSIDPid(const unsigned short Pid);
	void setNextProgramm(Programm *Prg);
	void setLastProgramm(Programm *Prg);
	Programm *getLastProgramm() const;
	Programm *getNextProgramm() const;
	const unsigned short getECMPid() const;
	const unsigned short getSIDPid() const;
	const unsigned short getPATPid() const;
	const unsigned short getPMTPid() const;
	const unsigned short getPCRPid() const;
	const unsigned short getAudioPid() const;
	const unsigned short getVideoPid() const;
	const unsigned short getTeleTextPid() const;
	const unsigned long getFrequency() const;
	const unsigned long getSymbolRate() const;
	const char *getName() const;
	const bool isVerticalPolarization() const;
	const bool isActive() const; //returns true if this programm is playing back video
	
	const bool isEqual(const Programm *Prog);

	HRESULT insert(TSTarget *TSTarget);

	HRESULT remove(TSTarget *TSTarget);


	HRESULT configureVideoWindow(HWND hWnd, RECT rect);

	HRESULT toggleVolume();

	HRESULT togglePlayback(); //pauses -> starts the playback

	HRESULT toggleFullScreen();

	HRESULT getProperties(HINSTANCE hInstOwner, HWND hWndOwner, HWND * Window, unsigned int * CommandId);

protected: 
	
};
LRESULT CALLBACK ProgrammDlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);




#endif