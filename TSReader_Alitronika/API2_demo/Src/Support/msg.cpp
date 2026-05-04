/**
  * @file		:	msg.cpp
  *
  * Purpose		:	Reporting functions
  *
  * (c) Copyright Engineering Spirit NL 2003. All rights reserved.
  *
  * By			:	Henk van de Berg
  *
  * Revision list:
  *
  * Date		Changes
  * 2006-01-24	Initial release
  */

/**********************************************************************************************************/
// includes
#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <unistd.h>
#endif

#include "msg.h"

/**********************************************************************************************************/
// static variables

static char * ProgramName;		//<	char *, Pointer to the program name
static bool ShowName = true;	//<	bool, Indicates if the name should be printed with program output

/**********************************************************************************************************/
// functions

/** Set the program name. When ShowName == true, the program name
  * should be set before Message() or Error() is called.
  * @param name char *, Name of the program
  */
void SetProgramName (char * name)
{
	ProgramName = name;
	return;
}

/** Set ShowName on or off
  * Indicate whether the program name should be showed on every line of
  * program output. This is useful to indicate where a message comes from.
  */
void ShowProgramName (bool Show)
{
	ShowName = Show;
	return;
}

/** Write message to stdout
  * Write a message to standard output, prefixed with the program name
  * when ShowName is true.
  * @param msg char *, The message to be displayed.
  */
void Message (char * msg)
{
	if (ShowName)
		printf ("%s: %s\n", ProgramName, msg);
	else
		printf ("%s\n",msg);
	
	return;
}

/** Write message to stdout, then exit with exit code 1.
  * @param msg char *, The message to write to stdout
  */
void Error (char * msg)
{
	Message (msg);
	_exit (1);
}

const char * GetProgramName()
{
	return ProgramName;
}