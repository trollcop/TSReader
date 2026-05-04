/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * bitmap tests I
 *
 ********************************************************/

#include "skl_tester.h"
#include "skl_2d/skl_window.h"
#include "skl_syst/skl_random.h"
#include "skl_2d/skl_pt.h"

SKL_MEM_I Mem;

//////////////////////////////////////////////////////////
// -- basic color conversion

static void Loop_Col(const SKL_FORMAT_SHIFT &fmts) {
  static SKL_RANDOM r(7423);
  fmts.Print_Infos();
  for( int i=0; i<5; ++i ) {
    SKL_UINT32 Col = r.Get_Int( fmts.All_Mask() );
    SKL_ARGB   Col2 = fmts.UnpackA(Col);
    SKL_UINT32 Col3 = fmts.PackA(Col2);
    CHECK(Col==Col3);
    //printf("Col:0x%.8x -> 0x%.8x -> 0x%.8x\n", Col, Col2, Col3 );
  }
}

TEST_FUNC(Test_Col1) 
{
  SKL_FORMAT fmt1(0x20565);
  fmt1.Print_Infos();
  SKL_FORMAT_SHIFT fmts1(fmt1);
  fmts1.Print_Infos();

  SKL_FORMAT fmt2(0x40777);
  fmt2.Print_Infos();
  SKL_FORMAT_SHIFT fmts2(fmt2);
  fmts2.Print_Infos();

  SKL_FORMAT fmt3(0x48647);
  fmt3.Print_Infos();
  SKL_FORMAT_SHIFT fmts3(fmt3);
  fmts3.Print_Infos();

  SKL_UINT32 Col = 0x00784265;
  SKL_ARGB   Col2 = fmts1.Unpack(Col);
  SKL_UINT32 Col3 = fmts1.Pack(Col2);
  CHECK(Col>=Col3);
//  printf("Col:0x%x -> 0x%x -> 0x%x\n", Col, Col2, Col3 );
  fmts1.Print_Col( Col2 );

  Loop_Col( SKL_FORMAT_SHIFT(SKL_FORMAT(0x20555)) );
  Loop_Col( SKL_FORMAT_SHIFT(SKL_FORMAT(0x47657)) );
  Loop_Col( SKL_FORMAT_SHIFT(SKL_FORMAT(0x10332)) );
  Loop_Col( SKL_FORMAT_SHIFT(SKL_FORMAT(0x48878)) );
}
END_FUNC

TEST_FUNC(Test_Col2) 
{
  SKL_COLOR Col1(0x84214623);
  SKL_COLOR Col2(Col1);
  SKL_ARGB c = (SKL_ARGB)Col1;
  printf( "ARGB c=0x%.8x\n", c );

  SKL_COLOR Col3(0x64f23532);
  for( int i=0; i<6; ++i ) {
    float x = 1.0f*i/5;
    SKL_COLOR Col4 = Col3.Mix(Col2, x);
    printf( "x=%f -> %d,%d,%d\n", x, Col4.R(), Col4.G(), Col4.B() );
  }  
}
END_FUNC

//////////////////////////////////////////////////////////
// -- CMap test

TEST_FUNC(Test_CMap1) 
{
  SKL_CMAP_X CMap1(256);
  printf( "CMap1 -> Nb=%d\n", CMap1.Get_Nb_Colors() );
  CHECK(CMap1.Get_Nb_Colors()==256);
  CMap1.Ramp( SKL_COLOR(32,65,100), SKL_COLOR(246,162,187), 10, 31 );

  int i;

  printf( "   CMAP_X #1:\n" );
  for( i=0; i<=30; i+=5 )
    printf( "#%d -> 0x%.6x\n", i, (SKL_UINT32)CMap1[i] );

  printf( "   -> CMAP #1 (RGB 0x20547):\n" );
  SKL_CMAP CMap2(CMap1, 0x20547 );
  CHECK((SKL_ANY)CMap1.Get_Colors()!=(SKL_ANY)CMap2.Get_Colors());
  for( i=0; i<=30; i+=5 )
    printf( "#%d -> 0x%.4x\n", i, CMap2[i] );

}
END_FUNC

//////////////////////////////////////////////////////////
// -- bitmaps

TEST_FUNC(Test_Btm1)
{
  SKL_BTM vbuf1(&Mem);
  SKL_CMAP cmap(0x20565,256);
  cmap.Ramp( SKL_COLOR(0,255,0), SKL_COLOR(0,0,255), 0, 256 );
  vbuf1.Set_CMap(cmap);
  SKL_CMAP_X *cmap_x = new (&Mem) SKL_CMAP_X(cmap);
  vbuf1.Set_CMap(cmap_x);
  int i=0;
  while( 1 ) {
    if (i>255) i = 255;
    CHECK(vbuf1.Get_CMap()[i]==(*cmap_x)[i]);
    printf( "#%d -> rgb888=0x%.6x \t rgb565 = 0x%.4x\n", i, (int)(*cmap_x)[i], (int)cmap[i] );
    if (i==255) break;
    i += 3;
  }
  delete cmap_x;

  CHECK(vbuf1.Width()==0);
  CHECK(vbuf1.Height()==0);
  CHECK(vbuf1.BpS()==0);
  CHECK(vbuf1.Format()==SKL_FORMAT::DFLT);
  vbuf1.Set(320,200,0x2033a);
  CHECK(vbuf1.Width()==320);
  CHECK(vbuf1.Height()==200);
  CHECK(vbuf1.BpS()>=320*2);
  CHECK(vbuf1.Format()==0x2033a);
}
END_FUNC

TEST_FUNC(Test_Btm2)   // test of copies
{
  SKL_BTM *vbuf0 = new (&Mem) SKL_BTM(&Mem);
  vbuf0->Set(320,200,0x10001);
  CHECK(vbuf0->Lock()!=0);
  CHECK(vbuf0->Owns_CMap()==0);
  CHECK(vbuf0->Get_CMap_Ptr()==0);
  CHECK(vbuf0->Is_Virtual()==0);
  CHECK(vbuf0->Format()==0x10001);
  delete vbuf0;

  vbuf0 = new (&Mem) SKL_BTM(&Mem);
  vbuf0->Set_Virtual(320,200,0x10002);
  CHECK(vbuf0->Lock()==0);
  CHECK(vbuf0->Owns_CMap()==0);
  CHECK(vbuf0->Get_CMap_Ptr()==0);
  CHECK(vbuf0->Is_Virtual());
  CHECK(vbuf0->Format()==0x10002);
  delete vbuf0;

  SKL_BTM vbuf1(&Mem);
  vbuf1.Set(320,200, 0x10000, 0, (SKL_CMAP_X*)1);
  CHECK(vbuf1.Width()==320);
  CHECK(vbuf1.Height()==200);
  CHECK(vbuf1.Format()==0x10000);
  CHECK(vbuf1.Get_CMap_Ptr()==(SKL_CMAP_X*)1);
  CHECK(vbuf1.Owns_CMap()==0);
  CHECK(vbuf1.Has_CMap()!=0);
  CHECK(vbuf1.Is_Virtual()==0);

  vbuf1.New_CMap();
  CHECK(vbuf1.Get_CMap_Ptr()!=0);
  CHECK(vbuf1.Has_CMap()!=0);

  SKL_BTM vbuf2(&Mem);
  vbuf2.Make_Virtual_Copy( &vbuf1 );
  CHECK(vbuf2.Is_Virtual()!=0);
  CHECK(vbuf2.Owns_CMap()==0);
  CHECK( vbuf2.Get_CMap_Ptr()==vbuf1.Get_CMap_Ptr() );

  SKL_BTM vbuf3(&Mem);
  vbuf3.Make_Copy( &vbuf1 );
  CHECK(vbuf3.Is_Virtual()==0);
  CHECK(vbuf3.Owns_CMap()!=0);
  CHECK( vbuf3.Get_CMap_Ptr()!=vbuf1.Get_CMap_Ptr() );
}
END_FUNC

// -- 

TEST_FUNC(Test_2D_Pt) {
  SKL_2D_PT Pt1;
  CHECKI( Pt1.x, 0);
  CHECKI( Pt1.y, 0);
  Pt1 += SKL_2D_PT(1,1);
  CHECKI( Pt1.x, 1);
  CHECKI( Pt1.y, 1);
  SKL_2D_PT Pt2(Pt1);
  CHECKI( Pt2.x, 1);
  CHECKI( Pt2.y, 1);
  SKL_2D_PT Pt3;
  Pt3 = Pt2;
  CHECKI( Pt3.x, 1);
  CHECKI( Pt3.y, 1);
  CHECK(Pt3==Pt2);
  Pt3 -= Pt3;
  CHECKI( Pt3.x, 0);
  CHECKI( Pt3.y, 0);

  SKL_2D_PT P1(12, 15);
  SKL_2D_PT P2(13, 16);
  SKL_2D_PT P3(13, 15);
  SKL_2D_PT P4(12, 17);   CHECK( P4 == SKL_2D_PT(12,17));
  CHECK( P2>P1 );
  CHECK( P3>P1 );
  CHECK( P4>P1 );
  CHECK( P2>=P1 );
  CHECK( P3>=P1 );
  CHECK( P4>=P1 );
  CHECK( P3<P2 );
  CHECK( P3<=P2 );

  CHECK( P1<=P2 );
  CHECK( P1<=P3 );
  CHECK( P1<=P4 );
  CHECK( P1<P2 );
  CHECK( P1<P3 );
  CHECK( P1<P4 );
  CHECK( P2>P3 );
  CHECK( P2>=P3 );

}
END_FUNC

// -- 

int main(int argc, char *argv[])
{
  SHOW_FLT_ERROR_OFF;

  SKL_T_START;
    SKL_TEST(1,Test_CMap1);
    SKL_TEST(2,Test_Col1);
    SKL_TEST(3,Test_Col2);
    SKL_TEST(4,Test_Btm1);
    SKL_TEST(5,Test_Btm2);
    SKL_TEST(6,Test_2D_Pt);
  SKL_T_END;

  SKL_T_RETURN;
}

//////////////////////////////////////////////////////////
