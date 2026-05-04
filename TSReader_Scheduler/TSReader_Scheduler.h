#include <windows.h>
#include "..\epgschedule.h"

// Definitions for TSReader_Scheduler.dll

typedef BOOL (* tdScheduler_EnsureServiceRunning) ();
BOOL (*Scheduler_EnsureServiceRunning) ();

typedef BOOL (*tdScheduler_CreateNewTask) (char * szProgram, char * szParameters, char * szTaskName, 
							 char * szComment, char * szUsername, char * szPassword, 
							 SYSTEMTIME * stRunDate, int nDuration, BOOL fRunNow, BOOL fRequiredLogin, BOOL fPowerResume);
BOOL (*Scheduler_CreateNewTask)(char * szProgram, char * szParameters, char * szTaskName, 
							 char * szComment, char * szUsername, char * szPassword, 
							 SYSTEMTIME * stRunDate, int nDuration, BOOL fRunNow, BOOL fRequiredLogin, BOOL fPowerResume);

typedef BOOL (* tdScheduler_EnumTasks) (PEPGSCHEDULE pepgschedule, int * nEPGScheduleItems, int * nEPGScheduleMax, char * szSourceName);
BOOL (*Scheduler_EnumTasks)(PEPGSCHEDULE pepgschedule, int * nEPGScheduleItems, int * nEPGScheduleMax, char * szSourceName);

typedef BOOL (*tdSchedule_DeleteSchedule) (char * szTaskName);
BOOL (*Schedule_DeleteSchedule) (char * szTaskName);
