/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * skl_mpg_2pass.h
 *
 *  MPEG4/H264 2-pass bitrate controler
 ********************************************************/

//////////////////////////////////////////////////////////
// 2-pass manager

struct SKL_FRAME_STATS {
  int Frame_Order;
  int Pic_Type;
  float Q, FCode, mMV, dMV;
  int Coded_Bytes, Txt_Bytes, MV_Bytes;
};

class SKL_MPG_2PASS
{
  public:
    int   _Type;   // 0:MPEG4 or 1:H264
    int   _Pass_Nb;
    int   _Frame_Cnt;
    float _Pass_RF;
    SKL_CST_STRING _Pass_File_Name;
    FILE *_Pass_File;
    int   _Desired_Bytes;
    int   _Missed_Bytes;
    float _Format_Version;

    int Open_Write_Pass_File(const int Width, const int Height);
    int Open_Read_Pass_File(const int Width, const int Height);

    int Set_Pass_File_Name(SKL_CST_STRING const Name);
    SKL_CST_STRING Get_Pass_File_Name() const { return _Pass_File_Name; }

    void Set_Pass_Nb(const int Nb) { _Pass_Nb = Nb; }
    int Get_Pass_Nb() const { return _Pass_Nb; }

    void Set_Pass_RF(const float RF) { _Pass_RF = RF; }
    float Get_Pass_RF() const { return _Pass_RF; }

    int Save_Frame_Params(const SKL_FRAME_STATS * const Stat);
    int Get_Next_Frame_Params(SKL_FRAME_STATS * const Stat);
    int Compute_New_Frame_Params(SKL_FRAME_STATS * const Stat);

    SKL_MPG_2PASS(const int File_Type);   // 0:MPEG4,  1:H264
    ~SKL_MPG_2PASS();
};

//////////////////////////////////////////////////////////
