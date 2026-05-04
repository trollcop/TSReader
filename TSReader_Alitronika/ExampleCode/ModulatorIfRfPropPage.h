// ModulatorIfRfPropPage.h: interface for the CModulatorIfRfPropPage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODULATORIFRFPROPPAGE_H__8039CEC0_7B11_4B30_8756_9E0CAEAE7D64__INCLUDED_)
#define AFX_MODULATORIFRFPROPPAGE_H__8039CEC0_7B11_4B30_8756_9E0CAEAE7D64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "LedControl.h"

class CModulatorIfRfPropPage : public CPropertyPage  
{

	DECLARE_DYNCREATE(CModulatorIfRfPropPage)

public:
	CModulatorIfRfPropPage();
	virtual ~CModulatorIfRfPropPage();

// Dialog Data
	//{{AFX_DATA(CModulatorIfRfPropPage)
	enum { IDD = IDD_MOD_RF_IF };
	CSpinButtonCtrl	m_RfSpinButCtrl;
	CLedControl		m_LedRfLocked;
	int				m_iRfIfSel;
	int				m_iIfFreqSel;
	double			m_dFreqIF;
	double			m_dFreqRF;
	double			m_dRFLevel;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CModulatorIfRfPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	void	OnInit();
protected:
	void	UpdateStatusLed();
	void	OnApplyNow();

	BOOL	m_bIgnoreTimer;

	enum ERfIfSel
	{
		SEL_IF,
		SEL_RF
	};

	enum EIfFreqSel
	{
		SEL_IF_36MHz,
		SEL_IF_70MHz,
		NUM_OF_IF_FREQS
	};

	struct SIfRangeList
	{
		EIfFreqSel 	IfFreqSel;
		double		dMinFreq;
		double		dMidFreq;
		double		dMaxFreq;
	};

	SIfRangeList *m_pIfRangeList;

	// clipping values for RF level edit boxes
	double	m_dRFLevelMin;
	double	m_dRFLevelMax;
	double	m_dRFLevelStepSize;
	
	// clipping values for RF and IF edit boxes
	double	m_dRFFreqMin;
	double	m_dRFFreqMax;
	double	m_dIFFreqMin;
	double	m_dIFFreqMax;
	// retrieved clipping values for RF edit boxes from modulator library
	double	m_dModRFFreqMin;
	double	m_dModRFFreqMax;
	
	// values that are saved in registry
	double	m_dStoreFreqIF[NUM_OF_IF_FREQS];	///< 36MHz value
	double	m_dStoreFreqRF;			
	double	m_dStoreRFLevel;
	int		m_iStoreRfIfSel;
	int		m_iStoreIfFreqSel;

	// Generated message map functions
	//{{AFX_MSG(CModulatorIfRfPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonApply();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnDestroy();
	afx_msg void OnDeltaposSpinRfLevel(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg	void OnActivate(UINT nState,CWnd* pWndOther,BOOL bMinimized);
	afx_msg void OnRadioIfrfSel();
	afx_msg void OnRadioIffreqSel();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_MODULATORIFRFPROPPAGE_H__8039CEC0_7B11_4B30_8756_9E0CAEAE7D64__INCLUDED_)
