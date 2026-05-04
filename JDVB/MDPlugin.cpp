#include "MDPlugin.h"

#include <windows.h>

//-------------------------------------------------------------------
MDPlugin::MDPlugin ()
{
	OnSendDllIDName_ = 0;
	OnStart_ = 0;
	OnMenuSelect_ = 0;
	OnExit_ = 0;
	OnChannelChange_ = 0;
	OnHotKey_ = 0;
	OnOsdKey_ = 0;
	OnFilterClose_ = 0;
	OnRecPlay_ = 0;
	OnVideoTextStream_ = 0;

	DLLID_ = 0;
	Instance_ = 0;
	Name_ = 0;
	HotKey_ = 0;
}

//-------------------------------------------------------------------
MDPlugin::~MDPlugin ()
{
	if(Instance_)
		FreeLibrary(Instance_);

	delete Name_;
}

//--------------------------------------------------------------------
HRESULT MDPlugin::initialize (const char *LibraryName)
{
	if(LibraryName == 0)
		return E_FAIL;

	if(Instance_)
		FreeLibrary(Instance_);

	Instance_ = LoadLibrary(LibraryName);

	if(Instance_ == 0)
		return E_FAIL;
	

	OnSendDllIDName_ = (On_Send_Dll_ID_Name) GetProcAddress(Instance_,"On_Send_Dll_ID_Name");
	OnStart_  = (On_Start) GetProcAddress(Instance_,"On_Start");
	OnMenuSelect_ = (On_Menu_Select) GetProcAddress(Instance_,"On_Menu_Select");
	OnExit_ = (On_Exit) GetProcAddress(Instance_,"On_Exit");
	OnChannelChange_ = (On_Channel_Change) GetProcAddress(Instance_,"On_Channel_Change");
	OnHotKey_ = (On_Hot_Key) GetProcAddress(Instance_,"On_Hot_Key");
	OnOsdKey_ = (On_Osd_Key) GetProcAddress(Instance_,"On_Osd_Key");
	OnFilterClose_ = (On_Filter_Close) GetProcAddress(Instance_,"On_Filter_Close");  
	OnRecPlay_ = (On_Rec_Play) GetProcAddress(Instance_, "On_Rec_Play");
	OnVideoTextStream_ = (On_VideoText_Stream) GetProcAddress(Instance_, "On_VideoText_Stream");

	return S_OK;

}
//---------------------------------------------------------------------
HRESULT MDPlugin::OnSendDllIDName (char *Name)
{
	if(OnSendDllIDName_ == 0 || Name == 0)
		return E_FAIL;

	(OnSendDllIDName_)(Name);

	if(Name[0] =='\0')
		return E_FAIL;
	
	Name_ = _strdup(Name);
	return S_OK;
}
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnStart(HINSTANCE MDInstance, HWND MDWnd, BOOL Log_Set, int DLL_ID, char *Api_Version)
{
	int Keep_Me_Running = 0;

	if(OnStart_ == 0)
		return E_FAIL;
	(OnStart_)(MDInstance,MDWnd,Log_Set,DLL_ID,&HotKey_,Api_Version, &Keep_Me_Running);

	DLLID_ = DLL_ID;
	if(Keep_Me_Running != 0) //see MultiDec API
		return E_FAIL;

	HMENU hMenu = GetMenu(MDWnd);

	if(hMenu == 0)
		return E_FAIL; 

	HMENU hExternMenu;

	hExternMenu = LoadMenu(Instance_,"EXTERN");

	if(hExternMenu == 0)
		return E_FAIL; //no configuration menu

	AppendMenu(hMenu,MF_POPUP,(UINT_PTR)hExternMenu,Name_);

	DrawMenuBar(MDWnd);

	return S_OK;
}
//---------------------------------------------------------------------
HRESULT MDPlugin::OnMenuSelect (unsigned int MenuID)
{
	if(OnMenuSelect_ == 0)
		return E_FAIL;

	(OnMenuSelect_)(MenuID);

	return S_OK;
}
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnExit(HINSTANCE hInst, HWND hWnd, BOOL Any)
{
	if(OnExit_ == 0)
		return E_FAIL;

	(OnExit_)(hInst,hWnd,Any);

	return S_OK;

}
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnChannelChange( struct TProgramm CurrentProgramm )
{
	if(OnChannelChange_ == 0)
		return E_FAIL;

	(OnChannelChange_)(CurrentProgramm);
	
	return S_OK;
}
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnHotKey()
{
	if(OnHotKey_ == 0)
		return E_FAIL;
	(OnHotKey)();

	return S_OK;
}
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnOsdKey(unsigned char Key)
{
	if(OnOsdKey_ == 0)
		return E_FAIL;
	
	(OnOsdKey_)(Key);

	return S_OK;
} 
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnFilterClose(unsigned int FilterOffset)
{
	if(OnFilterClose_ == 0)
		return E_FAIL;
	
	(OnFilterClose_)(FilterOffset);

	return S_OK;
}  
//---------------------------------------------------------------------	
HRESULT MDPlugin::OnRecPlay(int Mode)
{
	if(OnRecPlay_ == 0)
		return E_FAIL;
	
	(OnRecPlay_)(Mode);
	
	return S_OK;
}
//---------------------------------------------------------------------
HRESULT MDPlugin::OnVideoTextStream(int length, unsigned char * data)
{
	if(OnVideoTextStream_ == 0)
		return E_FAIL;

	(OnVideoTextStream_)(length, data);

	return S_OK;
}
//---------------------------------------------------------------------
const int MDPlugin::getDLLID() const
{
	return DLLID_;
}
//---------------------------------------------------------------------
const char MDPlugin::getHotKey() const
{
	return HotKey_;
}
