// ModulatorCablePropPage.cpp: implementation of the CModulatorCablePropPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbrecorder.h"
#include "ModulatorCablePropPage.h"
#include "frontend.h"
#include "ATBoard.h"
#include "CurrBoard.h"

class CDVBRecorderApp;
extern CDVBRecorderApp theApp;
const char strCableModKey[]		= "Modulators";
const char strEntryModeSel[]	= "ModulationSel";
const char strEntryJ83A[]		= "J83A_Modulation";
const char strEntryJ83B[]		= "J83B_Modulation";
const char strEntryJ83C[]		= "J83C_Modulation";
const char strEntryDVBC[]		= "DVBC_Modulation";
const char strEntryFree[]		= "FREE_Modulation";
#define IDT_UPDATE	3

struct SCableModDefaults
{
	int		m_iDfltQamMode;
	int		m_iDfltInterLeav;
	int		m_iDfltFltRollOff;
	int		m_iDfltSpecInv;
	double	m_dDfltSymbRate;
	BOOL	m_bDfltAnnexB;
};

const SCableModDefaults cCableModDefaults[5] = 
{	
	//	QamMode,	InterLeav,			FltRollOff,		SpecInv,		SymbRate,	AnnexB
	{	QAM_16,		INTLEAV_I12_J17,	FLT_ALPHA_15,	INVERSION_OFF,	6.0,		FALSE	},	//	J83 annex A
	{	QAM_64,		INTLEAV_I128_J1,	FLT_ALPHA_18,	INVERSION_OFF,	5.056941,	TRUE	},	//	J83 annex B
	{	QAM_64,		INTLEAV_I12_J17,	FLT_ALPHA_13,	INVERSION_OFF,	6.0,		FALSE	},	//	J83 annex C
	{	QAM_16,		INTLEAV_I12_J17,	FLT_ALPHA_15,	INVERSION_OFF,	6.0,		FALSE	},	//	DVB-C
	{	QAM_16,		INTLEAV_I128_J1,	FLT_ALPHA_12,	INVERSION_OFF,	6.0,		FALSE	}	//	free
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CModulatorCablePropPage, CPropertyPage)

CModulatorCablePropPage::CModulatorCablePropPage() : CPropertyPage(CModulatorCablePropPage::IDD)
{
	//{{AFX_DATA_INIT(CModulatorCablePropPage)
	//}}AFX_DATA_INIT
	m_bIgnoreTimer = FALSE;
	m_dSymbolRateMin = 0.0;
	m_dSymbolRateMax = 10.0;
}

CModulatorCablePropPage::~CModulatorCablePropPage()
{
	m_bIgnoreTimer = TRUE;
	
	if (m_hWnd)
		KillTimer(IDT_UPDATE);

}

void CModulatorCablePropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModulatorCablePropPage)
	DDX_Control(pDX, IDC_COMBO_INVERSION, m_SpecInvCtrl);
	DDX_Control(pDX, IDC_COMBO_FLT_MODE, m_FltModeCtrl);
	DDX_Control(pDX, IDC_COMBO_INTLEAV_MODE, m_InterLeavCtrl);
	DDX_Control(pDX, IDC_COMBO_QAM_MODE, m_QamModeCtrl);
	DDX_Control(pDX, IDC_SIG_LED_IN_SYNC_PCR, m_LedInSyncPcr);
	DDX_Control(pDX, IDC_SIG_LED_204PCK_PCR, m_Led204PckPcr);
	DDX_Control(pDX, IDC_SIG_LED_TS_OFLOW_PCR, m_LedTsOFlowPcr);
	DDX_Radio(pDX, IDC_RADIO_DVB_SEL1, m_iDvbCModeSel);
	DDX_Text(pDX, IDC_EDIT_SYMBOL_RATE, m_dSymbolRate);
	DDX_Check(pDX, IDC_CHECK_ANNEXB_EN, m_bAnnexBEn);
	//}}AFX_DATA_MAP
	DDV_MinMaxFloat(pDX, (float)m_dSymbolRate, (float)m_dSymbolRateMin, (float)m_dSymbolRateMax);
}

BEGIN_MESSAGE_MAP(CModulatorCablePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CModulatorCablePropPage)
	ON_BN_CLICKED(ID_APPLY_NOW, OnApplyNow)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO_DVB_SEL1, OnRadioDvbCSel)
	ON_BN_CLICKED(IDC_RADIO_DVB_SEL2, OnRadioDvbCSel)
	ON_BN_CLICKED(IDC_RADIO_DVB_SEL3, OnRadioDvbCSel)
	ON_BN_CLICKED(IDC_RADIO_DVB_SEL4, OnRadioDvbCSel)
	ON_BN_CLICKED(IDC_RADIO_DVB_SEL5, OnRadioDvbCSel)
	ON_CBN_SELCHANGE(IDC_COMBO_QAM_MODE, OnSelchangeComboQamMode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModulatorCablePropPage message handlers
BOOL CModulatorCablePropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// set leds to default
	m_LedInSyncPcr.Set(CLedControl::LED_RED);
	m_Led204PckPcr.Set(CLedControl::LED_RED);
	m_LedTsOFlowPcr.Set(CLedControl::LED_RED);

	OnRadioDvbCSel();
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModulatorCablePropPage::OnInit()
{
	// get device information
	CCurrBoard CurrBoard;
	if (CurrBoard.IsValid())
	{
		CATBoard &TheBoard = CurrBoard;
		TheBoard.DvbSource(SRC_INIT, NULL);
	}
		
	CString Str;

	// get last mode used
	Str = theApp.GetProfileString(strCableModKey, strEntryModeSel);

	if(!(sscanf(Str, "%d", &m_iDvbCModeSel) > 0))
	{
		m_iDvbCModeSel = 0;		// J83 annexA standart
	}

	// get last DVB cable mode
	RestoreSettings();

	SetModulator();
}

void CModulatorCablePropPage::OnDestroy() 
{
	m_bIgnoreTimer = TRUE;
	KillTimer(IDT_UPDATE);
	CPropertyPage::OnDestroy();
}

void CModulatorCablePropPage::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	if (bShow)
	{
		UpdateStatus();
		SetTimer(IDT_UPDATE, 200, NULL);
	}
	else
	{
		KillTimer(IDT_UPDATE);
	}
}

void CModulatorCablePropPage::OnTimer(UINT nIDEvent) 
{
	if(!m_bIgnoreTimer)
	{
		if (nIDEvent==IDT_UPDATE && IsWindowVisible())
			UpdateStatus();

		CCurrBoard CurrBoard;
		CATBoard &TheBoard = CurrBoard;

		if (TheBoard.GetDeviceEnable())
			CPropertyPage::OnTimer(nIDEvent);
	}
}

void CModulatorCablePropPage::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
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

void CModulatorCablePropPage::OnApplyNow() 
{
	UpdateData(TRUE);

	m_iQamMode		= m_QamModeCtrl.GetItemData(m_QamModeCtrl.GetCurSel());
	m_iFltRollOff	= m_FltModeCtrl.GetItemData(m_FltModeCtrl.GetCurSel());
	m_iInterleaver	= m_InterLeavCtrl.GetItemData(m_InterLeavCtrl.GetCurSel());
	m_iSpecInv		= m_SpecInvCtrl.GetItemData(m_SpecInvCtrl.GetCurSel());

	SetModulator();
}

void CModulatorCablePropPage::SetModulator()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	dvb_at2700_parameters DvbCParams;

	DvbCParams.modulation			= (src_modulation_t)m_iQamMode;
	DvbCParams.filter_rolloff		= (src_filter_rolloff_t)m_iFltRollOff;
	DvbCParams.interleaver			= (src_interleaver_t)m_iInterleaver;
	DvbCParams.inversion			= (src_spectral_inversion_t)m_iSpecInv;
	DvbCParams.symbol_rate			= (u32)(m_dSymbolRate*1000000.0);
	if(m_bAnnexBEn)
		DvbCParams.annexb_enable	= TRUE;
	else
		DvbCParams.annexb_enable	= FALSE;

	if (!m_bIgnoreTimer)
		TheBoard.DvbSource(SRC_SET_MOD_PARAMS, &DvbCParams);

	StoreSettings();
}

void CModulatorCablePropPage::UpdateStatus()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	src_status_at2700_t eStatus;

	if (!m_bIgnoreTimer)
		TheBoard.DvbSource(SRC_GET_MOD_STAT, &eStatus);

 	if (eStatus & DST_HAS_PCR_TS_INSYNC_CH0)
 		m_LedInSyncPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_LedInSyncPcr.Set(CLedControl::LED_RED);

 	if (eStatus & DST_HAS_PCR_TS_204PKT_CH0)
 		m_Led204PckPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_Led204PckPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_PCR_TS_OFLOW_CH0)
 		m_LedTsOFlowPcr.Set(CLedControl::LED_RED);
 	else
 		m_LedTsOFlowPcr.Set(CLedControl::LED_GRAY);
}

void CModulatorCablePropPage::OnRadioDvbCSel() 
{
	UpdateData(TRUE);
	RestoreSettings();

	switch (m_iDvbCModeSel)
	{
	case 0:			// J83 annex A
		SetJ83AnnexAMode();
		break;
	case 1:			// J83 annex B
		SetJ83AnnexBMode();
		break;
	case 2:			// J83 annex C
		SetJ83AnnexCMode();
		break;
	case 3:			// DVB-C
		SetDVBCMode();
		break;
	case 4:			// free mode
		SetFreeMode();
		break;
	}

	UpdateData(FALSE);

	OnSelchangeComboQamMode();
}

void CModulatorCablePropPage::SetJ83AnnexAMode()
{
	int nIdx, i;

	m_QamModeCtrl.ResetContent();
	m_InterLeavCtrl.ResetContent();
	m_FltModeCtrl.ResetContent();
	m_SpecInvCtrl.ResetContent();

	// set modulation combo box
	nIdx = m_QamModeCtrl.AddString("QAM 16");
	m_QamModeCtrl.SetItemData(nIdx, QAM_16);
	nIdx = m_QamModeCtrl.AddString("QAM 32");
	m_QamModeCtrl.SetItemData(nIdx, QAM_32);
	nIdx = m_QamModeCtrl.AddString("QAM 64");
	m_QamModeCtrl.SetItemData(nIdx, QAM_64);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iQamMode == m_QamModeCtrl.GetItemData(i))
		{
			m_QamModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set interleaver combo box
	nIdx = m_InterLeavCtrl.AddString("I12/J17");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I12_J17);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iInterleaver == m_InterLeavCtrl.GetItemData(i))
		{
			m_InterLeavCtrl.SetCurSel(i);
			break;
		}
	}

	// set filter roll off combo box
	nIdx = m_FltModeCtrl.AddString("15%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_15);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iFltRollOff == m_FltModeCtrl.GetItemData(i))
		{
			m_FltModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set spectral inversion combo box
	nIdx = m_SpecInvCtrl.AddString("Normal  ");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_OFF);
	nIdx = m_SpecInvCtrl.AddString("Inverted");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_ON);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iSpecInv == m_SpecInvCtrl.GetItemData(i))
		{
			m_SpecInvCtrl.SetCurSel(i);
			break;
		}
	}

	m_dSymbolRateMin = 0.0;
	// max symbol rate = BW / 1+Filter roll off = 8000000/1.15
	m_dSymbolRateMax = 8.0/1.15;

	GetDlgItem(IDC_EDIT_SYMBOL_RATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_ANNEXB_EN)->EnableWindow(FALSE);
}

void CModulatorCablePropPage::SetJ83AnnexBMode()
{
	int nIdx, i;

	m_QamModeCtrl.ResetContent();
	m_InterLeavCtrl.ResetContent();
	m_FltModeCtrl.ResetContent();
	m_SpecInvCtrl.ResetContent();

	// set modulation combo box
	nIdx = m_QamModeCtrl.AddString("QAM 64");
	m_QamModeCtrl.SetItemData(nIdx, QAM_64);
	nIdx = m_QamModeCtrl.AddString("QAM 256");
	m_QamModeCtrl.SetItemData(nIdx, QAM_256);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iQamMode == m_QamModeCtrl.GetItemData(i))
		{
			m_QamModeCtrl.SetCurSel(i);
			break;
		}
	}
	
	// set spectral inversion combo box
	nIdx = m_SpecInvCtrl.AddString("Normal  ");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_OFF);
	nIdx = m_SpecInvCtrl.AddString("Inverted");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_ON);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iSpecInv == m_SpecInvCtrl.GetItemData(i))
		{
			m_SpecInvCtrl.SetCurSel(i);
			break;
		}
	}

	m_dSymbolRateMin = 5.056941;
	m_dSymbolRateMax = 5.360537;


	// set filter roll off combo box
	if (m_iQamMode==QAM_64)
	{
		// set symbol rate
		m_dSymbolRate = m_dSymbolRateMin;

		// set filter mode
		nIdx = m_FltModeCtrl.AddString("18%");
		m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_18);
		m_FltModeCtrl.SetCurSel(0);

		// set interleaver combo box
		nIdx = m_InterLeavCtrl.AddString("I128/J1");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J1);
		m_InterLeavCtrl.SetCurSel(0);

	} else
	if (m_iQamMode==QAM_256)
	{
		// set symbol rate
		m_dSymbolRate = m_dSymbolRateMax;

		// set filter roll off combo box
		nIdx = m_FltModeCtrl.AddString("12%");
		m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_12);
		m_FltModeCtrl.SetCurSel(0);

		// set interleaver combo box
		nIdx = m_InterLeavCtrl.AddString("I128/J1");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J1);
		nIdx = m_InterLeavCtrl.AddString("I128/J2");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J2);
		nIdx = m_InterLeavCtrl.AddString(" I64/J2");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I64_J2);
		nIdx = m_InterLeavCtrl.AddString("I128/J3");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J3);
		nIdx = m_InterLeavCtrl.AddString(" I32/J4");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I32_J4);
		nIdx = m_InterLeavCtrl.AddString("I128/J4");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J4);
		nIdx = m_InterLeavCtrl.AddString(" I16/J8");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I16_J8);
		nIdx = m_InterLeavCtrl.AddString("I128/J5");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J5);
		nIdx = m_InterLeavCtrl.AddString(" I8/J16");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I8_J16);
		nIdx = m_InterLeavCtrl.AddString("I128/J6");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J6);
		nIdx = m_InterLeavCtrl.AddString("I128/J7");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J7);
		nIdx = m_InterLeavCtrl.AddString("I128/J8");
		m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J8);
		for (i = 0; i <= nIdx; i++)
		{
			if (m_iInterleaver == m_InterLeavCtrl.GetItemData(i))
			{
				m_InterLeavCtrl.SetCurSel(i);
				break;
			}
		}
	}

	GetDlgItem(IDC_EDIT_SYMBOL_RATE)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_ANNEXB_EN)->EnableWindow(FALSE);
}

void CModulatorCablePropPage::SetJ83AnnexCMode()
{
	int nIdx, i;

	m_QamModeCtrl.ResetContent();
	m_InterLeavCtrl.ResetContent();
	m_FltModeCtrl.ResetContent();
	m_SpecInvCtrl.ResetContent();

	// set modulation combo box
	nIdx = m_QamModeCtrl.AddString("QAM 64");
	m_QamModeCtrl.SetItemData(nIdx, QAM_64);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iQamMode == m_QamModeCtrl.GetItemData(i))
		{
			m_QamModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set interleaver combo box
	nIdx = m_InterLeavCtrl.AddString("I12/J17");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I12_J17);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iInterleaver == m_InterLeavCtrl.GetItemData(i))
		{
			m_InterLeavCtrl.SetCurSel(i);
			break;
		}
	}

	// set filter roll off combo box
	nIdx = m_FltModeCtrl.AddString("13%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_13);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iFltRollOff == m_FltModeCtrl.GetItemData(i))
		{
			m_FltModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set spectral inversion combo box
	nIdx = m_SpecInvCtrl.AddString("Normal  ");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_OFF);
	nIdx = m_SpecInvCtrl.AddString("Inverted");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_ON);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iSpecInv == m_SpecInvCtrl.GetItemData(i))
		{
			m_SpecInvCtrl.SetCurSel(i);
			break;
		}
	}

	m_dSymbolRateMin = 0.0;
	// max symbol rate = BW / 1+Filter roll off = 8000000/1.15
	m_dSymbolRateMax = 6.0/1.13;

	GetDlgItem(IDC_EDIT_SYMBOL_RATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_ANNEXB_EN)->EnableWindow(FALSE);
}

void CModulatorCablePropPage::SetDVBCMode()
{
	int nIdx, i;

	m_QamModeCtrl.ResetContent();
	m_InterLeavCtrl.ResetContent();
	m_FltModeCtrl.ResetContent();
	m_SpecInvCtrl.ResetContent();

	// set modulation combo box
	nIdx = m_QamModeCtrl.AddString("QAM 16");
	m_QamModeCtrl.SetItemData(nIdx, QAM_16);
	nIdx = m_QamModeCtrl.AddString("QAM 32");
	m_QamModeCtrl.SetItemData(nIdx, QAM_32);
	nIdx = m_QamModeCtrl.AddString("QAM 64");
	m_QamModeCtrl.SetItemData(nIdx, QAM_64);
	nIdx = m_QamModeCtrl.AddString("QAM 128");
	m_QamModeCtrl.SetItemData(nIdx, QAM_128);
	nIdx = m_QamModeCtrl.AddString("QAM 256");
	m_QamModeCtrl.SetItemData(nIdx, QAM_256);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iQamMode == m_QamModeCtrl.GetItemData(i))
		{
			m_QamModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set interleaver combo box
	nIdx = m_InterLeavCtrl.AddString("I12/J17");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I12_J17);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iInterleaver == m_InterLeavCtrl.GetItemData(i))
		{
			m_InterLeavCtrl.SetCurSel(i);
			break;
		}
	}

	// set filter roll off combo box
	nIdx = m_FltModeCtrl.AddString("15%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_15);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iFltRollOff == m_FltModeCtrl.GetItemData(i))
		{
			m_FltModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set spectral inversion combo box
	nIdx = m_SpecInvCtrl.AddString("Normal  ");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_OFF);
	nIdx = m_SpecInvCtrl.AddString("Inverted");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_ON);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iSpecInv == m_SpecInvCtrl.GetItemData(i))
		{
			m_SpecInvCtrl.SetCurSel(i);
			break;
		}
	}

	m_dSymbolRateMin = 0.0;
	// max symbol rate = BW / 1+Filter roll off = 8000000/1.15
	m_dSymbolRateMax = 8.0/1.15;

	GetDlgItem(IDC_EDIT_SYMBOL_RATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_ANNEXB_EN)->EnableWindow(FALSE);
}

void CModulatorCablePropPage::SetFreeMode()
{
	int nIdx, i;

	m_QamModeCtrl.ResetContent();
	m_InterLeavCtrl.ResetContent();
	m_FltModeCtrl.ResetContent();
	m_SpecInvCtrl.ResetContent();

	// set modulation combo box
	nIdx = m_QamModeCtrl.AddString("QAM 16");
	m_QamModeCtrl.SetItemData(nIdx, QAM_16);
	nIdx = m_QamModeCtrl.AddString("QAM 32");
	m_QamModeCtrl.SetItemData(nIdx, QAM_32);
	nIdx = m_QamModeCtrl.AddString("QAM 64");
	m_QamModeCtrl.SetItemData(nIdx, QAM_64);
	nIdx = m_QamModeCtrl.AddString("QAM 128");
	m_QamModeCtrl.SetItemData(nIdx, QAM_128);
	nIdx = m_QamModeCtrl.AddString("QAM 256");
	m_QamModeCtrl.SetItemData(nIdx, QAM_256);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iQamMode == m_QamModeCtrl.GetItemData(i))
		{
			m_QamModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set interleaver combo box
	nIdx = m_InterLeavCtrl.AddString("I128/J1");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J1);
	nIdx = m_InterLeavCtrl.AddString("I128/J2");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J2);
	nIdx = m_InterLeavCtrl.AddString(" I64/J2");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I64_J2);
	nIdx = m_InterLeavCtrl.AddString("I128/J3");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J3);
	nIdx = m_InterLeavCtrl.AddString(" I32/J4");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I32_J4);
	nIdx = m_InterLeavCtrl.AddString("I128/J4");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J4);
	nIdx = m_InterLeavCtrl.AddString(" I16/J8");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I16_J8);
	nIdx = m_InterLeavCtrl.AddString("I128/J5");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J5);
	nIdx = m_InterLeavCtrl.AddString(" I8/J16");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I8_J16);
	nIdx = m_InterLeavCtrl.AddString("I128/J6");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J6);
	nIdx = m_InterLeavCtrl.AddString("I128/J7");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J7);
	nIdx = m_InterLeavCtrl.AddString("I128/J8");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I128_J8);
	nIdx = m_InterLeavCtrl.AddString("I12/J17");
	m_InterLeavCtrl.SetItemData(nIdx, INTLEAV_I12_J17);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iInterleaver == m_InterLeavCtrl.GetItemData(i))
		{
			m_InterLeavCtrl.SetCurSel(i);
			break;
		}
	}

	// set filter roll off combo box
	nIdx = m_FltModeCtrl.AddString("12%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_12);
	nIdx = m_FltModeCtrl.AddString("13%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_13);
	nIdx = m_FltModeCtrl.AddString("15%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_15);
	nIdx = m_FltModeCtrl.AddString("18%");
	m_FltModeCtrl.SetItemData(nIdx, FLT_ALPHA_18);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iFltRollOff == m_FltModeCtrl.GetItemData(i))
		{
			m_FltModeCtrl.SetCurSel(i);
			break;
		}
	}

	// set spectral inversion combo box
	nIdx = m_SpecInvCtrl.AddString("Normal  ");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_OFF);
	nIdx = m_SpecInvCtrl.AddString("Inverted");
	m_SpecInvCtrl.SetItemData(nIdx, INVERSION_ON);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iSpecInv == m_SpecInvCtrl.GetItemData(i))
		{
			m_SpecInvCtrl.SetCurSel(i);
			break;
		}
	}

	m_dSymbolRateMin = 0.0;
	m_dSymbolRateMax = 10.0;

	GetDlgItem(IDC_EDIT_SYMBOL_RATE)->EnableWindow(TRUE);
	GetDlgItem(IDC_CHECK_ANNEXB_EN)->EnableWindow(TRUE);
}

void CModulatorCablePropPage::RestoreSettings()
{
	CString Str;

	// get last setting of last selected mode
	switch (m_iDvbCModeSel)
	{
	case 0:
		Str = theApp.GetProfileString(strCableModKey, strEntryJ83A);
		break;
	case 1:
		Str = theApp.GetProfileString(strCableModKey, strEntryJ83B);
		break;
	case 2:
		Str = theApp.GetProfileString(strCableModKey, strEntryJ83C);
		break;
	case 3:
		Str = theApp.GetProfileString(strCableModKey, strEntryDVBC);
		break;
	case 4:
		Str = theApp.GetProfileString(strCableModKey, strEntryFree);
		break;
	}

	if(!(sscanf(Str, "%d,%d,%d,%d,%d,%lg",	&m_iQamMode,
											&m_iInterleaver,
											&m_iFltRollOff,
											&m_bAnnexBEn,
											&m_iSpecInv,
											&m_dSymbolRate) > 0))
	{
		m_iQamMode		= cCableModDefaults[m_iDvbCModeSel].m_iDfltQamMode;
		m_iInterleaver	= cCableModDefaults[m_iDvbCModeSel].m_iDfltInterLeav;
		m_iFltRollOff	= cCableModDefaults[m_iDvbCModeSel].m_iDfltFltRollOff;
		m_bAnnexBEn		= cCableModDefaults[m_iDvbCModeSel].m_bDfltAnnexB;
		m_iSpecInv		= cCableModDefaults[m_iDvbCModeSel].m_iDfltSpecInv;
		m_dSymbolRate	= cCableModDefaults[m_iDvbCModeSel].m_dDfltSymbRate;
	}
}

void CModulatorCablePropPage::StoreSettings()
{
	CString Str;

	// write currently selected mode
	Str.Format("%d",m_iDvbCModeSel);
	theApp.WriteProfileString(strCableModKey, strEntryModeSel, Str);

	// write settings of currently selected mode
	Str.Format("%d,%d,%d,%d,%d,%.6f",
				m_iQamMode,
				m_iInterleaver,
				m_iFltRollOff,
				m_bAnnexBEn,
				m_iSpecInv,
				m_dSymbolRate
				);

	switch (m_iDvbCModeSel)
	{
	case 0:
		theApp.WriteProfileString(strCableModKey, strEntryJ83A, Str);
		break;
	case 1:
		theApp.WriteProfileString(strCableModKey, strEntryJ83B, Str);
		break;
	case 2:
		theApp.WriteProfileString(strCableModKey, strEntryJ83C, Str);
		break;
	case 3:
		theApp.WriteProfileString(strCableModKey, strEntryDVBC, Str);
		break;
	case 4:
		theApp.WriteProfileString(strCableModKey, strEntryFree, Str);
		break;
	}
}

void CModulatorCablePropPage::OnSelchangeComboQamMode() 
{
	UpdateData(TRUE);

	// change filter roll and symbol rate for for annexB
	if (m_iDvbCModeSel==1)
	{
		m_iQamMode = m_QamModeCtrl.GetItemData(m_QamModeCtrl.GetCurSel());
		SetJ83AnnexBMode();
	}

	UpdateData(FALSE);
}
