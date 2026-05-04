/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * test_dsp.cpp
 *
 *    DSP tests I
 *
 ********************************************************/

#include "skl_tester.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_random.h"
#include "skl_syst/skl_ptimer.h"

extern "C" double fabs(double);

//////////////////////////////////////////////////////////

SKL_CPU_FEATURE Cpu_List[] = {
  SKL_CPU_C,
  SKL_CPU_X86,
  SKL_CPU_MMX,
  SKL_CPU_SSE,
  SKL_CPU_REF,
  SKL_CPU_LAST
};

#define PRINT_NxN(S,I,J,BPS)      \
  { for(j=0; j<J; ++j) {          \
     for(i=0; i<I; ++i)  printf( "%3d ", (int)(S)[i+j*BPS] ); \
     printf( "\n" ); } printf( "-----\n" ); }

//////////////////////////////////////////////////////////

TEST_FUNC(Test_DCT_Ref)
{
  SHOW_FLT_ERROR_ON;
  const int MAX=3;
  const int Sizes[MAX] = { 6, 12, 36 };
  const double Crcs[MAX] = { -3.664200, -2849.800781, 615.346741 };
  SKL_RANDOM Rnd(1324);

  for(int k=0; k<MAX; ++k)
  {
    int N = Sizes[k];
    float In[36], Out[2*36];

    int i;
    for(i=0; i<N; i++)
      In[i] = 1.0f*( Rnd.Get_Float(256.0f) );

    Skl_Generic_IDct_Ref( N, In, Out );

    double Crc = 0.0;
    for(i=0; i<2*N; i++) {
      Crc += Out[i];
//      printf( "#%d Out=%f\n", i, Out[i] );
    }
    CHECKFEPS( Crc, Crcs[k], 1.0e-6f );
  }
}
END_FUNC

//////////////////////////////////////////////////////////

#define TEST1_PROLOG(CNT, OFF)                      \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Img_DSP( &Dsp, *Cpu );                 \
    tm.Reset();                                     \
    for(n=0; n<CNT; ++n)                            \
      for(Crc=0, j=0; j<WIDTH-(OFF); ++j)           \
        for(i=0; i<WIDTH-(OFF); ++i) {              \
          SKL_BYTE *Src2=Src+i+j*WIDTH; (void)Src2; \
          Crc += Dsp.

#define TEST1_EPILOG(CRC0, CALL)                    \
        }                                           \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    Crc /= i*j;                                     \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL); \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST_IMG_2(CNT, CRC0, D,S, CALL)            \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Img_DSP( &Dsp, *Cpu );                 \
    tm.Reset();                                     \
    for(j=0; j<18; ++j) for(i=0; i<18; ++i)         \
      Src[i+j*WIDTH] = ((i^j)*7-5)&255;             \
    for(i=0; i<8*8; ++i) Dst[i]=255;                \
    for(n=0; n<CNT*10000; ++n) {                    \
      Dsp.CALL((D),8,(S)+1+WIDTH,WIDTH);            \
      /*if (n==0) PRINT_NxN((D), 8,8,8);*/          \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<8; ++j) for(i=0; i<8; ++i)    \
      Crc += (D)[i+j*8]^i;                          \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

TEST_FUNC(Test_Img_DSP)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_MMX,
    SKL_CPU_SSE,
    SKL_CPU_LAST
  };
  SKL_IMG_DSP Dsp;

  int i, j, n;
  const int CNT1 = 25, CNT2 = 120, CNT3 = 200;
  const int WIDTH = 128;
  const int SIZE = 32;
  SKL_BYTE Dst0[8*8+31];
  SKL_BYTE Src0[WIDTH*WIDTH+31];
  SKL_BYTE *Dst = (SKL_BYTE*)(((SKL_SAFE_INT)Dst0 + 31)&~31);
  SKL_BYTE *Src = (SKL_BYTE*)(((SKL_SAFE_INT)Src0 + 31)&~31);

  SKL_UINT32 Crc;
  for(i=0; i<WIDTH*WIDTH; ++i) Src[i] = i&255;
  SKL_PTIMER tm;
  SKL_CPU_FEATURE *Cpu;

  printf("== SAD  16x16 ==\n");
  TEST1_PROLOG(CNT2, 16) SAD_16x16(Src,Src2,WIDTH); TEST1_EPILOG(23488, SAD_16x16);
  printf("== SAD  16x8  ==\n");
  TEST1_PROLOG(CNT2, 16) SAD_16x8_Field(Src,Src2,WIDTH); TEST1_EPILOG(15296, SAD_16x16);
  printf("== SAD  8 x 8 ==\n");
  TEST1_PROLOG(CNT3, 8) SAD_8x8(Src,Src2,WIDTH); TEST1_EPILOG(6000, SAD_8x8);
  printf("== SAD  16x 7 ==\n");
  TEST1_PROLOG(CNT3, 8) SAD_16x7_Self(Src,WIDTH); TEST1_EPILOG(14336, SAD_16x7_Self);
  printf("== SAD  4x4 ==\n");
  TEST1_PROLOG(CNT2, 4) SAD_4x4(Src,Src2,WIDTH); TEST1_EPILOG(1516, SAD_4x4);

  printf("== SSD  16x16 ==\n");
  TEST1_PROLOG(CNT2, 16) SSD_16x16(Src,Src2,WIDTH)>>8; TEST1_EPILOG(12317, SSD_16x16);
  printf("== SSD  16x8  ==\n");
  TEST1_PROLOG(CNT2, 16) SSD_16x8_Field(Src,Src2,WIDTH)>>8; TEST1_EPILOG(9710, SSD_16x16);
  printf("== SSD  8 x 8 ==\n");
  TEST1_PROLOG(CNT3, 8) SSD_8x8(Src,Src2,WIDTH)>>8; TEST1_EPILOG(3232, SSD_8x8);
  printf("== SSD  4x4 ==\n");
  TEST1_PROLOG(CNT2, 4) SSD_4x4(Src,Src2,WIDTH)>>8; TEST1_EPILOG(828, SSD_4x4);

  printf("== Mean 16x16 ==\n");
  TEST1_PROLOG(CNT2, 16) Mean_16x16(Src,WIDTH); TEST1_EPILOG(71, Mean_16x16);
  printf("== Mean 8 x 8 ==\n");
  TEST1_PROLOG(CNT3, 8) Mean_8x8(Src,WIDTH); TEST1_EPILOG(67, Mean_8x8);
  printf("== Mean 4x4 ==\n");
  TEST1_PROLOG(CNT2, 4) Mean_4x4(Src,WIDTH); TEST1_EPILOG(65, Mean_4x4);

  printf("== Sqr. 16x16 ==\n");
  TEST1_PROLOG(CNT2, 16) Sqr_16x16(Src2,WIDTH); TEST1_EPILOG(21291, Sqr_16x16);
  printf("== Sqr. 8 x 8 ==\n");
  TEST1_PROLOG(CNT3, 8) Sqr_8x8(Src2,WIDTH); TEST1_EPILOG(21429, Sqr_8x8);
  printf("== Sqr. 4x4 ==\n");
  TEST1_PROLOG(CNT2, 4) Sqr_4x4(Src2,WIDTH); TEST1_EPILOG(21507, Sqr_4x4);

  printf("== SAD  W x H ==\n");
  TEST1_PROLOG(CNT1, SIZE) SAD(Src,Src2,SIZE,SIZE,WIDTH); TEST1_EPILOG(89856, SAD);
  printf("== Mean W x H ==\n");
  TEST1_PROLOG(CNT1, SIZE) Mean(Src,SIZE,SIZE,WIDTH); TEST1_EPILOG(79, Mean);

  printf("==  Sqr. Dev  ==\n");
  TEST1_PROLOG(CNT1, SIZE) Square_Dev(Src2,SIZE,SIZE,WIDTH); TEST1_EPILOG(4307, Square_Dev);
  printf("==  Abs. Dev  ==\n");
  TEST1_PROLOG(CNT1, SIZE) Abs_Dev(Src2,SIZE,SIZE,WIDTH); TEST1_EPILOG(65536, Abs_Dev);

  printf("==  Smooth  ==\n");
  TEST_IMG_2(CNT1, 4968, Dst, Src, Smooth_18x18_To_8x8 );
  printf("==  Gradx   ==\n");
  TEST_IMG_2(CNT1, 268, (SKL_INT8*)Dst, Src, Gradx_18x18_To_8x8 );
  printf("==  Grady   ==\n");
  TEST_IMG_2(CNT1, 364, (SKL_INT8*)Dst, Src, Grady_18x18_To_8x8 );
  printf("==  Grad2   ==\n");
  TEST_IMG_2(CNT1, 8490, Dst, Src, Grad2_18x18_To_8x8 );
}
END_FUNC

//////////////////////////////////////////////////////////

#define TEST2(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT; ++n) {                          \
      Rnd.Set_Seed(WIDTH);                          \
      for(i=0; i<16*WIDTH; ++i)                     \
        Dst[i] = (SKL_BYTE)Rnd.Get_Int(256);        \
      for(j=0; j<WIDTH-16-1; ++j)                   \
        for(i=0; i<WIDTH-16-1; ++i) {               \
          SKL_BYTE *Src2=Src+i+j*WIDTH;             \
          Dsp.CALL(Dst,Src2,WIDTH);                 \
        }                                           \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<8; ++j) for(i=0; i<16; ++i)   \
      Crc += Dst[i+j*WIDTH];                        \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST3(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT*10; ++n) {                       \
      for(i=0; i<16*WIDTH; ++i) Dst[i] = i;         \
      for(i=0; i<5; ++i) {                          \
          SKL_INT16 *Src2=Src6+i*64;                \
          Dsp.CALL(Dst,Src2,WIDTH);                 \
        }                                           \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<8; ++j) for(i=0; i<16; ++i)   \
      Crc += Dst[i+j*WIDTH]^i^j;                    \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST4(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT; ++n) {                          \
      for(i=0; i<2*64; ++i) Dst16[i] = i;           \
      for(j=0; j<WIDTH-16-1; ++j)                   \
        for(i=0; i<WIDTH-16-1; ++i) {               \
          SKL_BYTE *Src2=Src+i+j*WIDTH;             \
          Dsp.CALL(Dst16,Src2,WIDTH);               \
        }                                           \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<2*64; ++j) Crc += Dst16[j];   \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST5(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT; ++n) {                          \
      for(j=0; j<WIDTH-16-1; ++j) {                 \
        for(i=0; i<2*64; ++i) Dst16[i] = 20000-i;   \
        for(i=0; i<WIDTH-16-1; ++i) {               \
          SKL_BYTE *Src2=Src+i+j*WIDTH;             \
          Dsp.CALL(Dst16,Src2,WIDTH);               \
        }                                           \
      }                                             \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<2*64; ++j) Crc += Dst16[j];   \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST6(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT; ++n) {                          \
      for(j=0; j<WIDTH-16-1; ++j) {                 \
        for(i=0; i<2*64; ++i) Dst16[i] = 20000-i;   \
        for(i=0; i<WIDTH-16-1; ++i) {               \
          SKL_BYTE *Src2=Src+i+j*WIDTH;             \
          Dsp.CALL(Dst16, Src, Src2,WIDTH);         \
        }                                           \
      }                                             \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<2*64; ++j) Crc += Dst16[j];   \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST71(S, CNT, CALL, CRC0)                  \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(i=0; i<4*64; ++i) Src6[i] = i;              \
    Crc = Dsp.CALL(Src6);                           \
    for(n=0; n<700*CNT; ++n) Dsp.CALL(Src6);        \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(j=0; j<4*64; ++j) Crc += (Src6[j]^j);       \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }
#define TEST72(S, CNT, CALL, CRC0)                  \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<700*CNT; ++n) {                      \
      for(i=0; i<4*64; ++i) Src6[i] = i;            \
      Dsp.CALL(Src6);                               \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0,j=0; j<4*64; ++j) Crc += (Src6[j]^j); \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define PRINT16x16(S)                               \
{ for(j=0; j<16; ++j) {                             \
    for(i=0; i<16; ++i)  printf( "%3d ", (int)(S)[i+j*WIDTH] ); \
    printf( "\n" ); } printf("---\n"); }

#define TEST81(S, CNT, CALL, CRC0, SHFT)            \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT*10; ++n) {                       \
      for(i=0; i<16*WIDTH; ++i) {                   \
        Dst[i] = 128;                               \
        Src6[i] = (SHFT)-(i&0xff);                  \
      }                                             \
      Dsp.CALL(Dst,Src6,WIDTH);                     \
      /*if (n==0) { PRINT16x16(Dst); PRINT16x16(Src6); }*/ \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<16; ++j) for(i=0; i<16; ++i)  \
      Crc += Dst[i+j*WIDTH]^i^j;                    \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    /*PRINT16x16(Dst);*/                            \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST82(S, CNT, CALL, CRC0)                  \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT*10; ++n) {                       \
      for(i=0; i<16*WIDTH; ++i) Dst[i] = (i*37+23)&0xff; \
      for(i=0; i<16; i+=2)                          \
       Dsp.HFilter_31(Dst+i*WIDTH,Dst+(i+1)*WIDTH,WIDTH/8); \
      for(i=0; i<WIDTH; i+=2)                       \
        Dsp.VFilter_31(Dst+i,Dst+1+i,WIDTH,2);      \
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, j=0; j<16; ++j) for(i=0; i<16; ++i)  \
      Crc += Dst[i+j*WIDTH]^i^j;                    \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n",      \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    /*PRINT16x16(Dst);*/                            \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST9(S, CNT, CALL, CRC0)                   \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    tm.Reset();                                     \
    for(n=0; n<CNT*200; ++n) {                      \
      for(i=0; i<64; ++i) Dst16[i] = ((i*73+31)&0x3ff)-512; \
      Dsp.CALL(Dst16, Src+1+WIDTH, WIDTH);          \
      /*if (n==0) { PRINT_NxN(Src,18,18,WIDTH);PRINT_NxN(Dst16,8,8,8); }*/\
    }                                               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    for(Crc=0, i=0; i<64; ++i) Crc += Dst16[i]^i;   \
    printf( "%s:\t%.3f s \tCrc: %d \tFunc:%p\n", \
      Dsp.Name, tm.Get_Sec(), Crc, Dsp.CALL);       \
    CHECKI( Crc, CRC0 );                            \
  }

#define TEST10(S, CNT, CALL, SAD0, SAD1, SAD2)      \
  printf("== %s ==\n", S);                    \
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {  \
    Skl_Init_Mb_DSP( &Dsp, *Cpu );                  \
    SKL_UINT32 Sad[3];                              \
    for(i=0; i<16*WIDTH; ++i) Dst[i] = (i*37+23)&0xff; \
    tm.Reset();                                     \
    for(n=0; n<CNT*150; ++n)                        \
      Dsp.CALL(Dst, Src, WIDTH, Sad);               \
    Dsp.Switch_Off();                               \
    tm.Stop();                                      \
    printf( "%s: \t%.3f s    Sad:H:0x%x V:0x%x  HV:0x%x   Func:%p\n", \
      Dsp.Name, tm.Get_Sec(), Sad[0], Sad[1], Sad[2], Dsp.CALL); \
    CHECKUI( Sad[0], SAD0 ); \
    CHECKUI( Sad[1], SAD1 ); \
    CHECKUI( Sad[2], SAD2 ); \
  }


TEST_FUNC(Test_Mb_DSP)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_X86,
    SKL_CPU_MMX,
    SKL_CPU_SSE,
    SKL_CPU_LAST
  };
  SKL_MB_DSP Dsp;

  SKL_RANDOM Rnd;
  int i, j, n;
  const int CNT1 = 150;
  const int WIDTH = 128;
  SKL_BYTE Src[WIDTH*WIDTH], Dst[16*WIDTH];
  SKL_INT16 Src16[WIDTH*WIDTH];
  SKL_INT16 Src6[6*64], Dst16[2*64];
  SKL_UINT32 Crc;
  SKL_PTIMER tm;
  SKL_CPU_FEATURE *Cpu;
  for(i=0; i<WIDTH*WIDTH; ++i) Src[i] = (SKL_BYTE)Rnd.Get_Int(256);
  for(i=0; i<WIDTH*WIDTH; ++i) Src16[i] = (SKL_INT16)Rnd.Get_Int(256);
  for(i=0; i<6*64; ++i) Src6[i] = (SKL_INT16)(Rnd.Get_Int(512)-128);

  TEST2("Add-FF-16      ", CNT1, Add->HP_16x8[0], 16796);
  TEST2("Add-FH-16 Rnd0 ", CNT1, Add->HP_16x8[1], 16836);
  TEST2("Add-HF-16 Rnd0 ", CNT1, Add->HP_16x8[2], 16720);
  TEST2("Add-HH-16 Rnd0 ", CNT1, Add->HP_16x8[3], 16697);
  TEST2("Add-FF-8       ", CNT1, Add->HP_8x8 [0], 16164);
  TEST2("Add-FH-8 Rnd0  ", CNT1, Add->HP_8x8 [1], 16289);
  TEST2("Add-HF-8 Rnd0  ", CNT1, Add->HP_8x8 [2], 16183);
  TEST2("Add-HH-8 Rnd0  ", CNT1, Add->HP_8x8 [3], 16268);
  TEST2("Add-FF-4       ", CNT1, Add->HP_8x4 [0], 15760);
  TEST2("Add-FH-4 Rnd0  ", CNT1, Add->HP_8x4 [1], 15715);
  TEST2("Add-HF-4 Rnd0  ", CNT1, Add->HP_8x4 [2], 15797);
  TEST2("Add-HH-4 Rnd0  ", CNT1, Add->HP_8x4 [3], 15765);

  TEST2("Copy-FF-16      ", CNT1, Copy[1]->HP_16x8[0], 16726);
  TEST2("Copy-FF-16      ", CNT1, Copy[0]->HP_16x8[0], 16726);
  TEST2("Copy-FH-16 Rnd1 ", CNT1, Copy[1]->HP_16x8[1], 16709);
  TEST2("Copy-FH-16 Rnd0 ", CNT1, Copy[0]->HP_16x8[1], 16780);
  TEST2("Copy-HF-16 Rnd1 ", CNT1, Copy[1]->HP_16x8[2], 16583);
  TEST2("Copy-HF-16 Rnd0 ", CNT1, Copy[0]->HP_16x8[2], 16653);
  TEST2("Copy-HH-16 Rnd1 ", CNT1, Copy[1]->HP_16x8[3], 16607);
  TEST2("Copy-HH-16 Rnd0 ", CNT1, Copy[0]->HP_16x8[3], 16643);
  TEST2("Copy-FF-8       ", CNT1, Copy[1]->HP_8x8 [0], 16199);
  TEST2("Copy-FF-8       ", CNT1, Copy[0]->HP_8x8 [0], 16199);
  TEST2("Copy-FH-8 Rnd1  ", CNT1, Copy[1]->HP_8x8 [1], 16381);
  TEST2("Copy-FH-8 Rnd0  ", CNT1, Copy[0]->HP_8x8 [1], 16412);
  TEST2("Copy-HF-8 Rnd1  ", CNT1, Copy[1]->HP_8x8 [2], 16174);
  TEST2("Copy-HF-8 Rnd0  ", CNT1, Copy[0]->HP_8x8 [2], 16214);
  TEST2("Copy-HH-8 Rnd1  ", CNT1, Copy[1]->HP_8x8 [3], 16351);
  TEST2("Copy-HH-8 Rnd0  ", CNT1, Copy[0]->HP_8x8 [3], 16370);
  TEST2("Copy-FF-4       ", CNT1, Copy[1]->HP_8x4 [0], 15714);
  TEST2("Copy-FF-4       ", CNT1, Copy[0]->HP_8x4 [0], 15714);
  TEST2("Copy-FH-4 Rnd1  ", CNT1, Copy[1]->HP_8x4 [1], 15631);
  TEST2("Copy-FH-4 Rnd0  ", CNT1, Copy[0]->HP_8x4 [1], 15643);
  TEST2("Copy-HF-4 Rnd1  ", CNT1, Copy[1]->HP_8x4 [2], 15772);
  TEST2("Copy-HF-4 Rnd0  ", CNT1, Copy[0]->HP_8x4 [2], 15791);
  TEST2("Copy-HH-4 Rnd1  ", CNT1, Copy[1]->HP_8x4 [3], 15723);
  TEST2("Copy-HH-4 Rnd0  ", CNT1, Copy[0]->HP_8x4 [3], 15733);

  TEST4("Copy-16x8_8To16 ", CNT1, Copy_16x8_8To16, 16726);
  TEST4("Copy-8x8_8To16  ", CNT1, Copy_8x8_8To16, 14670);

  TEST5("Diff-16x8_8To16 ", CNT1, Diff_16x8_8To16, 770213);
  TEST5("Diff-8x8_8To16  ", CNT1, Diff_8x8_8To16, 1659376);
  TEST6("Diff-16x8_88To16", CNT1, Diff_16x8_88To16, -1011);
  TEST6("Diff-8x8_88To16 ", CNT1, Diff_8x8_88To16, 1273630);

  TEST71("SAD-16x7-Frame ", CNT1, SAD_16x7_Frame, 1792);
  TEST71("SAD-16x7-Field ", CNT1, SAD_16x7_Field, 5632);
  TEST72("Reorder-Frame  ", CNT1, Reorder_Frame_16x16, 23552);

  TEST81("Copy-Up8x8_16To8", CNT1*10, Copy_Upsampled_8x8_16To8, 57216, 255);
  TEST81("Add-Up8x8_16To8 ", CNT1*10, Add_Upsampled_8x8_16To8, 24897, 0);

  TEST82("H/V Filter_31   ", CNT1*10, HFilter_31, 31872);
  TEST9("DownFilt_31      ", CNT1*10, Filter_18x18_To_8x8, 8326);
  TEST9("DownFilt_Diff_31 ", CNT1*10, Filter_Diff_18x18_To_8x8, -10774);

  TEST10("SAD-16-HP-Rnd0 ", CNT1* 4, Copy[0]->SAD_HP_16x16, 0x4b2e, 0x4a64, 0x46d6);
  TEST10("SAD-16-HP-Rnd1 ", CNT1* 4, Copy[1]->SAD_HP_16x16, 0x4b18, 0x4a58, 0x46d7);
  TEST10("SAD- 8-HP-Rnd0 ", CNT1*16, Copy[0]->SAD_HP_8x8  , 0x1497, 0x12a8, 0x1118);
  TEST10("SAD- 8-HP-Rnd1 ", CNT1*16, Copy[1]->SAD_HP_8x8  , 0x148e, 0x12a3, 0x111a);

}
END_FUNC

////////////////////////////////////////////////////////// 
// Quarter-pixel test

static
void QP_16x16(SKL_BYTE *Dst, const SKL_BYTE *Src,
              int Quads, SKL_BYTE *YTmp,
              int BpS, const SKL_MB_FUNCS * const Ops)
{
  switch(Quads) {
    case 0:
      Ops->HP_16x8[0](Dst, Src, BpS);
      Ops->HP_16x8[0](Dst+8*BpS, Src+8*BpS, BpS);
    break;
    case 1:
      Ops->H_Pass_Avrg(Dst, Src, 16, BpS);
    break;
    case 2:
      Ops->H_Pass(Dst, Src, 16, BpS);
    break;
    case 3:
      Ops->H_Pass_Avrg_Up(Dst, Src, 16, BpS);
    break;
    case 4:
      Ops->V_Pass_Avrg(Dst, Src, 16, BpS);
    break;
    case 5:
      Ops->H_Pass_Avrg(YTmp, Src, 17, BpS);
      Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);
    break;
    case 6:
      Ops->H_Pass(YTmp, Src,   17, BpS);
      Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);
    break;
    case 7:
      Ops->H_Pass_Avrg_Up(YTmp, Src, 17, BpS);
      Ops->V_Pass_Avrg(Dst, YTmp, 16, BpS);
    break;
    case 8:
      Ops->V_Pass(Dst, Src, 16, BpS);
    break;
    case 9:
      Ops->H_Pass_Avrg(YTmp, Src, 17, BpS);
      Ops->V_Pass(Dst, YTmp, 16, BpS);
    break;
    case 10:
      Ops->H_Pass(     YTmp, Src, 17, BpS);
      Ops->V_Pass(Dst, YTmp, 16, BpS);
    break;
    case 11:
      Ops->H_Pass_Avrg_Up(YTmp, Src, 17, BpS);
      Ops->V_Pass(Dst, YTmp, 16, BpS);
    break;
    case 12:
      Ops->V_Pass_Avrg_Up(Dst, Src, 16, BpS);
    break;
    case 13:
      Ops->H_Pass_Avrg(YTmp, Src, 17, BpS);
      Ops->V_Pass_Avrg_Up(Dst, YTmp, 16, BpS);
    break;
    case 14:
      Ops->H_Pass(YTmp, Src,17, BpS);
      Ops->V_Pass_Avrg_Up( Dst, YTmp, 16, BpS);
    break;
    case 15:
      Ops->H_Pass_Avrg_Up(YTmp, Src, 17, BpS);
      Ops->V_Pass_Avrg_Up(Dst, YTmp, 16, BpS);
    break;
  }
}

static
void QP_8x8(SKL_BYTE *Dst, const SKL_BYTE *Src,
            int Quads, SKL_BYTE *YTmp,
            int BpS, const SKL_MB_FUNCS * const Ops)
{

  switch(Quads) {
    case 0:
      Ops->HP_8x8[0](Dst, Src, BpS);
    break;
    case 1:
      Ops->H_Pass_Avrg_8(Dst, Src, 8, BpS);
    break;
    case 2:
      Ops->H_Pass_8(Dst, Src, 8, BpS);
    break;
    case 3:
      Ops->H_Pass_Avrg_Up_8(Dst, Src, 8, BpS);
    break;
    case 4:
      Ops->V_Pass_Avrg_8(Dst, Src, 8, BpS);
    break;
    case 5:
      Ops->H_Pass_Avrg_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);
    break;
    case 6:
      Ops->H_Pass_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);
    break;
    case 7:
      Ops->H_Pass_Avrg_Up_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_8(Dst, YTmp, 8, BpS);
    break;
    case 8:
      Ops->V_Pass_8(Dst, Src, 8, BpS);
    break;
    case 9:
      Ops->H_Pass_Avrg_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_8(Dst, YTmp, 8, BpS);
    break;
    case 10:
      Ops->H_Pass_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_8(Dst, YTmp, 8, BpS);
    break;
    case 11:
      Ops->H_Pass_Avrg_Up_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_8(Dst, YTmp, 8, BpS);
    break;
    case 12:
      Ops->V_Pass_Avrg_Up_8(Dst, Src, 8, BpS);
    break;
    case 13:
      Ops->H_Pass_Avrg_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 8, BpS);
    break;
    case 14:
      Ops->H_Pass_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_Up_8( Dst, YTmp, 8, BpS);
    break;
    case 15:
      Ops->H_Pass_Avrg_Up_8(YTmp, Src, 9, BpS);
      Ops->V_Pass_Avrg_Up_8(Dst, YTmp, 8, BpS);
    break;
  }
}

#if 0
#define PRINT(S)                      \
  if (q==0) {                         \
    printf( " == Src ==\n" );         \
    for(j=0; j<=(S); ++j)             \
      { for(i=0; i<=(S); ++i) printf( "%3d ", Src[i+j*WIDTH] ); printf( "\n" ); } \
  } \
  printf( " == Dst(%d) ==\n", q );  \
  for(j=0; j<=(S); ++j)             \
      { for(i=0; i<=(S); ++i) printf( "%3d ", Dst[i+j*WIDTH] ); printf( "\n" ); }
#else
#define PRINT(S)
#endif

TEST_FUNC(Test_Mb_QP_DSP)
{
//#define DONT_DO_16x16
//#define DONT_DO_8x8
//#define DONT_DO_COPY
//#define DONT_DO_ADD

  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_MMX,
    SKL_CPU_REF,
    SKL_CPU_LAST
  };
  SKL_MB_DSP Dsp;

  SKL_RANDOM Rnd;
  int i, j, n;
  const int CNT = 1000 * 100;
  const int WIDTH = 64;
  SKL_BYTE Src[17*WIDTH], Dst[17*WIDTH];
  SKL_BYTE Tmp[16*17];
  SKL_UINT32 Crc;
  SKL_PTIMER tm;

  for(i=0; i<17*WIDTH; ++i) Src[i] = (SKL_BYTE)Rnd.Get_Int(256);

  static int Crcs[2][16][4] = {   // [rnd=0/1][q=0..15][copy/add - 16x16/8x8]
    {
      {598249, 598172, 593475, 598728}, {596752, 598425, 593556, 598704},
      {597568, 599293, 594172, 598697}, {598796, 598076, 594210, 598099},
      {598668, 598868, 593975, 599170}, {592954, 599059, 592947, 598786},
      {595466, 598931, 592671, 599157}, {594275, 598930, 593278, 598851},
      
      {598323, 597485, 593909, 598789}, {593878, 598707, 592971, 598567},
      {594254, 598445, 593021, 599224}, {594198, 598567, 592933, 598858},
      {597465, 596799, 593393, 598755}, {593883, 598291, 592874, 599033},
      {594904, 598322, 593010, 599033}, {593220, 599312, 592937, 598731}
    },
    {
      {598249, -1, 593475, -1}, {596718, -1, 593523, -1},
      {597565, -1, 594171, -1}, {598540, -1, 594201, -1},
      {598620, -1, 593953, -1}, {593800, -1, 593132, -1},
      {595183, -1, 592621, -1}, {594211, -1, 593187, -1},

      {598325, -1, 593909, -1}, {593806, -1, 593091, -1},
      {594260, -1, 593014, -1}, {594137, -1, 592899, -1},
      {597898, -1, 593349, -1}, {593804, -1, 592890, -1},
      {594704, -1, 592963, -1}, {593122, -1, 592884, -1}
    }
  };

  for(int Rounding=0; Rounding<=1; ++Rounding) {
    for(int q=0; q<=15; ++q) {
      for(SKL_CPU_FEATURE *Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) 
      {
        Skl_Init_Mb_DSP( &Dsp, *Cpu );
#ifndef DONT_DO_16x16
#ifndef DONT_DO_COPY
        tm.Reset();
        for(n=0; n<CNT; ++n) {
          SKL_BZERO(Dst, sizeof(Dst));
          QP_16x16(Dst, Src, q, Tmp, WIDTH, Dsp.Copy[Rounding]);
        }
        Dsp.Switch_Off();
        tm.Stop();
        for(Crc=0,j=0; j<17*WIDTH; ++j) Crc += (Dst[j]^j);
        printf( "%s: copy 16x16 quads=%d Rounding=%d \t%.3f usec/call \tCrc: %d\n",
          Dsp.Name, q, Rounding, tm.Get_Sec() * 1.e6f/n, Crc );
        CHECKI(Crc, Crcs[Rounding][q][0]);
        PRINT(16)
#endif
#ifndef DONT_DO_ADD
        if (Rounding==0) {
          tm.Reset();
          for(n=0; n<CNT; ++n) {
            for(j=0; j<17*WIDTH; ++j) Dst[j] = (j*53+22)&0xff;
            QP_16x16(Dst, Src, q, Tmp, WIDTH, Dsp.Add);
          }
          Dsp.Switch_Off();
          tm.Stop();
          for(Crc=0,j=0; j<17*WIDTH; ++j) Crc += (Dst[j]^j);
          printf( "%s: add  16x16 quads=%d Rounding=%d \t%.3f usec/call \tCrc: %d\n",
            Dsp.Name, q, Rounding, tm.Get_Sec() * 1.e6f/n, Crc );
          CHECKI(Crc, Crcs[Rounding][q][1]);
          PRINT(16)
        }
#endif
#endif

#ifndef DONT_DO_8x8
#ifndef DONT_DO_COPY
        tm.Reset();
        for(n=0; n<CNT; ++n) {
          SKL_BZERO(Dst, sizeof(Dst));
          QP_8x8(Dst, Src, q, Tmp, WIDTH, Dsp.Copy[Rounding]);
        }
        Dsp.Switch_Off();
        tm.Stop();
        for(Crc=0,j=0; j<17*WIDTH; ++j) Crc += (Dst[j]^j);
        printf( "%s: copy 8x8   quads=%d Rounding=%d \t%.3f usec/call \tCrc: %d\n",
          Dsp.Name, q, Rounding, tm.Get_Sec() * 1.e6f/n, Crc );
        CHECKI(Crc, Crcs[Rounding][q][2]);
        PRINT(8)
#endif
#ifndef DONT_DO_ADD
        if (Rounding==0) {
          tm.Reset();
          for(n=0; n<CNT; ++n) {
            for(j=0; j<17*WIDTH; ++j) Dst[j] = (j*53+22)&0xff;
            QP_8x8(Dst, Src, q, Tmp, WIDTH, Dsp.Add);
          }
          Dsp.Switch_Off();
          tm.Stop();
          for(Crc=0,j=0; j<17*WIDTH; ++j) Crc += (Dst[j]^j);
          printf( "%s: add  8x8   quads=%d Rounding=%d \t%.3f usec/call \tCrc: %d\n",
            Dsp.Name, q, Rounding, tm.Get_Sec() * 1.e6f/n, Crc );
          CHECKI(Crc, Crcs[Rounding][q][3]);
          PRINT(8)
        }
#endif
#endif
      }
    }
  }
}
END_FUNC

#undef PRINT

//////////////////////////////////////////////////////////

TEST_FUNC(Test_GMC_DSP)
{
  SKL_GMC_DSP Dsp;
  SKL_CPU_FEATURE Cpu_List[] = {
//    SKL_CPU_C,
    SKL_CPU_REF,
    SKL_CPU_LAST
  };

  SKL_RANDOM Rnd;
  int i, j, n;
  const int CNT = 1000 * 50;
  const int WIDTH = 64;
  SKL_BYTE Src[(WIDTH+8+8)*WIDTH], Dst[WIDTH*WIDTH] = {0};
  SKL_UINT32 Crc;
  int Pts[4][2];
  SKL_PTIMER tm;
  for(i=0; i<(int)sizeof(Src); ++i) Src[i] = 0x57;
  for(j=0; j<WIDTH; j++)
    for(i=0; i<WIDTH; i++)
      Src[i+j*WIDTH] = Rnd.Get_Int(256);
  SKL_BYTE *S = Src + 8 +8*WIDTH;

  SKL_UINT32 Crcs[3][4][2][2] = {
    {{{8393568,8388819},{8393399,8388982}},{{8393067,8389288},{8394034,8388660}},
     {{8394324,8389557},{8392501,8388292}},{{8391667,8388039},{8391581,8387852}}},
    {{{8392223,8388635},{8392433,8388450}},{{8393452,8389064},{8392916,8388292}},
     {{8392911,8388341},{8394027,8389546}},{{8393499,8388410},{8391674,8388356}}},
    {{{8392360,8388039},{8394520,8389246}},{{8393808,8388341},{8392711,8389546}},
     {{8393138,8388510},{8392848,8388274}},{{8393739,8388628},{8392100,8387880}}}
  };
  int MV_Crcs[3][4][2][4] = {
   {{{0,0,0,0},{0,0,0,1}},{{0,0,1,1},{0,-1,1,-1}},
    {{1,1,1,2},{1,0,1,0}},{{1,0,2,0},{1,0,2,0}}},
   {{{-7,-4,-14,-8},{5,9,10,18}},{{5,10,10,21},{5,-4,10,-8}},
    {{-3,-1,-6,-2},{1,5,2,10}},{{-7,-8,-14,-17},{5,-10,11,-20}}},
   {{{1,8,2,17},{1,-9,2,-18}},{{1,-1,2,-2},{-7,-3,-14,-6}},
    {{5,10,10,21},{-7,-9,-14,-19}},{{-3,8,-6,17},{5,-8,10,-17}}}
  };
  for(int Nb_Pts=1; Nb_Pts<=3; ++Nb_Pts) {
    for(int Acc=0; Acc<=3; ++Acc) {
      for(int Rounding=0; Rounding<=1; ++Rounding) {
        Rnd.Set_Seed( Rounding + Acc*57 + Nb_Pts*39 + 3 );
        for(int k=0; k<4; ++k) {
          Pts[k][0] = Rnd.Get_SInt(-2,2);
          Pts[k][1] = Rnd.Get_SInt(-2,2);
        }

        for(SKL_CPU_FEATURE *Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) 
        {
          int mv[2][2];
          int x, y;
          Skl_Init_GMC_DSP( &Dsp, *Cpu );
          Dsp.Setup(WIDTH,WIDTH, Pts, Nb_Pts, Acc);

          for(i=0; i<(int)sizeof(Dst); ++i) Dst[i] = 0;
          tm.Reset();
          for(n=0; n<CNT; ++n) {
            for(y=0; y<WIDTH/16; y+=16) {
              for(x=0; x<WIDTH/16; x+=16)
                Dsp.Predict_16x16(&Dsp, Dst+x+y*WIDTH, S, WIDTH,x,y, Rounding);
                Dsp.Get_Average_MV(mv[0], x,y,0);
                Dsp.Get_Average_MV(mv[1], x,y,1);
            }
          }
          Dsp.Switch_Off();
          tm.Stop();
          for(Crc=0,j=0; j<(int)sizeof(Dst); ++j) Crc += (Dst[j]^j);
          printf( "%s: 16x16 Nb:%d Acc=%d Rounding=%d \t%.3f ms \tCrc: %d\n",
            Dsp.Name, Nb_Pts, Acc, Rounding, tm.Get_mSec(), Crc );

          CHECKI(Crc, Crcs[Nb_Pts-1][Acc][Rounding][0]);
          CHECKI(mv[0][0], MV_Crcs[Nb_Pts-1][Acc][Rounding][0]);
          CHECKI(mv[0][1], MV_Crcs[Nb_Pts-1][Acc][Rounding][1]);
          CHECKI(mv[1][0], MV_Crcs[Nb_Pts-1][Acc][Rounding][2]);
          CHECKI(mv[1][1], MV_Crcs[Nb_Pts-1][Acc][Rounding][3]);

          for(i=0; i<(int)sizeof(Dst); ++i) Dst[i] = 0;

          tm.Reset();
          for(n=0; n<CNT*2; ++n) {
            for(y=0; y<WIDTH/32; y+=8) {
              for(x=0; x<WIDTH/32; x+=8)
                Dsp.Predict_8x8(&Dsp, Dst+x+y*WIDTH, S, WIDTH/2, WIDTH,x,y,Rounding);
            }
          }
          Dsp.Switch_Off();
          tm.Stop();
          for(Crc=0,j=0; j<(int)sizeof(Dst); ++j) Crc += (Dst[j]^j);
          printf( "%s: 8x8   Nb:%d Acc=%d Rounding=%d \t%.3f ms \tCrc: %d\n",
            Dsp.Name, Nb_Pts, Acc, Rounding, tm.Get_mSec(), Crc );
          CHECKI(Crc, Crcs[Nb_Pts-1][Acc][Rounding][1]);
        }
      }
    }
  }
}
END_FUNC

//////////////////////////////////////////////////////////
// Quant tests

#define TEST_Q1(S, I, CNT, CALL, CRC0)        \
  Dsp.Init_Quantizer(Q,M, 0L, I);             \
  tm.Reset();                                 \
  for(n=0; n<CNT; ++n)                        \
    for(Crc=0, q=MIN_Q; q<=MAX_Q; q++) {      \
      Dsp.CALL(Dst, Src, Q, q, 1+q*2);        \
      for(i=0; i<8*8; ++i) Crc += Dst[i]^i;   \
    }                                         \
  Dsp.Switch_Off();                           \
  tm.Stop();                                  \
  printf( "%s:\t%.3f s \tCrc: %d\n",    \
    S, tm.Get_Sec(), Crc);                    \
  CHECKI(Crc, CRC0)

#define TEST_Q2(S, I, CNT, CALL, CRC0)        \
  Dsp.Init_Quantizer(Q,M, 0L,I);              \
  tm.Reset();                                 \
  for(n=0; n<CNT; ++n)                        \
    for(Crc=0, q=MIN_Q; q<=MAX_Q; ++q) {      \
      Dsp.CALL(Dst, Src, Q, q);               \
      for(i=0; i<8*8; ++i) Crc += Dst[i]^i;   \
    }                                         \
  Dsp.Switch_Off();                           \
  tm.Stop();                                  \
  printf( "%s:\t%.3f s \tCrc: %d\n",    \
    S, tm.Get_Sec(), Crc);                    \
  CHECKI(Crc, CRC0)

#define TEST_Q3(S, I, CNT, CALL, CRC0)        \
  Dsp.Init_Quantizer(Q,M, 0L,I);              \
  tm.Reset();                                 \
  for(n=0; n<CNT; ++n)                        \
    for(Crc=0, q=MIN_Q; q<=MAX_Q; ++q) {      \
      Dsp.CALL(Dst, Src, Q, q, 0xff);         \
      for(i=0; i<8*8; ++i) Crc += Dst[i]^i;   \
    }                                         \
  Dsp.Switch_Off();                           \
  tm.Stop();                                  \
  printf( "%s:\t%.3f s \tCrc: %d\n",    \
    S, tm.Get_Sec(), Crc);                    \
  CHECKI(Crc, CRC0)

#define TEST_Q4(S, FUNC, SIZE, CNT)           \
  for(i=0; i<SIZE; ++i) Dst[i] = 0xdead;      \
  tm.Reset();                                 \
  for(n=0; n<CNT; ++n) Dsp.FUNC(Dst);         \
  Dsp.Switch_Off();                           \
  tm.Stop();                                  \
  for(i=0; i<SIZE; ++i) CHECK(Dst[i]==0);     \
  printf( "%s:\t%.3f s\n", S, tm.Get_Sec())

TEST_FUNC(Test_Quant_DSP)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,   // SKL_CPU_X86,
    SKL_CPU_MMX,
    SKL_CPU_SSE,
//    SKL_CPU_SSE2,
    SKL_CPU_REF, // SKL_CPU_ALT,
    SKL_CPU_LAST
  };

  SKL_INT16 Src[8*8], Dst[8*8];
  SKL_BYTE M[64];

  int i, q, n;
  const int MAX_Q = 31;
  const int MIN_Q = 1;
  const int CNT1 = 7000;
  SKL_UINT32 Crc;
  SKL_PTIMER tm;
  SKL_CPU_FEATURE *Cpu;

  for(i=0; i<64; ++i) M[i] = (i+1)*255/64; // (i<8) ? 8 : i;

  for(i=0; i<8*8; ++i) {
    Src[i] = 1 + (i-32) * (i&6);
    Dst[i] = 0;
  }
  Src[0] = 129;
  SKL_INT16 Q[4][31][2][64];
  SKL_QUANT_DSP Dsp;

  printf( "== Quant/Dequant H263  (CNT=%d) ==\n", CNT1 );
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++)
  {
    Skl_Init_Quant_DSP( &Dsp, *Cpu, 0 );
    printf( "--- Impl:%s ---\n", Dsp.Name);
    TEST_Q4("       -Zero-Coeffs-", Zero, 64, 150*CNT1);
    TEST_Q4("       -Zero16-Coeffs-", Zero16, 16, 16*150*CNT1);
    TEST_Q1("Quant  -Intra-H263 ", 1, CNT1, Quant_Intra, 47876);
    TEST_Q2("Quant  -Inter-H263 ", 0, CNT1, Quant_Inter, 47901);
    TEST_Q1("Dequant-Intra-H263 ", 1, CNT1, Dequant_Intra, 146631);
    TEST_Q3("Dequant-Inter-H263 ", 0, CNT1, Dequant_Inter, 145753);
  }

  printf( "== Quant/Dequant MPEG4 (CNT=%d) ==\n", CNT1 );
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++)
  {
    Skl_Init_Quant_DSP( &Dsp, *Cpu, 1 );
    printf( "--- Impl:%s ---\n", Dsp.Name);
// C/MMX MPEG-Quantization is BROKEN for now ('coz of precision)
//    TEST_Q1("Quant  -Intra-MPEG4", 1, CNT1, Quant_Intra, 54922);
//    TEST_Q2("Quant  -Inter-MPEG4", 0, CNT1, Quant_Inter, 59144);
    TEST_Q1("Dequant-Intra-MPEG4", 1, CNT1, Dequant_Intra, 282711);
    TEST_Q3("Dequant-Inter-MPEG4", 0, CNT1, Dequant_Inter, 304735);
  }
  
  printf( "== Quant/Dequant MPEG2 (CNT=%d) ==\n", CNT1 );
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++)
  {
    Skl_Init_Quant_DSP( &Dsp, *Cpu, 2 );
    printf( "--- Impl:%s ---\n", Dsp.Name);
      // MMX hack does not perform mismatch control. => different CRC
    int Crc = (!strcmp(Dsp.Name,"MPEG2-Ref")) ? 354040 : 354022;
    TEST_Q1("Dequant-Intra-MPEG2", 1, CNT1, Dequant_Intra, Crc);
    TEST_Q3("Dequant-Inter-MPEG2", 0, CNT1, Dequant_Inter, 333603);
  }
}
END_FUNC


TEST_FUNC(Test_Quant_Sparse)
{
  SKL_QUANT_DSP Dsp;
  SKL_CPU_FEATURE *Cpu;
  const int CNT = 2000;
  SKL_PTIMER tm;

  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_MMX,
    SKL_CPU_SSE,
    SKL_CPU_LAST
  };
  SKL_INT16 Src0[256][64], Src1[64], Src2[64];
  int i,j,k,n, Rows;
  SKL_RANDOM Rnd(7641452);


  printf( "=== Sparse_8x8 tests ===\n" );

  for(Rows = 0x00; Rows<=0xff; ++Rows) {
    for(j=0; j<8; ++j)
      if (Rows & (1<<8))
        for(i=0; i<8; ++i)
          Src0[Rows][j*8+i] = Rnd.Get_SInt(-2048, 2047);
  }

  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++)
  {
    Skl_Init_Quant_DSP( &Dsp, *Cpu, 0 );
    tm.Reset();
    for(n=0; n<CNT; ++n) {
      for(Rows = 0x00; Rows<=0xff; ++Rows) {
        for(k=0; k<64; ++k) Src1[k] = Src2[k] = Src0[Rows][k];
        Dsp.IDct(Src1);
        Dsp.IDct_Sparse(Src2);
        for(i=0; i<64; ++i)  CHECKI(Src2[i], Src1[i]);
      }
    }
    Dsp.Switch_Off();
    tm.Stop();
    printf( "- %s - \tSparse    : %.3f s", Dsp.Name, tm.Get_Sec() );

    tm.Reset();
    for(n=0; n<CNT; ++n) {
      for(Rows = 0x00; Rows<=0xff; ++Rows) {
        SKL_BYTE Dst[64];
        for(k=0; k<64; ++k) Src1[k] = Src2[k] = Src0[Rows][k];
        Dsp.IDct_Put(Src1, Dst, 8);
        Dsp.IDct_Add(Src1, Dst, 8);
      }
    }
    Dsp.Switch_Off();
    tm.Stop();
    printf( "      Put/Add   \t%.3f s\n", tm.Get_Sec() );
  }


  printf( "=== Sparse_8x4 tests ===\n" );

  for(Rows = 0x00; Rows<=0x0f; ++Rows) {
    SKL_BZERO( Src0[Rows], 64*sizeof(Src0[0][0]) );
    for(j=0; j<4; ++j)
      if (Rows & (1<<8))
        for(i=0; i<4; ++i)
          Src0[Rows][j*8+i] = Rnd.Get_SInt(-2048, 2047);
  }

  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++)
  {
    Skl_Init_Quant_DSP( &Dsp, *Cpu, 0 );
    SKL_PTIMER tm;
    tm.Reset();
    for(n=0; n<CNT*16; ++n) {
      for(int Rows = 0x00; Rows<=0x0f; ++Rows) {
        for(k=0; k<64; ++k) Src1[k] = Src2[k] = Src0[Rows][k];
        Dsp.IDct(Src1);
        Dsp.IDct_Sparse_8x4(Src2);
        for(i=0; i<64; ++i)  CHECKI(Src2[i], Src1[i]);
      }
    }
    Dsp.Switch_Off();
    tm.Stop();
    printf( "- %s - \tSparse 8x4: %.3f s", Dsp.Name, tm.Get_Sec() );
    
    tm.Reset();
    for(n=0; n<CNT*16; ++n) {
      for(Rows = 0x00; Rows<=0x0f; ++Rows) {
        SKL_BYTE Dst[64];
        for(k=0; k<64; ++k) Src1[k] = Src2[k] = Src0[Rows][k];
        Dsp.IDct_Put_8x4(Src1, Dst, 8);
        Dsp.IDct_Add_8x4(Src1, Dst, 8);
      }
    }
    Dsp.Switch_Off();
    tm.Stop();
    printf( "      Put/Add    \t%.3f s\n", tm.Get_Sec() );
  }
}
END_FUNC

//////////////////////////////////////////////////////////

extern "C" void Skl_Hadamard_Ref(SKL_INT16 *Out);
extern "C" void Skl_Hadamard_C(SKL_INT16 *Out);
extern "C" void Skl_Hadamard_MMX(SKL_INT16 *Out);
extern "C" void Skl_Hadamard_SSE(SKL_INT16 *Out);

#define TEST(FUNC,O,COUNT,S) \
  Rnd.Set_Seed(154);                          \
  printf( "=======%s========\n", S);  \
  for(n=0; n<64; ++n) O[n] = Rnd.Get_Int(256) - 128; FUNC(O); SKL_EMMS; \
  for(n=0; n<8; ++n) { for(i=0; i<8; ++i) printf( "%3d ", O[n*8+i] ); printf( "\n" ); } \
  if (COUNT>0) for(n=0; n<64; ++n) CHECKI(Ref[n],O[n]); \
  { SKL_INT16 Tmp[64]; for(n=0; n<64; ++n) Tmp[n] = Rnd.Get_Int(256) - 128; \
  pTimer.Reset(); for(n=0; n<COUNT; n++) FUNC(Tmp); SKL_EMMS; }; \
  pTimer.Stop(); printf( "   %.3f ms\n", pTimer.Get_mSec() )

TEST_FUNC(Test_Hadamard_8x8)
{
  SKL_INT16 Ref[64], Out[64];
  int n, i;
  SKL_RANDOM Rnd(1324);
  SKL_PTIMER pTimer;
  const int CNT = 500000;

  TEST(Skl_Hadamard_Ref,  Ref,   0, "Ref    ");
  TEST(Skl_Hadamard_C,    Out, CNT, "C      ");
#ifdef SKL_USE_ASM
  TEST(Skl_Hadamard_SSE,  Out, CNT, "SSE    ");
#endif
}
END_FUNC
   
#undef TEST

//////////////////////////////////////////////////////////

extern "C" SKL_UINT32 Skl_Hadamard_SAD_4x4_Ref(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_4x4_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_4x4_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_8x8_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_16x16_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_8x8_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_16x8_Field_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);
extern "C" SKL_UINT32 Skl_Hadamard_SAD_16x16_MMX(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS);

#define TEST(FUNC,COUNT,S) \
  printf( "=======%s========\n", S);  \
  pTimer.Reset(); for(n=0; n<COUNT; n++) FUNC(Ref,Cur,SIZE); \
  Crc =  FUNC(Ref,Cur,SIZE); SKL_EMMS; \
  pTimer.Stop(); printf( "   %.3f ms   Crc=%d\n", pTimer.Get_mSec(), Crc )

TEST_FUNC(Test_Hadamard_SAD)
{
  enum { SIZE=256 };
  SKL_BYTE Ref[16*SIZE], Cur[16*SIZE];
  int n, i;
  SKL_UINT32 Crc;
  SKL_RANDOM Rnd(1324);
  for(i=0; i<16*SIZE;++i) {
    Ref[i] = Rnd.Get_Int(256);
    Cur[i] = Rnd.Get_Int(256);
  }

  SKL_PTIMER pTimer;
  const int CNT = 500000;

  TEST(Skl_Hadamard_SAD_4x4_Ref,  CNT, "4x4-Ref   ");
  TEST(Skl_Hadamard_SAD_4x4_C,    CNT, "4x4-C   ");
  TEST(Skl_Hadamard_SAD_4x4_MMX,  CNT, "4x4-MMX   ");
  TEST(Skl_Hadamard_SAD_8x8_C,    CNT, "8x8-C   ");
  TEST(Skl_Hadamard_SAD_16x16_C,  CNT, "16x16-C   ");
#ifdef SKL_USE_ASM
  TEST(Skl_Hadamard_SAD_8x8_MMX,  CNT, "8x8-MMX   ");
  TEST(Skl_Hadamard_SAD_16x8_Field_MMX,  CNT, "16x8-MMX  ");
  TEST(Skl_Hadamard_SAD_16x8_Field_MMX,  CNT, "16x8-C  ");
  TEST(Skl_Hadamard_SAD_16x16_MMX,CNT, "16x16-MMX ");
#endif
}
END_FUNC

#undef TEST

//////////////////////////////////////////////////////////
// Edges replication

TEST_FUNC(Test_Edges)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_X86,
    SKL_CPU_MMX,
    SKL_CPU_SSE,
    SKL_CPU_LAST
  };

  const int W = 320, H = 240, BpS = W+2*16;
  const int YSize = (H+2*16)*BpS;
  const int UVSize = (H/2+2*8)*BpS;
  SKL_BYTE YU[YSize+UVSize];
  SKL_BYTE *YUV[3] = {
    YU+16+16*BpS,
    YU+YSize+8+8*BpS,
    YU+YSize+8+8*BpS +W/2+16 };
  SKL_PTIMER tm;
  const int CNT = 20000;

  SKL_CPU_FEATURE *Cpu;
  for(Cpu=Cpu_List; *Cpu != SKL_CPU_LAST; Cpu++) {
    int i, n, Crc;
    SKL_MB_DSP Dsp;
    Skl_Init_Mb_DSP( &Dsp, *Cpu );
    for(i=0; i<(int)sizeof(YU); ++i) YU[i] = (i*54+23)&0xff;    
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.Make_Edges(YUV, W, H, BpS);
    tm.Stop();
    Dsp.Switch_Off();
    for(Crc=0,i=0; i<(int)sizeof(YU); ++i) Crc = ((Crc+YU[i]^i)^(Crc>>8))&0xffff;
    printf( "%s -Make_Edges- :\t%.3f s \tCrc: %d\n", Dsp.Name, tm.Get_Sec(), Crc );
    CHECKI(Crc, 12299);
  }
}
END_FUNC

//////////////////////////////////////////////////////////

TEST_FUNC(Test_YUV)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_MMX,
    SKL_CPU_REF,
    SKL_CPU_LAST
  };
  SKL_YUV_DSP Dsp;

  const int W = 640, H = 480, BpS = W;
  const int YSize = H*BpS;
  const int UVSize = (H/2)*BpS;
  SKL_BYTE YU[YSize+UVSize];
  SKL_BYTE RGB[4*W*H];
  SKL_BYTE *Y = YU;
  SKL_BYTE *U = YU+YSize;
  SKL_BYTE *V = U +W/2;
  SKL_PTIMER tm;
  int i, n;
  const int CNT = 200;

  for(i=0; i<(int)sizeof(YU); ++i) YU[i] = (i*54+23)&0xff;

  for(SKL_CPU_FEATURE *Cpu = Cpu_List; *Cpu!=SKL_CPU_LAST; ++Cpu)
  {
    Skl_Init_YUV_DSP( &Dsp, *Cpu );
    printf( "====== YUV: '%s' ======\n", Dsp.Name );
    SKL_BZERO(RGB, sizeof(RGB));
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.YUV_TO_RGB565(RGB, 2*W, Y, U, V, BpS, W, H);
    Dsp.Switch_Off();    
    tm.Stop();
    printf( "YUV_To_RGB565    :\t%.3f s\n", tm.Get_Sec());

    SKL_BZERO(RGB, sizeof(RGB));
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.YUV_TO_RGB32(RGB, 4*W, Y, U, V, BpS, W, H);
    Dsp.Switch_Off();    
    tm.Stop();
    printf( "YUV_To_RGB32     :\t%.3f s\n", tm.Get_Sec());
  }
}
END_FUNC

TEST_FUNC(Test_RGB)
{
  SKL_CPU_FEATURE Cpu_List[] = {
    SKL_CPU_C,
    SKL_CPU_MMX,
    SKL_CPU_REF,
    SKL_CPU_LAST
  };

  const int W = 640, H = 480, BpS = W;
  const int YSize = H*BpS;
  const int UVSize = (H/2)*BpS;
  SKL_BYTE YU[YSize+UVSize];
  SKL_BYTE RGB[4*W*H];
  SKL_BYTE *Y = YU;
  SKL_BYTE *U = YU+YSize;
  SKL_BYTE *V = U +W/2;
  SKL_PTIMER tm;
  int i, n;
  const int CNT = 200;
  SKL_YUV_DSP Dsp;

  for(i=0; i<(int)sizeof(RGB); ++i) RGB[i] = (i*54+23)&0xff;

  for(SKL_CPU_FEATURE *Cpu = Cpu_List; *Cpu!=SKL_CPU_LAST; ++Cpu)
  {
    Skl_Init_YUV_DSP( &Dsp, *Cpu );
    printf( "====== RGB: '%s' ======\n", Dsp.Name );

    SKL_BZERO(YU, sizeof(YU));
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.RGB24_TO_YUV(Y, U, V, BpS, RGB, 3*W, W, H);
    Dsp.Switch_Off();    
    tm.Stop();
    printf( "RGB24_TO_YUV     :\t%.3f s\n", tm.Get_Sec());

    SKL_BZERO(YU, sizeof(YU));
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.RGB565_TO_YUV(Y, U, V, BpS, RGB, 2*W, W, H);
    Dsp.Switch_Off();    
    tm.Stop();
    printf( "RGB565_TO_YUV    :\t%.3f s\n", tm.Get_Sec());

    SKL_BZERO(YU, sizeof(YU));
    tm.Reset();
    for(n=0; n<CNT; ++n) Dsp.RGB32_TO_YUV(Y, U, V, BpS, RGB, 4*W, W, H);
    Dsp.Switch_Off();    
    tm.Stop();
    printf( "RGB32_TO_YUV     :\t%.3f s\n", tm.Get_Sec());
  }
}
END_FUNC

//////////////////////////////////////////////////////////

#include "skl_2d/skl_btm_cvrt.h"

TEST_FUNC(Test_Cvrt)
{
  const int W = 256*4, H = 256;
  SKL_BYTE Src[W*H], Dst[W*H];
  SKL_UINT32 Tab[4*256];
  SKL_PTIMER tm;
  int i, dIn, dOut, n;
  const int CNT = 500;

  for(i=0; i<4*256*4;++i) ((SKL_BYTE*)Tab)[i] = i | (i<<8) | (i<<16) | (i<<24);

  printf( "====== Converters ======\n" );
  for(dIn=1; dIn<=4; ++dIn) {
    for(dOut=1; dOut<=4; ++dOut) {
      SKL_BZERO(Dst, sizeof(Dst));
      for(i=0; i<W*H;++i) Src[i] = (i*54+23)&0xff;
      tm.Reset();
      for(n=0; n<CNT; ++n) 
        (Skl_Get_Cvrt_Convert_Ops(dIn,dOut))(Dst, W, Src, W, W/4, 256, Tab);
      tm.Stop();
      int Crc = 0;
      for(i=0; i<W*H; ++i) Crc = (((Dst[i]<<3) | (Crc>>3)) + Crc) & 0xffff;
      printf( " RGB%2d => RGB%2d    :\t%.3f s    Crc=0x%x\n", dIn*8, dOut*8, tm.Get_Sec(), Crc);
    }
    printf( " ----- \n" );
  }
}
END_FUNC

//////////////////////////////////////////////////////////

TEST_FUNC(Test_CPU)
{
  static const char *CPU_Names[] = { "  C ", " X86", " MMX", " SSE", "SSE2" };

  SKL_CPU_FEATURE Cpu = Skl_Detect_CPU_Feature();
  printf( "CPU Detected: [%s]\n", (Cpu<=SKL_CPU_SSE2 ? CPU_Names[Cpu] : "Unknown") );
  
  SKL_CPU_SPECS Specs;
  printf( "Specs infos:\n" );
  Specs.Print_Infos();
}
END_FUNC

//////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  SKL_T_START;

    SKL_TEST( 1,Test_Img_DSP);
    SKL_TEST( 2,Test_Mb_DSP);
    SKL_TEST( 3,Test_Mb_QP_DSP);
    SKL_TEST( 4,Test_GMC_DSP);
    SKL_TEST( 5,Test_Quant_DSP);
    SKL_TEST( 6,Test_Quant_Sparse);
    SKL_TEST( 7,Test_Hadamard_8x8);
    SKL_TEST( 8,Test_Hadamard_SAD);
    SKL_TEST( 9,Test_Edges);
    SKL_TEST(10,Test_YUV);
    SKL_TEST(11,Test_RGB);
    SKL_TEST(12,Test_Cvrt);
    SKL_TEST(13,Test_CPU);

  SKL_T_END;
  SKL_T_RETURN;
}

//////////////////////////////////////////////////////////
