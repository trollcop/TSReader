
#ifndef MDPLUGIN_H___
#define MDPLUGIN_H___

/*********************************************************************************
 *                                                                               *
 * MDPlugin.h  Class representing MD Plugin object                               *
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

#include "MDAPI.h"

#include <windows.h>


typedef void (__cdecl *On_Send_Dll_ID_Name) (char *Name);
typedef void (__cdecl *On_Start)(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set, int DLL_ID, char *My_Hot_Key, char *Api_Version, int *Keep_me_running);
typedef void (__cdecl *On_Menu_Select) (unsigned int MenuID);
typedef void (__cdecl *On_Exit)(HINSTANCE, HWND, BOOL);
typedef void (__cdecl *On_Channel_Change)( struct TProgramm CurrentProgramm ); 
typedef void (__cdecl *On_Hot_Key)(void);
typedef void (__cdecl *On_Osd_Key)(unsigned char Key);  
typedef void (__cdecl *On_Filter_Close)(unsigned int FilterOffset);   
typedef void (__cdecl *On_Rec_Play)(int Mode);
typedef void (__cdecl *On_VideoText_Stream)(int length, unsigned char * data);

typedef long HRESULT;


struct TProgramm;


class MDPlugin
{
protected:
On_Send_Dll_ID_Name OnSendDllIDName_;
On_Start OnStart_;
On_Menu_Select OnMenuSelect_;
On_Exit OnExit_;
On_Channel_Change OnChannelChange_;
On_Hot_Key OnHotKey_;
On_Osd_Key OnOsdKey_;
On_Filter_Close OnFilterClose_;
On_Rec_Play OnRecPlay_;
On_VideoText_Stream OnVideoTextStream_;

int DLLID_;
char *Name_;
char HotKey_;
HMODULE Instance_;


public:
	MDPlugin();

	virtual ~MDPlugin();

	HRESULT initialize(const char *LibraryName);
	
	const int getDLLID() const;
	const char getHotKey() const;
	

	HRESULT OnSendDllIDName (char *Name);
	HRESULT OnStart(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set, int DLL_ID, char *Api_Version);
	HRESULT OnMenuSelect (unsigned int MenuID);
	HRESULT OnExit(HINSTANCE, HWND, BOOL);
	HRESULT OnChannelChange( struct TProgramm CurrentProgramm ); 
	HRESULT OnHotKey();
	HRESULT OnOsdKey(unsigned char Key);  
	HRESULT OnFilterClose(unsigned int FilterOffset);   
	HRESULT OnRecPlay(int Mode);
	HRESULT OnVideoTextStream(int length, unsigned char * data);
};
#endif