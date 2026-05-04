#include "MDPluginManager.h"
#include "MDPluginManager.h"
#include "MDPluginLoader.h"
#include "MDAPI.h"
#include "MDPlugin.h"

#include <winerror.h>
#include <stdio.h>

//-------------------------------------------------------------------
MDPluginManager::MDPluginManager (TSSource *Source, HINSTANCE hInst, HWND hWnd)
{
	Source_ = Source;
	hWnd_ = hWnd;
	hInst_ = hInst;
	Head_ = 0;
	Tail_ = 0;
	Dll_Id_ = 0;
}	
//-------------------------------------------------------------------
MDPluginManager::~MDPluginManager ()
{
	if(Head_ == 0)
		return;

	do
	{
		Tail_ = Head_->Next ;
		delete Head_->Plugin ;
		delete Head_;
		Head_ = Tail_;
	}
	while(Head_);
}
//-------------------------------------------------------------------
HRESULT MDPluginManager::loadMDPlugins (const char *Path)
{
	
	if(Head_ == 0)
		Head_ = Tail_;

	//FIXME 
	//implement Directory scanning

	if(FAILED(loadMDPlugin("Soft-Ci.dll")))
		return E_FAIL;

	//if(FAILED(loadMDPlugin("Yankse.dll")))
	//	return E_FAIL;

	return S_OK;
}

//-------------------------------------------------------------------
HRESULT MDPluginManager::changeChannel (TProgramm *Programm)
{
	if(Head_ == 0)
		return E_FAIL;

	MDPluginList *Current = Head_;

	do
	{
		Current->Plugin->OnChannelChange (*Programm);
	}
	while(Current);
	
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT MDPluginManager::loadMDPlugin (const char *Path)
{
	if(Path == 0 || hWnd_ == 0 || hInst_ == 0)
		return E_FAIL;

	MDPlugin * Plugin = new MDPlugin();
	if(FAILED(Plugin->initialize (Path)))
	{
		delete Plugin;
		return E_FAIL;
	}

	TCHAR Buffer[300];

	if(FAILED(Plugin->OnSendDllIDName (Buffer)))
	{
		delete Plugin;
		return E_FAIL;
	}

	MDPluginList *TailCopy = Tail_;

	if(Head_ == 0) //first plugin successfully loaded
	{
		Head_ = Tail_= new MDPluginList();
	}
	else
	{
		Tail_->Next = new MDPluginList();
		Tail_ = Tail_->Next;
	}
	Tail_->Next = 0;

	static char ApiVersion [] = "MD-API Version 01.02 Root";
	if(FAILED(Plugin->OnStart (hInst_,hWnd_,true,Dll_Id_, ApiVersion)))
	{
		if(Head_ == Tail_)
		{
			delete Tail_;
			Head_ = Tail_ = 0;
		}
		else
		{
			delete Tail_;
			Tail_ = TailCopy;
		}
		delete Plugin;
		return E_FAIL;
	}
	Tail_->Plugin = Plugin;
	
	Dll_Id_++;
	return S_OK;
}
//-------------------------------------------------------------------
HRESULT MDPluginManager::dispatch (UINT MessageType,WPARAM *wParam,LPARAM lParam)
{
	if(Head_ == 0)
		return E_FAIL;

	TCHAR Buffer [256];
	unsigned char index;
	

	UNREFERENCED_PARAMETER(Buffer);

	switch(MessageType)
	{
		case MDAPI_ON_MENU_SELECT:
		{
			unsigned int wmId = (unsigned int)*wParam;
			//sprintf(buffer,"Id%u",wmId); MessageBox(hWnd,buffer,"Information",MB_OK);
			
			MDPluginList * Current = Head_;
			do
			{
				Current->Plugin->OnMenuSelect (wmId);
				Current = Current->Next;
			}
			while(Current);
			
			break;
		}
		
		case MDAPI_ON_EXIT:
		{
			MDPluginList * Current = Head_;
			do
			{
				Current->Plugin->OnExit (hInst_,hWnd_,true);
				Current = Current->Next;
			}
			while(Head_);			
		}
		case MDAPI_ON_CHANNEL_CHANGE:
		{
			sprintf(Buffer,"MDAPI_ON_CHANNEL_CHANGE is currently UNIMPLEMENTED");
			return E_NOTIMPL;
		}
		case MDAPI_ON_HOT_KEY:
		{
			unsigned char HotKey = (unsigned char )*wParam;
			MDPluginList * Current = Head_;
			do
			{
				if(Current->Plugin->getHotKey () == HotKey)
				{
					Current->Plugin->OnHotKey ();
					break;
				}
				Current = Current->Next;
			}
			while(Head_);
			break;
		}
		case MDAPI_ON_OSD_KEY:
		{
			//FIXME
			//feature is unsupported
			sprintf(Buffer,"MDAPI_ON_OSD_KEY is currently UNIMPLEMENTED");
			MessageBox(hWnd_,Buffer,"Information",MB_OK);
			return E_NOTIMPL;
		}
		case MDAPI_ON_FILTER_CLOSE:
		{
			sprintf(Buffer,"MDAPI_ON_FILTER_CLOSE is currently UNIMPLEMENTED");
			MessageBox(hWnd_,Buffer,"Information",MB_OK);
			return E_NOTIMPL;
		}
		case MDAPI_ON_REC_PLAY:
		{
			sprintf(Buffer,"MDAPI_ON_REC_PLAY is currently UNIMPLEMENTED");
			MessageBox(hWnd_,Buffer,"Information",MB_OK);
			return E_NOTIMPL;
			break;
		}
			
//--------------------------------------------------------------------------------
//real MDAPI_messages

		case MDAPI_START_FILTER:
		{
			TSTART_FILTER *filter = (TSTART_FILTER*)lParam;
			sprintf(Buffer,"MDAPI_START_FILTER is currently UNIMPLEMENTED");
			//sprintf(buffer,"MD_API_START_FILTER, Name %s, Dll ID %u,FilterID %u, Pid %u, Address %x\n",filter->Name , filter->DLL_ID , filter->Filter_ID , filter->Pid , filter->Irq_Call_Adresse );
			MessageBox(hWnd_,Buffer,"Information",MB_OK);
			break;
		}
		case MDAPI_GET_TRANSPONDER:
		{
			MessageBox(hWnd_,"MDAPI_GET_TRANSPONDER is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}

		case MDAPI_SET_TRANSPONDER:
		{
			MessageBox(hWnd_,"MDAPI_SET_TRANSPONDER is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_GET_PROGRAMM:
		{
			MessageBox(hWnd_,"MDAPI_GET_PROGRAMM is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}

		case MDAPI_SET_PROGRAMM:
		{
			MessageBox(hWnd_,"MDAPI_SET_PROGRAMM is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_RESCAN_PROGRAMM:
		{
			MessageBox(hWnd_,"MDAPI_RESCAN_PROGRAMM is currently UNIMPLEMENTED","Information",MB_OK);	
			break;
		}

		case MDAPI_SAVE_PROGRAMM:
		{
			MessageBox(hWnd_,"MDAPI_SAVE_PROGRAMM is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_GET_PROGRAMM_NUMMER:
		{
			MessageBox(hWnd_,"MDAPI_GET_PROGRAMM_NUMBER is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_SET_PROGRAMM_NUMMER:
		{
			MessageBox(hWnd_,"MDAPI_SET_PROGRAMM_NUMBER is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		
		case MDAPI_STOP_FILTER:
		{
			//int len = sprintf(buffer,"MDAPI_STOP_FILTER: %s\n",MDFilter[(int)lParam]);
			//fwrite(buffer,len,1,log);
			MessageBox(hWnd_,"MDAPI_STOP_FILTER is currently UNIMPLEMENTED","Information",MB_OK);
			//MessageBox(hWnd,buffer,"Information",MB_OK);
			//stopMDAPIFilter((int)lParam);
			break;
		}
		
		case MDAPI_SCAN_CURRENT_TP:
		{
			MessageBox(hWnd_,"MDAPI_SCAN_CURRENT_TP is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_SCAN_CURRENT_CAT:
		{
			MessageBox(hWnd_,"MDAPI_SCAN_CURRENT_CAT  is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}

		case MDAPI_START_OSD:
		{
			MessageBox(hWnd_,"MDAPI_START_OSD  is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_OSD_DRAWBLOCK:
		{
			MessageBox(hWnd_,"MDAPI_OSD_DRAWBLOCK is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_OSD_SETFONT:
		{
			MessageBox(hWnd_,"MDAPI_OSD_SETFONT is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_OSD_TEXT:
		{
			MessageBox(hWnd_,"MDAPI_OSD_TEXT is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_SEND_OSD_KEY:
		{
			MessageBox(hWnd_,"MDAPI_SEND_OSD_KEY is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_STOP_OSD:
		{
			MessageBox(hWnd_,"MDAPI_STOP_OSD is currently UNIMPLEMENTED","Information",MB_OK);
			break;
		}
		case MDAPI_GET_VERSION:
		{
		//	MessageBox(hWnd,"MDAPI_GET_VERSION","Information",MB_OK);
			strcpy((char *)lParam,"MD-API Version 01.02 Root 1.04");
			break;
		}
		case MDAPI_DVB_COMMAND:
		{
			MessageBox(hWnd_,"MDAPI_DVB_COMMAND is currently UNIMPLEMENTED","Information",MB_OK);		
			break;
		}
		default:
		{
			MessageBox(hWnd_,"MDAPI_UNKNOWN_MESSAGE","Information",MB_OK);
			break;
		}
	}

	return S_OK;
}
//-------------------------------------------------------------------