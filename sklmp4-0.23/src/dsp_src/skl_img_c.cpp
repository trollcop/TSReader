/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 *  skl_img_c.cpp
 *
 *   Image processing
 *
 ********************************************************/

extern "C" {

  // so we don't even have to include skl.h
typedef int SKL_INT32;
typedef unsigned int SKL_UINT32;
typedef unsigned char SKL_BYTE;

//////////////////////////////////////////////////////////
// Sum Of Absolute Differences
//////////////////////////////////////////////////////////

#define SAD(WIDTH,HEIGHT)                                       \
  SKL_UINT32 Sum = 0;                                           \
  int H = HEIGHT;                                               \
  while(H-->0) {                                                \
    for(int i=-WIDTH; i<0; i++) {                               \
      SKL_INT32 D = Src[WIDTH+i] - Dst[WIDTH+i];                \
      Sum += (D>=0) ? D : -D;                                   \
    }                                                           \
    Src += BpS;                                                 \
    Dst += BpS;                                                 \
  }                                                             \
  return Sum

SKL_UINT32 Skl_SAD_16x16_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS) { SAD(16,16); }
SKL_UINT32 Skl_SAD_8x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS) { SAD(8,8); }
SKL_UINT32 Skl_SAD_4x4_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS) { SAD(4,4); }

#undef SAD

SKL_UINT32 Skl_SAD_16x8_Field_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int H = 8;
  while(H-->0) {
    for(int i=-16; i<0; i++) {
      SKL_INT32 D = Src1[16+i] - Src2[16+i];
      Sum += (D>=0) ? D : -D;
    }
    Src1 += 2*BpS;
    Src2 += 2*BpS;
  }
  return Sum;
}

SKL_UINT32 Skl_SAD_16x7_Self_C(const SKL_BYTE *Src1, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int H = 7;
  const SKL_BYTE *Src2 = Src1 + BpS;
  while(H-->0) {
    for(int i=-16; i<0; ++i) {
      SKL_INT32 D = Src1[16+i] - Src2[16+i];
      Sum += (D>=0) ? D : -D;
    }
    Src1 = Src2;
    Src2 += BpS;
  }
  return Sum;
}

//////////////////////////////////////////////////////////
// Averaging SAD versions

#define SAD_AVRG(WIDTH,HEIGHT)                                  \
  SKL_UINT32 Sum = 0;                                           \
  int H = HEIGHT;                                               \
  while(H-->0) {                                                \
    for(int i=-WIDTH; i<0; i++) {                               \
      const int Avrg = (Src1[WIDTH+i] + Src2[WIDTH+i] + 1)>>1;  \
      const int D = Avrg - Dst[WIDTH+i];                        \
      Sum += (D>=0) ? D : -D;                                   \
  }                                                             \
  Src1 += BpS;                                                  \
    Src2 += BpS;                                                \
    Dst  += BpS;                                                \
  }                                                             \
  return Sum

SKL_UINT32 Skl_SAD_Avrg_16x16_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS) { SAD_AVRG(16,16); }
SKL_UINT32 Skl_SAD_Avrg_16x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS) { SAD_AVRG(16,8); }
SKL_UINT32 Skl_SAD_Avrg_8x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS) { SAD_AVRG(8,8); }
SKL_UINT32 Skl_SAD_Avrg_4x4_C(const SKL_BYTE *Dst, const SKL_BYTE *Src1, const SKL_BYTE *Src2, SKL_INT32 BpS) { SAD_AVRG(4,4); }

#undef SAD_AVRG 

//////////////////////////////////////////////////////////
// Sum Of Squared Differences
//////////////////////////////////////////////////////////

#define SSD(WIDTH,HEIGHT,BPS)                                   \
  SKL_UINT32 Sum = 0;                                           \
  int H = HEIGHT;                                               \
  while(H-->0) {                                                \
    for(int i=-WIDTH; i<0; i++) {                               \
      SKL_INT32 D = Src[WIDTH+i] - Dst[WIDTH+i];                \
      Sum += D*D;                                               \
    }                                                           \
    Src += (BPS);                                               \
    Dst += (BPS);                                               \
  }                                                             \
  return Sum

SKL_UINT32 Skl_SSD_16x16_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS) { SSD(16,16,BpS); }
SKL_UINT32 Skl_SSD_8x8_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS)   { SSD(8,8,BpS); }
SKL_UINT32 Skl_SSD_4x4_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS)   { SSD(4,4,BpS); }
SKL_UINT32 Skl_SSD_16x8_Field_C(const SKL_BYTE *Dst, const SKL_BYTE *Src, SKL_INT32 BpS) { SSD(16,8,2*BpS); }

#undef SSD

//////////////////////////////////////////////////////////
// Mean
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Mean_4x4_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 4;
  while(j-->0) {
    for(int i=-4; i<0; i++) Sum += Src[4+i];
    Src += BpS;
  }
  return Sum >> 4;
}

SKL_UINT32 Skl_Mean_8x8_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 8;
  while(j-->0) {
    for(int i=-8; i<0; i++) Sum += Src[8+i];
    Src += BpS;
  }
  return Sum >> 6;
}

SKL_UINT32 Skl_Mean_16x16_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 16;
  while(j-->0) {
    for(int i=-16; i<0; i++) Sum += Src[16+i];
    Src += BpS;
  }
  return Sum >> 8;
}

//////////////////////////////////////////////////////////
// Square sum
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Sqr_4x4_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 4;
  while(j-->0) {
    for(int i=-4; i<0; i++) Sum += Src[4+i]*Src[4+i];
    Src += BpS;
  }
  return Sum >> 4;
}

SKL_UINT32 Skl_Sqr_8x8_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 8;
  while(j-->0) {
    for(int i=-8; i<0; i++) Sum += Src[8+i]*Src[8+i];
    Src += BpS;
  }
  return Sum >> 6;
}

SKL_UINT32 Skl_Sqr_16x16_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  int j = 16;
  while(j-->0) {
    for(int i=-16; i<0; i++) Sum += Src[16+i]*Src[16+i];
    Src += BpS;
  }
  return Sum >> 8;
}

//////////////////////////////////////////////////////////
// Deviation
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_Abs_Dev_16x16_C(const SKL_BYTE *Src0, SKL_INT32 BpS)
{
  SKL_UINT32 Mean=0, Sum=0;
  int i, j;
  const SKL_BYTE *Src = Src0;
  for(j=16; j>0; --j) {
    for(i=-16; i<0; i++) Mean += Src[16+i];
    Src += BpS;
  }
  Mean >>= 8;

  Src = Src0;
  Sum = 0;
  for(j=16; j>0; --j) {
    for(i=-16; i<0; i++) {
      SKL_INT32 D = Src[16+i] - Mean;
      Sum += (D>=0) ? D : -D;
    }
    Src += BpS;
  }
  return Sum;
}

SKL_UINT32 Skl_Sqr_Dev_16x16_C(const SKL_BYTE *Src, SKL_INT32 BpS)
{
  SKL_UINT32 Sum=0, Sum2=0;
  for(int j=16; j>0; --j) {
    for(int i=-16; i<0; i++) {
      SKL_INT32 D = Src[16+i];
      Sum  += D;
      Sum2 += D*D;
    }
    Src += BpS;
  }
  Sum  >>= 8;
  Sum2 >>= 8;
  return Sum2 - Sum*Sum;
}

//////////////////////////////////////////////////////////
// Any-size versions (really useful??)
//////////////////////////////////////////////////////////

SKL_UINT32 Skl_SAD_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  Src1 += W;
  Src2 += W;
  while(H-->0) {
    for(int i=-W; i<0; ++i) {
      SKL_INT32 D = Src1[i] - Src2[i];
      Sum += (D>=0) ? D : -D;
    }
    Src1 += BpS;
    Src2 += BpS;
  }
  return Sum;
}

SKL_UINT32 Skl_SSD_C(const SKL_BYTE *Src1, const SKL_BYTE *Src2, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  Src1 += W;
  Src2 += W;
  while(H-->0) {
    for(int i=-W; i<0; ++i) {
      SKL_INT32 D = Src1[i] - Src2[i];
      Sum += D*D;
    }
    Src1 += BpS;
    Src2 += BpS;
  }
  return Sum;
}

SKL_UINT32 Skl_Mean_C(const SKL_BYTE *Src, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  Src += W;
  int j = H;
  while(j-->0) {
    for(int i=-W; i<0; i++)
      Sum += Src[i];
    Src += BpS;
  }
  return Sum / (W*H);
}

SKL_UINT32 Skl_Sqr_C(const SKL_BYTE *Src, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0;
  Src += W;
  int j = H;
  while(j-->0) {
    for(int i=-W; i<0; i++)
      Sum += Src[i]*Src[i];
    Src += BpS;
  }
  return Sum / (W*H);
}

SKL_UINT32 Skl_Square_Dev_C(const SKL_BYTE *Src, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Sum = 0, Sum2=0;
  Src += W;
  int j = H;
  while(j-->0) {
    for(int i=-W; i<0; i++) {
      Sum += Src[i];
      Sum2 += Src[i]*Src[i];
    }
    Src += BpS;
  }
  Sum /= (W*H);
  Sum2 /= (W*H);
  return Sum2 - Sum*Sum;
}

SKL_UINT32 Skl_Abs_Dev_C(const SKL_BYTE *Src, int W, int H, SKL_INT32 BpS)
{
  SKL_UINT32 Mean = Skl_Mean_C(Src, W, H, BpS);
  SKL_UINT32 Sum = 0;
  Src += W;
  int j = H;
  while(j-->0) {
    for(int i=-W; i<0; i++) {
      SKL_INT32 D = Src[i] - Mean;
      Sum += (D>=0) ? D : -D;
    }
    Src += BpS;
  }
  return Sum;
}

//////////////////////////////////////////////////////////

}   // extern "C"
