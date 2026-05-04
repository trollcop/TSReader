/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * test of dynamic load
 *
 ********************************************************/

#include "skl_tester.h"
#include "skl_syst/skl_dyn_load.h"
#include "./fake.h"

#ifndef SKL_USE_DYN_LOAD
#include "./fake.cpp"
#endif

TEST_FUNC(Test_Dyn)
{
  SKL_DYN_LIBRARY( FAKE_LIB, "lib_so/fake", 0100 );
  SKL_DYN_OBJECT(FAKE, FAKE_LIB);
  FAKE *f = SKL_DYN_INSTANCE(FAKE, FAKE);
  CHECK(f->Print()==7349);
  FAKE *f2 = SKL_DYN_INSTANCE(FAKE, FAKE);
  CHECK(f2->Print()==7349+1);
  ::delete f;
  ::delete f2;
  SKL_DYN_LIBRARY_UNLOAD(FAKE_LIB);
}
END_FUNC

// --

int main(int argc, char *argv[])
{
  SKL_T_START;
    Test_Dyn();
  SKL_T_END;
  SKL_T_RETURN;
}

//////////////////////////////////////////////////////////

