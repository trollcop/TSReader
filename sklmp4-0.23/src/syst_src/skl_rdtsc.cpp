/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_rdtsc.cpp
 *
 * time stamp (INTEL only)
 ********************************************************/

#include "skl.h"

#ifdef SKL_USE_ASM

#include "skl_syst/skl_rdtsc.h"
#include "skl_syst/skl_exception.h"
#include <stdio.h>

extern "C" {

//////////////////////////////////////////////////////////

SKL_INT32 Skl_RCount_[16];
SKL_INT32 Skl_Cur_Count_;
SKL_INT32 Skl_Tics_;
SKL_INT32 Skl_EAX_In_;
SKL_INT32 Skl_EBX_In_;  
SKL_INT32 Skl_ECX_In_;
SKL_INT32 Skl_EDX_In_;
SKL_INT32 Skl_EDI_In_;
SKL_INT32 Skl_ESI_In_;
SKL_INT32 Skl_EBP_In_;
SKL_INT32 Skl_ESP_In_;
SKL_INT32 Skl_EAX_Out_;
SKL_INT32 Skl_EBX_Out_;
SKL_INT32 Skl_ECX_Out_;
SKL_INT32 Skl_EDX_Out_;
SKL_INT32 Skl_EDI_Out_;
SKL_INT32 Skl_ESI_Out_;
SKL_INT32 Skl_EBP_Out_;
SKL_INT32 Skl_ESP_Out_;
float Skl_f_In_[8];
float Skl_f_Out_[8];  

//////////////////////////////////////////////////////////

void Skl_Print_Tics(SKL_INT32 Offset)
{
  int i;
  fprintf( stderr, "Counters:\n" );
  for( i=15; i>=0; --i )
    fprintf( stderr, "[%d]", Skl_RCount_[i]-Offset );
  fprintf( stderr, "\n" );

  fprintf( stderr, "///////////////////IN////////////////\n" );
  fprintf( stderr, "EAX:0x%.8x   EBX:0x%.8x   ECX:0x%.8x   EDX:0x%.8x\n",
    Skl_EAX_In_, Skl_EBX_In_, Skl_ECX_In_, Skl_EDX_In_ );
  fprintf( stderr, "ESI:0x%.8x   EDI:0x%.8x   EBP:0x%.8x   ESP:0x%.8x\n",
    Skl_ESI_In_, Skl_EDI_In_, Skl_EBP_In_, Skl_ESP_In_ );
  fprintf( stderr, "st0:%.3e  st1:%.3e  st2:%.3e  st3:%.3e\n", 
    Skl_f_In_[0], Skl_f_In_[1], Skl_f_In_[2], Skl_f_In_[3] );
  fprintf( stderr, "st4:%.3e  st5:%.3e  st6:%.3e  st7:%.3e\n", 
    Skl_f_In_[4], Skl_f_In_[5], Skl_f_In_[6], Skl_f_In_[7] );

  fprintf( stderr, "//////////////////OUT////////////////\n" );
  fprintf( stderr, "EAX:0x%.8x   EBX:0x%.8x   ECX:0x%.8x   EDX:0x%.8x\n",
    Skl_EAX_Out_, Skl_EBX_Out_, Skl_ECX_Out_, Skl_EDX_Out_ );
  fprintf( stderr, "ESI:0x%.8x   EDI:0x%.8x   EBP:0x%.8x   ESP:0x%.8x\n",
    Skl_ESI_Out_, Skl_EDI_Out_, Skl_EBP_Out_, Skl_ESP_Out_ );
  fprintf( stderr, "st0:%.3e  st1:%.3e  st2:%.3e  st3:%.3e\n", 
    Skl_f_Out_[0], Skl_f_Out_[1], Skl_f_Out_[2], Skl_f_Out_[3] );
  fprintf( stderr, "st4:%.3e  st5:%.3e  st6:%.3e  st7:%.3e\n", 
    Skl_f_Out_[4], Skl_f_Out_[5], Skl_f_Out_[6], Skl_f_Out_[7] );
  fprintf( stderr, "/////////////////////////////////////\n" );

  Skl_Throw( SKL_MSG_EXCEPTION( "RDTSC timing finished." ) );
}

} // extern "C"

//////////////////////////////////////////////////////////

#endif      /* SKL_USE_ASM */
