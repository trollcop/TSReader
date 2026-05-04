/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg4_anl.h
 *
 *  Default Built-In analyzer for MPEG4 encoder
 ********************************************************/

#include "skl_syst/skl_mpg4.h"
#include "skl_syst/skl_dsp.h"
#include "skl_syst/skl_bits.h"
#include "skl_syst/skl_exception.h"

#include "./skl_mpg_2pass.h"

extern "C" double sqrt(double);
extern "C" double fabs(double);
extern "C" double pow(double, double);

//////////////////////////////////////////////////////////
// Let's implement the interface
//////////////////////////////////////////////////////////

class SKL_MP4_ANALYZER_I : public SKL_MP4_ANALYZER
{
  protected:

    float Q_Cur, Q0, Q_Base;
    float MV_Search_Window, MV_Search_Window0;
    int Quant_Type;
    int Rounding;
    int Intra_Max, Intra_Count;
    int Inter_Coding_Threshold;
    int Use_Trellis;
    int Use_BFrame;
    int Interlace_Field, Interlace_DCT;
    int Reduced_Frame;
    int Frequency, Ticks_Per_Frame;    /* ticks per seconds/ticks per frame */
    int Intra_Limit;
    int SAD_Low_Limit, SAD_Hi_Limit;
    int Search_Metric;
    int Search_Method;
    int Luminance_Masking;
    float dQuant_Amp;
    int Inter4V_Probing, Field_Pred_Probing;
    int Hi_Mem;
    int Subpixel_Precision;
    int Bit_Rate;
    int Buffer_Size_Max;
    float Frame_Rate;
    int Verbose;
    float Lambda;     // scaling factor

    int GMC_Pts;
    int GMC_Mode;
    int GMC_Warps[4][2];
    int GMC_Accuracy;

          // 2-Pass
    SKL_MPG_2PASS Pass;

          // aux data for ME

    size_t    _Aux_Pic_Size;
    SKL_BYTE *_Aux_Data;

    void Alloc_Aux(size_t Pic_Size);
    void Free_Aux();

    SKL_BYTE *_HPels[4];
    void Set_Half_Pels(const SKL_MP4_INFOS * const Infos, const SKL_MP4_PIC * const Ref);

          // some stats

    int Nb_MV_Stat;
    int MVx_Mean_Stat, MVy_Mean_Stat;
    int MV_Max_Stat;
    int dMV_Max_Stat;
    int MVx_SSE_Stat, MVy_SSE_Stat;
    SKL_MV Last_Avrg_MV;
    double _mv, _dv;
    void Reset_Stats();
    void Add_To_Stats(const SKL_MV v);
    void Add_Max_MV_To_Stats(const SKL_MV v);
    void Compile_Stats();
    static void Store_Max(SKL_MV Max, const SKL_MV v, const SKL_MV Pred);
    static void Set_Max(SKL_MV Max, const SKL_MV v, const SKL_MV Pred);

  protected:

    void Scene_Changed();
    static void Reset_Frame(const SKL_MP4_PIC *Pic,
                            const SKL_MP4_INFOS *Infos);

    float Mask_Luminance(const SKL_MP4_INFOS * const Frame,
                         int For_BVOP) const;

    void Global_Pass(SKL_MP4_INFOS * const Frame);
    void Global_GMC_Pass(SKL_MP4_INFOS * const Frame);

  public:

    SKL_MP4_ANALYZER_I(SKL_MEM_I *Mem=0);
    virtual ~SKL_MP4_ANALYZER_I();

    virtual void Wake_Up(SKL_MP4_ANALYZER *Previous);
    virtual void Shut_Down();
    virtual int Analyze(SKL_MP4_INFOS * const Frame);
    virtual void Analyze_dQ(SKL_MP4_INFOS * const Frame, int For_BVOP);
    virtual void Post_Coding_Update(const SKL_MP4_INFOS * const Infos);
    virtual int Set_Param(const char * const Param, int Value);
    virtual int Get_Param(const char * const Param, int *Value) const;
    virtual int Set_Param(const char * const Param, float Value);
    virtual int Get_Param(const char * const Param, float *Value) const;
    virtual int Set_Param(const char * const Param, const char * const Value);
    virtual int Get_Param(const char * const Param, const char ** const Value) const;
    virtual const int *Get_Param(const char * const Param) const;
    virtual SKL_MEM_I *Set_Memory_Manager(SKL_MEM_I *New_Mem);
    virtual void Print_Caps(); // prints capabilities. 
};

//////////////////////////////////////////////////////////
