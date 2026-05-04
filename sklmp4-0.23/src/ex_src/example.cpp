/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * example.cpp
 *
 *  Simple API example of encoding/decoding loop.
 *  (won't work, of course).
 ********************************************************/

#include "skl_syst/skl_mpg4.h"

void Encoding_Loop(FILE *Out_File)
{
  SKL_MP4_ENC *Enc = Skl_MP4_New_Encoder();
  Enc->Get_Analyzer()->Set_Param( "my-param", 0);
  Enc->Get_Analyzer()->Set_Param( "my-param", 0 /*param value*/);

  while(1) {
    const SKL_MP4_PIC *Pic = Enc->Prepare_Next_Frame(320, 200);

    /* here, fill in Pic->Y, Pic->U, Pic->V with pixels */

    if (Enc->Encode())
      fwrite( Enc->Get_Bits(), Enc->Get_Bits_Length(), 1, Out_File );
  }
  Skl_MP4_Delete_Encoder(Enc);
}

void Decoding_Loop(const SKL_BYTE *Buffer, int Buffer_Len)
{
  SKL_MP4_DEC *Dec = Skl_MP4_New_Decoder();
  SKL_MP4_PIC Decoded_Frame;
  while(1) {
    int Bytes_Consumed = Dec->Decode(Buffer, Buffer_Len);

    /* advance/replenish Buffer here.. */

    if (Dec->Is_Frame_Ready())
      Dec->Consume_Frame( &Decoded_Frame);
  }
  Skl_MP4_Delete_Decoder(Dec);
}


int main() { return 0; }    // keep linker happy
