/********************************************************
 * Some code. Copyright (C) 2003 by Pascal Massimino.   *
 * All Rights Reserved.      (http://skal.planet-d.net) *
 * For Educational/Academic use ONLY. See 'LICENSE.TXT'.*
 ********************************************************/
/*
 * fake.h
 *
 * Test header for dynamic loading of objects.
 * tested in test2.cpp + fake.cpp (->fake.so)
 ********************************************************/

#ifndef _FAKE_H_
#define _FAKE_H_

//////////////////////////////////////////////////////////

// interface.
class FAKE {

  public:
    FAKE() {}
    virtual int Print() const;
    virtual ~FAKE();
};

//////////////////////////////////////////////////////////

#endif  /* _FAKE_H_ */
