#include "MDPluginLoader.h"
#include "MDPlugin.h"
#include <windows.h>



//-------------------------------------------------------------------
MDPluginLoader::MDPluginLoader ()
{
	Tail_ =0;
	Dll_Id_ = 0;
}
//-------------------------------------------------------------------
MDPluginLoader::~MDPluginLoader ()
{
	Tail_ = 0;
}
//-------------------------------------------------------------------
HRESULT MDPluginLoader::loadMDPlugins (const char *DirectoryPath, HWND hWnd,
									   HINSTANCE hInst,
									   MDPluginList **PluginList)
{
	if(DirectoryPath == 0 || hWnd == 0)
		return E_FAIL;

	//FIXME
	//Implement Directory Scanning

	if(FAILED(loadMDPlugin("Yankse.dll",hWnd,hInst,PluginList)))
		return E_FAIL;


	//if(FAILED(loadMDPlugin("Soft-Ci.dll",hWnd,hInst, PluginList)))
	//	return E_FAIL;

	return S_OK;
}
//-------------------------------------------------------------------
HRESULT MDPluginLoader::loadMDPlugin (char *PluginPath,
									  HWND hWnd,
									  HINSTANCE hInst,
									  MDPluginList **PluginList)
{
	
}

//-------------------------------------------------------------------


//-------------------------------------------------------------------