/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * fake.cpp
 *
 * Test object for dynamic loading of objects.
 * tested in test_dyn.cpp
 * This can serve as a template for building a
 * dynamicaly-loaded class.
 ********************************************************/

#include "skl.h"
#include "skl_syst/skl_dyn_load.h"
#include "./fake.h"

//////////////////////////////////////////////////////////

int FAKE::Print() const {
  static int Cnt = 7349;
  printf("hello world!\n");
  return Cnt++;
}
FAKE::~FAKE() {}

//////////////////////////////////////////////////////////

// -- external symbol exported
SKL_DFLT_DYN_FACTORY(FAKE);
 
//////////////////////////////////////////////////////////
