/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_gmc_dsp.cpp
 *
 *   level Sprite (MPEG4's GMC) processing
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_bits.h"

extern "C" {

//////////////////////////////////////////////////////////
// Raw-C version

#define RDIV(a,b) (((a)>0 ? (a) + ((b)>>1) : (a) - ((b)>>1))/(b))
#define RSHIFT(a,b) ( (a)>0 ? ((a) + (1<<((b)-1)))>>(b) : ((a) + (1<<((b)-1))-1)>>(b))


#define MLT(i)  (((16-(i))<<16) + (i))
static const SKL_UINT32 MTab[16] = {  
  MLT( 0), MLT( 1), MLT( 2), MLT( 3), MLT( 4), MLT( 5), MLT( 6), MLT( 7),
  MLT( 8), MLT( 9), MLT(10), MLT(11), MLT(12), MLT(13), MLT(14), MLT(15)
};
#undef MLT

//////////////////////////////////////////////////////////
// Pts = 2 or 3

static
void Predict_16x16_C(const SKL_GMC_DSP * const This,
                     SKL_BYTE *Dst, const SKL_BYTE *Src, 
                     int BpS, int x, int y, int Rounding)
{
  const int W   = This->Width;
  const int H   = This->Height;

  const int Rho = 3-This->Accuracy;
  const SKL_INT32 Rounder = ( 128 - (Rounding<<(2*Rho)) ) << 16;

  const SKL_UINT32 W2 = W<<(16-Rho);
  const SKL_UINT32 H2 = H<<(16-Rho);

  Dst += 16;
  const SKL_INT32 dUx = This->dU[0];
  const SKL_INT32 dVx = This->dV[0];
  const SKL_INT32 dUy = This->dU[1];
  const SKL_INT32 dVy = This->dV[1];

  SKL_INT32 Uo = This->Uo + 16*(dUy*y + dUx*x);
  SKL_INT32 Vo = This->Vo + 16*(dVy*y + dVx*x);
  for(int j=16; j>0; --j)
  {
#ifdef USE_ASM
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (Dst-16));
#endif
    SKL_INT32 U = Uo, V = Vo;
    Uo += dUy; Vo += dVy;
    if ( (SKL_UINT32)U<W2 && (SKL_UINT32)(U+15*dUx)<W2 &&
         (SKL_UINT32)V<H2 && (SKL_UINT32)(V+15*dVx)<H2 )
    {    
      for(int i=-16; i<0; ++i)
      {
        SKL_INT32 u = ( U >> 16 ) << Rho;
        SKL_INT32 v = ( V >> 16 ) << Rho;
        U += dUx;  V += dVx;

        int Offset = (u>>4) + (v>>4)*BpS;
        const SKL_UINT32 ri = MTab[u&15];
        const SKL_UINT32 rj = MTab[v&15];
        SKL_UINT32 f0, f1;
        f0  = Src[ Offset     +0 ];
        f0 |= Src[ Offset     +1 ] << 16;
        f1  = Src[ Offset+BpS +0 ];
        f1 |= Src[ Offset+BpS +1 ] << 16;
        f0 = (ri*f0)>>16;
        f1 = (ri*f1) & 0x0fff0000;
        f0 |= f1; 
        f0 = ( rj*f0 + Rounder ) >> 24;
        Dst[i] = (SKL_BYTE)f0;
      }
    }
    else {
      for(int i=-16; i<0; ++i)
      {
        int Offset;

        SKL_INT32 u = ( U >> 16 ) << Rho;
        SKL_INT32 v = ( V >> 16 ) << Rho;
        U += dUx; V += dVx;

        if ((SKL_UINT32)u<(SKL_UINT32)W) Offset = u>>4;
        else {
          if (u>=W) Offset = W>>4;
          else Offset = 0;
          u = 0;
        }
        if ((SKL_UINT32)v<(SKL_UINT32)H) Offset += (v>>4)*BpS;
        else {
          if (v>=H) Offset += (H>>4)*BpS;
          v = 0;
        }
        if (u&15) {
          const SKL_UINT32 ri = MTab[u&15];
          if (v&15) {
            const SKL_UINT32 rj = MTab[v&15];
            SKL_UINT32 f0, f1;
            f0  = Src[ Offset     +0 ];
            f0 |= Src[ Offset     +1 ] << 16;
            f1  = Src[ Offset+BpS +0 ];
            f1 |= Src[ Offset+BpS +1 ] << 16;
            f0 = (ri*f0)>>16;
            f1 = (ri*f1) & 0x0fff0000;
            f0 |= f1; 
            f0 = ( rj*f0 + Rounder ) >> 24;

            Dst[i] = (SKL_BYTE)f0;
          }
          else {
            SKL_UINT32 f0;
            f0  = Src[ Offset   ];
            f0 |= Src[ Offset+1 ] << 16;
            f0 = (ri*f0 + (Rounder>>4))>>20;
            Dst[i] = (SKL_BYTE)f0;
          }
        }
        else {
          if (v&15) {
            const SKL_UINT32 rj = MTab[v&15];
            SKL_UINT32 f0;
            f0  = Src[ Offset      ];
            f0 |= Src[ Offset +BpS ] << 16;
            f0 = (rj*f0 + (Rounder>>4))>>20;
            Dst[i] = (SKL_BYTE)f0;
          }
          else {
            Dst[i] = Src[Offset];
          }
        }
      }
    }
    Dst += BpS;
  }
}

static
void Predict_8x8_C(const SKL_GMC_DSP * const This,
                   SKL_BYTE *uDst, const SKL_BYTE *uSrc, 
                   SKL_SAFE_INT uv_Coloc,
                   int BpS, int x, int y, int Rounding)
{
  const int W   = This->Width >> 1;
  const int H   = This->Height>> 1;
  const int Rho = 3-This->Accuracy;
  const SKL_INT32 Rounder = ( 128 - (Rounding<<(2*Rho)) ) << 16;
  const SKL_UINT32 W2 = W<<(16-Rho);   
  const SKL_UINT32 H2 = H<<(16-Rho);

  uDst += 8;
  SKL_BYTE       *vDst = uDst + uv_Coloc;
  const SKL_BYTE *vSrc = uSrc + uv_Coloc;
  const SKL_INT32 dUx = This->dU[0];
  const SKL_INT32 dVx = This->dV[0];
  const SKL_INT32 dUy = This->dU[1];
  const SKL_INT32 dVy = This->dV[1];

  SKL_INT32 Uo = This->Uco + 8*(dUy*y + dUx*x);
  SKL_INT32 Vo = This->Vco + 8*(dVy*y + dVx*x);

  for(int j=8; j>0; --j)
  {
#ifdef USE_ASM
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (uDst-8));
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (vDst-8));
#endif
    SKL_INT32 U = Uo, V = Vo;
    Uo += dUy; Vo += dVy;
    if ( (SKL_UINT32)U<W2 && (SKL_UINT32)(U+15*dUx)<W2 &&
         (SKL_UINT32)V<H2 && (SKL_UINT32)(V+15*dVx)<H2 )
    {
      for(int i=-8; i<0; ++i) 
      {
        SKL_INT32 u = ( U >> 16 ) << Rho;
        SKL_INT32 v = ( V >> 16 ) << Rho;
        U += dUx; V += dVx;

        const int Offset = (u>>4) + (v>>4)*BpS;
        const SKL_UINT32 ri = MTab[u&15];

        const SKL_UINT32 rj = MTab[v&15];
        SKL_UINT32 f0, f1;

        f0  = uSrc[ Offset     +0 ];
        f0 |= uSrc[ Offset     +1 ] << 16;
        f1  = uSrc[ Offset+BpS +0 ];
        f1 |= uSrc[ Offset+BpS +1 ] << 16;
        f0 = (ri*f0)>>16;
        f1 = (ri*f1) & 0x0fff0000;
        f0 |= f1; 
        f0 = ( rj*f0 + Rounder ) >> 24;

        uDst[i] = (SKL_BYTE)f0;

        f0  = vSrc[ Offset     +0 ];
        f0 |= vSrc[ Offset     +1 ] << 16;
        f1  = vSrc[ Offset+BpS +0 ];
        f1 |= vSrc[ Offset+BpS +1 ] << 16;
        f0 = (ri*f0)>>16;  
        f1 = (ri*f1) & 0x0fff0000;
        f0 |= f1; 
        f0 = ( rj*f0 + Rounder ) >> 24;

        vDst[i] = (SKL_BYTE)f0;
      }
    }
    else {
      for(int i=-8; i<0; ++i) 
      {
        SKL_INT32 u = ( U >> 16 ) << Rho;
        SKL_INT32 v = ( V >> 16 ) << Rho;
        U += dUx; V += dVx;

        int Offset;
        if ((SKL_UINT32)u<(SKL_UINT32)W) Offset = u>>4;
        else {
          if (u>=W) Offset = W>>4;
          else      Offset = 0;
          u = 0;
        }
        if ((SKL_UINT32)v<(SKL_UINT32)H) Offset += (v>>4)*BpS;
        else {
          if (v>=H) Offset += (H>>4)*BpS;
          v = 0;
        }
        if (u&15) {
          const SKL_UINT32 ri = MTab[u&15];
          if (v&15) {
            SKL_UINT32 f0, f1;
            const SKL_UINT32 rj = MTab[v&15];

            f0  = uSrc[ Offset     +0 ];
            f0 |= uSrc[ Offset     +1 ] << 16;
            f1  = uSrc[ Offset+BpS +0 ];
            f1 |= uSrc[ Offset+BpS +1 ] << 16;
            f0 = (ri*f0)>>16;
            f1 = (ri*f1) & 0x0fff0000;
            f0 |= f1; 
            f0 = ( rj*f0 + Rounder ) >> 24;

            uDst[i] = (SKL_BYTE)f0;

            f0  = vSrc[ Offset     +0 ];
            f0 |= vSrc[ Offset     +1 ] << 16;
            f1  = vSrc[ Offset+BpS +0 ];
            f1 |= vSrc[ Offset+BpS +1 ] << 16;
            f0 = (ri*f0)>>16;  
            f1 = (ri*f1) & 0x0fff0000;
            f0 |= f1; 
            f0 = ( rj*f0 + Rounder ) >> 24;

            vDst[i] = (SKL_BYTE)f0;
          }
          else {
            SKL_UINT32 f0;
            f0  = uSrc[ Offset   ];
            f0 |= uSrc[ Offset+1 ] << 16;
            f0 = (ri*f0 + (Rounder>>4))>>20;
            uDst[i] = (SKL_BYTE)f0;

            f0  = vSrc[ Offset   ];
            f0 |= vSrc[ Offset+1 ] << 16;
            f0 = (ri*f0 + (Rounder>>4))>>20;
            vDst[i] = (SKL_BYTE)f0;
          }
        }
        else {
          if (v&15) {
            const SKL_UINT32 rj = MTab[v&15];
            SKL_UINT32 f0;
            f0  = uSrc[ Offset      ];
            f0 |= uSrc[ Offset +BpS ] << 16;
            f0 = (rj*f0 + (Rounder>>4))>>20;
            uDst[i] = (SKL_BYTE)f0;
            f0  = vSrc[ Offset      ];
            f0 |= vSrc[ Offset +BpS ] << 16;
            f0 = (rj*f0 + (Rounder>>4))>>20;
            vDst[i] = (SKL_BYTE)f0;
          }
          else {
            uDst[i] = uSrc[Offset];
            vDst[i] = vSrc[Offset];
          }
        }
      }
    }
    uDst += BpS;
    vDst += BpS;
  }
}


//////////////////////////////////////////////////////////
// simplified Pts = 1

static
void Predict_1pts_16x16_C(const SKL_GMC_DSP * const This,
                          SKL_BYTE *Dst, const SKL_BYTE *Src, 
                          int BpS, int x, int y, int Rounding)
{
  const int W   = This->Width;
  const int H   = This->Height;
  const int Rho = 3-This->Accuracy;
  const SKL_INT32 Rounder = ( 128 - (Rounding<<(2*Rho)) ) << 16;

  Dst += 16;

  SKL_INT32 uo = This->Uo + (x<<8);     // ((16*x)<<4)
  SKL_INT32 vo = This->Vo + (y<<8);
  SKL_UINT32 ri = MTab[uo & 15];
  SKL_UINT32 rj = MTab[vo & 15];

  SKL_SAFE_INT Offset;
  if ((SKL_UINT32)vo<(SKL_UINT32)H)   Offset  = (vo>>4)*BpS;
  else if (vo>=H)                   { Offset  = ( H>>4)*BpS; rj = MTab[0]; }
  else if (vo<=-16)                 { Offset  =-16*BpS;      rj = MTab[0]; }
  else                                Offset  = (vo>>4)*BpS;
  if ((SKL_UINT32)uo<(SKL_UINT32)W)   Offset += (uo>>4);
  else if (uo>=W)                   { Offset += ( W>>4);     ri = MTab[0]; }
  else if (uo<=-16)                 { Offset -= 16;          ri = MTab[0]; }
  else                                Offset += (uo>>4);

  for(int j=16; j>0; --j, Offset+=BpS-16)
  {
#ifdef USE_ASM
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (Dst-16));
#endif
    for(int i=-16; i<0; ++i, ++Offset)
    {
      SKL_UINT32 f0, f1;
      f0  = Src[ Offset     +0 ];
      f0 |= Src[ Offset     +1 ] << 16;
      f1  = Src[ Offset+BpS +0 ];
      f1 |= Src[ Offset+BpS +1 ] << 16;
      f0 = (ri*f0)>>16;
      f1 = (ri*f1) & 0x0fff0000;
      f0 |= f1; 
      f0 = ( rj*f0 + Rounder ) >> 24;
      Dst[i] = (SKL_BYTE)f0;
    }
    Dst += BpS;
  }
}   

static
void Predict_1pts_8x8_C(const SKL_GMC_DSP * const This,
                        SKL_BYTE *uDst, const SKL_BYTE *uSrc, 
                        SKL_SAFE_INT uv_Coloc,
                        int BpS, int x, int y, int Rounding)
{
  const int W   = This->Width >>1;
  const int H   = This->Height>>1;
  const int Rho = 3-This->Accuracy;
  const SKL_INT32 Rounder = ( 128 - (Rounding<<(2*Rho)) ) << 16;

  uDst += 8;
  SKL_BYTE       *vDst = uDst + uv_Coloc;
  const SKL_BYTE *vSrc = uSrc + uv_Coloc;

  SKL_INT32 uo = This->Uco + (x<<7);
  SKL_INT32 vo = This->Vco + (y<<7);
  SKL_UINT32 ri = MTab[uo & 15];
  SKL_UINT32 rj = MTab[vo & 15];

  SKL_SAFE_INT Offset;
  if ((SKL_UINT32)vo<(SKL_UINT32)H)   Offset  = (vo>>4)*BpS;
  else if (vo>=H)                   { Offset  = ( H>>4)*BpS; rj = MTab[0]; }
  else if (vo<=-8)                  { Offset  =-8*BpS;       rj = MTab[0]; }
  else                                Offset  = (vo>>4)*BpS;
  if ((SKL_UINT32)uo<(SKL_UINT32)W)   Offset += (uo>>4);
  else if (uo>=W)                   { Offset += ( W>>4);     ri = MTab[0]; }
  else if (uo<=-8)                  { Offset -= 8;           ri = MTab[0]; }
  else                                Offset += (uo>>4);


  for(int j=8; j>0; --j, Offset+=BpS-8)
  {
#ifdef USE_ASM
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (uDst-8));
//    __asm__ __volatile__ ("prefetcht0 (%0)\n\t" : : "r" (vDst-8));
#endif

    for(int i=-8; i<0; ++i, Offset++)
    {
      SKL_UINT32 f0, f1;
      f0  = uSrc[ Offset     +0 ]; 
      f0 |= uSrc[ Offset     +1 ] << 16;
      f1  = uSrc[ Offset+BpS +0 ];
      f1 |= uSrc[ Offset+BpS +1 ] << 16;
      f0 = (ri*f0)>>16;
      f1 = (ri*f1) & 0x0fff0000; 
      f0 |= f1; 
      f0 = ( rj*f0 + Rounder ) >> 24;
      uDst[i] = (SKL_BYTE)f0;   

      f0  = vSrc[ Offset     +0 ];   
      f0 |= vSrc[ Offset     +1 ] << 16;
      f1  = vSrc[ Offset+BpS +0 ];
      f1 |= vSrc[ Offset+BpS +1 ] << 16;
      f0 = (ri*f0)>>16;
      f1 = (ri*f1) & 0x0fff0000;
      f0 |= f1;
      f0 = ( rj*f0 + Rounder ) >> 24;
      vDst[i] = (SKL_BYTE)f0;
    }
    uDst += BpS;
    vDst += BpS;
  }
}

//////////////////////////////////////////////////////////

void SKL_GMC_DSP::Setup(int W, int H, const int Pts[][2], int Nb, int Acc)
{
  Width    = W << 4;
  Height   = H << 4;
  Accuracy = Acc;
  Nb_Pts = Nb;

  const int Rho   = 3 - Accuracy;  // = {3,2,1,0} for Acc={0,1,2,3}
  int Alpha = SKL_BMASKS::Log2(W-1);
  int Ws = 1 << Alpha;

  SKL_ASSERT(Accuracy>=0 && Accuracy<=3 && W>=16 && H>=16);

/*
  if (Nb_Pts<2 || (Pts[2][0]==0 && Pts[2][1]==0 && Pts[1][0]==0 && Pts[1][1]==0)) {
    if (Nb_Pts<2 || (Pts[1][0]==0 && Pts[1][1]==0)) {
      if (Nb_Pts<1 || (Pts[0][0]==0 && Pts[0][1]==0)) {
        Nb_Pts = 0;
      }
      else Nb_Pts = 1;
    }
    else Nb_Pts = 2;
  }
*/
    // now, Nb_Pts stores the actual number of points required for interpolation

  if (Nb_Pts<=1)
  {
    if (Nb_Pts==1) {
        // store as 4b fixed point
      Uo = Pts[0][0] << Accuracy;
      Vo = Pts[0][1] << Accuracy;
      Uco = ((Pts[0][0]>>1) | (Pts[0][0]&1)) << Accuracy;     // DIV2RND()
      Vco = ((Pts[0][1]>>1) | (Pts[0][1]&1)) << Accuracy;     // DIV2RND()
    }
    else {
      Uo  = Vo  = 0;
      Uco = Vco = 0;
    }

    Predict_16x16 = Predict_1pts_16x16_C;
    Predict_8x8   = Predict_1pts_8x8_C;
  }
  else {
    dU[0] = 16*Ws + RDIV( 8*Ws*Pts[1][0], W );   // dU/dx
    dV[0] =         RDIV( 8*Ws*Pts[1][1], W );   // dV/dx

    if (Nb_Pts==2) {
      dU[1] = -dV[0];  // -Sin
      dV[1] =  dU[0];  //  Cos
    }
    else {
      const int Beta = SKL_BMASKS::Log2(H-1);
      const int Hs = 1<<Beta;
      dU[1] =         RDIV( 8*Hs*Pts[2][0], H );   // dU/dy
      dV[1] = 16*Hs + RDIV( 8*Hs*Pts[2][1], H );   // dV/dy
      if (Beta>Alpha) {
        dU[0] <<= (Beta-Alpha);
        dV[0] <<= (Beta-Alpha);
        Alpha = Beta;
        Ws = Hs;
      }
      else {
        dU[1] <<= Alpha - Beta;
        dV[1] <<= Alpha - Beta;
      }
    }
      // upscale to 16b fixed-point
    dU[0] <<= (16-Alpha - Rho);
    dU[1] <<= (16-Alpha - Rho);
    dV[0] <<= (16-Alpha - Rho);
    dV[1] <<= (16-Alpha - Rho);

    Uo  = ( Pts[0][0]   <<(16+ Accuracy)) + (1<<15);
    Vo  = ( Pts[0][1]   <<(16+ Accuracy)) + (1<<15);
    Uco = ((Pts[0][0]-1)<<(17+ Accuracy)) + (1<<17);
    Vco = ((Pts[0][1]-1)<<(17+ Accuracy)) + (1<<17);
    Uco = (Uco + dU[0] + dU[1])>>2;
    Vco = (Vco + dV[0] + dV[1])>>2;

    Predict_16x16 = Predict_16x16_C;
    Predict_8x8   = Predict_8x8_C;
  }
//  printf( "Lum=(%d %d) Chr=(%d %d)  dU:%d %d   dV:%d %d ", Uo, Vo, Uco, Vco, dU[0], dU[1], dV[0], dV[1] );
//  printf( "(Pts:%d=>%d, Acc:%d)\n", Nb, Nb_Pts, Accuracy );
}

void SKL_GMC_DSP::Get_Average_MV(int MV[2], int x, int y, int Quarter) const
{
  if (Nb_Pts>=2)
  {
    int vx = 0, vy = 0;

    SKL_INT32 uo = Uo + 16*(dU[1]*y + dU[0]*x);
    SKL_INT32 vo = Vo + 16*(dV[1]*y + dV[0]*x);
    for (int j=16; j>0; --j)
    {
      SKL_INT32 U = uo; uo += dU[1];
      SKL_INT32 V = vo; vo += dV[1];
      for (int i=16; i>0; --i)   
      {
        SKL_INT32 u = U >> 16, v = V >> 16;
        U += dU[0]; V += dV[0];

        vx += u; vy += v;
      }
    }
    vx -= (256*x+120) << (5+Accuracy);  // 120 = 15*16/2
    vy -= (256*y+120) << (5+Accuracy);

    MV[0] = RSHIFT( vx, 8+Accuracy-Quarter );
    MV[1] = RSHIFT( vy, 8+Accuracy-Quarter );
  }
  else {
    MV[0] = RSHIFT(Uo<<Quarter, 3);
    MV[1] = RSHIFT(Vo<<Quarter, 3);
  }
//  printf( "[%d] (%d,%d): %d %d\n", Nb_Pts, x,y, MV[0], MV[1]);
}

//////////////////////////////////////////////////////////

static SKL_GMC_DSP GMC_DSP_C = { 
  "GMC--C-", Skl_Switch_None,
  0,0 // sanity check
};

//////////////////////////////////////////////////////////
// MMX version

#ifdef SKL_USE_ASM

static SKL_GMC_DSP GMC_DSP_MMX = { 
  "GMC-MMX", Skl_Switch_MMX,
  Predict_16x16_C, Predict_8x8_C
};

#else 

#define GMC_DSP_MMX GMC_DSP_C

#endif

//////////////////////////////////////////////////////////
// Reference version == C-version

#define GMC_DSP_Ref GMC_DSP_C

//////////////////////////////////////////////////////////
// Initialization
//////////////////////////////////////////////////////////

int Skl_Init_GMC_DSP(SKL_GMC_DSP *Dsp, SKL_CPU_FEATURE Cpu)
{
  SKL_ASSERT(Dsp!=0);
  if (Cpu==SKL_CPU_DETECT) Cpu = Skl_Detect_CPU_Feature();

  if      (Cpu==SKL_CPU_C)    *Dsp = GMC_DSP_C;
  else if (Cpu==SKL_CPU_MMX)  *Dsp = GMC_DSP_MMX;
  else                        *Dsp = GMC_DSP_C;

  if (Dsp->Switch_Off==0) Dsp->Switch_Off = Skl_Get_Switch(Cpu);
  return 1;
}

//////////////////////////////////////////////////////////

} /* extern "C" */
