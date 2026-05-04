/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_2pass.cpp
 *
 *  2-Pass driver for Built-In analyzer
 ********************************************************/

#include "skl.h"
#include "./skl_mpg_2pass.h"

// #define MONITOR_PASS2

//////////////////////////////////////////////////////////

SKL_MPG_2PASS::SKL_MPG_2PASS(const int File_Type)
  : _Type(File_Type)
  , _Pass_Nb(0)
  , _Frame_Cnt(0)
  , _Pass_File_Name("pass1.log")
  , _Pass_File(0)
  , _Format_Version(1.0f)
{
  _Missed_Bytes = 0;
  _Desired_Bytes = 0;
}

SKL_MPG_2PASS::~SKL_MPG_2PASS()
{
  if (_Pass_File!=0) {
    fclose(_Pass_File);
    _Pass_File = 0;
  }
}

int SKL_MPG_2PASS::Set_Pass_File_Name(SKL_CST_STRING const Name)
{
  if (_Pass_File!=0) return 0;
  _Pass_File_Name = Name;
  return 1;
}

int SKL_MPG_2PASS::Open_Read_Pass_File(const int Width, const int Height)
{
  SKL_ASSERT(_Pass_Nb>0);
  if (_Pass_File!=0) return 1;

  _Pass_File = fopen( _Pass_File_Name, "r" );
  if (_Pass_File==0) {
    return 0;
  }
  int W, H, n;
  char Buf[256];
  for(n=0; n<255; ++n) {
    int c = fgetc(_Pass_File);
    Buf[n] = c;
    if (c==EOF || c=='\n') break;
  }
  Buf[n] = 0;
  if (sscanf( Buf, "## TMP4 v%f %d x %d", &_Format_Version, &W, &H )==3)
    _Type = 0;
  else if (sscanf( Buf, "## TH264 v%f %d x %d", &_Format_Version, &W, &H )==3)
    _Type = 1;
  else
    return 0;

  if (W!=Width || H!=Height)
    return 0;
  return 1;
}

int SKL_MPG_2PASS::Open_Write_Pass_File(const int Width, const int Height)
{
  SKL_ASSERT(_Pass_Nb>0);
  if (_Pass_File!=0) return 1;

  _Pass_File = fopen( _Pass_File_Name, "w" );
  if (_Pass_File==0) {
    return 0;
  }
  fprintf( _Pass_File, "## %s v%1.1f %d x %d\n",
    (_Type==0) ? "TMP4" : "TH264",
    _Format_Version, Width, Height );
  return 1;
}

int SKL_MPG_2PASS::Save_Frame_Params(const SKL_FRAME_STATS * const Stat)
{
  SKL_ASSERT(Stat!=0);

  fprintf( _Pass_File, "%d %d %.2f %.2f %.2f %.2f %d %d %d\n",
    Stat->Frame_Order, Stat->Pic_Type, 
    Stat->Q, Stat->FCode, 
    Stat->mMV, Stat->dMV, 
    Stat->Coded_Bytes, Stat->Txt_Bytes, Stat->MV_Bytes );
  _Frame_Cnt++;

  return 1;
}

int SKL_MPG_2PASS::Get_Next_Frame_Params(SKL_FRAME_STATS * const Stat)
{
  SKL_ASSERT(Stat!=0);

  if (fscanf(_Pass_File, "%d %d %f %f %f %f %d %d %d", 
    &Stat->Frame_Order, &Stat->Pic_Type, &Stat->Q, &Stat->FCode, 
    &Stat->mMV, &Stat->dMV, 
    &Stat->Coded_Bytes, &Stat->Txt_Bytes, &Stat->MV_Bytes)==9)
  {
    _Desired_Bytes = Stat->Coded_Bytes;
    return 1;
  }
  else return 0;
}


int SKL_MPG_2PASS::Compute_New_Frame_Params(SKL_FRAME_STATS * const Stat)
{
  SKL_ASSERT(Stat!=0);

  _Missed_Bytes += Stat->Coded_Bytes-_Desired_Bytes;
  float Missed = 1.0f * _Missed_Bytes / Stat->Coded_Bytes;
  float dQ;
  if (_Type==0) {
    dQ = 4.f * Missed*_Pass_RF;
    if      (dQ> 2.5f) dQ = 2.5f;
    else if (dQ<-2.5f) dQ =-2.5f;
    Stat->Q += dQ;
    if      (Stat->Q>31.49f) Stat->Q = 31.49f;
    else if (Stat->Q< 0.50f) Stat->Q =  0.50f;
  }
  else {
    dQ = 2.f * Missed*_Pass_RF;
    if      (dQ> 2.5f) dQ = 2.5f;
    else if (dQ<-2.5f) dQ =-2.5f;
    Stat->Q += dQ;
    if      (Stat->Q>51.49f) Stat->Q = 51.49f;
    else if (Stat->Q< 0.40f) Stat->Q = 0.40f;
  }

#ifdef MONITOR_PASS2
  static int Total1 = 0, Total2 = 0;
  Total1 += Stat->Coded_Bytes;
  Total2 += _Desired_Bytes;
  printf( "%d %d %d %d %.2f\n", Stat->Frame_Order, Total1, Total2, _Missed_Bytes, Stat->Q );
#endif

  _Frame_Cnt++;
  return 1;
}

//////////////////////////////////////////////////////////
