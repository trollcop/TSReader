/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_c.cpp
 *
 * Plain-C MPEG4 interface
 ********************************************************/

#include "./skl_mpg4i.h"
#include "skl_syst/skl_exception.h"

#define SKL_C_API_ONLY
#include "skl_mpg4_c.h"

extern "C" {

//////////////////////////////////////////////////////////
// C-interface for SKL_MP4_DEC (decoder class)
//////////////////////////////////////////////////////////

int Skl_MP4_Dec_Decode(SKL_MP4_DEC * const Dec, const char *Buf, int Len) {
  try { return Dec->Decode((const SKL_BYTE*)Buf, Len); } catch(...) { return 0; }
}

int Skl_MP4_Dec_Decode_MPEG12(SKL_MP4_DEC * const Dec, const char *Buf, int Len) {
  try { return Dec->Decode_MPEG12((const SKL_BYTE*)Buf, Len); } catch(...) { return 0; }
}

int Skl_MP4_Dec_Get_Frame_Number(const SKL_MP4_DEC * const Dec) {
  try { return Dec->Get_Frame_Number(); } catch(...) { return 0; }
}

int Skl_MP4_Dec_Is_Frame_Ready(const SKL_MP4_DEC * const Dec) {
  try { return Dec->Is_Frame_Ready(); } catch(...) { return 0; }
}

void Skl_MP4_Dec_Consume_Frame(SKL_MP4_DEC * const Dec, SKL_MP4_PIC *Pic) {
  try { Dec->Consume_Frame(Pic); } catch(...) { }
}

void Skl_MP4_Dec_Set_CPU(SKL_MP4_DEC * const Dec, SKL_CPU_FEATURE Cpu) {
  try { Dec->Set_CPU(Cpu); } catch(...) { }
}

void Skl_MP4_Dec_Set_Slicer(SKL_MP4_DEC * const Dec, SKL_MP4_SLICER Slicer) {
  try { Dec->Set_Slicer(Slicer); } catch(...) {}
}

//////////////////////////////////////////////////////////
//  C-interface for SKL_MP4_ENC (encoder class)
//////////////////////////////////////////////////////////

const SKL_MP4_PIC *Skl_MP4_Enc_Prepare_Next_Frame(SKL_MP4_ENC * const Enc, int Width, int Height) {
  try { return Enc->Prepare_Next_Frame(Width, Height); } catch(...) { return 0; }
}

const SKL_MP4_PIC *Skl_MP4_Enc_Get_Next_Frame(const SKL_MP4_ENC * const Enc) {
  try { return Enc->Get_Next_Frame(); } catch(...) { return 0; }
}

const SKL_MP4_PIC *Skl_MP4_Enc_Get_Last_Coded_Frame(const SKL_MP4_ENC * const Enc) {
  try { return Enc->Get_Last_Coded_Frame(); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Encode(SKL_MP4_ENC * const Enc) {
  try { return Enc->Encode(); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Finish_Encoding(SKL_MP4_ENC * const Enc) {
  try { return Enc->Finish_Encoding(); } catch(...) { return 0; }
}

const char *Skl_MP4_Enc_Get_Bits(const SKL_MP4_ENC * const Enc) {
  try { return (const char*)Enc->Get_Bits(); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Get_Bits_Length(const SKL_MP4_ENC * const Enc) {
  try { return Enc->Get_Bits_Length(); } catch(...) { return 0; }
}

void Skl_MP4_Enc_Set_CPU(SKL_MP4_ENC * const Enc, SKL_CPU_FEATURE Cpu) {
  try { Enc->Set_CPU(Cpu); } catch(...) { }
}

void Skl_MP4_Enc_Set_Custom_Matrix(SKL_MP4_ENC * const Enc, int Intra, const char *M) {
  try { Enc->Set_Custom_Matrix(Intra, (const SKL_BYTE*)M); } catch(...) {}
}

void Skl_MP4_Enc_Set_Slicer(SKL_MP4_ENC * const Enc, SKL_MP4_SLICER Slicer, void *Slicer_Data) {
  try { Enc->Set_Slicer(Slicer, Slicer_Data); } catch(...) {}
}

void Skl_MP4_Enc_Get_All_Frames(const SKL_MP4_ENC * const Enc, SKL_MP4_PIC *Pic) {
  try { Enc->Get_All_Frames(Pic); } catch(...) {}
}

int Skl_MP4_Enc_Ioctl(SKL_MP4_ENC * const Enc, const char * const Param) {
  try { return Enc->Ioctl(Param); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Set_Analyzer_Param_I(const SKL_MP4_ENC * const Enc, const char * const Param, int Value) {
  try { return Enc->Get_Analyzer()->Set_Param(Param, Value); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Set_Analyzer_Param_F(const SKL_MP4_ENC * const Enc, const char * const Param, float Value) {
  try { return Enc->Get_Analyzer()->Set_Param(Param, Value); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Get_Analyzer_Param_I(const SKL_MP4_ENC * const Enc, const char * const Param, int *Value) {
  try { return Enc->Get_Analyzer()->Get_Param(Param, Value); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Get_Analyzer_Param_F(const SKL_MP4_ENC * const Enc, const char * const Param, float *Value) {
  try { return Enc->Get_Analyzer()->Get_Param(Param, Value); } catch(...) { return 0; }
}

const int *Skl_MP4_Enc_Get_Analyzer_Param_P(const SKL_MP4_ENC * const Enc, const char * const Param) {
  try { return Enc->Get_Analyzer()->Get_Param(Param); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Get_Analyzer_Param_S(const SKL_MP4_ENC * const Enc, const char * const Param, const char ** const Value) {
  try { return Enc->Get_Analyzer()->Get_Param(Param, Value); } catch(...) { return 0; }
}

int Skl_MP4_Enc_Set_Analyzer_Param_S(const SKL_MP4_ENC * const Enc, const char * const Param, const char * const Value) {
  try { return Enc->Get_Analyzer()->Set_Param(Param, Value); } catch(...) { return 0; }
}

//////////////////////////////////////////////////////////

}   /* extern "C" */
