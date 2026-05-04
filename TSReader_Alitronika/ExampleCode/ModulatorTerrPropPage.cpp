// ModulatorTerrPropPage.cpp: implementation of the CModulatorTerrPropPage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "dvbrecorder.h"
#include "ModulatorTerrPropPage.h"
#include "frontend.h"
#include "ATBoard.h"
#include "CurrBoard.h"

class CDVBRecorderApp;
extern CDVBRecorderApp theApp;
const char strTerModKey[] = "Modulators";
const char strEntry[] = "TerrestrialModulation";
#define IDT_UPDATE	2


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNCREATE(CModulatorTerrPropPage, CPropertyPage)

CModulatorTerrPropPage::CModulatorTerrPropPage() : CPropertyPage(CModulatorTerrPropPage::IDD)
{
	//{{AFX_DATA_INIT(CModulatorTerrPropPage)
	//}}AFX_DATA_INIT
	m_bIgnoreTimer = FALSE;
}

CModulatorTerrPropPage::~CModulatorTerrPropPage()
{
	m_bIgnoreTimer = TRUE;
	
	if (m_hWnd)
		KillTimer(IDT_UPDATE);
}

void CModulatorTerrPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModulatorTerrPropPage)
	DDX_Control(pDX, IDC_COMBO_INVERSION, m_SpecInvCtrl);
	DDX_Control(pDX, IDC_COMBO_TRAN_MODE, m_TranModeCtrl);
	DDX_Control(pDX, IDC_COMBO_MODULATION, m_ModulationCtrl);
	DDX_Control(pDX, IDC_COMBO_GUARD_INT, m_GuartIntCtrl);
	DDX_Control(pDX, IDC_COMBO_FEC_CODE_RATE, m_HPFecCodeRateCtrl);
	DDX_Control(pDX, IDC_COMBO_BANDWIDTH, m_BandWidthCtrl);
	DDX_Control(pDX, IDC_SIG_LED_IN_SYNC_PCR, m_LedInSyncPcr);
	DDX_Control(pDX, IDC_SIG_LED_204PCK_PCR, m_Led204PckPcr);
	DDX_Control(pDX, IDC_SIG_LED_TS_OFLOW_PCR, m_LedTsOFlowPcr);
	DDX_Control(pDX, IDC_SIG_LED_MOD_UFLOW, m_LedModUFlow);
	DDX_Text(pDX, IDC_EDIT_CELLID, m_nCellID);
	DDV_MinMaxUInt(pDX, m_nCellID, 0, 65535);
	DDX_Check(pDX, IDC_CHECK_DVBH_EN, m_bDvbHEn);
	DDX_Check(pDX, IDC_CHECK_INDEPTH_INT, m_bInDepthInt);
	DDX_Check(pDX, IDC_CHECK_MPE_FEC, m_bMpeFec);
	DDX_Check(pDX, IDC_CHECK_TIME_SLICE, m_bTimeSlice);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CModulatorTerrPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CModulatorTerrPropPage)
	ON_BN_CLICKED(ID_APPLY_NOW, OnApplyNow)
	ON_WM_SHOWWINDOW()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK_DVBH_EN, OnCheckDvbhEn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropPageCableTuner message handlers

BOOL CModulatorTerrPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// set leds to default
	m_LedInSyncPcr.Set(CLedControl::LED_RED);
	m_Led204PckPcr.Set(CLedControl::LED_RED);
	m_LedTsOFlowPcr.Set(CLedControl::LED_RED);
	m_LedModUFlow.Set(CLedControl::LED_RED);

	int nIdx, i, j;
	// set combo box strings
	nIdx = m_ModulationCtrl.AddString("QPSK  ");
	m_ModulationCtrl.SetItemData(nIdx, QPSK);
	nIdx = m_ModulationCtrl.AddString("QAM 16");
	m_ModulationCtrl.SetItemData(nIdx, QAM_16);
	nIdx = m_ModulationCtrl.AddString("QAM 64");
	m_ModulationCtrl.SetItemData(nIdx, QAM_64);
	for (i = 0; i <= nIdx; i++)
	{
		j=m_ModulationCtrl.GetItemData(i);
		if (m_iModulation == m_ModulationCtrl.GetItemData(i))
		{
			m_ModulationCtrl.SetCurSel(i);
			break;
		}
	}

	nIdx = m_GuartIntCtrl.AddString("1/4 ");
	m_GuartIntCtrl.SetItemData(nIdx, GUARD_INTERVAL_1_4);
	nIdx = m_GuartIntCtrl.AddString("1/8 ");
	m_GuartIntCtrl.SetItemData(nIdx, GUARD_INTERVAL_1_8);
	nIdx = m_GuartIntCtrl.AddString("1/16");
	m_GuartIntCtrl.SetItemData(nIdx, GUARD_INTERVAL_1_16);
	nIdx = m_GuartIntCtrl.AddString("1/32");
	m_GuartIntCtrl.SetItemData(nIdx, GUARD_INTERVAL_1_32);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iGuardInt == m_GuartIntCtrl.GetItemData(i))
		{
			m_GuartIntCtrl.SetCurSel(i);
			break;
		}
	}

	nIdx = m_HPFecCodeRateCtrl.AddString("1/2");
	m_HPFecCodeRateCtrl.SetItemData(nIdx, FEC_1_2);
	nIdx = m_HPFecCodeRateCtrl.AddString("2/3");
	m_HPFecCodeRateCtrl.SetItemData(nIdx, FEC_2_3);
	nIdx = m_HPFecCodeRateCtrl.AddString("3/4");
	m_HPFecCodeRateCtrl.SetItemData(nIdx, FEC_3_4);
	nIdx = m_HPFecCodeRateCtrl.AddString("5/6");
	m_HPFecCodeRateCtrl.SetItemData(nIdx, FEC_5_6);
	nIdx = m_HPFecCodeRateCtrl.AddString("7/8");
	m_HPFecCodeRateCtrl.SetItemData(nIdx, FEC_7_8);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iHPFecCodeRate == m_HPFecCodeRateCtrl.GetItemData(i))
		{
			m_HPFecCodeRateCtrl.SetCurSel(i);
			break;
		}
	}

	nIdx = m_BandWidthCtrl.AddString("8 MHz");
	m_BandWidthCtrl.SetItemData(nIdx, BANDWIDTH_8_7_MHZ);	// 8 MHz
	nIdx = m_BandWidthCtrl.AddString("7 MHz");
	m_BandWidthCtrl.SetItemData(nIdx, BANDWIDTH_7_6_MHZ);	// 7 MHz
	nIdx = m_BandWidthCtrl.AddString("6 MHz");
	m_BandWidthCtrl.SetItemData(nIdx, BANDWIDTH_6_MHZ);
	nIdx = m_BandWidthCtrl.AddString("5 MHz");
	m_BandWidthCtrl.SetItemData(nIdx, BANDWIDTH_5_MHZ);
	for (i = 0; i <= nIdx; i++)
	{
		if (m_iBandWidth == m_BandWidthCtrl.GetItemData(i))
		{
			m_BandWidthCtrl.SetCurSel(i);
			break;
		}
	}

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

	OnCheckDvbhEn();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModulatorTerrPropPage::OnInit()
{
	// get device information
	CCurrBoard CurrBoard;
	if (CurrBoard.IsValid())
	{
		CATBoard &TheBoard = CurrBoard;
		TheBoard.DvbSource(SRC_INIT, NULL);
	}
		
	// get last settings from registry
	CString Str;
	Str = theApp.GetProfileString(strTerModKey, strEntry);

	if(!(sscanf(Str, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", &m_iTranMode, 
														 &m_iModulation, 
														 &m_iGuardInt, 
														 &m_iHPFecCodeRate, 
														 &m_iBandWidth, 
														 &m_iSpecInv, 
														 &m_nCellID, 
														 &m_bDvbHEn, 
														 &m_bInDepthInt, 
														 &m_bMpeFec, 
														 &m_bTimeSlice) > 0))
	{
		m_iTranMode = TRANSMISSION_MODE_8K;
		m_iModulation = QAM_64;
		m_iGuardInt = GUARD_INTERVAL_1_32;
		m_iHPFecCodeRate = FEC_7_8;
		m_iBandWidth = BANDWIDTH_8_7_MHZ;
		m_iSpecInv = INVERSION_OFF;
		m_nCellID = 0;
		m_bDvbHEn = FALSE;
		m_bInDepthInt = FALSE;		
		m_bMpeFec = FALSE;
		m_bTimeSlice = FALSE;
	}

	SetModulator();
}

void CModulatorTerrPropPage::OnDestroy() 
{
	m_bIgnoreTimer = TRUE;
	KillTimer(IDT_UPDATE);
	CPropertyPage::OnDestroy();
}

void CModulatorTerrPropPage::OnShowWindow(BOOL bShow, UINT nStatus) 
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

void CModulatorTerrPropPage::OnTimer(UINT nIDEvent) 
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

void CModulatorTerrPropPage::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
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

void CModulatorTerrPropPage::OnApplyNow() 
{
	UpdateData(TRUE);

	m_iModulation = m_ModulationCtrl.GetItemData(m_ModulationCtrl.GetCurSel());
	m_iTranMode = m_TranModeCtrl.GetItemData(m_TranModeCtrl.GetCurSel());
	m_iGuardInt = m_GuartIntCtrl.GetItemData(m_GuartIntCtrl.GetCurSel());
	m_iHPFecCodeRate = m_HPFecCodeRateCtrl.GetItemData(m_HPFecCodeRateCtrl.GetCurSel());
	m_iBandWidth = m_BandWidthCtrl.GetItemData(m_BandWidthCtrl.GetCurSel());
	m_iSpecInv = m_SpecInvCtrl.GetItemData(m_SpecInvCtrl.GetCurSel());

	SetModulator();
}

void CModulatorTerrPropPage::SetModulator()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	dvb_at2800_parameters DvbTOfdmParams;

	DvbTOfdmParams.constellation		= (src_modulation_t)m_iModulation;
	DvbTOfdmParams.transmission_mode	= (src_transmit_mode_t)m_iTranMode;
	DvbTOfdmParams.guard_interval		= (src_guard_interval_t)m_iGuardInt;
	DvbTOfdmParams.code_rate_HP			= (src_code_rate_t)m_iHPFecCodeRate;
	DvbTOfdmParams.code_rate_LP			= FEC_7_8;
	DvbTOfdmParams.bandwidth			= (src_bandwidth_t)m_iBandWidth;
	DvbTOfdmParams.hierarchy_information= HIERARCHY_NONE;
	DvbTOfdmParams.inversion			= (src_spectral_inversion_t)m_iSpecInv;
	DvbTOfdmParams.dvb_h_cell_id		= (u16)m_nCellID;
	DvbTOfdmParams.dvb_h_LP_time_slice	= (bool)FALSE;
	DvbTOfdmParams.dvb_h_LP_MPE_FEC		= (bool)FALSE;
	
	if (m_bMpeFec)
		DvbTOfdmParams.dvb_h_HP_time_slice	= (bool)TRUE;
	else
		DvbTOfdmParams.dvb_h_HP_time_slice	= (bool)FALSE;

	if (m_bTimeSlice)
		DvbTOfdmParams.dvb_h_HP_MPE_FEC		= (bool)TRUE;
	else
		DvbTOfdmParams.dvb_h_HP_MPE_FEC		= (bool)FALSE;
	
	if (m_bDvbHEn)
		DvbTOfdmParams.dvb_h_en				= (bool)TRUE;
	else
		DvbTOfdmParams.dvb_h_en				= (bool)FALSE;

	if (m_bInDepthInt)
		DvbTOfdmParams.dvb_h_indepth_int	= (bool)TRUE;
	else
		DvbTOfdmParams.dvb_h_indepth_int	= (bool)FALSE;
	
	if (!m_bIgnoreTimer)
		TheBoard.DvbSource(SRC_SET_MOD_PARAMS, &DvbTOfdmParams);

	CString Str;

	Str.Format("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
				m_iTranMode,
				m_iModulation,
				m_iGuardInt,
				m_iHPFecCodeRate,
				m_iBandWidth,
				m_iSpecInv,
				m_nCellID,
				m_bDvbHEn,
				m_bInDepthInt,
				m_bMpeFec,
				m_bTimeSlice
				);

	theApp.WriteProfileString(strTerModKey, strEntry, Str);
}

void CModulatorTerrPropPage::UpdateStatus()
{
	CCurrBoard CurrBoard;
	if (!CurrBoard.IsValid())
		return;

	CATBoard &TheBoard = CurrBoard;

	src_status_at2800 eStatus;

	if (!m_bIgnoreTimer)
		TheBoard.DvbSource(SRC_GET_MOD_STAT, &eStatus);

 	if (eStatus & DST_HAS_HP_PCR_TS_INSYNC)
 		m_LedInSyncPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_LedInSyncPcr.Set(CLedControl::LED_RED);

 	if (eStatus & DST_HAS_HP_PCR_TS_204PKT)
 		m_Led204PckPcr.Set(CLedControl::LED_GREEN);
 	else
 		m_Led204PckPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_HP_PCR_TS_OFLOW)
 		m_LedTsOFlowPcr.Set(CLedControl::LED_RED);
 	else
 		m_LedTsOFlowPcr.Set(CLedControl::LED_GRAY);

 	if (eStatus & DST_HAS_MOD_UFLOW)
 		m_LedModUFlow.Set(CLedControl::LED_RED);
 	else
 		m_LedModUFlow.Set(CLedControl::LED_GRAY);
}

void CModulatorTerrPropPage::OnCheckDvbhEn() 
{
	int nIdx, i;
	UpdateData(TRUE);
	
	m_TranModeCtrl.ResetContent();

	if (m_bDvbHEn)
	{
		GetDlgItem(IDC_CHECK_INDEPTH_INT)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_MPE_FEC)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_TIME_SLICE)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_CELLID)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_CELLID)->EnableWindow(TRUE);

		nIdx = m_TranModeCtrl.AddString("2k");
		m_TranModeCtrl.SetItemData(nIdx, TRANSMISSION_MODE_2K);
		nIdx = m_TranModeCtrl.AddString("4k");
		m_TranModeCtrl.SetItemData(nIdx, TRANSMISSION_MODE_4K);
		nIdx = m_TranModeCtrl.AddString("8k");
		m_TranModeCtrl.SetItemData(nIdx, TRANSMISSION_MODE_8K);
		for (i = 0; i <= nIdx; i++)
		{
			if (m_iTranMode == m_TranModeCtrl.GetItemData(i))
			{
				m_TranModeCtrl.SetCurSel(i);
				break;
			}
		}
	}
	else
	{
		GetDlgItem(IDC_CHECK_INDEPTH_INT)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_MPE_FEC)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_TIME_SLICE)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_CELLID)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_CELLID)->EnableWindow(FALSE);

		nIdx = m_TranModeCtrl.AddString("2k");
		m_TranModeCtrl.SetItemData(nIdx, TRANSMISSION_MODE_2K);
		nIdx = m_TranModeCtrl.AddString("8k");
		m_TranModeCtrl.SetItemData(nIdx, TRANSMISSION_MODE_8K);
		for (i = 0; i <= nIdx; i++)
		{
			if (m_iTranMode == m_TranModeCtrl.GetItemData(i))
			{
				m_TranModeCtrl.SetCurSel(i);
				break;
			}
		}
	}
}
