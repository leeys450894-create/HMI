#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_Ionizer.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(CDlgSubManual_Ionizer, CDialogEx)

CDlgSubManual_Ionizer::CDlgSubManual_Ionizer(CWnd* pParent /*=NULL*/)
	:CDialogEx(CDlgSubManual_Ionizer::IDD, pParent)
{
	//m_pSystem = NULL;
	//m_pSystem = SMEM_GetSharedMemory();
}

CDlgSubManual_Ionizer::~CDlgSubManual_Ionizer()
{
}

void CDlgSubManual_Ionizer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_CHECK_1, m_chkIonUse);
	DDX_Control(pDX, IDC_CHECK_2, m_chkIonAutoOff);
	DDX_Control(pDX, IDC_CHECK_3, m_chkIonAlarmUse);
	DDX_Control(pDX, IDC_CHECK_4, m_chkIonLoadOn);
	DDX_Control(pDX, IDC_CHECK_5, m_chkIonUnloadOn);
	DDX_Control(pDX, IDC_BUTTON_5, m_chkStop);
	DDX_Control(pDX, IDC_COMBO_1, m_cmbIonizerCnt);
	DDX_Control(pDX, IDC_EDIT_2, m_edtProcCount);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_Ionizer, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_COMMAND_RANGE(IDC_BUTTON_1, IDC_BUTTON_2, OnIonizer)
	ON_COMMAND(IDC_BUTTON_3, OnUpdateDelay)
	ON_COMMAND(IDC_BUTTON_4, OnAutoProcess)
	ON_COMMAND(IDC_BUTTON_5, OnStop)
	ON_COMMAND(IDC_CHECK_1, OnIonizerUse)
	ON_COMMAND(IDC_CHECK_2, OnIonizerAutoOff)
	ON_COMMAND(IDC_CHECK_3, OnIonizerAlarmUse)
	ON_COMMAND(IDC_CHECK_4, OnIonizerLoadOn)
	ON_COMMAND(IDC_CHECK_5, OnIonizerUnloadOn)
	ON_CONTROL(CBN_SELCHANGE, IDC_COMBO_1, OnSelectItem)
END_MESSAGE_MAP()

BOOL CDlgSubManual_Ionizer::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CDlgSubManual_Ionizer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	//SetBackgroundColor(RGB_MAIN3);
	//
	CString strPath = _T(""); /*gf_GetFilePath(_T("dfDEFAULT_CONFIG_FILE_PATH"));*/
	
	TCHAR szValue[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);
	GetPrivateProfileString(_T("IONIZER"), _T("IonizerDelay"), _T("3"), szValue, MAX_PATH, strPath);
	int nDelay = _ttoi(szValue);
	//m_pSystem->stInterLock.m_nIonizerDelay = nDelay;
	
	CString str;
	str.Format(_T("%d"), nDelay);
	GetDlgItem(IDC_EDIT_1)->SetWindowText(str);
	
	//
	GetPrivateProfileString(_T("IONIZER"), _T("USE"), _T("1"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_bIonizerUse = atoi(szValue);
	m_chkIonUse.SetImage(IDB_CHECKNO32);
	m_chkIonUse.SetCheckedImage(IDB_CHECK32);
	m_chkIonUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	//m_chkIonUse.SetCheck(m_pSystem->stInterLock.m_bIonizerUse);
	
	//
	GetPrivateProfileString(_T("IONIZER"), _T("AutoOff"), _T("1"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_bIonizerAutoOff = atoi(szValue);
	m_chkIonAutoOff.SetImage(IDB_CHECKNO32);
	m_chkIonAutoOff.SetCheckedImage(IDB_CHECK32);
	m_chkIonAutoOff.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	//m_chkIonAutoOff.SetCheck(m_pSystem->stInterLock.m_bIonizerAutoOff);
	
	//
	GetPrivateProfileString(_T("ALARM"), _T("USEIONIZERALM"), _T("1"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM] = atoi(szValue);
	m_chkIonAlarmUse.SetImage(IDB_CHECKNO32);
	m_chkIonAlarmUse.SetCheckedImage(IDB_CHECK32);
	m_chkIonAlarmUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	//m_chkIonAlarmUse.SetCheck(m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM]);
	
	//
	GetPrivateProfileString(_T("IONIZER"), _T("GlassLoad_Use"), _T("1"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD] = atoi(szValue);
	m_chkIonLoadOn.SetImage(IDB_CHECKNO32);
	m_chkIonLoadOn.SetCheckedImage(IDB_CHECK32);
	m_chkIonLoadOn.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	//m_chkIonLoadOn.SetCheck(m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD]);
	
	//
	GetPrivateProfileString(_T("IONIZER"), _T("GlassUnload_Use"), _T("1"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD] = atoi(szValue);
	m_chkIonUnloadOn.SetImage(IDB_CHECKNO32);
	m_chkIonUnloadOn.SetCheckedImage(IDB_CHECK32);
	m_chkIonUnloadOn.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	//m_chkIonUnloadOn.SetCheck(m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD]);
	
	//
	m_chkStop.SetImage(IDB_CHECKNO32);
	m_chkStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_chkStop.ShowWindow(FALSE);
	
	m_edtProcCount.ShowWindow(FALSE);
	
	m_cmbIonizerCnt.AddString(_T("1"));
	m_cmbIonizerCnt.AddString(_T("3"));
	m_cmbIonizerCnt.AddString(_T("5"));
	m_cmbIonizerCnt.AddString(_T("7"));
	m_cmbIonizerCnt.AddString(_T("10"));
	m_cmbIonizerCnt.SetCurSel(0);
	
	m_cmbIonizerCnt.GetWindowText(str);
	//m_pSystem->stInterLock.m_nIonizerCycleCount = atoi(str);
	
	//m_pSystem->stInterLock.m_bIonizerOffReq = FALSE;

	return TRUE;
}

void CDlgSubManual_Ionizer::UpdateStatus(BOOL bEnable)
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) m_chkStop.ShowWindow(FALSE);
	//else m_chkStop.ShowWindow(!bEnable);

	if (m_chkStop.IsWindowVisible()) {
		m_edtProcCount.ShowWindow(TRUE);
	//	m_edtProcCount.SetWindowText(dfcStr(_T("Proc Cnt : %d/%d"), m_pSystem->stInterLock.m_nIonizerProcCount, m_pSystem->stInterLock.m_nIonizerCycleCount));
	}
	else m_edtProcCount.ShowWindow(FALSE);

	GetDlgItem(IDC_BUTTON_1)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_2)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_3)->EnableWindow(bEnable);
	GetDlgItem (IDC_BUTTON_4)->EnableWindow (bEnable);
	GetDlgItem(IDC_CHECK_1)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_2)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_3)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_4)->EnableWindow(bEnable);
	GetDlgItem(IDC_CHECK_5)->EnableWindow(bEnable);
	m_cmbIonizerCnt.EnableWindow (bEnable);
}

void CDlgSubManual_Ionizer::OnIonizer(UINT nID)
{
	//if (!m_pSystem->stInterLock.m_bIonizerUse) {
	//	AfxMessageBox(_T("Ionizer Use 상태가 아닙니다. Use 상태로 변경후 진행 하십시오."));
	//}

	switch (nID) {
	case IDC_BUTTON_1:
		//if (m_pSystem->stInterLock.m_bIonizerUse) m_pDlgNaviMaint->m_dlgNaviMaintPLC.CmSetIonizerON(TRUE);
		break;

	case IDC_BUTTON_2:
		//if (m_pSystem->stInterLock.m_bIonizerUse) m_pDlgNaviMaint->m_dlgNaviMaintPLC.CmSetIonizerON(FALSE);
		break;
	}
}

void CDlgSubManual_Ionizer::OnUpdateDelay()
{
	if (AfxMessageBox(_T("현재 설정값을 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strPath = _T(""); /*gf_GetFilePath(_T("dfDEFAULT_CONFIG_FILE_PATH"));*/
	CString strVal;
	GetDlgItem(IDC_EDIT_1)->GetWindowText(strVal);

	int nDelay = 0;
	nDelay = abs(_ttoi(strVal));
	strVal.Format(_T("%d"), nDelay);
	WritePrivateProfileString(_T("IONIZER"), _T("IonizerDelay"), strVal, strPath);
	//m_pSystem->stInterLock.m_nIonizerDelay = nDelay;

	//strVal.Format(_T("%d"), m_pSystem->stInterLock.m_bIonizerUse);
	WritePrivateProfileString(_T("IONIZER"), _T("USE"), strVal, strPath);

	//strVal.Format(_T("%d"), m_pSystem->stInterLock.m_bIonizerAutoOff);
	WritePrivateProfileString(_T("IONIZER"), _T("AutoOff"), strVal, strPath);

	//strVal.Format(_T("%d"), m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM]);
	WritePrivateProfileString(_T("ALARM"), _T("USEIONIZERALM"), strVal, strPath);

	//strVal.Format(_T("%d"), m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD]);
	WritePrivateProfileString(_T("IONIZER"), _T("GlassLoad_Use"), strVal, strPath);

	//strVal.Format(_T("%d"), m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD]);
	WritePrivateProfileString(_T("IONIZER"), _T("GlassUnload_Use"), strVal, strPath);
}

void CDlgSubManual_Ionizer::OnAutoProcess()
{
	if (AfxMessageBox (_T("Ionizer 테스트 프로세스를 시작 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString str, strLog;
	GetDlgItem (IDC_BUTTON_4)->GetWindowText (str);
	strLog.Format(_T("OnClickBtn(CDlgSubManual_Ionizer) : %s"), str);
	//gLog(m_pSystem->hLog, strLog, LOG_LEVEL_O);


	//int nRtn = m_pJobProc->Is_LoadStatus();
	int nRtn = 0;
	if (nRtn != VTM_RV_SUCCESSED) {
		AfxMessageBox (_T("Unload 위치가 아닙니다. 위치 확인후 진행 하십시오."));
		return;
	}

	//m_pJobProc->DoStartIonizerTest ();
}

void CDlgSubManual_Ionizer::OnStop()
{
	//gLog(m_pSystem->hLog, "CDlgSubManual_Ionizer::OnClickBtnNaviOperateStop()", LOG_LEVEL_O);
	//m_pJobProc->ActionStop();
}

void CDlgSubManual_Ionizer::OnIonizerUse()
{
	//m_pSystem->stInterLock.m_bIonizerUse = !m_pSystem->stInterLock.m_bIonizerUse;
	//m_chkIonUse.SetCheck(m_pSystem->stInterLock.m_bIonizerUse);
}

void CDlgSubManual_Ionizer::OnIonizerAutoOff()
{
	//m_pSystem->stInterLock.m_bIonizerAutoOff = !m_pSystem->stInterLock.m_bIonizerAutoOff;
	//m_chkIonAutoOff.SetCheck(m_pSystem->stInterLock.m_bIonizerAutoOff);
}

void CDlgSubManual_Ionizer::OnIonizerAlarmUse()
{
	//m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM] = !m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM];
	//m_chkIonAlarmUse.SetCheck(m_pSystem->stInterLock.m_bUseAlarm[df_USE_PLC_IONIZER_ALARM]);
}

void CDlgSubManual_Ionizer::OnIonizerLoadOn()
{
	//m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD] = !m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD];
	//m_chkIonLoadOn.SetCheck(m_pSystem->stInterLock.m_bIonizerSet[IONIZER_LOAD]);
}

void CDlgSubManual_Ionizer::OnIonizerUnloadOn()
{
	//m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD] = !m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD];
	//m_chkIonUnloadOn.SetCheck(m_pSystem->stInterLock.m_bIonizerSet[IONIZER_UNLOAD]);
}

void CDlgSubManual_Ionizer::OnSelectItem()
{
	CString str;
	//m_cmbIonizerCnt.GetWindowText(str);
	//m_pSystem->stInterLock.m_nIonizerCycleCount = atoi(str);
}