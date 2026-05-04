#include <windows.h>
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <wchar.h>
#include <stdio.h>

#define TASKS_TO_RETRIEVE          1000

#include "..\epgschedule.h"

// Microsoft's wcstombs & mbstowcs functions stop on NULL characters; these ones don't
size_t my_wcstombs(char *mbstr, wchar_t *wcstr, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		*mbstr++ = (char) *wcstr++;
	}
	return count;
}

size_t my_mbstowcs(wchar_t *wcstr, char *mbstr, size_t count)
{
	size_t i;

	for (i = 0; i < count; i++)
	{
		(wchar_t)*wcstr++ = (TBYTE)*mbstr++ & 0xFF;
	}
	return count;
}

BOOL Scheduler_CreateNewTask(char * szProgram, char * szParameters, char * szTaskName, 
							 char * szComment, char * szUsername, char * szPassword, 
							 SYSTEMTIME * stRunDate, int nDuration, BOOL fRunNow,
							 BOOL fRequiredLogin, BOOL fPowerResume)
{
	HRESULT hr = S_OK;
	ITaskScheduler *pITS;
	int i;
	char szWorkingDirectory[MAX_PATH];
	wchar_t lszWorkingDirectory[512];
	wchar_t lszProgram[512];
	wchar_t lszParameters[512];
	wchar_t lszTaskName[512];
	wchar_t lszComment[512];
	wchar_t lszUsername[512];
	wchar_t lszPassword[512];

	lstrcpy(szWorkingDirectory, szProgram);
	for (i = lstrlen(szWorkingDirectory); i > 0; i--)
	{
		if (szWorkingDirectory[i] == '\\')
		{
			szWorkingDirectory[i] = '\0';
			break;
		}
	}
	my_mbstowcs(lszWorkingDirectory, szWorkingDirectory, lstrlen(szProgram) + 1);
	my_mbstowcs(lszProgram, szProgram, lstrlen(szProgram) + 1);
	my_mbstowcs(lszParameters, szParameters, lstrlen(szParameters) + 1);
	my_mbstowcs(lszTaskName, szTaskName, lstrlen(szTaskName) + 1);
	my_mbstowcs(lszComment, szComment, lstrlen(szComment) + 1);
	my_mbstowcs(lszUsername, szUsername, lstrlen(szUsername) + 1);
	my_mbstowcs(lszPassword, szPassword, lstrlen(szPassword) + 1);

	// CoCreateInstance to get the Task Scheduler object. 
	hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **) &pITS);
	if (FAILED(hr))
		return FALSE;

	// Call ITaskScheduler::NewWorkItem to create new task.
	ITask *pITask;
	IPersistFile *pIPersistFile;
	hr = pITS->NewWorkItem(lszTaskName,				// Name of task
					 CLSID_CTask,					// Class identifier 
					 IID_ITask,						// Interface identifier
					 (IUnknown**)&pITask);			// Address of task interface
	pITS->Release();
	if (FAILED(hr))
		return FALSE;

	// Set the comment, parameters and program
	int nFlags = TASK_FLAG_DELETE_WHEN_DONE;

	if (fRequiredLogin)
		nFlags |= TASK_FLAG_RUN_ONLY_IF_LOGGED_ON;
	if (fPowerResume)
		nFlags |= TASK_FLAG_SYSTEM_REQUIRED;
	pITask->SetFlags(nFlags);
	pITask->SetComment(lszComment);
	pITask->SetApplicationName(lszProgram);
	pITask->SetParameters(lszParameters);
	pITask->SetWorkingDirectory(lszWorkingDirectory);
	pITask->SetAccountInformation(lszUsername, lszPassword);

	// Call ITask::CreateTrigger to create new trigger.
	ITaskTrigger *pITaskTrigger;
	WORD piNewTrigger;
	hr = pITask->CreateTrigger(&piNewTrigger, &pITaskTrigger);
	if (FAILED(hr))
	{
		OutputDebugString("Failed calling ITask::CreateTrigger\n");
		return FALSE;
	}

	// Define TASK_TRIGGER structure. Note that wBeginDay,
	// wBeginMonth, and wBeginYear must be set to a valid 
	// day, month, and year respectively.
	TASK_TRIGGER pTrigger;
	memset(&pTrigger, 0, sizeof(pTrigger));
	
	SYSTEMTIME stRunDateLocal;
	SystemTimeToTzSpecificLocalTime(NULL, stRunDate, &stRunDateLocal);

	pTrigger.cbTriggerSize = sizeof(TASK_TRIGGER); 
	pTrigger.wBeginDay = stRunDateLocal.wDay;
	pTrigger.wBeginMonth = stRunDateLocal.wMonth;
	pTrigger.wBeginYear = stRunDateLocal.wYear;
	pTrigger.wStartHour = stRunDateLocal.wHour;
	pTrigger.wStartMinute = stRunDateLocal.wMinute;
	pTrigger.TriggerType = TASK_TIME_TRIGGER_ONCE;
	pTrigger.Type.Daily.DaysInterval = 0;

	// Call ITaskTrigger::SetTrigger to set trigger criteria.
	hr = pITaskTrigger->SetTrigger (&pTrigger);
	if (FAILED(hr))
	{
		OutputDebugString("Failed calling ITaskTrigger::SetTrigger\n");
		return FALSE;
	}

	// Call IUnknown::QueryInterface to get a pointer to 
	// IPersistFile and IPersistFile::Save to save 
	// the new task to disk.
	hr = pITask->QueryInterface(IID_IPersistFile, (void **)&pIPersistFile);
	hr = pIPersistFile->Save(NULL, TRUE);
	if (fRunNow)
		pITask->Run();
	pITask->Release();
	pITaskTrigger->Release();
	if (FAILED(hr))
		return FALSE;
	pIPersistFile->Release();
	if (FAILED(hr))
		return FALSE;

	return TRUE;
}

BOOL Schedule_DeleteSchedule(char * szTaskName)
{
	HRESULT hr;
	wchar_t lszTaskName[512];

	my_mbstowcs(lszTaskName, szTaskName, lstrlen(szTaskName) + 1);

	// CoCreateInstance to get the Task Scheduler object.
	ITaskScheduler *pITS;
	hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **) &pITS);
	if (FAILED(hr))
		return FALSE;

	hr = pITS->Delete(lszTaskName);

	//Release ITaskScheduler interface.
	pITS->Release();

	return TRUE;

}

BOOL ReadTask(LPWSTR lpcwszTaskName, PEPGSCHEDULE pepgschedule, int * nEPGScheduleItems, int * nEPGScheduleMax, char * szSourceName)
{
	HRESULT hr;

	// CoCreateInstance to get the Task Scheduler object.
	ITaskScheduler *pITS;
	hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **) &pITS);
	if (FAILED(hr))
		return FALSE;

	// Call ITaskScheduler::Activate to get the Task object.
	ITask *pITask;
	hr = pITS->Activate(lpcwszTaskName, IID_ITask, (IUnknown**) &pITask);

	//Release ITaskScheduler interface.
	pITS->Release();
	if (FAILED(hr))
		return FALSE;

	// Call ITask::GetComment. Note that this method is 
	// inherited from IScheduledWorkItem.
	LPWSTR ppwszParameters;
	char szParameters[512];
	char * szSourceNamePtr;
	char * szSpace;

	hr = pITask->GetParameters(&ppwszParameters);
	my_wcstombs(szParameters, ppwszParameters, lstrlenW(ppwszParameters) + 1);
	szSourceNamePtr = strstr(szParameters, "-s");
	if (szSourceNamePtr == NULL)
		return FALSE;
	szSourceNamePtr += 3; // skip the '-s '
	szSpace = strstr(szSourceNamePtr, " ");
	if (szSpace == NULL)
		return FALSE;
	*szSpace = '\0';
	if (lstrcmp(szSourceNamePtr, szSourceName) != 0)
		return FALSE;		// recording is for another input source

	LPWSTR ppwszComment;
	char * szTSReaderSection;
	char szComment[512];
	hr = pITask->GetComment(&ppwszComment);

	my_wcstombs(szComment, ppwszComment, lstrlenW(ppwszComment) + 1);
	szTSReaderSection = strstr(szComment, "TSReader:[");
	if (szTSReaderSection != NULL)
	{
		int nCount;
		int i;
		BYTE * pData = (BYTE *)&pepgschedule[*nEPGScheduleItems];
		char szTemp[4];

		memset(pData, 0, sizeof(pepgschedule[*nEPGScheduleItems]));
		szTemp[2] = '\0';
		szTSReaderSection += 10;	// skip to the hex
		nCount = lstrlen(szTSReaderSection) - 1;
		for (i = 0; i < nCount; i++)
		{
			int nValue;

			szTemp[0] = *(szTSReaderSection++);
			szTemp[1] = *(szTSReaderSection++);
			sscanf(szTemp, "%02x", &nValue);
			*pData++ = nValue;
		}
		*nEPGScheduleItems = *nEPGScheduleItems + 1;
		if (*nEPGScheduleItems == *nEPGScheduleMax)
		{
			MessageBox(NULL, "Too many scheduled events - please tell rod@coolstf.com you saw this", "TSReader Scheduler", MB_ICONSTOP);
		}
	}

	// Release the ITask interface.
	pITask->Release();
	if (FAILED(hr))
		return FALSE;

	CoTaskMemFree(ppwszComment);
	return TRUE;
}

BOOL Scheduler_EnumTasks(PEPGSCHEDULE pepgschedule, int * nEPGScheduleItems, int * nEPGScheduleMax, char * szSourceName)
{
	HRESULT hr = S_OK;
	ITaskScheduler *pITS;

	// CoCreateInstance to get the Task Scheduler object. 
	hr = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **) &pITS);
	if (FAILED(hr))
		return FALSE;

	// Call ITaskScheduler::Enum to get an enumeration object.
	IEnumWorkItems *pIEnum;
	hr = pITS->Enum(&pIEnum);
	pITS->Release();
	if (FAILED(hr))
		return FALSE;

	// Call IEnumWorkItems::Next to retrieve tasks. Note that 
	// this example tries to retrieve five tasks for each call.
	LPWSTR *lpwszNames;
	DWORD dwFetchedTasks = 0;
	while (SUCCEEDED(pIEnum->Next(TASKS_TO_RETRIEVE, &lpwszNames, &dwFetchedTasks)) && (dwFetchedTasks != 0))
	{
		// Process each task. Note that this example prints the 
		// name of each task to the screen.
		while (dwFetchedTasks)
		{
			ReadTask(lpwszNames[--dwFetchedTasks], pepgschedule, nEPGScheduleItems, nEPGScheduleMax, szSourceName);
			CoTaskMemFree(lpwszNames[dwFetchedTasks]);
		}
		CoTaskMemFree(lpwszNames);
	}

	pIEnum->Release();
	return TRUE;
}

#define SCHED_CLASS             TEXT("SAGEWINDOWCLASS")
#define SCHED_TITLE             TEXT("SYSTEM AGENT COM WINDOW")
#define SCHED_SERVICE_APP_NAME  TEXT("mstask.exe")
#define SCHED_SERVICE_NAME      TEXT("Schedule")

BOOL Scheduler_EnsureServiceRunning()
{
	OSVERSIONINFO osver;
	osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// Determine which version of OS you are running.
	GetVersionEx(&osver);

	if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		// If Windows 95, check to see if Windows 95 
		// version of Task Scheduler is running.
		HWND hwnd = FindWindow(SCHED_CLASS, SCHED_TITLE);

		if (hwnd != NULL)
		{
			// It is already running.
			return TRUE;
		}

		//  Execute the task scheduler process.
		STARTUPINFO         sui;
		PROCESS_INFORMATION pi;
		ZeroMemory(&sui, sizeof(sui));
		sui.cb = sizeof (STARTUPINFO);
		TCHAR szApp[MAX_PATH];
		LPTSTR pszPath;

		DWORD dwRet = SearchPath(NULL,	SCHED_SERVICE_APP_NAME,	NULL, MAX_PATH, szApp, &pszPath);
		if (dwRet == 0)
			return FALSE;

		BOOL fRet = CreateProcess(szApp, NULL, NULL, NULL, FALSE, CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP, NULL, NULL, &sui, &pi);
		if (fRet == 0)
			return FALSE;

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
	}
	else
	{
		// If not Windows 95, check to see if Windows NT 
		// version of Task Scheduler is running.
		SC_HANDLE   hSC = NULL;
		SC_HANDLE   hSchSvc = NULL;

		hSC = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
		if (hSC == NULL)
			return FALSE;

		hSchSvc = OpenService(hSC, SCHED_SERVICE_NAME, 	SERVICE_START | SERVICE_QUERY_STATUS);
		CloseServiceHandle(hSC);
		if (hSchSvc == NULL)
			return FALSE;

		SERVICE_STATUS SvcStatus;
		if (QueryServiceStatus(hSchSvc, &SvcStatus) == FALSE)
		{
			CloseServiceHandle(hSchSvc);
			return FALSE;
		}
		if (SvcStatus.dwCurrentState == SERVICE_RUNNING)
		{
			// The service is already running.
			CloseServiceHandle(hSchSvc);
			return TRUE;
		}

		if (StartService(hSchSvc, 0, NULL) == FALSE)
		{
			CloseServiceHandle(hSchSvc);
			return FALSE;
		}

		CloseServiceHandle(hSchSvc);
		return TRUE;
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
    case DLL_PROCESS_ATTACH:
		CoInitialize(NULL);
		break;
    case DLL_PROCESS_DETACH:
		CoUninitialize();
		break;
    }
    return TRUE;
}

	

