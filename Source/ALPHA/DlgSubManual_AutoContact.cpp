#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AutoContact.h"
#include "AuroraMgr.h"					// LYS 2026-08-10 Phase 1
#include "Manager\ShareMemoryMgr.h"	// LYS 2026-08-10 Phase 1
#include "Manager\MaintControlMgr.h"	// LYS 2026-08-10 Phase 1
#include "Recipe\RecipeEditMgr.h"		// LYS 2026-08-10 Phase 1
#include "DoWork\AutoContactInfo.h"		// LYS 2026-08-10 Phase 4
#include "Manager\CommTcpIpFactoryMgr.h"	// LYS 2026-08-10 Phase 4
#include "Manager\SequenceManager.h"		// LYS 2026-08-10 Phase 4
#include "Alarm\AlarmController.h"		// LYS 2026-08-10 Phase 4
#include "SystemParamShare.h"			// LYS 2026-08-10 Phase 1
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CDlgSubManual_AutoContact, CDialogEx)

CDlgSubManual_AutoContact::CDlgSubManual_AutoContact(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=NULL*/) 
	:CDialogEx(CDlgSubManual_AutoContact::IDD, pParent)
	, m_pAuroraMgr(pAuroraMgr)	// LYS 2026-08-10 Phase 1
{
	ZeroMemory(m_fAutoContactParam, sizeof(m_fAutoContactParam));	// LYS 2026-08-10
	m_bAutoCntRunning = FALSE;	// LYS 2026-08-10 Phase 4
	m_nAcStickCnt = -1;			// LYS 2026-08-11
	m_nAcSelStick = -1;
	m_nAcProgState = 0;
	m_nAcProgCur = 0;
	m_nAcProgTotal = 0;
	//m_pSystem = NULL;
	//m_pSystem = SMEM_GetSharedMemory();
}

CDlgSubManual_AutoContact::~CDlgSubManual_AutoContact()
{
}

void CDlgSubManual_AutoContact::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_BUTTON_1, m_btnStop);
	DDX_Control(pDX, IDC_CHECK_1, m_btnAutoContactUse);
	DDX_Control(pDX, IDC_CHECK_2, m_btnAutoModelChangeUse);
	DDX_Control(pDX, IDC_CHECK_3, m_btnAutoProbeChangeUse);
	DDX_Control(pDX, IDC_CHECK_4, m_btnAutoProbeAlways);
	DDX_Control(pDX, IDC_COMBO_AC_STICK, m_comboAcStick);	// LYS 2026-08-11
}


BEGIN_MESSAGE_MAP(CDlgSubManual_AutoContact, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()	// LYS 2026-08-10 Phase 1
	ON_COMMAND(IDC_BUTTON_1, OnStop)
	ON_COMMAND(IDC_BUTTON_2, OnRcpOpen)
	ON_COMMAND(IDC_BUTTON_3, OnAutoContactSave)
	ON_COMMAND(IDC_BUTTON_4, OnAutoContactProcStrart)
	ON_COMMAND(IDC_BUTTON_5, OnPgRecipeDownload)

	ON_COMMAND(IDC_CHECK_1, OnAutoContactUse)
	ON_COMMAND(IDC_CHECK_2, OnModelChangeUse)
	ON_COMMAND(IDC_CHECK_3, OnProbeChangeUse)
	ON_COMMAND(IDC_CHECK_4, OnAlwaysUse)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDlgSubManual_AutoContact, CDialog)

END_EVENTSINK_MAP()

BOOL CDlgSubManual_AutoContact::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

BOOL CDlgSubManual_AutoContact::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	//---------------------------------------------------------------------
	// Farpoint Spread 생성 
	//---------------------------------------------------------------------
	m_spreadAutoContact = new CMySpreadDll(IDC_FS_AUTO_CONTACT, FPS_RESOURCE_ID + 22, this, TRUE);

	if (m_spreadAutoContact->GetSafeHwnd() != NULL)
	{
		m_spreadAutoContact->InitDefault();

		SS_CELLTYPE cellType;
		CString sComboList;
		m_spreadAutoContact->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
		m_spreadAutoContact->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);
		
		sComboList = _T("1\t2\t3\t");
		m_spreadAutoContact->SetTypeComboBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, sComboList);
		m_spreadAutoContact->SetCellTypeRange(1, 7, 1, 7, &cellType);

		m_spreadAutoContact->MakeFont(&m_hfont);
		m_spreadAutoContact->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

		m_spreadAutoContact->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_spreadAutoContact->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_spreadAutoContact->InitMySpread_AUTOCONTACT();
	}

	//m_strIniPath = gf_GetCnfPath(m_pSystem->stEquip.m_nEQType, dfDEFAULT_CONFIG_FILE);
	//m_strAutoLogPath = gf_GetFilePath(dfDEFAULT_AUTOCNT_LOG_FILE_PATH);
	// LYS 2026-08-10 Phase 1: HMI config path (C7: Config.ini [PROBE_AUTO_CONTACT])
	m_strIniPath.Format(_T("%s\\Config.ini"), CSystemParamShare::GetInstance()->GetConfigFolderPath());
	m_strAutoLogPath.Format(_T("%s\\AutoContactLog.ini"), CSystemParamShare::GetInstance()->GetConfigFolderPath());

	m_btnStop.SetImage(IDB_CHECKNO32);
	m_btnStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_btnStop.ShowWindow(FALSE);

	//memset(m_pSystem->stInterLock.m_fAutoContactData, 0.0, sizeof(double) * AUTO_CONTACT_ITEM_MAX);
	//memset(m_pSystem->stInterLock.nAutoContactStatus, 0, sizeof(int) * RTEST_MAX);
	//m_pSystem->stInterLock.m_fProbeOD = 0.0;

	m_fAutoCntReady = 0.0;
	m_fAutoCntContactPos = 0.0;
	m_fAutoCntODPos = 0.0;
	m_strAutoCntRecipeID = _T("");
	m_strAutoCnttProbeID = _T("");
	m_bAutoCntUse = FALSE;
	m_bAutoCntModel = FALSE;
	m_bAutoCntProbeID = FALSE;
	m_bAutoCntAlways = FALSE;

	GetAutoContactData();

	m_btnAutoContactUse.SetImage(IDB_CHECKNO32);
	m_btnAutoContactUse.SetCheckedImage(IDB_CHECK32);
	m_btnAutoContactUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnAutoContactUse.SetCheck(m_bAutoCntUse);

	m_btnAutoModelChangeUse.SetImage(IDB_CHECKNO32);
	m_btnAutoModelChangeUse.SetCheckedImage(IDB_CHECK32);
	m_btnAutoModelChangeUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnAutoModelChangeUse.SetCheck(m_bAutoCntModel);

	m_btnAutoProbeChangeUse.SetImage(IDB_CHECKNO32);
	m_btnAutoProbeChangeUse.SetCheckedImage(IDB_CHECK32);
	m_btnAutoProbeChangeUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnAutoProbeChangeUse.SetCheck(m_bAutoCntProbeID);

	m_btnAutoProbeAlways.SetImage(IDB_CHECKNO32);
	m_btnAutoProbeAlways.SetCheckedImage(IDB_CHECK32);
	m_btnAutoProbeAlways.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnAutoProbeAlways.SetCheck(m_bAutoCntAlways);

	// LYS 2026-08-10 Phase 1: load params into grid (CMySpreadDll: data col 1, rows 1..8)
	CString strTemp;
	for (int i = 0; i < AUTO_CONTACT_ITEM_MAX; i++) {
		if (i == AUTO_CT_PORT_FAIL)	strTemp.Format(_T("%d"), (int)m_fAutoContactParam[i]);	// combo row: text=value
		else						strTemp.Format(_T("%.4lf"), m_fAutoContactParam[i]);
		m_spreadAutoContact->SetData(1, i + 1, strTemp);
	}

	// LYS 2026-08-10 Phase 1: show current recipe id (always from RecipeEditMgr)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());

	UpdateStickList();		// LYS 2026-08-11 manual-only: stick selector

	// LYS 2026-10-13 Hybrid Phase 7 D7-17 segment label, created hidden right of the recipe box (DLU 140,53-340,62 is free in IDD_DLG_SUB_MANUAL_AUTO_CONTACT,
	//   .rc unchanged). Only UpdateAcSegLabel shows it, and only for a hybrid recipe - a 1-segment recipe never sees it
	{
		CRect rcAcSeg(140, 53, 340, 62);
		MapDialogRect(&rcAcSeg);
		if (m_stAcSeg.Create(_T(""), WS_CHILD | SS_LEFT | SS_NOPREFIX, rcAcSeg, this) != FALSE)
			m_stAcSeg.SetFont(GetFont());
	}

	SetTimer(1, 500, NULL);	// LYS 2026-08-10 Phase 1: status refresh

	return TRUE;
}

void CDlgSubManual_AutoContact::GetAutoContactData()
{
	CString strPath = m_strIniPath;
	
	TCHAR szValue[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ReadyPos"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_READY] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_READY] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("FirstLimitPos"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_FLIMIT] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_FLIMIT] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("FirstMoveGap"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_FGAP] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_FGAP] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("SecondMoveGap"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_SGAP] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_SGAP] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ODMax"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_OD_MAX] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_OD_MAX] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("Percent"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_PERCENT] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_PERCENT] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("Portfail"), _T("0.0"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_PORT_FAIL] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_PORT_FAIL] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("AddOD"), _T("0.01"), szValue, MAX_PATH, strPath);
	//m_pSystem->stInterLock.m_fAutoContactData[AUTO_CT_ADD_OD] = atof(szValue);
	m_fAutoContactParam[AUTO_CT_ADD_OD] = _tstof(szValue);	// LYS 2026-08-10 Phase 1
	
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataReady"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fAutoCntReady = _tstof(szValue);	// LYS 2026-08-10 fix: was _ttoi (double truncation)
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos1"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fAutoCntContactPos = _tstof(szValue);	// LYS 2026-08-10 fix
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos2"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fAutoCntODPos = _tstof(szValue);	// LYS 2026-08-10 fix
	
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataRcpID"), _T("UNKNOWN"), szValue, MAX_PATH, strPath);
	m_strAutoCntRecipeID = szValue;
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataProbeID"), _T("UNKNOWN"), szValue, MAX_PATH, strPath);
	m_strAutoCnttProbeID = szValue;
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataSucess"), _T("0"), szValue, MAX_PATH, strPath);
	m_bAutoCntSucess = _ttoi(szValue);
	
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataCntUse"), _T("0"), szValue, MAX_PATH, strPath);
	m_bAutoCntUse = _ttoi(szValue);
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataModelChange"), _T("0"), szValue, MAX_PATH, strPath);
	m_bAutoCntModel = _ttoi(szValue);
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataProbeChange"), _T("0"), szValue, MAX_PATH, strPath);
	m_bAutoCntProbeID = _ttoi(szValue);
	GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataAlways"), _T("0"), szValue, MAX_PATH, strPath);
	m_bAutoCntAlways = _ttoi(szValue);
}

void CDlgSubManual_AutoContact::UpdateStatus(BOOL bEnable)
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) m_btnStop.ShowWindow(FALSE);
	//else m_btnStop.ShowWindow(!bEnable);
	m_btnStop.ShowWindow(m_bAutoCntRunning ? SW_SHOW : SW_HIDE);	// LYS 2026-08-10 Phase 4

	// LYS 2026-08-10: always show the currently open recipe in EDIT_1
	// (recipe can be opened from other screens - refreshed by the 500ms timer)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
	{
		CString strCurRcp, strEditRcp;
		strCurRcp = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
		GetDlgItem(IDC_EDIT_1)->GetWindowText(strEditRcp);
		if (strCurRcp != strEditRcp) GetDlgItem(IDC_EDIT_1)->SetWindowText(strCurRcp);
	}
	// LYS 2026-10-13 Hybrid Phase 7 G5 timer guard: while a hybrid recipe is being measured the measure thread rewrites the active stick list
	//   by value (SetActiveSegment) - skip the stick-list read, ints only (HARD RULE 3). 1-segment recipe (cache 1): unchanged
	bool bAcListBusy = (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegCountCache() > 1
		&& (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false || CSystemParamShare::GetInstance()->IsCycleState()));
	if (bAcListBusy == false)
		UpdateStickList();	// LYS 2026-08-11: follows recipe change (guarded by stick-count cache)
	UpdateAcSegLabel();	// LYS 2026-10-13 Hybrid Phase 7 D7-17 segment label (hybrid only; DLL read only while idle)

	// LYS 2026-08-11: auto-contact progress display
	{
		CString strProg;
		switch (m_nAcProgState)
		{
		case 1: strProg.Format(_T("Stick %d/%d moving"), m_nAcProgCur, m_nAcProgTotal); break;
		case 2: strProg.Format(_T("Stick %d/%d searching"), m_nAcProgCur, m_nAcProgTotal); break;
		case 3: strProg = _T("Done"); break;
		case 4: strProg.Format(_T("Fail (stick %d)"), m_nAcProgCur); break;
		case 5: strProg = _T("Stopped"); break;
		default: strProg = _T("Idle"); break;
		}
		CString strProgOld;
		GetDlgItem(IDC_STATIC_AC_PROG)->GetWindowText(strProgOld);
		if (strProgOld != strProg) GetDlgItem(IDC_STATIC_AC_PROG)->SetWindowText(strProg);
	}

	for (int i = IDC_BUTTON_2; i < IDC_BUTTON_5 + 1; i++) GetDlgItem(i)->EnableWindow(bEnable);
	for (int i = IDC_CHECK_1; i < IDC_CHECK_3 + 1; i++) GetDlgItem(i)->EnableWindow(bEnable);

	CString strPos;
	//strPos.Format(_T("%.4lf"), m_pSystem->stMotionStatus[AXIS_UNIT_Z].dAxisPos);
	// LYS 2026-08-10 Phase 2: INI 계약에서 탐색 결과 상태를 갱신
	// (시퀀스는 CAutoContactInfo로 갱신하고, UI 타이머는 공유 싱글톤을 건드리지 않고
	//  파일을 직접 읽는다 - 스레드 안전)
	if (!m_strIniPath.IsEmpty())
	{
		TCHAR szValue[MAX_PATH];
		ZeroMemory(szValue, sizeof(szValue));
		GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataReady"), _T("0.0"), szValue, MAX_PATH, m_strIniPath);
		m_fAutoCntReady = _tstof(szValue);
		GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos1"), _T("0.0"), szValue, MAX_PATH, m_strIniPath);
		m_fAutoCntContactPos = _tstof(szValue);
		GetPrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos2"), _T("0.0"), szValue, MAX_PATH, m_strIniPath);
		m_fAutoCntODPos = _tstof(szValue);
		m_bAutoCntSucess = (BOOL)GetPrivateProfileInt(_T("PROBE_AUTO_CONTACT"), _T("ContactDataSucess"), 0, m_strIniPath);
	}

	// LYS 2026-08-10 Phase 1: OCP mode P5 probe Z (C1)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetMaintControlMgr() != NULL)
		strPos.Format(_T("%.4lf"), m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_PROBE_Z5));
	GetDlgItem(IDC_EDIT_2)->SetWindowText(strPos);

	strPos.Format(_T("Ready:%.4lf, Contact:%.4lf, OD:%.4lf"), m_fAutoCntReady, m_fAutoCntContactPos, m_fAutoCntODPos);
	GetDlgItem(IDC_EDIT_3)->SetWindowText(strPos);
}

// LYS 2026-08-10 Phase 1: periodic status refresh
void CDlgSubManual_AutoContact::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1 && IsWindowVisible())
	{
		UpdateStatus(!m_bAutoCntRunning);	// LYS 2026-08-10 Phase 4: disable buttons while running
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CDlgSubManual_AutoContact::OnStop()
{
	//gLog(m_pSystem->hLog, "CDlgSubManual_AutoContact::OnStop()", LOG_LEVEL_O);
	//m_pJobProc->ActionStop();
	// LYS 2026-08-10 Phase 4: stop the running search
	CAutoContactInfo::GetInstance()->RequestStop();
}

void CDlgSubManual_AutoContact::OnRcpOpen()
{
	if (!IsOfflineStatus()) return;

	// LYS 2026-09-29 Hybrid 3b A13 no recipe change while a hybrid recipe is being measured (RecipeChange mid-run resets the segments). 1-segment recipes: unchanged
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1
		&& (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false || CSystemParamShare::GetInstance()->IsCycleState()))
	{
		AfxMessageBox(_T("하이브리드 레시피 측정 중에는 이 작업을 할 수 없습니다.\n측정이 끝난 뒤 다시 시도하십시오."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-10-12 1-segment recipes too (user request): a recipe open / save / reload / START mid-run changes the running glass
	//   (e.g. Sampling) and resets the EFA / cell ID / result latches and the cell status - blocked while any glass is being measured
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false || CSystemParamShare::GetInstance()->IsCycleState())
	{
		AfxMessageBox(_T("측정 중에는 이 작업을 할 수 없습니다.\n측정이 끝난 뒤 다시 시도하십시오."), MB_ICONWARNING);
		return;
	}

	//gLog(m_pSystem->hLog, "CDlgSubManual_AutoContact::OnRcpOpen()", LOG_LEVEL_O);

	CString filename = _T(""), strRcpFile;
	CString filter = _T("Recipe Files (*.rcp)|*.rcp|");
	//filename.Format("%s%s%s*.rcp", m_pSystem->m_strRootPath, STR_DIR_PARAM, dfRECIPE_DATA_FILE_PATH);

	CFileDialog dlg(TRUE, _T("rcp"), filename, OFN_FILEMUSTEXIST, filter); //OFN_FILEMUSTEXIST (존재하지 않는 파일명을 입력할 수 없음)
	//dlg.GetFileName().Format("%s", m_pSystem->stLotInfo.m_szRecipeID);

	if (dlg.DoModal() == IDOK)
	{
		filename = dlg.GetFileName();

		if (filename.IsEmpty()) MessageBox(_T("File is not an Recipe file or is locked and cannot be imported."));
		else
		{

			int j = 0;
			j = filename.Find(_T(".rcp"));

			if (j > 0)
			{
				strRcpFile = filename.Left(j);		// remove ".rcp"
			}
			else
			{
				strRcpFile = filename;
			}

			//if (m_pDlgNaviRecipe->LoadPPID2(strRcpFile, TRUE) == VTM_RV_SUCCESSED)
			// LYS 2026-08-11: real recipe load via RecipeChange (AgingTest pattern - P1 omission fix)
			if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL
				&& m_pAuroraMgr->GetRecipeEditMgrPtr()->RecipeChange(strRcpFile) == VTM_RV_SUCCESSED)
			{
				MessageBox(_T("Recipe file be imported complete."));
				GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());
				UpdateStickList();	// LYS 2026-08-11 manual-only

				//GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pSystem->stLotInfo.m_szRecipeID);

				return;
			}
			else	// LYS 2026-08-11
			{
				AfxMessageBox(_T("Load Recipe Error"));
			}
		}
	}

	return;
}

void CDlgSubManual_AutoContact::OnAutoContactSave()
{
	if (!IsOfflineStatus()) return;

	if (AfxMessageBox(_T("현재 설정값을 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strPath = m_strIniPath;
	CString strTemp;

	double fTemp[AUTO_CONTACT_ITEM_MAX];
	memset(fTemp, 0.0, sizeof(double) * AUTO_CONTACT_ITEM_MAX);

	//m_spreadAutoContact.SetCol(2);
	for (int i = 0; i < AUTO_CONTACT_ITEM_MAX; i++) {
		//m_spreadAutoContact.SetRow(i + 2);
		strTemp = m_spreadAutoContact->GetDataString(1, i + 1);
		fTemp[i] = _tstof(strTemp);	// LYS 2026-08-10 fix: was _ttoi (double truncation)
		//m_pSystem->stInterLock.m_fAutoContactData[i] = atof (strTemp);
	}

	if (fTemp[AUTO_CT_READY] >= fTemp[AUTO_CT_FLIMIT]) {
		AfxMessageBox(_T("1차 Limit 위치를 Ready 위치값보다 크게 입력하십시오."), MB_ICONWARNING);
		return;
	}

	if (fTemp[AUTO_CT_FGAP] > 0.5) {
		AfxMessageBox(_T("1차 Move Gap 값은 0.5mm 이하로 입력 하십시오."), MB_ICONWARNING);
		return;
	}

	if (fTemp[AUTO_CT_SGAP] > 0.1) {
		AfxMessageBox(_T("2차 Move Gap 값은 0.1mm 이하로 입력 하십시오."), MB_ICONWARNING);
		return;
	}

	if (fTemp[AUTO_CT_OD_MAX] > 1.0) {
		AfxMessageBox(_T("OD Max 값은 1mm 이하로 입력 하십시오."), MB_ICONWARNING);
		return;
	}

	if (fTemp[AUTO_CT_ADD_OD] > 0.5) {
		AfxMessageBox(_T("Add OD 값은 0.5mm 이하로 입력 하십시오."), MB_ICONWARNING);
		return;
	}

	if (fTemp[AUTO_CT_PERCENT] < 0.0 || fTemp[AUTO_CT_PERCENT] > 100.0)
	{
		AfxMessageBox(_T("R-Test 성공률 값은 0.0 ~ 100.0 이하로 입력 하십시오."), MB_ICONWARNING);
		return;
	}

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_READY]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ReadyPos"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_FLIMIT]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("FirstLimitPos"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_FGAP]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("FirstMoveGap"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_SGAP]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("SecondMoveGap"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_OD_MAX]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ODMax"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_PERCENT]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("Percent"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_PORT_FAIL]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("PortFail"), strTemp, strPath);

	strTemp.Format(_T("%.4lf"), fTemp[AUTO_CT_ADD_OD]);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("AddOD"), strTemp, strPath);

	for (int i = 0; i < AUTO_CONTACT_ITEM_MAX; i++) {
		//m_pSystem->stInterLock.m_fAutoContactData[i] = fTemp[i];
		m_fAutoContactParam[i] = fTemp[i];	// LYS 2026-08-10 Phase 1 local store (Phase 2: helper)
	}

	AutoCntPosReset();
}

void CDlgSubManual_AutoContact::OnAutoContactProcStrart()
{
	// LYS 2026-08-10 Phase 4: 오토 컨택 탐색 1회 실행
	// (C5: 시퀀스 Contact 스텝과 동일 루틴, 단발 워커)
	if (!IsOfflineStatus()) return;
	if (m_pAuroraMgr == NULL) return;
	if (m_bAutoCntRunning == TRUE) return;

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE
		|| CSequenceManager::GetInstance()->IsEngineCompleteAll() != true)
	{
		AfxMessageBox(_T("Process is running.\n Run after the process completes."), MB_ICONWARNING);
		return;
	}

	if (m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_PROBE_Z5) > 0.1)
	{
		AfxMessageBox(_T("Probe Z(P5) is not at 0 position.\n Move Z to 0 first."), MB_ICONWARNING);
		return;
	}

	CString strRcp;
	GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	if (strRcp == _T("") || strRcp == _T("UNKNOWN"))
	{
		AfxMessageBox(_T("No recipe info.\n Open a recipe first."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-08-11: stick selection (ALL or a single stick)
	int nSelAcStick = m_comboAcStick.GetCurSel();
	if (nSelAcStick == CB_ERR)
	{
		AfxMessageBox(_T("Select a stick first."), MB_ICONWARNING);
		return;
	}
	m_nAcSelStick = (nSelAcStick == 0) ? -1 : nSelAcStick - 1;

	if (AfxMessageBox(_T("Run Auto Contact Proc.\n Continue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	if (strRcp != m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID())
	{
		AfxMessageBox(_T("Recipe differs from the loaded one.\n Reload the recipe first."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-10-13 Hybrid Phase 7 (14)-I START gate + D7-16 (a) SPS mark - hybrid runtime recipe only (1 segment: returns true, no log)
	if (CheckHybridAcStart() == false) return;

	m_bAutoCntRunning = TRUE;
	CAutoContactInfo::GetInstance()->ClearStopRequest();
	CWinThread* pAcThread = AfxBeginThread(AutoContactSearchThread, this);	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 3): return value checked below
	if (pAcThread == NULL && m_bAcSpsMarked == TRUE)	// hybrid START marked SPS but no worker (no guard) will run: undo it here. 1 segment: never marked, unchanged
	{
		m_bAcSpsMarked = FALSE;
		m_bAutoCntRunning = FALSE;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("[HYBRID] AutoContact worker thread not started - SPS_IDLE restored"));
		AfxMessageBox(_T("AUTO CONTACT 작업 스레드를 시작하지 못했습니다.\n다시 실행하십시오."), MB_ICONWARNING);
	}

	//if (!IsOfflineStatus()) return;
	//if (SMEM_GetSys_ProcessStatus() == SPS_PROCESS) {
	//	AfxMessageBox(_T("프로세스 진행중 입니다.\n 프로세스 완료후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//if (m_pSystem->stMotionStatus[AXIS_UNIT_Z].dAxisPos > 0.1) {
	//	AfxMessageBox(_T("UNIT Z축 0위치가 아닙니다.\n 레시피 설정 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//CString strRcp;
	//GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	//if (strRcp == _T("") || strRcp == _T("UNKNOWN")) {
	//	AfxMessageBox(_T("레시피 정보가 없습니다.\n 레시피 설정 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//if (AfxMessageBox(_T("Auto Contact Proc 를 진행 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	//
	//if (strRcp != m_pSystem->stLotInfo.m_szRecipeID) {
	//	AfxMessageBox(_T("현재 로드된 레시피의 정보와 다릅니다.\n 레시피 리로드 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//m_pJobProc->DoStartAutoContact();
}

void CDlgSubManual_AutoContact::OnPgRecipeDownload()
{
	// LYS 2026-08-10 Phase 4: PG recipe deployment (C4: HMI Recipe-step equivalent)
	if (!IsOfflineStatus()) return;
	if (m_pAuroraMgr == NULL) return;
	if (m_bAutoCntRunning == TRUE) return;

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE
		|| CSequenceManager::GetInstance()->IsEngineCompleteAll() != true)
	{
		AfxMessageBox(_T("Process is running.\n Run after the process completes."), MB_ICONWARNING);
		return;
	}

	CString strRcp;
	GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	if (strRcp == _T("") || strRcp == _T("UNKNOWN"))
	{
		AfxMessageBox(_T("No recipe info.\n Open a recipe first."), MB_ICONWARNING);
		return;
	}

	if (AfxMessageBox(_T("Run PG Recipe Download.\n Continue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	if (strRcp != m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID())
	{
		AfxMessageBox(_T("Recipe differs from the loaded one.\n Reload the recipe first."), MB_ICONWARNING);
		return;
	}

	m_bAutoCntRunning = TRUE;
	AfxBeginThread(PgDownloadThread, this);

	//if (!IsOfflineStatus()) return;
	//if (SMEM_GetSys_ProcessStatus() == SPS_PROCESS) {
	//	AfxMessageBox(_T("프로세스 진행중 입니다.\n 프로세스 완료후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//CString strRcp;
	//GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	//if (strRcp == _T("") || strRcp == _T("UNKNOWN")) {
	//	AfxMessageBox(_T("레시피 정보가 없습니다.\n 레시피 설정 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//if (AfxMessageBox(_T("PG Recipe Download 를 진행 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	//
	//if (strRcp != m_pSystem->stLotInfo.m_szRecipeID) {
	//	AfxMessageBox(_T("현재 로드된 레시피의 정보와 다릅니다.\n 레시피 리로드 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//m_pJobProc->DoStartPgDownload();
}

BOOL CDlgSubManual_AutoContact::IsOfflineStatus()
{
	// LYS 2026-08-10 Phase 1: restore offline gate via ShareMemoryMgr (AgingSet pattern)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetShareMemoryMgrPtr() != NULL) {
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE) {
			AfxMessageBox(_T("Not in Offline mode.\n Switch to Offline mode first."), MB_ICONWARNING);
			return FALSE;
		}
	}

	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) {
	//	AfxMessageBox(_T("Offline 모드가 아닙니다..\n Offline 모드로 변경 후 진행 하십시오."), MB_ICONWARNING);
	//	return FALSE;
	//}

	return TRUE;
}

void CDlgSubManual_AutoContact::OnAutoContactUse()
{
	if (AfxMessageBox(_T("프로브 자동 컨텍 사용 유무를 변경 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) {
		m_btnAutoContactUse.SetCheck(m_bAutoCntUse);
		return;
	}

	m_bAutoCntUse = !m_bAutoCntUse;
	m_btnAutoContactUse.SetCheck(m_bAutoCntUse);

	CString strTemp;
	strTemp.Format(_T("%d"), m_bAutoCntUse);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataCntUse"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::OnModelChangeUse()
{
	if (AfxMessageBox(_T("프로브 자동 컨텍 모델 체인지 사용 유무를 변경 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) {
		m_btnAutoModelChangeUse.SetCheck(m_bAutoCntModel);
		return;
	}

	m_bAutoCntModel = !m_bAutoCntModel;
	m_btnAutoModelChangeUse.SetCheck(m_bAutoCntModel);

	CString strTemp;
	strTemp.Format(_T("%d"), m_bAutoCntModel);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataModelChange"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::OnProbeChangeUse()
{
	if (AfxMessageBox(_T("프로브 자동 컨텍 프로브 체인지 사용 유무를 변경 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) {
		m_btnAutoProbeChangeUse.SetCheck(m_bAutoCntProbeID);
		return;
	}

	m_bAutoCntProbeID = !m_bAutoCntProbeID;
	m_btnAutoProbeChangeUse.SetCheck(m_bAutoCntProbeID);

	CString strTemp;
	strTemp.Format(_T("%d"), m_bAutoCntProbeID);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataProbeChange"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::OnAlwaysUse()
{
	if (AfxMessageBox(_T("프로브 자동 컨텍 상시 사용 유무를 변경 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) {
		m_btnAutoProbeAlways.SetCheck(m_bAutoCntAlways);
		return;
	}

	m_bAutoCntAlways = !m_bAutoCntAlways;
	m_btnAutoProbeAlways.SetCheck(m_bAutoCntAlways);

	CString strTemp;
	strTemp.Format(_T("%d"), m_bAutoCntAlways);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataAlways"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::SaveAutoCntPos(double fReady, double fPreContact, double fOD)
{
	CString strTemp;

	m_fAutoCntReady = fReady;
	strTemp.Format(_T("%.4lf"), m_fAutoCntReady);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataReady"), strTemp, m_strIniPath);

	m_fAutoCntContactPos = fPreContact;
	strTemp.Format(_T("%.4lf"), m_fAutoCntContactPos);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos1"), strTemp, m_strIniPath);

	m_fAutoCntODPos = fOD;
	strTemp.Format(_T("%.4lf"), m_fAutoCntODPos);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataPos2"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::AutoCntPosReset()
{
	m_fAutoCntReady = 0.0;
	m_fAutoCntContactPos = 0.0;
	m_fAutoCntODPos = 0.0;
	//m_bAutoCntSucess     = FALSE;
	SetAutoCntSucess(FALSE);
}

void CDlgSubManual_AutoContact::SetAutoCntRcpID(CString strID)
{
	CString strTemp;

	m_strAutoCntRecipeID = strID;
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataRcpID"), m_strAutoCntRecipeID, m_strIniPath);
}

void CDlgSubManual_AutoContact::SetAutoCntProbeID(CString strID)
{
	CString strTemp;

	m_strAutoCnttProbeID = strID;
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("m_cAutoCnttProbeID"), m_strAutoCnttProbeID, m_strIniPath);
}

void CDlgSubManual_AutoContact::SetAutoCntSucess(BOOL bSucess)
{
	CString strTemp;

	m_bAutoCntSucess = bSucess;
	strTemp.Format(_T("%d"), m_bAutoCntSucess);
	WritePrivateProfileString(_T("PROBE_AUTO_CONTACT"), _T("ContactDataSucess"), strTemp, m_strIniPath);
}

void CDlgSubManual_AutoContact::SetAutoCntLog()
{
	CString strTemp;

	strTemp.Format(_T("%.4lf"), m_fAutoCntReady);
	WritePrivateProfileString(m_strAutoCnttProbeID, _T("Ready"), strTemp, m_strAutoLogPath);

	strTemp.Format(_T("%.4lf"), m_fAutoCntContactPos);
	WritePrivateProfileString(m_strAutoCnttProbeID, _T("ContactPos"), strTemp, m_strAutoLogPath);

	strTemp.Format(_T("%.4lf"), m_fAutoCntODPos);
	WritePrivateProfileString(m_strAutoCnttProbeID, _T("OD"), strTemp, m_strAutoLogPath);
}

// LYS 2026-10-13 Hybrid Phase 7 (14)-A hybrid runtime predicate (fix stage, impl review 1): live DLL count > 1, or the segment-count cache > 1 while a LoadRcpFile runs.
//   The live count is read only while no LoadRcpFile runs (*pbLoadBusy); *pnLive = that count (1 while loading). No log, no write.
static bool IsAcHybridRuntime(CRecipeEditMgr* pRcp, bool* pbLoadBusy, int* pnLive)
{
	if (pbLoadBusy != NULL) *pbLoadBusy = false;
	if (pnLive != NULL) *pnLive = 1;
	if (pRcp == NULL) return false;
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	bool bLoadBusy = (pFunc != NULL && pFunc->IsRcpLoadBusy() == TRUE);
	int nLive = (pFunc != NULL && bLoadBusy == false) ? pFunc->GetSegmentLoopCount() : 1;
	if (pbLoadBusy != NULL) *pbLoadBusy = bLoadBusy;
	if (pnLive != NULL) *pnLive = nLive;
	return bLoadBusy ? (pRcp->GetSegCountCache() > 1) : (nLive > 1);	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 1): a stale cache alone = 1 segment
}

// LYS 2026-08-11 수동 전용: 선택 stick(또는 ALL) 이동 + 오토 컨택 탐색.
// stick별 흐름: 프로브 Z 상승 -> stage R/X 이동(MoveMeasureArea OCP 규칙)
// -> PG 상태 stick 슬롯 -> DoSearch. 얼라인 체인은 미포함(정렬된 상태 전제).
UINT CDlgSubManual_AutoContact::AutoContactSearchThread(LPVOID pParam)
{
	CDlgSubManual_AutoContact* pThis = (CDlgSubManual_AutoContact*)pParam;
	if (pThis == NULL || pThis->m_pAuroraMgr == NULL) return 0;

	CAuroraMgr* pMgr = pThis->m_pAuroraMgr;
	CAutoContactInfo* pAutoCnt = CAutoContactInfo::GetInstance();
	// LYS 2026-10-13 Hybrid Phase 7 (14)-I: START set SPS_CYCLETEST for a hybrid recipe (m_bAcSpsMarked) - this guard restores SPS_IDLE on EVERY exit
	//   below (the early return after the stick range check and the normal end). 1-segment recipe: nothing marked, the guard does nothing
	struct CAcSpsGuard
	{
		CDlgSubManual_AutoContact* m_pDlg;
		explicit CAcSpsGuard(CDlgSubManual_AutoContact* pDlg) : m_pDlg(pDlg) {}
		~CAcSpsGuard()
		{
			if (m_pDlg->m_bAcSpsMarked != TRUE) return;
			m_pDlg->m_bAcSpsMarked = FALSE;
			m_pDlg->m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);
			CLogWrapper::GetInstance()->KMLOG(OPER, _T("[HYBRID] AutoContact worker end - SPS_IDLE restored"));
		}
	} acSpsGuard(pThis);
	bool bAcHybrid = (pThis->m_bAcSpsMarked == TRUE) || IsAcHybridRuntime(pMgr->GetRecipeEditMgrPtr(), NULL, NULL);	// LYS 2026-10-13 Hybrid Phase 7 worker backstop below
	pAutoCnt->LoadAll();

	// reset judgment moved here from the sequence (manual-only, A2)
	CString strProbeID;
	strProbeID.Format(_T("%s"), pMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[4]);
	strProbeID.TrimRight();
	pAutoCnt->CheckAndReset(pMgr->GetRecipeEditMgrPtr()->GetRecipeID(), strProbeID);

	S_RecipeData* pRcpData = pMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	S_RecipeInfo* pRcpInfo = pMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	S_RecipeMgr*  pslData  = pMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
	int nMaxStick = (pRcpData != NULL) ? (int)pRcpData->m_vecStick.size() : 0;
	int nCellCnt  = (pslData != NULL) ? pslData->m_nCellCnt : 0;

	int nFrom = 0, nTo = nMaxStick - 1;
	if (pThis->m_nAcSelStick >= 0) nFrom = nTo = pThis->m_nAcSelStick;

	if (nMaxStick <= 0 || nTo >= nMaxStick || pRcpInfo == NULL)
	{
		pThis->m_nAcProgState = 4;
		pThis->m_bAutoCntRunning = FALSE;
		return 0;
	}

	pThis->m_nAcProgTotal = nMaxStick;

	int nRtn = VTM_RV_SUCCESSED;
	for (int iStick = nFrom; iStick <= nTo; iStick++)
	{
		pThis->m_nAcProgCur = iStick + 1;
		pThis->m_nAcProgState = 1;	// moving

		if (pAutoCnt->IsStopRequested()) { nRtn = VTM_RV_STOP_BY_USER; break; }

		// lift probe Z before the stage move
		nRtn = (int)pMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_PROBE_Z5, 0.0, true, SPEED_HIGH);
		if (nRtn != VTM_RV_SUCCESSED) break;
		nRtn = (int)pMgr->GetMaintControlMgr()->DoWork_MotionDone(AXIS_PROBE_Z5);	// LYS 2026-08-11 in-position
		if (nRtn != VTM_RV_SUCCESSED) break;

		// stage R then X to the stick position (MoveMeasureArea OCP rule)
		double dStageY = pRcpData->m_vecStick[iStick].m_fStageY + pRcpInfo->m_sMotionOffsetPos.m_fStageY;
		double dStageR = pRcpData->m_vecStick[iStick].m_fStageR + pRcpInfo->m_sMotionOffsetPos.m_fStageRotate;

		// LYS 2026-10-13 Hybrid Phase 7 worker backstop (R-A / R-C, hybrid only): START checked every target stick - refuse here too if the stick
		//   list or the Glass Align mark changed since. The probe is already up (Z5 -> 0 above); no stage R / X move after a refusal. 1 segment: skipped
		if (bAcHybrid)
		{
			double dAcRefR = 0.0;
			bool bAcRef = pMgr->GetMaintControlMgr()->GetGlassAlignRefR(&dAcRefR);
			bool bAcOri = (bAcRef && fabs(dStageR - dAcRefR) > 2.0);	// D7-6 2.0 deg, same coordinate as the MoveCmdPosA target below (no modulo)
			if (bAcOri || (bAcRef == false && pRcpInfo->m_sOptionItem.m_bUseAlign == TRUE))
			{
				CString strAcLog, strAcMsg, strAcRef(_T("none")), strAcRefK(_T("없음"));
				if (bAcRef) { strAcRef.Format(_T("%.3f"), dAcRefR); strAcRefK.Format(_T("%.1f"), dAcRefR); }
				strAcLog.Format(_T("[HYBRID] AutoContact worker refused - %s (stick %d, target R %.3f, align R %s) - stopped before the stage move"),
					bAcOri ? _T("orientation") : _T("align"), iStick + 1, dStageR, (LPCTSTR)strAcRef);
				CLogWrapper::GetInstance()->KMLOG(WARN, strAcLog);
				strAcMsg.Format(_T("Glass Align 방향(R %s)과 스틱 %d 방향(R %.1f)이 다르거나 Align이 없어 AUTO CONTACT를 멈췄습니다.\n\n수동 측정 화면에서 MOVE SAFETY 후 ALIGN 동작을 먼저 하십시오."),
					(LPCTSTR)strAcRefK, iStick + 1, dStageR);
				AfxMessageBox(strAcMsg, MB_ICONWARNING);
				nRtn = VTM_RV_STOP_BY_USER;	// state 5 "Stopped", no alarm
				break;
			}
		}

		if (fabs(dStageR - pMgr->GetMaintControlMgr()->GetCurPos(AXIS_LOADER_R)) > 0.0001)
		{
			nRtn = (int)pMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_LOADER_R, dStageR, true, SPEED_HIGH);
			if (nRtn != VTM_RV_SUCCESSED) break;
			nRtn = (int)pMgr->GetMaintControlMgr()->DoWork_MotionDone(AXIS_LOADER_R);	// LYS 2026-08-11 in-position
			if (nRtn != VTM_RV_SUCCESSED) break;
		}

		nRtn = (int)pMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH);
		if (nRtn != VTM_RV_SUCCESSED) break;
		nRtn = (int)pMgr->GetMaintControlMgr()->DoWork_MotionDone(AXIS_X);	// LYS 2026-08-11 stage move done
		if (nRtn != VTM_RV_SUCCESSED) break;

		// PG status stick slot (same rotate conversion as OST/RTEST steps)
		if (pRcpInfo->m_sOptionItem.m_nGlsRotate == 1)
			pMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = iStick;
		else
			pMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nMaxStick - iStick - 1;

		Sleep(2000);
		pThis->m_nAcProgState = 2;	// searching
		nRtn = pAutoCnt->DoSearch(pMgr->GetMaintControlMgr(), pMgr->GetCommTcpIpFactoryMgrPtr(), nCellCnt);
		if (nRtn != VTM_RV_SUCCESSED) break;
	}

	if (nRtn == VTM_RV_SUCCESSED)
	{
		// LYS 2026-08-11: display-only success mark (the sequence no longer consumes it)
		pAutoCnt->SetSuccess(true);
		pAutoCnt->SaveLog();
		pThis->m_nAcProgState = 3;	// done
	}
	else if (nRtn == VTM_RV_STOP_BY_USER)
	{
		pThis->m_nAcProgState = 5;	// stopped
	}
	else
	{
		pThis->m_nAcProgState = 4;	// fail
		CAlarmController::GetInstance()->SetAlarm(nRtn);
	}

	pThis->m_bAutoCntRunning = FALSE;
	return 0;
}

// LYS 2026-08-10 Phase 4: PG 레시피 다운로드 워커
// (OCP ExePGCtrl(5/6/2/0/1) -> HMI DoWorkRecipeOpen(PG), C4)
UINT CDlgSubManual_AutoContact::PgDownloadThread(LPVOID pParam)
{
	CDlgSubManual_AutoContact* pThis = (CDlgSubManual_AutoContact*)pParam;
	if (pThis == NULL || pThis->m_pAuroraMgr == NULL) return 0;

	long nRtn = pThis->m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);

	pThis->m_bAutoCntRunning = FALSE;

	if (nRtn != VTM_RV_SUCCESSED)
		AfxMessageBox(_T("PG Recipe Download failed."), MB_ICONWARNING);

	return 0;
}

// LYS 2026-08-11 manual-only: fill the stick selector from the open recipe (ALL + 1..N)
void CDlgSubManual_AutoContact::UpdateStickList()
{
	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return;
	if (m_comboAcStick.GetSafeHwnd() == NULL) return;

	int nStick = 0;
	S_RecipeData* pRcpData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	if (pRcpData != NULL) nStick = (int)pRcpData->m_vecStick.size();

	if (nStick == m_nAcStickCnt) return;	// rebuild only on count change
	m_nAcStickCnt = nStick;

	m_comboAcStick.ResetContent();
	if (nStick <= 0) return;

	m_comboAcStick.AddString(_T("ALL"));
	CString strNo;
	for (int i = 0; i < nStick; i++)
	{
		strNo.Format(_T("%d"), i + 1);
		m_comboAcStick.AddString(strNo);
	}
	m_comboAcStick.SetCurSel(0);
}

// LYS 2026-10-13 Hybrid Phase 7 (14)-I AutoContact START gate (p7x_plan (4-1)). UI thread, after the confirm dialog and the recipe check.
//   1-segment runtime recipe (cache 1, live 1): returns true at once - no stick-list read, no log, no SPS mark (D7-16 (a)).
//   Hybrid: idle re-check (CS-14, the confirm dialog was modal) -> selected stick in range (CS-1 / C-4) -> R-A for every target stick and R-C (D7-5)
//   -> SPS_CYCLETEST + m_bAcSpsMarked (the worker guard restores SPS_IDLE). The idle re-check comes first so the stick list is read only while idle.
bool CDlgSubManual_AutoContact::CheckHybridAcStart()
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	if (pRcp == NULL) return true;
	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 2): the idle ints first - while busy the hybrid test reads the segment-count cache int only (HARD RULE 3),
	//   the live DLL count only while idle (busy ? cache > 1 : live > 1, impl review 1). 1-segment recipe (cache 1, live 1): returns true, no log, no write
	int nSps = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus();
	bool bEngineDone = CSequenceManager::GetInstance()->IsEngineCompleteAll();
	bool bCycle = CSystemParamShare::GetInstance()->IsCycleState();
	C_OutRecipeFunc* pIdleFunc = pRcp->GetRecipeFuncPtr();
	bool bLoadBusy = (pIdleFunc != NULL && pIdleFunc->IsRcpLoadBusy() == TRUE);
	int nLive = 1;
	if (nSps != SPS_IDLE || bEngineDone != true || bCycle || bLoadBusy)
	{
		if (pRcp->GetSegCountCache() <= 1) return true;	// busy, 1 segment by the cache: unchanged path, no log
	}
	else if (IsAcHybridRuntime(pRcp, &bLoadBusy, &nLive) == false)
	{
		if (bLoadBusy == false && pRcp->GetSegCountCache() > 1)	// stale cache over a 1-segment reload: treated as 1 segment, silent re-sync (as the manual Meas gate), no log
			pRcp->SyncCellStatusStickBase();	// UI-thread write only while idle (just checked; the confirm dialog before was modal)
		return true;	// 1 segment: unchanged path, no log
	}

	CString strLog, strMsg;
	// (1) idle re-check - existing English popup (the idle ints read above, after the confirm dialog; IsAcHybridRuntime may have seen a load start)
	if (nSps != SPS_IDLE || bEngineDone != true || bCycle || bLoadBusy)
	{
		strLog.Format(_T("[HYBRID] AutoContact refused - process busy after the confirm dialog (SPS %d, engine complete %d, cycle %d, recipe load %d)"),
			nSps, bEngineDone ? 1 : 0, bCycle ? 1 : 0, bLoadBusy ? 1 : 0);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		AfxMessageBox(_T("Process is running.\n Run after the process completes."), MB_ICONWARNING);
		return false;
	}

	S_RecipeData* pRcpData = pRcp->GetRecipeData();
	S_RecipeInfo* pRcpInfo = pRcp->GetRecipeInfo();
	int nStick = (pRcpData != NULL) ? (int)pRcpData->m_vecStick.size() : 0;
	int nSeg = pRcp->GetActiveSegment() + 1;
	// (2) the stick combo may still list another segment's sticks (the timer runs only while this screen is visible)
	if (m_nAcSelStick >= nStick)
	{
		int nSel = m_nAcSelStick + 1;
		strLog.Format(_T("[HYBRID] AutoContact refused - stick %d out of range (segment %d/%d has %d sticks) - stick list reloaded"), nSel, nSeg, nLive, nStick);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		m_nAcStickCnt = -1;	// force the combo rebuild
		UpdateStickList();
		strMsg.Format(_T("선택한 스틱(%d)이 현재 세그먼트의 스틱 수(%d)를 넘습니다.\n스틱 목록을 다시 읽었습니다. 다시 선택하십시오."), nSel, nStick);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return false;
	}

	// (3) R-A per target stick (same R as AutoContactSearchThread dStageR: stick R + user offset R, no modulo - D7-3 / D7-6) and R-C (D7-5)
	CMaintControlMgr* pM = m_pAuroraMgr->GetMaintControlMgr();
	double dRef = 0.0;
	bool bRef = (pM != NULL && pM->GetGlassAlignRefR(&dRef));
	int nFrom = 0, nTo = nStick - 1;
	if (m_nAcSelStick >= 0) nFrom = nTo = m_nAcSelStick;
	int iBad = -1;
	double dBadR = 0.0;
	bool bAlignMissing = false;
	if (pRcpData != NULL && pRcpInfo != NULL && nFrom <= nTo)
	{
		for (int i = nFrom; i <= nTo && bRef; i++)
		{
			double dR = pRcpData->m_vecStick[i].m_fStageR + pRcpInfo->m_sMotionOffsetPos.m_fStageRotate;
			if (fabs(dR - dRef) > 2.0) { iBad = i; dBadR = dR; break; }
		}
		if (bRef == false && pRcpInfo->m_sOptionItem.m_bUseAlign == TRUE)
		{
			bAlignMissing = true;
			iBad = nFrom;
			dBadR = pRcpData->m_vecStick[nFrom].m_fStageR + pRcpInfo->m_sMotionOffsetPos.m_fStageRotate;
		}
	}
	CString strRef(_T("none")), strRefK(_T("없음"));
	if (bRef) { strRef.Format(_T("%.3f"), dRef); strRefK.Format(_T("%.1f"), dRef); }
	if (iBad >= 0)
	{
		if (bAlignMissing)
			strLog.Format(_T("[HYBRID] AutoContact refused - align (seg %d/%d stick %d, ALIGN USE on, no glass align mark, target R %.3f)"), nSeg, nLive, iBad + 1, dBadR);
		else
			strLog.Format(_T("[HYBRID] AutoContact refused - orientation (seg %d/%d stick %d, target R %.3f, align R %s)"), nSeg, nLive, iBad + 1, dBadR, (LPCTSTR)strRef);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		strMsg.Format(_T("Glass Align 방향(R %s)과 스틱 방향(R %.1f)이 다르거나 Align이 없습니다.\nAUTO CONTACT를 시작하지 않습니다.\n\n수동 측정 화면에서 MOVE SAFETY 후 ALIGN 동작을 먼저 하십시오."),
			(LPCTSTR)strRefK, dBadR);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return false;
	}

	// (4) D7-16 (a) mark the manual worker: the manual Meas worker, DLL recipe commands, CYCLE and AgingTest / AgingNew START refuse while SPS != IDLE
	m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_CYCLETEST);
	m_bAcSpsMarked = TRUE;
	CString strSel(_T("ALL"));
	if (m_nAcSelStick >= 0) strSel.Format(_T("%d"), m_nAcSelStick + 1);
	strLog.Format(_T("[HYBRID] AutoContact start - seg %d/%d stick %s (%d sticks), align R %s - SPS_CYCLETEST until the worker ends"), nSeg, nLive, (LPCTSTR)strSel, nStick, (LPCTSTR)strRef);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
	return true;
}

// LYS 2026-10-13 Hybrid Phase 7 D7-17 segment label (p7x_plan (4-1), (14)-I): "segment k/N (stick list = this segment)" for a hybrid recipe only.
//   GetActiveSegment / GetSegmentLoopCount read the DLL segment vectors, so they are read only while idle; busy keeps the last text (CS-15).
//   1-segment recipe: the label stays hidden and is never touched.
void CDlgSubManual_AutoContact::UpdateAcSegLabel()
{
	if (m_stAcSeg.GetSafeHwnd() == NULL || m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL) return;
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE
		|| CSequenceManager::GetInstance()->IsEngineCompleteAll() != true || CSystemParamShare::GetInstance()->IsCycleState())
		return;	// busy (a run, CYCLE, any manual worker incl. this screen's): ints only, keep the last text
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	bool bLoadBusy = false;
	int nLive = 1;
	bool bHyb = IsAcHybridRuntime(pRcp, &bLoadBusy, &nLive);
	if (bLoadBusy) return;	// a LoadRcpFile runs in another thread: keep the last text
	if (bHyb == false || nLive <= 1)
	{
		if (m_bAcSegShown == TRUE) { m_stAcSeg.ShowWindow(SW_HIDE); m_bAcSegShown = FALSE; }
		return;
	}
	CString strSeg, strOld;
	strSeg.Format(_T("세그먼트 %d/%d (스틱 목록 = 이 세그먼트)"), pRcp->GetActiveSegment() + 1, nLive);
	m_stAcSeg.GetWindowText(strOld);
	if (strOld != strSeg) m_stAcSeg.SetWindowText(strSeg);
	if (m_bAcSegShown == FALSE) { m_stAcSeg.ShowWindow(SW_SHOW); m_bAcSegShown = TRUE; }
}
