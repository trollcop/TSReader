// ModulatorCablePropPage.h: interface for the CModulatorCablePropPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULATORCABLEPROPPAGE_H__DA64F9FC_B582_41EF_92EC_DDC2CEF7A308__INCLUDED_)
#define AFX_MODULATORCABLEPROPPAGE_H__DA64F9FC_B582_41EF_92EC_DDC2CEF7A308__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LedControl.h"

class CModulatorCablePropPage : public CPropertyPage  
{
	DECLARE_DYNCREATE(CModulatorCablePropPage)

// constructor destructor
public:
	CModulatorCablePropPage();
	virtual ~CModulatorCablePropPage();

// Dialog Data
	//{{AFX_DATA(CModulatorCablePropPage)
	enum { IDD = IDD_MOD_CABLEMOD };
	CComboBox		m_SpecInvCtrl;
	CComboBox		m_FltModeCtrl;
	CComboBox		m_InterLeavCtrl;
	CComboBox		m_QamModeCtrl;
 	CLedControl		m_LedInSyncPcr;
 	CLedControl		m_Led204PckPcr;
 	CLedControl		m_LedTsOFlowPcr;
	int				m_iDvbCModeSel;
	double			m_dSymbolRate;
	BOOL			m_bAnnexBEn;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CModulatorCablePropPage)
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
	void	SetJ83AnnexAMode();
	void	SetJ83AnnexBMode();
	void	SetJ83AnnexCMode();
	void	SetDVBCMode();
	void	SetFreeMode();
	void	RestoreSettings();
	void	StoreSettings();

	BOOL	m_bIgnoreTimer;

	int		m_iQamMode;
	int		m_iInterleaver;
	int		m_iFltRollOff;
	int		m_iSpecInv;

	double	m_dSymbolRateMin;
	double	m_dSymbolRateMax;

	// Generated message map functions
	//{{AFX_MSG(CModulatorCablePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnApplyNow();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg	void OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized);
	afx_msg void OnRadioDvbCSel();
	afx_msg void OnSelchangeComboQamMode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MODULATORCABLEPROPPAGE_H__DA64F9FC_B582_41EF_92EC_DDC2CEF7A308__INCLUDED_)
