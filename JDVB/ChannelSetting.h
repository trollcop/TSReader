
#ifndef FILESETTINGS_H___
#define FILESETTINGS_H___

/*********************************************************************************
 *                                                                               *
 * ChannelSetting.h Channel Setting Writer / Loader                              *
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
 * Class ChannelSetting 
 *
 * This class is used to load / write Programm Information to disk
 * Due to my lack of interest on implemented on existing standard DVB
 * file format, i have choosen to write a sample one. If anybody is interested
 * in supporting an other file format for this Application, pm me at
 * j_anderw@sbox.tugraz.at
 *
 *******************************************************************/





#include <stdio.h>

const unsigned char SatelliteId = 'S';
const unsigned char TransponderId ='T';
const unsigned char ProgrammId = 'P';


typedef struct __SatelliteInfo__
{
	unsigned char TableId;
	char *Name;
	char *Device;
}SatelliteInfo;
typedef struct __TransponderInfo__
{
	unsigned char TableId;
	unsigned char BitField1;
	//unsigned char Polarization:1;
	//unsigned char Tone:1;
	//unsigned char Reserved:6;
	unsigned long Frequency;
	unsigned long SymbolRate;
}TransponderInfo;

typedef struct __ProgrammInfo__
{
	unsigned char TableId;
	unsigned char BitField1;
	//unsigned char ProgrammType:4;
	//unsigned char Scrambled:1;
	//unsigned char AC3:1;
	//unsigned char Reserved:2;
	unsigned short SIDPid;
	unsigned short PMTPid;
	unsigned short AudioPid;
	unsigned short VideoPid;
	unsigned short TeleTextPid;
	unsigned short ECMPid;
	unsigned short PCRPid;
	char *ProgrammName;
}ProgrammInfo;

class Programm;
typedef long HRESULT;


class ChannelSetting
{
protected:

public:	
	ChannelSetting();

	virtual ~ChannelSetting();

	HRESULT write(Programm *Prog, const int NumOfProgramm, const char *FileName, const char *SatelliteName, const char * DeviceName); 

	HRESULT read(Programm **Prog, const char *FileName);

protected:
	HRESULT writeProgramm(Programm *Prog, FILE *fd);
	HRESULT readTransponder(TransponderInfo *TransInfo, FILE *fd);
	HRESULT readSatellite(SatelliteInfo *SatInfo, FILE *fd);
 	HRESULT readProgramm(Programm *Prog, FILE *fd, TransponderInfo *TransInfo);
	HRESULT writeTransponder(TransponderInfo *TransInfo, FILE *fd);
	HRESULT writeSatellite(const char *SatelliteName, const char *DeviceName, FILE *fd);
};
#endif