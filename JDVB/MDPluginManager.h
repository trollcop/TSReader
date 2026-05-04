

#ifndef MDPLUGINMANAGER_H___
#define MDPLUGINMANAGER_H___

/*********************************************************************************
 *                                                                               *
 * MDPluginManager.h Class Managing MD Plugin Objects                            *
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


#include <windows.h>

class TSSource;
class MDPlugin;
struct TProgramm;




typedef struct __MDPluginList__
{
	MDPlugin *Plugin;
	struct __MDPluginList__ *Next;

}MDPluginList;


typedef long HRESULT;

class MDPluginManager
{
protected:
	MDPluginList * Head_;
	MDPluginList * Tail_;
	HWND hWnd_;
	HINSTANCE hInst_;
	TSSource *Source_;
	int Dll_Id_;
public:
	MDPluginManager(TSSource *Source, HINSTANCE hInst, HWND hWnd);

	virtual ~MDPluginManager();

	HRESULT changeChannel(TProgramm *Programm);

	HRESULT dispatch(UINT message, WPARAM *wParam, LPARAM lParam);

	HRESULT loadMDPlugins(const char *Path);
	
protected:

	HRESULT loadMDPlugin(const char *Path);


};

#endif