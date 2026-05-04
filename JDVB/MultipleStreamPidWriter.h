
#ifndef MULTIPLESTREAMWRITER_H___
#define MULTIPLESTREAMWRITER_H___

/*********************************************************************************
 *                                                                               *
 * MultipleStreamWriter.h Writes Multiple Streams to Disk                        *
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
 *     Version: 0.0.0
 *
 *********************************************************************************/ 



/********************************************************************
 *
 *
 * Class which is supposed to write multiple streams to disk
 *
 *
 *
 *
 *
 *******************************************************************/

typedef long HRESULT;


class MultipleStreamWriter : public TSPidFilter
{
protected:
	unsigned char Pids_[255];
	unsigned char NumOfPids_;
	char *FileName_;
	FILE *File_;

public:
	MultipleStreamWriter(const unsigned short Pid, const char *PidName);

	virtual ~MultipleStreamWriter();

	HRESULT addPid(const unsigned short Pid);
	HRESULT delPid(const unsigned short Pid);

	const unsigned char getPidCount();
	const unsigned short getPid(const unsigned char PidIndex);
	const int getPriority();



};
#endif