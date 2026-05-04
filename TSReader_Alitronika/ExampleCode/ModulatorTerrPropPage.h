// ModulatorTerrPropPage.h: interface for the CModulatorTerrPropPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULATORTERRPROPPAGE_H__33D7AAD5_5542_4D1C_AB34_3B2C01B6119C__INCLUDED_)
#define AFX_MODULATORTERRPROPPAGE_H__33D7AAD5_5542_4D1C_AB34_3B2C01B6119C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LedControl.h"

class CModulatorTerrPropPage : public CPropertyPage  
{
	DECLARE_DYNCREATE(CModulatorTerrPropPage)

// constructor destructor
public:
	CModulatorTerrPropPage();
	virtual ~CModulatorTerrPropPage();

// Dialog Data
	//{{AFX_DATA(CModulatorTerrPropPage)
	enum { IDD = IDD_MOD_TERRMOD };
	CComboBox		m_SpecInvCtrl;
	CComboBox		m_TranModeCtrl;
	CComboBox		m_ModulationCtrl;
	CComboBox		m_GuartIntCtrl;
	CComboBox		m_HPFecCodeRateCtrl;
	CComboBox		m_BandWidthCtrl;
 	CLedControl		m_LedInSyncPcr;
 	CLedControl		m_Led204PckPcr;
 	CLedControl		m_LedTsOFlowPcr;
 	CLedControl		m_LedModUFlow;
	UINT			m_nCellID;
	BOOL			m_bDvbHEn;
	BOOL			m_bInDepthInt;
	BOOL			m_bMpeFec;
	BOOL			m_bTimeSlice;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CModulatorTerrPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void	OnInit();
// Implementation
protected:
	void	UpdateStatus();
	void	SetModulator();

	BOOL	m_bIgnoreTimer;

	int		m_iTranMode;
	int		m_iModulation;
	int		m_iGuardInt;
	int		m_iHPFecCodeRate;
	int		m_iBandWidth;
	int		m_iSpecInv;

	// Generated message map functions
	//{{AFX_MSG(CModulatorTerrPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyNow();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg	void OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized);
	afx_msg void OnCheckDvbhEn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MODULATORTERRPROPPAGE_H__33D7AAD5_5542_4D1C_AB34_3B2C01B6119C__INCLUDED_)
