/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_system.cpp
 *
 *  various low-level funcs
 ********************************************************/
 
#include "skl.h"
#include "skl_syst/skl_event.h"
#include "skl_syst/skl_exception.h"
#include "skl_syst/skl_destroy.h"
#include "skl_syst/skl_driver.h"

#include <stdio.h>

//////////////////////////////////////////////////////////
// EXCEPTIONS
//////////////////////////////////////////////////////////

char SKL_EXCEPTION::_Msg[MAX_LEN];
char SKL_ASSERT_EXCEPTION::_Text[MAX_LEN];
char SKL_ASSERT_EXCEPTION::_File[MAX_LEN];
size_t SKL_MEM_EXCEPTION::_Size         = 0;
int SKL_ASSERT_EXCEPTION::_Line         = 0;

void Skl_Throw( const SKL_EXCEPTION &e ) { 
  throw e;
}

  // ctors + print...

SKL_EXCEPTION::SKL_EXCEPTION(SKL_CST_STRING s)
{
  if (s) strncpy(_Msg, s, MAX_LEN);
  else _Msg[0] = 0;
}

void SKL_EXCEPTION::Print() const { 
  fprintf( stdout, _Msg );
  fflush( stdout );
}

//////////////////////////////////////////////////////////

SKL_MSG_EXCEPTION::SKL_MSG_EXCEPTION(SKL_CST_STRING s, ...)
  : SKL_EXCEPTION()
{
  if (s) {
    va_list Args;
    va_start( Args, s );
    vsprintf(_Msg, s, Args);  // TODO: Dangerous!
    va_end(Args);
  }
}

//////////////////////////////////////////////////////////

SKL_MEM_EXCEPTION::SKL_MEM_EXCEPTION(SKL_CST_STRING Msg, int Size)
  : SKL_EXCEPTION(Msg) { 
  _Size = Size;
}

void SKL_MEM_EXCEPTION::Print() const {
  printf("Malloc failure ");
  if (_Size) printf("[%d bytes]", _Size);
  printf("\n");
  SKL_EXCEPTION::Print();
}

//////////////////////////////////////////////////////////

SKL_ASSERT_EXCEPTION::SKL_ASSERT_EXCEPTION(SKL_CST_STRING Msg,
                                           SKL_CST_STRING Text,
                                           SKL_CST_STRING File,
                                           int Line)
  : SKL_EXCEPTION(Msg)
{ 
  if (Text) strncpy(_Text, Text, MAX_LEN);
  else _Text[0] = 0;
  if (File) strncpy(_File, File, MAX_LEN);
  else _File[0] = 0;
  _Line = Line;
}

void SKL_ASSERT_EXCEPTION::Print() const {
  fprintf(stdout, "Assertion '%s' failed\n", _Text);
  fprintf(stdout, " -> File '%s', line %d.\n", _File, _Line);
  SKL_EXCEPTION::Print();
}

//////////////////////////////////////////////////////////
// ASSERT
//////////////////////////////////////////////////////////

void Skl_Do_Assert(SKL_CST_STRING Condition, 
                   SKL_CST_STRING File, 
                   int Line, 
                   SKL_CST_STRING Msg)
{
  Skl_Throw( SKL_ASSERT_EXCEPTION(Msg, Condition, File, Line) );
}

//////////////////////////////////////////////////////////
// SKL_DRIVER_I
//////////////////////////////////////////////////////////

SKL_DRIVER_I::SKL_DRIVER_I(SKL_CST_STRING Name)
  : _Name(Name)
  , _Ok(0)
{}

SKL_DRIVER_I::~SKL_DRIVER_I() {}
          
void SKL_DRIVER_I::Wake_Up() {
  printf( "Waking up driver '%s'\n", Get_Name() );
}

void SKL_DRIVER_I::Stand_By() {
  printf( "Driver '%s' is now sleeping\n", Get_Name() );
}
void SKL_DRIVER_I::Print_Infos() const {
  printf( "Driver: %s (%s).\n",
    Get_Name(), Ok() ? "Initialized" : "Uninitialized" );
  
}

//////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////

#ifdef __UNIX__

extern "C" {

#include <signal.h>
// #include <stdlib.h>

#ifdef __IRIX__   /* grrr!!!! Sometimes, i'm tired of Unices :)*/
#define Signal_Func_Signature ssignal
static int User_Abort( int Sig )
#else
#define Signal_Func_Signature signal
static void User_Abort( int Sig )
#endif
{
  printf( "Exiting upon signal ");
  if (Sig==SIGINT) printf( "SIGINT\n" );
  else if (Sig==SIGKILL) printf( "SIGKILL\n" );
  else if (Sig==SIGSEGV) printf( "SIGSEGV\n" );
//  else if (Sig==SIGABRT) printf( "SIGABRT\n" );
  else printf( "%d\n", Sig );
  SKL_DESTROYABLE::Kill_All();
  exit(0);  // bye bye

#ifdef __IRIX__
  return 0; /* ...really really tired... :) */
#endif
}

static void Skl_External_System_Init() {
  Signal_Func_Signature( SIGINT, User_Abort );
  Signal_Func_Signature( SIGKILL, User_Abort );
  Signal_Func_Signature( SIGSEGV, User_Abort );
//  Signal_Func_Signature( SIGABRT, User_Abort );
}
}
#endif  /* __UNIX__ */

//////////////////////////////////////////////////////////
// SKL_DESTROYABLE
//////////////////////////////////////////////////////////

// TODO: all this is not MT-safe. Far from...

extern "C" {
  // we put it as 'extern "C"' because that's
  // the only way to make sure they are initialized
  // to zero *in the binary* (not at run-time)
static SKL_DESTROYABLE *To_Kill = 0;
static int Init = 0;
}

void SKL_DESTROYABLE::Add_Me() const {
  SKL_DESTROYABLE* Myself = (SKL_DESTROYABLE*)this;
  Myself->_Next_Destroyable = To_Kill;
  To_Kill = Myself;
}

void SKL_DESTROYABLE::Remove_Me() {
    // TODO: this search can be slow...
  SKL_DESTROYABLE **Cur = &To_Kill;
  while((*Cur)!=this) Cur=&(*Cur)->_Next_Destroyable;
  *Cur = _Next_Destroyable;
  _Next_Destroyable = 0;
}

SKL_DESTROYABLE::SKL_DESTROYABLE() {
  if (Init==0) {
#ifdef __UNIX__
    Skl_External_System_Init();
#endif
    Init = 1;
  }
  Add_Me(); 
}

void SKL_DESTROYABLE::Suicide() { /* does nothing */ 
//  this->~SKL_DESTROYABLE();
}

SKL_DESTROYABLE::~SKL_DESTROYABLE() {
  if (Init!=0)
    Remove_Me();  // not guaranted to succeed (if coming from Kill_All())
}

void SKL_DESTROYABLE::Kill_All() {
  int n = 0;
  Init = 0; // marker for Final Destruction
  while(To_Kill!=0) {
    SKL_DESTROYABLE* Kill = To_Kill;
    To_Kill = To_Kill->_Next_Destroyable;
    Kill->Suicide();
    n++;
  }
}

//////////////////////////////////////////////////////////
