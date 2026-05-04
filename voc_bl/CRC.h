// CRC.h: interface for the CCRC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CRC_H__FEABA917_4DB4_4D28_ACC3_88BBF0CDE77E__INCLUDED_)
#define AFX_CRC_H__FEABA917_4DB4_4D28_ACC3_88BBF0CDE77E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CCRC  
{
public:
	unsigned int Calc(unsigned char *b, unsigned int len);
	CCRC();
	virtual ~CCRC();

};

#endif // !defined(AFX_CRC_H__FEABA917_4DB4_4D28_ACC3_88BBF0CDE77E__INCLUDED_)
