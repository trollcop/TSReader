/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * Bits I/O tests
 *
 ********************************************************/

#include "skl_tester.h"
#include "skl_syst/skl_bits.h"
#include "skl_syst/skl_random.h"
#include "skl_syst/skl_ptimer.h"

//////////////////////////////////////////////////////////

TEST_FUNC(Test_FBB1)
{
  const int MAX=24; // 24bits max

  SKL_PTIMER Timer;
  int i, v, n;
  SKL_FBB Fbb;
  SKL_SAFE_INT Total;
  int Crc, Crc0;

  Total = 0;
  Crc0 = 0;
  for(i=1; i<MAX; ++i)
    for(v=0; v<(1<<i); v++) {
      Total += i;
      Crc0 = ((Crc0+v) ^ (Crc0>>3))&0xffff;
    }
  CHECKUI( Crc0, 0xe1c4 );

  size_t Size = (Total+7)>>3; // ~47Megs!!
  SKL_BYTE *Buf = (SKL_BYTE*)SKL_MEM->New(Size);

  Timer.Reset();
  for(n=0; n<3; ++n) {
    Fbb.Init(Buf);
    for(i=1; i<MAX; ++i) {
      const int vmax = 1<<i;
      for(v=0; v<vmax; v++)
        Fbb.Put_Bits(v, i); 
    }
  }
  Timer.Stop();
  printf( "Put Bits:%1f ms\n", Timer.Get_mSec() );
  CHECKUI( Fbb.Safe_Write_Bit_Pos(Buf), Total );
  Fbb.Flush_Write();

  Fbb.Init(Buf);
  for(i=1; i<MAX; ++i) {
    for(v=0; v<(1<<i); v++) {
      const SKL_UINT32 v0 = Fbb.Get_Bits(i);
      CHECKUI(v0,v);
    }
  }

  Timer.Reset();
  for(n=0; n<3; ++n) {
    Crc = 0;
    Fbb.Init(Buf);
    for(i=1; i<MAX; ++i) {
      const int vmax = 1<<i;
      for(v=0; v<vmax; v++) {
        const SKL_UINT32 v0 = Fbb.Get_Bits(i);
        Crc = ((Crc+v0) ^ (Crc>>3))&0xffff;
      }
    }
  }
  Timer.Stop();
  printf( "Get Bits:%1f ms\n", Timer.Get_mSec() );
  CHECKUI( Fbb.Bit_Pos()-8*(SKL_SAFE_INT)Buf, Total );
  CHECKUI( Crc, Crc0 );

  Timer.Reset();
  for(n=0; n<3; ++n) {
    Crc = 0;
    Fbb.Init(Buf);
    for(i=1; i<MAX; ++i) {
      const int vmax = 1<<i;
      for(v=0; v<vmax; v++) {
        Fbb.Check_Bits(i);
        const SKL_UINT32 v0 = Fbb.See_Bits_Safe(i);
        Crc = ((Crc+v0) ^ (Crc>>3))&0xffff;
        Fbb.Discard_Safe(i);
      }
    }
  }
  Timer.Stop();
  printf( "See+Get Bits:%1f ms\n", Timer.Get_mSec() );
  CHECKUI( Fbb.Bit_Pos()-8*(SKL_SAFE_INT)Buf, Total );
  CHECKUI( Crc, Crc0 );

  SKL_MEM->Delete(Buf, Size);
}
END_FUNC

//////////////////////////////////////////////////////////

TEST_FUNC(Test_Golomb)
{
  const int MAX=24; // 24bits max
  const int CNT = 2;
  SKL_PTIMER Timer;
  int i, v, n;
  SKL_FBB Fbb;
  SKL_SAFE_INT Total;
  int Crc;
  size_t Size;
  SKL_BYTE *Buf;

  Total = 0;
  for(i=0; i<MAX; ++i) {
    const int vmin = (1<<i)-1;
    const int vmax = (1<<(i+1))-1;
    for(v=vmin; v<vmax; v++) {
      Total += 2*i+1;
//      printf("v=%d, len=%d Total:%d\n", v, i, Total );
    }
  }
  Size = (Total+7)>>3;
  Buf = (SKL_BYTE*)SKL_MEM->New(Size);
  printf( "Test UEG: mem size=%d\n", Size );

  Timer.Reset();
  for(n=0; n<CNT; ++n) {
    Fbb.Init(Buf);
    for(i=0; i<MAX; ++i) {
      const int vmin = (1<<i)-1;
      const int vmax = (1<<(i+1))-1;
      for(v=vmin; v<vmax; v++) {
        Fbb.Put_UEG(v); 
      }
    }
  }
  CHECKUI( Fbb.Safe_Write_Bit_Pos(Buf), Total );
  Fbb.Flush_Write();
  Timer.Stop();
  printf( "Put UEG Bits:%1f ms\n", Timer.Get_mSec() );


  Fbb.Init(Buf);
  for(i=0; i<MAX; ++i) {
    const int vmin = (1<<i)-1;
    const int vmax = (1<<(i+1))-1;
    for(v=vmin; v<vmax; v++) {
      const SKL_UINT32 v0 = Fbb.UEG();
      CHECKUI(v0,v);
    }
  }

  Timer.Reset();
  for(n=0; n<CNT; ++n) {
    Crc = 0;
    Fbb.Init(Buf);
    for(i=0; i<MAX; ++i) {
      const int vmin = (1<<i)-1;
      const int vmax = (1<<(i+1))-1;
      for(v=vmin; v<vmax; v++)
        Crc += Fbb.UEG();
    }
  }
  Timer.Stop();
  printf( "Get UEG Bits:%1f ms\n", Timer.Get_mSec() );
  CHECKUI( Fbb.Bit_Pos()-8*(SKL_SAFE_INT)Buf, Total );

  SKL_MEM->Delete(Buf, Size);



  Total = 0;
  for(i=0; i<MAX; ++i) {
    const int vmin = (1<<i)>>1;
    const int vmax = (1<<i);
    for(v=vmin; v<vmax; v++)
      Total += 2*(2*i+1);
  }
  Size = (Total+7)>>3;
  Buf = (SKL_BYTE*)SKL_MEM->New(Size);
  printf( "\n" );
  printf( "Test SEG: mem size=%d\n", Size );

  Timer.Reset();
  for(n=0; n<CNT; ++n) {
    Fbb.Init(Buf);
    for(i=0; i<MAX; ++i) {
      const int vmin = (1<<i)>>1;
      const int vmax = (1<<i);
      for(v=vmin; v<vmax; v++) {
        Fbb.Put_SEG(v); 
        Fbb.Put_SEG(-v);
      }
    }
  }
  CHECKUI( Fbb.Safe_Write_Bit_Pos(Buf), Total );
  Fbb.Flush_Write();
  Timer.Stop();
  printf( "Put SEG Bits:%1f ms\n", Timer.Get_mSec() );

  Fbb.Init(Buf);
  for(i=0; i<MAX; ++i) {
    const int vmin = (1<<i)>>1;
    const int vmax = (1<<i);
    for(v=vmin; v<vmax; v++) {
      const SKL_INT32 vp = Fbb.SEG();
      const SKL_INT32 vm = Fbb.SEG();
      CHECKI(vp, v);
      CHECKI(vm,-v);
    }
  }

  Timer.Reset();
  for(n=0; n<CNT; ++n) {
    Crc = 0;
    Fbb.Init(Buf);
    for(i=0; i<MAX; ++i) {
      const int vmin = (1<<i)>>1;
      const int vmax = (1<<i);
      for(v=vmin; v<vmax; v++) {
        Crc += Fbb.SEG();
        Crc += Fbb.SEG();
      }
    }
  }
  Timer.Stop();
  printf( "Get SEG Bits:%1f ms\n", Timer.Get_mSec() );
  CHECKUI( Fbb.Bit_Pos()-8*(SKL_SAFE_INT)Buf, Total );

  SKL_MEM->Delete(Buf, Size);
}
END_FUNC

//////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  SKL_T_START;

    SKL_TEST(1, Test_FBB1);
    SKL_TEST(2, Test_Golomb);

  SKL_T_END;
  SKL_T_RETURN;
}

//////////////////////////////////////////////////////////
