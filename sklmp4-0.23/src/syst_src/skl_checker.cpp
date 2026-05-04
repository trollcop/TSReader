/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_checker.cpp
 *
 * Managment and report of check errors
 ********************************************************/

// TODO: use snprintf(), but some plateform (Alpha e.g.) don't
//       have it...

#include "skl.h"
#include "skl_syst/skl_checker.h"

int Skl_Nb_Check=0, Skl_Nb_Error=0, Skl_Nb_Flt_Error=0;
int Skl_Show_Flt_Error=1;

//////////////////////////////////////////////////////////

void Skl_Check_Print()
{
  printf( "Checks: %d\nErrors: %d\n", Skl_Nb_Check, Skl_Nb_Error );
  if (Skl_Nb_Flt_Error) printf( "Float Errors: %d\n", Skl_Nb_Flt_Error);
}

//////////////////////////////////////////////////////////
// factorized reporting funcs (hence, not inlined)

void Skl_PError_Bool(int Line, const char *What)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: Check failed for '%s'\n", Line, What);
  printf( Buf );
  Skl_Nb_Error++;
}
void Skl_PError_Str(int Line, SKL_CST_STRING v1, const char *What1, SKL_CST_STRING v2, const char *What2)
{
  char Buf[4096];
  sprintf( Buf, "ERROR: Line %d: strings %s [%s] and %s [%s] differ\n",
    Line, What1, v1, What2, v2 );
  printf( Buf );
  Skl_Nb_Error++;
}

void Skl_PError_Ptr(int Line, SKL_ANY v1, const char *What1, SKL_ANY v2, const char *What2)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: pointers %s=0x%x and %s=0x%x differ\n",
    Line, What1, (SKL_SAFE_INT)v1, What2, (SKL_SAFE_INT)v2 );
  printf( Buf );
  Skl_Nb_Error++;
}
void Skl_PError_Int(int Line, int v1, const char *What1, int v2, const char *What2)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: int32 %s=%d and %s=%d differ\n", Line, What1, v1, What2, v2 );
  printf( Buf );
  Skl_Nb_Error++;
}
void Skl_PError_UInt(int Line, SKL_UINT32 v1, const char *What1, SKL_UINT32 v2, const char *What2)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: uint32 %s=0x%x and %s=0x%x differ\n", Line, What1, v1, What2, v2 );
  printf( Buf );
  Skl_Nb_Error++;
}
void Skl_PError_Eq(int Line, const char *What1, const char *What2)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: objects %s and %s not equal\n", Line, What1, What2 );
  printf( Buf );
  Skl_Nb_Error++;
}
void Skl_PError_Float(int Line, const char *What1, float v1, const char *What2, float v2, float delta, float eps)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: floats %s=%f and %s=%f differ (delta=%f,eps=%f)\n", Line, What1, v1, What2, v2, delta, eps);
  if (Skl_Show_Flt_Error) printf( Buf );
  Skl_Nb_Flt_Error++;
}
void Skl_PError_Int_Eps(int Line, const char *What1, int v1, const char *What2, int v2, int delta, int eps)
{
  char Buf[2048];
  sprintf( Buf, "ERROR: Line %d: int %s=%d and %s=%d differ (delta=%d,eps=%d)\n", Line, What1, v1, What2, v2, delta, eps);
  if (Skl_Show_Flt_Error) printf( Buf );
  Skl_Nb_Flt_Error++;
}

//////////////////////////////////////////////////////////
