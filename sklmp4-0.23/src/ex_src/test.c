/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * test.c
 *
 *  Simple Plain-C API example of encoding/decoding loop.
 *  (won't work, of course).
 *
 * Can be compiled using, e.g.:
 *   g++3 -o test test.c -lm libskl_syst.a
 ********************************************************/

#include "skl_mpg4_c.h"
#include <stdio.h>

void Encoding_Loop(FILE *Out_File)
{
  SKL_MP4_ENC *Enc;

  Enc = Skl_MP4_New_Encoder();
  Skl_MP4_Enc_Set_Analyzer_Param_I(Enc, "my-param", 0);
  Skl_MP4_Enc_Set_Analyzer_Param_F(Enc, "my-param", 0.f); /* param value */

  while(1) {
    const SKL_MP4_PIC *Pic = Skl_MP4_Enc_Prepare_Next_Frame(Enc, 320, 200);

    /* here, fill in Pic->Y, Pic->U, Pic->V with pixels */

    if (Skl_MP4_Enc_Encode(Enc))
      fwrite( Skl_MP4_Enc_Get_Bits(Enc), Skl_MP4_Enc_Get_Bits_Length(Enc), 1, Out_File );
  }
  Skl_MP4_Delete_Encoder(Enc);
}

void Decoding_Loop(const char *Buffer, int Buffer_Len)
{
  SKL_MP4_DEC *Dec;
  SKL_MP4_PIC Decoded_Frame;

  Dec = Skl_MP4_New_Decoder();
  while(1) {
    int Bytes_Consumed = Skl_MP4_Dec_Decode(Dec, Buffer, Buffer_Len);

    /* advance/replenish Buffer here.. */

    if (Skl_MP4_Dec_Is_Frame_Ready(Dec))
      Skl_MP4_Dec_Consume_Frame( Dec, &Decoded_Frame );
  }
  Skl_MP4_Delete_Decoder(Dec);
}

int main() { return 0; }    // keep linker happy
