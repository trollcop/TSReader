// ModulatorIfRfPropPage.cpp: implementation of the CModulatorIfRfPropPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbrecorder.h"
#include "ModulatorIfRfPropPage.h"
#include "frontend.h"
#include "ATBoard.h"
#include "CurrBoard.h"

class CDVBRecorderApp;
extern CDVBRecorderApp theApp;
const char strRfIfKey[] = "Modulators";
const char strEntry[] = "RfIf";
#define IDT_UPDATE	3

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CModulatorIfRfPropPage, CPropertyPage)

CModulatorIfRfPropPage::CModulatorIfRfPropPage() : CPropertyPage(CModulatorIfRfPropPage::IDD)
{
	//{{AFX_DATA_INIT(CModulatorIfRfPropPage)
	//}}AFX_DATA_INIT
	m_bIgnoreTimer = FALSE;
	m_pIfRangeList = NULL;
}

CModulatorIfRfPropPage::~CModulatorIfRfPropPage()
{
	m_bIgnoreTimer = TRUE;
	
	if (m_hWnd)
		KillTimer(IDT_UPDATE);

	if (m_pIfRangeList)
		delete m_pIfRangeList;
}

void CModulatorIfRfPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModulatorIfRfPropPage)
	DDX_Control(pDX, IDC_SPIN_RF_LEVEL, m_RfSpinButCtrl);
	DDX_Control(pDX, IDC_SIG_LED_RF_LOCKED, m_LedRfLocked);
	DDX_Text(pDX, IDC_EDIT_RF_LEVEL, m_dRFLevel);
	DDX_Radio(pDX, IDC_RADIO_IFRF_SEL1, m_iRfIfSel);
	DDX_Radio(pDX, IDC_RADIO_IFFREQ_SEL1, m_iIfFreqSel);
	DDX_Text(pDX, IDC_EDIT_IF_FREQ, m_dFreqIF);
	DDX_Text(pDX, IDC_EDIT_RF_FREQ, m_dFreqRF);
	//}}AFX_DATA_MAP
	DDV_MinMaxFloat(pDX, (float)m_dFreqRF, (float)m_dRFFreqMin, (float)m_dRFFreqMax);
	DDV_MinMaxFloat(pDX, (float)m_dFreqIF, (float)m_dIFFreqMin, (float)m_dIFFreqMax);
	DDV_MinMaxFloat(pDX, (float)m_dRFLevel, (float)m_dRFLevelMin, (float)m_dRFLevelMax);
}

BEGIN_MESSAGE_MAP(CModulatorIfRfPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CModulatorIfRfPropPage)
	ON_WM_SHOWWINDOW()
	ON_WM_DESTROY()
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_RF_LEVEL, OnDeltaposSpinRfLevel)
	ON_BN_CLICKED(IDC_RADIO_IFRF_SEL1, OnRadioIfrfSel)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, OnButtonApply)
	ON_BN_CLICKED(IDC_RADIO_IFFREQ_SEL1, OnRadioIffreqSel)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RADIO_IFRF_SEL2, OnRadioIfrfSel)
	ON_BN_CLICKED(IDC_RADIO_IFFREQ_SEL2, OnRadioIffreqSel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageCableTuner message handlers
BOOL CModulatorIfRfPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// set leds to default
	m_LedRfLocked.Set(CLedControl::LED_RED);

	// set rf level range
	m_RfSpinButCtrl.SetRange((s16)(m_dRFLevelMin/m_dRFLevelStepSize), (s16)(m_dRFLevelMax/m_dRFLevelStepSize));

	UpdateData(FALSE);

	OnRadioIfrfSel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModulatorIfRfPropPage::OnInit()
{
	// get device information
	CCurrBoard CurrBoard;
	if (CurrBoard.IsValid())
	{
		CATBoard &TheBoard = CurrBoard;
		dvb_frontend_info DvbFrontInfo;
		TheBoard.DvbSource(SRC_GET_INFO, &DvbFrontInfo);

		m_dModRFFreqMin		= DvbFrontInfo.frequency_min/1000000.0;
		m_dModRFFreqMax		= DvbFrontInfo.frequency_max/1000000.0;
		m_dRFLevelMin		= DvbFrontInfo.rflevel_min/10.0;
		m_dRFLevelMax		= DvbFrontInfo.rflevel_max/10.0;
		m_dRFLevelStepSize	= DvbFrontInfo.rflevel_stepsize/10.0;

		m_pIfRangeList		= new SIfRangeList[DvbFrontInfo.nrIffreqs];
		for (int i = 0; i < DvbFrontInfo.nrIffreqs; i++)
		{
			m_pIfRangeList[i].IfFreqSel	= (EIfFreqSel)i;
			m_pIfRangeList[i].dMinFreq	= (double)DvbFrontInfo.iffreqs[i]-(DvbFrontInfo.ifrange/2.0);
			m_pIfRangeList[i].dMidFreq	= (double)DvbFrontInfo.iffreqs[i];
			m_pIfRangeList[i].dMaxFreq	= (double)DvbFrontInfo.iffreqs[i]+(DvbFrontInfo.ifrange/2.0);
			m_pIfRangeList[i].dMinFreq	/= 1000000.0;
			m_pIfRangeList[i].dMidFreq	/= 1000000.0;
			m_pIfRangeList[i].dMaxFreq	/= 1000000.0;
		}
	}
	else
	{
		m_dModRFFreqMin		= 140.0;
		m_dModRFFreqMax		= 1000.0;
		m_dRFLevelMin		= -30.0;
		m_dRFLevelMax		= 10.0;
		m_dRFLevelStepSize	= 0.5;

		m_pIfRangeList				= new SIfRangeList[NUM_OF_IF_FREQS];
		m_pIfRangeList[0].IfFreqSel	= SEL_IF_36MHz;
		m_pIfRangeList[0].dMinFreq	= 35.0;
		m_pIfRangeList[0].dMidFreq	= 36.0;
		m_pIfRangeList[0].dMaxFreq	= 37.0;
		m_pIfRangeList[0].IfFreqSel	= SEL_IF_70MHz;
		m_pIfRangeList[0].dMinFreq	= 69.0;
		m_pIfRangeList[0].dMidFreq	= 70.0;
		m_pIfRangeList[0].dMaxFreq	= 71.0;
	}

	// get last settings from registry
	CString Str;
	Str = theApp.GetProfileString(strRfIfKey, strEntry);

	if (!(sscanf(Str, "%lg,%lg,%lg,%lg,%i,%i", &m_dStoreFreqIF[SEL_IF_36MHz],
											   &m_dStoreFreqIF[SEL_IF_70MHz],
											   &m_dStoreFreqRF,
											   &m_dStoreRFLevel,
											   &m_iStoreRfIfSel,
											   &m_iStoreIfFreqSel) > 0))
	{
 		m_dStoreFreqIF[SEL_IF_36MHz]	= m_pIfRangeList[SEL_IF_36MHz].dMidFreq;
 		m_dStoreFreqIF[SEL_IF_70MHz]	= m_pIfRangeList[SEL_IF_70MHz].dMidFreq;
 		m_dStoreFreqRF					= m_dModRFFreqMin;
 		m_dStoreRFLevel					= m_dRFLevelMin;
 		m_iStoreRfIfSel					= SEL_IF;
 		m_iStoreIfFreqSel				= SEL_IF_70MHz;
	}
	
	m_iRfIfSel		= m_iStoreRfIfSel;
	m_iIfFreqSel	= m_iStoreIfFreqSel;
	m_dRFLevel		= m_dStoreRFLevel;

	if (m_iRfIfSel == SEL_IF)
	{
		m_dFreqRF		= 0.0;
		m_dRFFreqMin	= -3000.0;
		m_dRFFreqMax	= +3000.0;
		m_dFreqIF		= m_dStoreFreqIF[m_iIfFreqSel];
		m_dIFFreqMin	= m_pIfRangeList[m_iIfFreqSel].dMinFreq;
		m_dIFFreqMax	= m_pIfRangeList[m_iIfFreqSel].dMaxFreq;
	}
	else
	{
		m_dFreqRF		= m_dStoreFreqRF;
		m_dRFFreqMin	= m_dModRFFreqMin;
		m_dRFFreqMax	= m_dModRFFreqMax;
		m_dFreqIF		= m_pIfRangeList[SEL_IF_70MHz].dMidFreq;
		m_dIFFreqMin	= -500.0;
		m_dIFFreqMax	= +500.0;
	}

	OnApplyNow();
}

void CModulatorIfRfPropPage::OnDestroy() 
{
	m_bIgnoreTimer = TRUE;
	KillTimer(IDT_UPDATE);
	CPropertyPage::OnDestroy();
}

void CModulatorIfRfPropPage::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	if (bShow)
	{
		UpdateStatusLed();
		SetTimer(IDT_UPDATE, 200, NULL);
	}
	else
	{
		KillTimer(IDT_UPDATE);
	}
}

void CModulatorIfRfPropPage::OnTimer(UINT nIDEvent) 
{
	if(!m_bIgnoreTimer)
	{
		if (nIDEvent==IDT_UPDATE && IsWindowVisible())
			UpdateStatusLed();

		CCurrBoard CurrBoard;
		CATBoard &TheBoard = CurrBoard;

		if (TheBoard.GetDeviceEnable())
			CPropertyPage::OnTimer(nIDEvent);
	}
}

void CModulatorIfRfPropPage::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	if (nState == WA_INACTIVE)
	{
		m_bIgnoreTimer = TRUE;
		KillTimer(IDT_UPDATE);
	}
	else if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
	{
		m_bIgnoreTimer = FALSE;
		SetTimer(IDT_UPDATE, 200, NULL);
	}	
	CPropertyPage::OnActivate(nState, pWndOther, bMinimized);
}

void CModulatorIfRfPropPage::OnDeltaposSpinRfLevel(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	// TODO: Add your control notification handler code here

	m_dRFLevel += pNMUpDown->iDelta*m_dRFLevelStepSize;
	if (m_dRFLevel < m_dRFLevelMin)
		m_dRFLevel = m_dRFLevelMin;
	if (m_dRFLevel > m_dRFLevelMax)
		m_dRFLevel = m_dRFLevelMax;
	*pResult = 0;

	UpdateData(FALSE);
}

void CModulatorIfRfPropPage::OnRadioIfrfSel() 
{
	UpdateData(TRUE);

	switch (m_iRfIfSel)
	{
	case SEL_IF:
		GetDlgItem(IDC_RADIO_IFFREQ_SEL1)->EnableWindow(TRUE);
		GetDlgItem(IDC_RADIO_IFFREQ_SEL2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_IF_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC1)->EnableWindow(TRUE);
		GetDlgItem(IDC_SPIN_RF_LEVEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RF_LEVEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_RF_FREQ)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC3)->EnableWindow(FALSE);
		m_dFreqIF		= m_dStoreFreqIF[m_iStoreIfFreqSel];
		m_dIFFreqMin	= m_pIfRangeList[m_iStoreIfFreqSel].dMinFreq;
		m_dIFFreqMax	= m_pIfRangeList[m_iStoreIfFreqSel].dMaxFreq;
		m_iIfFreqSel	= m_iStoreIfFreqSel;
		m_dFreqRF		= 0.0;
		m_dRFFreqMin	= -3000.0;
		m_dRFFreqMax	= +3000.0;
		break;
	case SEL_RF:
		GetDlgItem(IDC_RADIO_IFFREQ_SEL1)->EnableWindow(FALSE);
		GetDlgItem(IDC_RADIO_IFFREQ_SEL2)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_IF_FREQ)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC1)->EnableWindow(FALSE);
		GetDlgItem(IDC_SPIN_RF_LEVEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RF_LEVEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC2)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_RF_FREQ)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC3)->EnableWindow(TRUE);
		m_dFreqIF		= m_pIfRangeList[SEL_IF_70MHz].dMidFreq;
		m_dIFFreqMin	= -500.0;
		m_dIFFreqMax	= +500.0;
		m_iIfFreqSel	= SEL_IF_70MHz; 
		m_dFreqRF		= m_dStoreFreqRF;
		m_dRFFreqMin	= m_dModRFFreqMin;
		m_dRFFreqMax	= m_dModRFFreqMax;
		break;
	}

	UpdateData(FALSE);
}

void CModulatorIfRfPropPage::OnRadioIffreqSel() 
{
	UpdateData(TRUE);

	m_dFreqIF		= m_dStoreFreqIF[m_iIfFreqSel];
	m_dIFFreqMin	= m_pIfRangeList[m_iIfFreqSel].dMinFreq;
	m_dIFFreqMax	= m_pIfRangeList[m_iIfFreqSel].dMaxFreq;

	UpdateData(FALSE);
}

void CModulatorIfRfPropPage::OnButtonApply() 
{
	UpdateData(TRUE);
	OnApplyNow();
 	UpdateData(FALSE);
}

void CModulatorIfRfPropPage::OnApplyNow()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	dvb_if_rf_parameters IfRfParams;

	// set RF and IF parameters
	if (m_iRfIfSel)
		IfRfParams.RfIfSel	 = TRUE;
	else
		IfRfParams.RfIfSel	 = FALSE;
	IfRfParams.IfFrequency	 = (s32)(m_dFreqIF*1000000.0);
	IfRfParams.RfFrequency	 = (u32)(m_dFreqRF*1000000.0);
	IfRfParams.RfoutputLevel = (s32)(m_dRFLevel*10);

	// send parameter list
	TheBoard.DvbSource(SRC_SET_MOD_IF_RF_PARAMS, &IfRfParams);

	CString Str;
	Str = theApp.GetProfileString(strRfIfKey, strEntry);

	// set register values that need to be stored
	m_iStoreRfIfSel = m_iRfIfSel;

	switch (m_iRfIfSel)
	{
	case SEL_IF:
		m_iStoreIfFreqSel			 = m_iIfFreqSel;
		m_dStoreFreqIF[m_iIfFreqSel] = m_dFreqIF;
		break;
	case SEL_RF:
		m_dStoreFreqRF	= m_dFreqRF;
		m_dStoreRFLevel = m_dRFLevel;
		break;
	}

	Str.Format("%.6f,%.6f,%.6f,%.6f,%i,%i",
				m_dStoreFreqIF[SEL_IF_36MHz],
				m_dStoreFreqIF[SEL_IF_70MHz],
				m_dStoreFreqRF,
				m_dStoreRFLevel,
				m_iStoreRfIfSel,
				m_iStoreIfFreqSel
				);

	theApp.WriteProfileString(strRfIfKey, strEntry, Str);

	// update recalculated RF or IF frequency
	TheBoard.DvbSource(SRC_GET_MOD_IF_RF_STAT, &IfRfParams);

	switch (m_iRfIfSel)
	{
	case SEL_IF:
		m_dFreqRF = 0.0;
		break;
	case SEL_RF:
		m_dFreqIF = IfRfParams.IfFrequency/1000000.0;
		break;
	}
}

void CModulatorIfRfPropPage::UpdateStatusLed()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	dvb_if_rf_parameters IfRfParams;

	if (m_bIgnoreTimer)
		return;

	TheBoard.DvbSource(SRC_GET_MOD_IF_RF_STAT, &IfRfParams);

	// set rf locked led
	if (m_iRfIfSel == SEL_IF)
		m_LedRfLocked.Set(CLedControl::LED_GRAY);
	else
	if (IfRfParams.RfLocked)
		m_LedRfLocked.Set(CLedControl::LED_GREEN);
	else
		m_LedRfLocked.Set(CLedControl::LED_RED);
}

