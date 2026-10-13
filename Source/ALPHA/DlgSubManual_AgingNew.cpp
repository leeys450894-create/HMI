// DlgSubManual_AgingNew.cpp
// LYS 2026-09-28 EFA: EFA(2-pass 에이징) 검사 화면 - OCP_EFA CDlgNaviManualAgingNew 이식.
// CDlgSubManual_AgingTest(2026-08-06 이식)를 복사해 EFA 차이만 반영했다:
//   - 모드 플래그 분리 : m_bAutoAgingMode 대신 VTM_SYSTEM::m_bNewAgingMode (OCP m_bNewAgingMode)
//   - rcp 폴더 분리    : 레지스트리 DIR\EFA_RCP (OCP는 실수로 DIR\AGING_RCP를 같이 써서 파일 충돌)
//   - 리스트 2행 고정  : T0(에이징 전) / T1(에이징 후), ITEM 추가/삭제 숨김
//   - Cycle 콤보 1..25, 패턴 콤보 0..25
// 파일 형식은 AgingTest와 같다({rcp}.rcp INI: [COUNT] [ITEM_n] [MASECELL]) - CAgingCycleInfo가 그대로 읽는다.
// /utf-8 없이 CP949로 컴파일되므로 사용자 메시지 문자열은 영어로 둔다(HMI/CLAUDE.md 규칙 7).
// LYS 2026-10-12 Hybrid Phase 6 note: the English-only rule above no longer holds - user strings here are Korean CP949 bytes (3c popups, Phase 6 title suffix)
#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AgingNew.h"
#include "AuroraMgr.h"
#include "Manager\ShareMemoryMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Manager\SequenceManager.h"
#include "Manager\SystemConfigMgr.h"
#include "DoWork\AgingCycleInfo.h"		// LYS 2026-09-30 Hybrid 3c A13 mask section names (engine loader)
#include "ResumePrompt.h"				// LYS 2026-10-08 Hybrid Phase 5 resume prompt on START (D5-11 a)
#include "afxdialogex.h"

#include <iostream>
#include <bitset>

#define EFA_TIMER_HIGHLIGHT		1
#define EFA_CELLMASK_BUF		1024	// [MASECELL] 한 줄 버퍼 (셀이 많으면 MAX_PATH로는 부족)

IMPLEMENT_DYNAMIC(CDlgSubManual_AgingNew, CDialogEx)

CDlgSubManual_AgingNew::CDlgSubManual_AgingNew(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSubManual_AgingNew::IDD, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{
	m_spreadAgingList = NULL;
	m_spreadAgingMeas = NULL;
	m_hfont = NULL;
	m_nHlRow = 0;
	m_nHlCol = 0;
	m_nMaskMismatch = 0;	// LYS 2026-09-30
	m_nViewSeg = 0;			// LYS 2026-09-30 Hybrid 3c A13
	m_nSegLostTick = 0;		// LYS 2026-09-30 Hybrid 3c A13 (S2 fix)
}

CDlgSubManual_AgingNew::~CDlgSubManual_AgingNew()
{
	// spread: CDlgSubManual_AgingTest와 같은 수명 처리 (자식 창은 다이얼로그와 함께 소멸)
}

void CDlgSubManual_AgingNew::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_BUTTON_1, m_chkStop);
	DDX_Control(pDX, IDC_COMBO_1, m_cmbCycle);
	DDX_Control(pDX, IDC_COMBO_EFA_SEG, m_cmbSeg);	// LYS 2026-09-30 Hybrid 3c A13 (control in ALPHA.rc, hidden by default)
	for (int i = 0; i < MAX_EFA_STICK; i++) DDX_Control(pDX, IDC_CHECK_1 + i, m_chkStick[i]);
}

BEGIN_MESSAGE_MAP(CDlgSubManual_AgingNew, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_COMMAND(IDC_BUTTON_1, OnStop)
	ON_COMMAND(IDC_BUTTON_2, OnRcpOpen)
	ON_COMMAND(IDC_BUTTON_3, OnItemClearAll)
	ON_COMMAND(IDC_BUTTON_4, OnItemAdd)
	ON_COMMAND(IDC_BUTTON_5, OnItemDel)
	ON_COMMAND(IDC_BUTTON_6, OnItemSave)
	ON_COMMAND(IDC_BUTTON_7, OnLoadAgingInfo)
	ON_COMMAND(IDC_BUTTON_8, OnAgingProcStrart)
	ON_CBN_SELCHANGE(IDC_COMBO_EFA_SEG, OnSelchangeSeg)	// LYS 2026-09-30 Hybrid 3c A13
	ON_MESSAGE(SSM_DBLCLK, OnSpreadDBClick)	// DLL spread가 더블클릭을 전달한다 (OCX EVENTSINK는 발생하지 않음)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDlgSubManual_AgingNew, CDialog)
	ON_EVENT(CDlgSubManual_AgingNew, IDC_FPSPREAD_AGING_CELL_STATUS, 11, CDlgSubManual_AgingNew::DblClickFpspreadCellStatus, VTS_I4 VTS_I4)
	ON_EVENT(CDlgSubManual_AgingNew, IDC_FPSPREAD_AGING_CELL_STATUS, 122, CDlgSubManual_AgingNew::SheetChangedFsMeasArea, VTS_I2 VTS_I2)
END_EVENTSINK_MAP()

BOOL CDlgSubManual_AgingNew::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

//=========================================================================
// 보조 함수
//=========================================================================
CString CDlgSubManual_AgingNew::GetEfaRcpPath()
{
	CString strPath = theApp.GetProfileString(_T("DIR"), _T("EFA_RCP"), _T("D:\\HBS\\PARAM_OCP\\PARAM\\EFARcp\\"));
	if (strPath.IsEmpty() == FALSE && strPath.Right(1) != _T("\\")) strPath += _T("\\");
	return strPath;
}

CString CDlgSubManual_AgingNew::GetRecipeID()
{
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		return m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	return _T("");
}

CString CDlgSubManual_AgingNew::GetEfaRcpFile()
{
	CString strRcpID = GetRecipeID();
	if (strRcpID.IsEmpty()) return _T("");
	return dfcStr(_T("%s%s.rcp"), m_strEfaRcpPath, strRcpID);
}

//=========================================================================
// 초기화
//=========================================================================
BOOL CDlgSubManual_AgingNew::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);
	m_LabelInfo.GetWindowText(m_strTitleBase);	// LYS 2026-10-12 Hybrid Phase 6 rc title; the highlight timer adds a suffix only while a hybrid EFA run measures
	m_strTitleShown = m_strTitleBase;

	m_chkStop.SetImage(IDB_CHECKNO32);
	m_chkStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_chkStop.ShowWindow(FALSE);

	// FarPoint spread (DLL). 리소스 ID +28/+29는 다른 수동 화면에서 쓰지 않는다.
	m_spreadAgingList = new CMySpreadDll(IDC_FPS_AGING_LIST, FPS_RESOURCE_ID + 28, this, TRUE);
	m_spreadAgingMeas = new CMySpreadDll(IDC_FPSPREAD_AGING_CELL_STATUS, FPS_RESOURCE_ID + 29, this, TRUE);

	if (m_spreadAgingList->GetSafeHwnd() != NULL)
	{
		m_spreadAgingList->InitDefault();

		SS_CELLTYPE cellType;
		m_spreadAgingList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
		m_spreadAgingList->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

		m_spreadAgingList->MakeFont(&m_hfont);
		m_spreadAgingList->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

		m_spreadAgingList->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_spreadAgingList->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_spreadAgingList->SetColHeaderRows(1);

		m_spreadAgingList->InitMySpread_AGINGTEST_LIST();	// AgingTest와 같은 7열 레이아웃
	}

	if (m_spreadAgingMeas->GetSafeHwnd() != NULL)
	{
		m_spreadAgingMeas->InitDefault();

		SS_CELLTYPE cellType;
		m_spreadAgingMeas->SetTypeCheckBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		m_spreadAgingMeas->SetCellTypeRange(1, 1, 10, 10, &cellType);

		m_spreadAgingMeas->MakeFont(&m_hfont);
		m_spreadAgingMeas->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

		m_spreadAgingMeas->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_spreadAgingMeas->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_spreadAgingMeas->SetColHeaderRows(1);

		m_spreadAgingMeas->InitMySpread_AGINGTEST_MEAS();
	}

	for (int i = 0; i < MAX_EFA_STICK; i++) {
		m_chkStick[i].SetImage(IDB_CHECKNO32);
		m_chkStick[i].SetCheckedImage(IDB_CHECK32);
		m_chkStick[i].m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
		m_chkStick[i].SetCheck(FALSE);
	}

	CString str;
	for (int i = 0; i < 25; i++) {	// OCP_EFA: Cycle 횟수 1..25 (AgingTest는 1..10)
		str.Format(_T("%d"), i + 1);
		m_cmbCycle.AddString(str);
	}
	m_cmbCycle.SetCurSel(0);

	m_strEfaRcpPath = GetEfaRcpPath();
	GetDlgItem(IDC_EDIT_2)->SetWindowText(m_strEfaRcpPath);
	GetDlgItem(IDC_EDIT_1)->SetWindowText(GetRecipeID());

	// EFA 리스트는 2행(T0/T1) 고정 - ITEM 추가/삭제 버튼은 숨김 유지 (rc: WS_VISIBLE 아님)
	GetDlgItem(IDC_BUTTON_4)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_5)->ShowWindow(SW_HIDE);

	SetTimer(EFA_TIMER_HIGHLIGHT, 500, NULL);	// 진행 하이라이트 폴링

	return TRUE;
}

void CDlgSubManual_AgingNew::UpdateStatus(BOOL bEnable)
{
	for (int i = IDC_BUTTON_2; i < IDC_BUTTON_8 + 1; i++) {
		if (GetDlgItem(i) != NULL) GetDlgItem(i)->EnableWindow(bEnable);
	}
}

void CDlgSubManual_AgingNew::OnTimer(UINT_PTR nIDEvent)
{
	// EFA pass 진행 하이라이트. 엔진이 m_nAgingListRow(1부터, pass = T0/T1 행)와
	// m_nAgingListCol(1=Aging 2=PTN 3=SEC 4=WAD 5=VISION 6=ANGLE)을 쓰고, 대기 중이면 0/0이다.
	if (nIDEvent == EFA_TIMER_HIGHLIGHT)
	{
		// LYS 2026-10-12 Hybrid Phase 6 (D3c-14) hybrid EFA run: segment / EFA pass in the title (ints only, any start path: CYCLE / CIM / START).
		//   1-segment recipe, idle, or a hybrid run that is not EFA (AgingTest / plain CYCLE): the rc title as before (D6-3 (a))
		{
			CString strTitle = m_strTitleBase;
			CRecipeEditMgr* pRcpT = (m_pAuroraMgr != NULL) ? m_pAuroraMgr->GetRecipeEditMgrPtr() : NULL;
			int nPassT = (pRcpT != NULL) ? pRcpT->GetUiPass() : -1;	// pass first (SetUiProgress writes it last)
			int nCntT = (pRcpT != NULL) ? pRcpT->GetUiPassCount() : 0;
			if (pRcpT != NULL && pRcpT->GetSegCountCache() > 1 && nPassT >= 0 && nCntT > 0)
			{
				CString strPart;
				int nSegT = pRcpT->GetUiSeg();
				strTitle += _T(" - 측정 중");
				if (nSegT >= 0)
				{
					strPart.Format(_T(": 세그먼트 %d/%d%s"), nSegT + 1, pRcpT->GetSegCountCache(), (pRcpT->GetUiSegReady() == 0) ? _T(" 전환 중") : _T(""));
					strTitle += strPart;
				}
				strPart.Format(_T(", EFA T%d (%d/%d)"), nPassT, nPassT + 1, nCntT);
				strTitle += strPart;
			}
			if (strTitle != m_strTitleShown && m_LabelInfo.GetSafeHwnd() != NULL)
			{
				m_strTitleShown = strTitle;
				m_LabelInfo.SetText(strTitle);
			}
		}

		// LYS 2026-09-30 Hybrid 3c A13 recipe changed outside this dialog to a 1-segment recipe -> hide the selector (the stamp is cleared,
		//   SAVE refused until LOAD / Recipe Open); hybrid run busy -> selector disabled. Engine / cycle flags and the segment count only,
		//   never the segment contents. 1-segment recipes: the combo is hidden, so nothing here runs.
		// LYS 2026-09-30 Hybrid 3c A13 (S2 fix) run flags are tested first: every cycle's recipe step reloads the recipe (OpenRecipe on the
		//   measure thread) and GetSegmentLoopCount() reads 1 while only segment 0 is parsed -> never hide during a run. The hide also
		//   needs 2 idle ticks in a row (a CIM PPID OpenRecipe can reload while idle) and is done here without UpdateSegmentCombo, which
		//   would re-read the count and could fill the labels from a recipe that is being reloaded.
		if (m_cmbSeg.GetSafeHwnd() != NULL && (m_cmbSeg.GetStyle() & WS_VISIBLE))
		{
			if (IsHybridRunBusy())
			{
				m_nSegLostTick = 0;
				if (m_cmbSeg.IsWindowEnabled()) m_cmbSeg.EnableWindow(FALSE);
			}
			else if (IsSegmented() == FALSE)
			{
				if (++m_nSegLostTick >= 2)
				{
					m_nSegLostTick = 0;
					m_nViewSeg = 0;
					m_strViewRcp.Empty();	// review F6: the grid no longer matches -> SAVE / segment switch refused as stale until LOAD
					m_cmbSeg.ResetContent();
					m_cmbSeg.EnableWindow(TRUE);
					m_cmbSeg.ShowWindow(SW_HIDE);
					if (GetDlgItem(IDC_STATIC_EFA_SEG) != NULL) GetDlgItem(IDC_STATIC_EFA_SEG)->ShowWindow(SW_HIDE);
					CLogWrapper::GetInstance()->KMLOG(OPER, _T("[HYBRID] EFA view: the recipe is no longer hybrid - segment selector hidden, LOAD / Recipe Open before saving"));
				}
			}
			else
			{
				m_nSegLostTick = 0;
				if (!m_cmbSeg.IsWindowEnabled()) m_cmbSeg.EnableWindow(TRUE);
			}
		}

		do
		{
			if (m_spreadAgingList == NULL || m_spreadAgingList->GetSafeHwnd() == NULL) break;
			if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL) break;
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() == NULL) break;

			int nRow = 0, nCol = 0;
			CRecipeEditMgr* pRcpHl = m_pAuroraMgr->GetRecipeEditMgrPtr();	// LYS 2026-10-12 Hybrid Phase 6 (D6-4 (b)) a hybrid EFA glass started by CYCLE / CIM recipe flag too
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_NewAgingMode()
				|| (pRcpHl != NULL && pRcpHl->GetUiPassCount() > 0 && pRcpHl->GetSegCountCache() > 1))
			{
				nRow = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow;
				nCol = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol;
			}

			if (nRow == m_nHlRow && nCol == m_nHlCol) break;	// 변화 없음

			if (m_nHlRow > 0 && m_nHlCol > 0 &&
				m_nHlRow <= m_spreadAgingList->GetMaxRows() && m_nHlCol <= m_spreadAgingList->GetMaxCols())
			{
				m_spreadAgingList->SetColor(m_nHlCol, m_nHlRow, RGB_WHITE, RGB_BLACK);	// FarPoint 좌표는 (Col, Row)
			}

			if (nRow > 0 && nCol > 0 &&
				nRow <= m_spreadAgingList->GetMaxRows() && nCol <= m_spreadAgingList->GetMaxCols())
			{
				m_spreadAgingList->SetColor(nCol, nRow, RGB_YELLOW, RGB_BLACK);
			}

			m_nHlRow = nRow;
			m_nHlCol = nCol;
		} while (false);
	}

	CDialogEx::OnTimer(nIDEvent);
}

//=========================================================================
// 버튼
//=========================================================================
void CDlgSubManual_AgingNew::OnStop()
{
	// OCP m_pJobProc->ActionStop()에 해당하는 HMI 처리
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_AgingNew::OnStop"));

	CSystemParamShare::GetInstance()->SetCycleState(false);

	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetShareMemoryMgrPtr() != NULL)
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_NewAgingMode(false);

	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false)
	{
		CSequenceManager::GetInstance()->EngineStop();
	}

	m_chkStop.ShowWindow(FALSE);
}

void CDlgSubManual_AgingNew::OnRcpOpen()
{
	// .rcp 선택 -> RecipeChange (OCP LoadPPID2) -> 셀 맵 재구성 + 새 레시피의 EFA 정보 로드
	if (!IsOfflineStatus()) return;
	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return;

	CString filename = _T(""), strRcpFile;
	CString filter = _T("Recipe Files (*.rcp)|*.rcp|");

	CFileDialog dlg(TRUE, _T("rcp"), filename, OFN_FILEMUSTEXIST, filter);

	if (dlg.DoModal() == IDOK)
	{
		filename = dlg.GetFileName();

		if (filename.IsEmpty()) MessageBox(_T("File is not an Recipe file or is locked and cannot be imported."));
		else
		{
			int j = filename.Find(_T(".rcp"));

			if (j > 0)	strRcpFile = filename.Left(j);		// ".rcp" 제거
			else		strRcpFile = filename;

			if (m_pAuroraMgr->GetRecipeEditMgrPtr()->RecipeChange(strRcpFile) == VTM_RV_SUCCESSED)
			{
				MessageBox(_T("Recipe file be imported complete."));

				GetDlgItem(IDC_EDIT_1)->SetWindowText(GetRecipeID());

				CString strEfaRcp = GetEfaRcpFile();
				if (strEfaRcp.IsEmpty() || ::GetFileAttributes(strEfaRcp) == INVALID_FILE_ATTRIBUTES)
				{
					CString strMsg;
					strMsg.Format(_T("No EFA recipe in %s.\nSave the EFA list first and reopen."), (LPCTSTR)m_strEfaRcpPath);
					AfxMessageBox(strMsg, MB_ICONWARNING);
				}

				LoadAgingInfo();	// 내부에서 UpdateAgingCell + MeasCellLoad까지 실행
				return;
			}
			else
			{
				AfxMessageBox(_T("Load Recipe Error"));
			}
		}
	}
}

void CDlgSubManual_AgingNew::OnItemClearAll()
{
	// OCP_EFA: "리스트 전체 초기화" -> 고정 2행 재구성
	if (AfxMessageBox(_T("Reset the EFA list to 2 empty items (T0 / T1).\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	AgingItemClearAll(TRUE);
}

void CDlgSubManual_AgingNew::OnItemAdd()
{
	return;	// OCP_EFA: 리스트 2행 고정 (버튼 숨김)
}

void CDlgSubManual_AgingNew::OnItemDel()
{
	return;	// OCP_EFA: 리스트 2행 고정 (버튼 숨김)
}

void CDlgSubManual_AgingNew::OnItemSave()
{
	CString strCurrentRcp = GetRecipeID();
	GetDlgItem(IDC_EDIT_1)->SetWindowText(strCurrentRcp);
	if (strCurrentRcp == _T("") || strCurrentRcp == _T("UNKNOWN")) {
		AfxMessageBox(_T("No base recipe.\nOpen a recipe first."), MB_ICONWARNING);
		return;
	}

	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("Save the current EFA list.\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	SaveAgingInfo();
}

void CDlgSubManual_AgingNew::OnLoadAgingInfo()
{
	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("Reload the saved EFA list.\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	LoadAgingInfo();
}

// LYS 2026-10-13 Hybrid Phase 7 D7-15 (b) START on a hybrid runtime recipe (segment-count cache > 1, fix stage note below) also needs SPS_IDLE:
//   a manual worker / recipe command (SPS_CYCLETEST) must not overlap an engine start (the engine-start backstop relies on it).
//   Called when the status is not SPS_PROCESS, and (fix stage, impl review 7) again right before the resume prompt. 1-segment recipe:
//   always false, no log (the SPS_PROCESS test alone, unchanged).
//   true = refuse (WARN logged, the caller shows the existing message).
static bool IsHybridStartNotIdle(CAuroraMgr* pMgr)
{
	int nSps = (int)pMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus();
	if (nSps == SPS_IDLE)
		return false;
	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 1/2): SPS is not idle here (a run, a manual worker or a recipe command), so the hybrid test reads
	//   the segment-count cache int only - no DLL segment vector read while busy (HARD RULE 3). 1-segment recipe (cache 1): false, no log
	CRecipeEditMgr* pRcp = pMgr->GetRecipeEditMgrPtr();
	if (pRcp == NULL || pRcp->GetSegCountCache() <= 1)
		return false;	// 1-segment recipe
	CString strLog;
	strLog.Format(_T("[HYBRID] EFA START refused - process status %d is not idle (manual worker / recipe command running)"), nSps);
	CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
	return true;
}

void CDlgSubManual_AgingNew::OnAgingProcStrart()
{
	// HMI 사이클 검사 경로로 EFA 사이클 테스트를 시작한다
	// (OCP_EFA: m_bNewAgingMode + nJobCount + JOB_Proc::DoStartCycleTest -> DoWorkStartMeasICP_EFA).
	// 엔진 쪽 2-pass 루프는 CDoWorkGlassMeasure::ThreadRun에 구현돼 있다(Phase 2 완료).
	if (!IsOfflineStatus()) return;

	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL ||
		m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return;

	// LYS 2026-10-13 Hybrid Phase 7 D7-15 (b): hybrid runtime recipe -> START also needs SPS_IDLE (IsHybridStartNotIdle); 1-segment recipe: SPS_PROCESS only, unchanged
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_PROCESS || IsHybridStartNotIdle(m_pAuroraMgr))
	{
		AfxMessageBox(_T("Process is running.\nTry again after it completes."), MB_ICONWARNING);
		return;
	}

	if (AfxMessageBox(_T("Start EFA Proc (T0 -> aging -> T1).\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strRcp = GetRecipeID();
	if (strRcp.IsEmpty() || strRcp == _T("UNKNOWN"))
	{
		AfxMessageBox(_T("No recipe information.\nOpen a recipe first."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-09-30 Hybrid 3c A13 (review F3) the cycle measures CSystemParamShare's recipe (the recipe step calls OpenRecipe with it),
	//   not the one this dialog opened with RecipeChange. START is refused unless both are the same recipe, otherwise the
	//   EFA flag / file / mask / segment checks below would describe another recipe.
	// LYS 2026-09-30 user request: 1-segment recipes too (was hybrid only). Run recipe "UNKNOWN" (nothing opened for a run yet) is refused.
	if (strRcp.CompareNoCase(CSystemParamShare::GetInstance()->GetRecipeID()) != 0)
	{
		CString strRunRcp = CSystemParamShare::GetInstance()->GetRecipeID();
		CString strRefuse;
		strRefuse.Format(_T("EFA START refused - dialog recipe %s != run recipe %s (the cycle measures the run recipe)"), (LPCTSTR)strRcp, (LPCTSTR)strRunRcp);
		CLogWrapper::GetInstance()->KMLOG(OPER, strRefuse);
		CString strMsg;
		strMsg.Format(_T("이 화면의 레시피(%s)와 운전 레시피(%s)가 다릅니다.\nSTART는 운전 레시피를 측정하므로 이 화면의 셀 맵과 사전 검사가 맞지 않습니다.\n운전 레시피를 이 레시피로 연 뒤(운전 화면 CYCLE 또는 수동 측정 화면의 레시피 열기) 다시 시도하십시오."), (LPCTSTR)strRcp, (LPCTSTR)strRunRcp);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return;
	}

	//=====================================================================
	// 사전 검사(AgingTest START와 동일): OCP 모드, 레시피 EFA 플래그, 항목이 있는 EFA rcp 파일.
	//=====================================================================
	if (m_pAuroraMgr->GetSystemConfigMgrPtr() == NULL ||
		m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP() == false)
	{
		AfxMessageBox(_T("EFA cycle runs in OCP mode only.\nCheck SystemConfig.ini [EQUIP_TYPE] MACHINE_TYPE."), MB_ICONWARNING);
		return;
	}

	S_RecipeInfo* pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo == NULL || pRecipeInfo->m_sInspectItem.m_bUseEFA == FALSE)
	{
		AfxMessageBox(_T("EFA is not enabled in the current recipe.\nTurn it on in RECIPE - Inspection Item, then retry."), MB_ICONWARNING);
		return;
	}

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty() || ::GetFileAttributes(strEfaRcp) == INVALID_FILE_ATTRIBUTES)
	{
		CString strChkMsg;
		strChkMsg.Format(_T("EFA recipe not found.\n%s\n\nSave the list first."), (LPCTSTR)strEfaRcp);
		AfxMessageBox(strChkMsg, MB_ICONWARNING);
		return;
	}

	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, sizeof(cValue));
	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strEfaRcp);
	int nFileItem = _ttoi(cValue);
	if (nFileItem < 1)
	{
		AfxMessageBox(_T("The saved EFA list is empty.\nSave at least the T0/T1 items."), MB_ICONWARNING);
		return;
	}
	if (nFileItem != EFA_FIXED_ITEM_COUNT)
	{
		CString strChkMsg;
		strChkMsg.Format(_T("EFA list normally has %d items (T0 / T1) but the file has %d.\nContinue anyway?"), EFA_FIXED_ITEM_COUNT, nFileItem);
		if (AfxMessageBox(strChkMsg, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	}

	// LYS 2026-09-30 Hybrid 3c A13 (D3c-6) hybrid: report each segment's EFA mask section (the engine reads the file, not this grid).
	//   A segment without its section measures every cell of the item's kinds (D4) -> Yes/No. IsOfflineStatus passed above.
	if (IsSegmented())
	{
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		int nSegCnt = (pSegFunc != NULL) ? pSegFunc->GetSegmentCount() : 1;
		CAgingCycleInfo efaInfo;
		efaInfo.Load(strRcp, true, nSegCnt);
		CString strSegLog = _T("[HYBRID] EFA START masks:"), strOne, strNoMask;
		for (int k = 0; k < nSegCnt && pSegFunc != NULL; k++)
		{
			int nRows = efaInfo.GetStickCount(k);
			strOne.Format(_T(" seg%d [%s] rows %d/%d"), k + 1, CAgingCycleInfo::GetMaskSection(k).GetString(), nRows, pSegFunc->GetSegmentStickCount(k));
			strSegLog += strOne;
			if (nRows == 0) { strOne.Format(_T("%s%d"), strNoMask.IsEmpty() ? _T("") : _T(", "), k + 1); strNoMask += strOne; }
		}
		CLogWrapper::GetInstance()->KMLOG(OPER, strSegLog);
		if (strNoMask.IsEmpty() == FALSE)
		{
			CString strAskSeg;
			strAskSeg.Format(_T("세그먼트 %s에는 EFA 셀 맵이 없어 모든 셀을 측정합니다.\n계속 하시겠습니까?"), (LPCTSTR)strNoMask);
			if (AfxMessageBox(strAskSeg, MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				CLogWrapper::GetInstance()->KMLOG(OPER, _T("[HYBRID] EFA START cancelled - segment(s) ") + strNoMask + _T(" without a mask section"));
				return;
			}
		}
	}

	int nMaxCount = m_spreadAgingList->GetMaxRows();	// HMI 그리드: MaxRows = 항목 수
	if (nMaxCount < 1)
	{
		AfxMessageBox(_T("No list information.\nLoad or save the list first."), MB_ICONWARNING);
		return;
	}

	// 콤보에서 Cycle 횟수를 읽는다
	CString strTemp;
	int nCycleCount = 1;
	m_cmbCycle.GetWindowText(strTemp);
	nCycleCount = _ttoi(strTemp);
	if (nCycleCount < 1) nCycleCount = 1;

	// 그리드 -> m_vecAginList (1..6열 = Aging/PTN/Sec/WAD/VISION/ANGLE)
	m_vecAginList.clear();
	S_AgingTest sAgingList;
	for (int i = 0; i < nMaxCount; i++)
	{
		int nRow = i + 1;
		sAgingList.m_bUseAging    = m_spreadAgingList->GetDataInt(1, nRow);
		sAgingList.m_nPtnRGB      = m_spreadAgingList->GetDataInt(2, nRow);
		sAgingList.m_nPtnSec      = m_spreadAgingList->GetDataInt(3, nRow);
		sAgingList.m_bUseProperty = m_spreadAgingList->GetDataInt(4, nRow);
		sAgingList.m_bUseQuality  = m_spreadAgingList->GetDataInt(5, nRow);
		sAgingList.m_bUseAngle    = m_spreadAgingList->GetDataInt(6, nRow);
		m_vecAginList.push_back(sAgingList);
	}

	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 7): D7-15 (b) again after the confirm dialog(s) above (modal - the status may have changed),
	//   before the resume prompt and the shared state below (nothing is changed yet). Hybrid runtime recipe only; 1-segment recipe: false, no log
	if (IsHybridStartNotIdle(m_pAuroraMgr))
	{
		AfxMessageBox(_T("Process is running.\nTry again after it completes."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-10-08 Hybrid Phase 5 (D5-11 a) journal bookmark -> the same resume prompt as CYCLE (an EFA glass resumes here);
	//   otherwise only the stale resume flag is cleared (D5-10 a). CANCEL = nothing changed.
	if (ResumeDialogEntry(m_pAuroraMgr, RESUME_ENTRY_AGINGNEW) == RESUME_PROMPT_CANCEL)
		return;

	// 공유 상태: EFA와 AgingTest 모드는 동시에 켜지지 않는다 (OCP도 시작할 때마다 둘 다 리셋)
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_AutoAgingMode(false);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_NewAgingMode(true);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nJobCount = nCycleCount;
	}

	// HMI 사이클 검사 시작 (DlgSubOperate 사이클 버튼 / AgingTest START와 같은 경로)
	CSystemParamShare::GetInstance()->SetCycleCount(nCycleCount);
	CSystemParamShare::GetInstance()->SetCycleState(true);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);

	m_chkStop.ShowWindow(TRUE);

	CString strLog;
	strLog.Format(_T("CDlgSubManual_AgingNew::OnAgingProcStrart - EFA cycle test start (rcp=%s items=%d cycle=%d)"), (LPCTSTR)strRcp, nFileItem, nCycleCount);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	CSequenceManager::GetInstance()->EngineStart();
}

LRESULT CDlgSubManual_AgingNew::OnSpreadDBClick(WPARAM wParam, LPARAM lParam)
{
	// CMySpreadDll이 전달 (wParam = 컴포넌트 ID, lParam = LPSS_CELLCOORD)
	if ((UINT)wParam != IDC_FPSPREAD_AGING_CELL_STATUS) return 0;

	LPSS_CELLCOORD pCoord = (LPSS_CELLCOORD)lParam;
	if (pCoord == NULL) return 0;

	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return 0;

	DblClickFpspreadCellStatus((long)pCoord->Col, (long)pCoord->Row);
	return 0;
}

//=========================================================================
// 리스트 그리드
//=========================================================================
void CDlgSubManual_AgingNew::UpdateListNum()
{
	// 행 번호를 헤더 0열에 쓴다. 1/2행은 OCP_EFA 의미에 맞춰 T0/T1로 표시한다.
	CString str;
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	for (int i = 1; i <= nMaxRow; i++) {
		if (i <= EFA_FIXED_ITEM_COUNT) str.Format(_T("T%d"), i - 1);
		else str.Format(_T("%d"), i);
		m_spreadAgingList->SetDataString(0, i, str);
	}
}

BOOL CDlgSubManual_AgingNew::IsOfflineStatus()
{
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetShareMemoryMgrPtr() != NULL) {
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE) {
			AfxMessageBox(_T("Not in Offline mode.\nSwitch to Offline mode and retry."), MB_ICONWARNING);
			return FALSE;
		}
	}

	// LYS 2026-09-29 Hybrid 3b A13 no recipe open / save / reload / START while a hybrid recipe is being measured (1-segment recipes: unchanged)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1
		&& (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false || CSystemParamShare::GetInstance()->IsCycleState()))
	{
		AfxMessageBox(_T("하이브리드 레시피 측정 중에는 이 작업을 할 수 없습니다.\n측정이 끝난 뒤 다시 시도하십시오."), MB_ICONWARNING);
		return FALSE;
	}

	// LYS 2026-10-12 1-segment recipes too (user request): a recipe open / save / reload / START mid-run changes the running glass
	//   (e.g. Sampling) and resets the EFA / cell ID / result latches and the cell status - blocked while any glass is being measured
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false || CSystemParamShare::GetInstance()->IsCycleState())
	{
		AfxMessageBox(_T("측정 중에는 이 작업을 할 수 없습니다.\n측정이 끝난 뒤 다시 시도하십시오."), MB_ICONWARNING);
		return FALSE;
	}

	return TRUE;
}

BOOL CDlgSubManual_AgingNew::LoadAgingInfo()
{
	// {EFA_RCP}\{RecipeID}.rcp -> 리스트 그리드 + m_vecAginList, 이어서 셀 맵.
	// HMI 그리드 좌표: 헤더 = SS_HEADER 행, 데이터 행 1..MaxRows, 열 1..6
	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return FALSE;
	UpdateSegmentCombo();	// LYS 2026-09-30 Hybrid 3c A13 reset/clamp the viewed segment before the grid is rebuilt (compares with the previous m_strViewRcp; 1-segment: stays hidden)

	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, sizeof(cValue));

	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strEfaRcp);
	int nMaxCount = _ttoi(cValue);

	AgingItemClearAll();
	if (nMaxCount <= 0) {
		// LYS 2026-09-30 no EFA rcp yet: still build this recipe's cell map. Before, SAVE wrote the dialog's
		//   default 10 x 5 one-sheet grid (the AgingTest dialog did exactly that on 2026-09-30 10:29)
		m_strViewRcp = GetRecipeID();
		UpdateAgingCell();
		MeasCellLoad();
		return FALSE;
	}

	m_vecAginList.clear();
	S_AgingTest sAgingList;

	CString strItem;
	for (int iItem = 0; iItem < nMaxCount; iItem++) {
		strItem.Format(_T("ITEM_%d"), iItem + 1);
		AgingItemAdd();

		int nRow = iItem + 1;

		GetPrivateProfileString(strItem, _T("USE_AGING"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(1, nRow, cValue);
		sAgingList.m_bUseAging = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(2, nRow, cValue);
		sAgingList.m_nPtnRGB = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_SEC"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(3, nRow, cValue);
		sAgingList.m_nPtnSec = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_PROPERTY"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(4, nRow, cValue);
		sAgingList.m_bUseProperty = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_QUALITY"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(5, nRow, cValue);
		sAgingList.m_bUseQuality = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_ANGLE"), _T("0"), cValue, MAX_PATH, strEfaRcp);
		m_spreadAgingList->SetData(6, nRow, cValue);
		sAgingList.m_bUseAngle = _ttoi(cValue);

		m_vecAginList.push_back(sAgingList);
	}

	m_strViewRcp = GetRecipeID();	// LYS 2026-09-30 the grid below is built for this recipe
	UpdateAgingCell();	// 3시트 맵을 먼저 만들고
	MeasCellLoad();		// 그다음 [MASECELL]로 채운다

	return TRUE;
}

void CDlgSubManual_AgingNew::SaveAgingInfo()
{
	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return;
	if (IsMaskViewStale()) return;	// LYS 2026-09-30 grid not built for this recipe (default 10 x 5 before LOAD / Recipe Open) - checked before any write

	dfcCreateFolder(m_strEfaRcpPath);

	CString strValue, strItem;
	int nMaxCount = m_spreadAgingList->GetMaxRows();
	if (nMaxCount < 1) {
		AfxMessageBox(_T("Add EFA items before saving."));
		return;
	}

	if (ConfirmMaskOverwrite() == FALSE) return;	// LYS 2026-09-30

	strValue.Format(_T("%d"), nMaxCount);
	WritePrivateProfileString(_T("COUNT"), _T("MaxCount"), strValue, strEfaRcp);

	for (int i = 0; i < nMaxCount; i++) {
		strItem.Format(_T("ITEM_%d"), i + 1);

		for (int j = 1; j < MAX_EFA_ITEM; j++) {
			strValue = m_spreadAgingList->GetDataString(j, i + 1);

			if (j == 1)			WritePrivateProfileString(strItem, _T("USE_AGING"),    strValue, strEfaRcp);
			else if (j == 2)	WritePrivateProfileString(strItem, _T("PTN_RGB"),      strValue, strEfaRcp);
			else if (j == 3)	WritePrivateProfileString(strItem, _T("PTN_SEC"),      strValue, strEfaRcp);
			else if (j == 4)	WritePrivateProfileString(strItem, _T("USE_PROPERTY"), strValue, strEfaRcp);
			else if (j == 5)	WritePrivateProfileString(strItem, _T("USE_QUALITY"),  strValue, strEfaRcp);
			else if (j == 6)	WritePrivateProfileString(strItem, _T("USE_ANGLE"),    strValue, strEfaRcp);
		}
	}

	MeasCellSave();
}

void CDlgSubManual_AgingNew::AgingItemClearAll(BOOL bFixed2)
{
	m_spreadAgingList->InitMySpread_AGINGTEST_LIST();

	if (bFixed2 == FALSE) return;

	// OCP_EFA AgingItemClearAll(TRUE): "EFA 리스트는 2행 고정"
	for (int i = 0; i < EFA_FIXED_ITEM_COUNT; i++) AgingItemAdd();
}

void CDlgSubManual_AgingNew::AgingItemAdd()
{
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	if (nMaxRow < 0) return;

	CString sComboList;
	SS_CELLTYPE cellType;

	m_spreadAgingList->SetMaxRows(nMaxRow + 1);

	for (int i = 1; i < MAX_EFA_ITEM; i++) {
		if (i == 1 || i == 4 || i == 5 || i == 6) {	// Aging / WAD / VISION / ANGLE 체크박스
			m_spreadAgingList->SetTypeCheckBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}

		if (i == 2) {	// 패턴 0..25 (OCP_EFA 20260403: 10 -> 25)
			sComboList.Empty();
			for (int p = 0; p <= 25; p++) sComboList += dfcStr(_T("%d\t"), p);
			m_spreadAgingList->SetTypeComboBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, sComboList);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}

		if (i == 3) {	// 유지 시간(초)
			m_spreadAgingList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}
	}

	UpdateListNum();
}

//=========================================================================
// 셀 사용 맵 (3시트 WAD/VISION/ANGLE, [MASECELL] STICK_n = 3비트 마스크 CSV) - AgingTest와 동일
//=========================================================================
void CDlgSubManual_AgingNew::UpdateAgingCell()
{
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	int nStick = 0, nCell = 0;
	S_RecipeData* pRD = NULL;
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
	{
		// LYS 2026-09-30 Hybrid 3c A13 hybrid: dimensions of the viewed segment (no SetActiveSegment - the measure thread owns it).
		//   Callers (LoadAgingInfo, OnSelchangeSeg) passed IsOfflineStatus. 1-segment: GetRecipeData() as before.
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		S_RecipeMgr* pSegMgr = (IsSegmented() && pSegFunc != NULL) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL;
		pRD = (pSegMgr != NULL) ? &pSegMgr->m_sData : m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	}

	if (pRD != NULL && pRD->m_vecStick.size() > 0)
	{
		nStick = (int)pRD->m_vecStick.size();
		nCell = (int)pRD->m_vecStick[0].m_vecCell.size();
	}

	static const TCHAR* szSheet[3] = { _T("WAD"), _T("VISION"), _T("ANGLE") };

	m_spreadAgingMeas->SetSheetCount(3);

	SS_CELLTYPE ctCheck;
	m_spreadAgingMeas->SetTypeCheckBox(&ctCheck, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	CString str;
	for (int iSheet = 0; iSheet < 3; iSheet++)
	{
		m_spreadAgingMeas->SetSheet(iSheet + 1);
		m_spreadAgingMeas->SetSheetName(iSheet + 1, szSheet[iSheet]);

		m_spreadAgingMeas->SetMaxCols(nCell);
		m_spreadAgingMeas->SetMaxRows(nStick);

		if (nStick <= 0 || nCell <= 0) continue;

		m_spreadAgingMeas->SetColWidth(SS_ALLCOLS, 4);
		m_spreadAgingMeas->SetRowHeight(SS_ALLROWS, 13);
		m_spreadAgingMeas->SetColWidth(SS_HEADER, 6);

		for (int nRow = 1; nRow <= nStick; nRow++)
		{
			str.Format(_T("S%d"), nRow);
			m_spreadAgingMeas->SetData(SS_HEADER, nRow, str);

			for (int nCol = 1; nCol <= nCell; nCol++)
			{
				m_spreadAgingMeas->SetCellType(nCol, nRow, &ctCheck);
				m_spreadAgingMeas->SetData(nCol, nRow, _T("0"));
			}
		}
	}

	m_spreadAgingMeas->SetSheet(1);
}

int CDlgSubManual_AgingNew::GetMeasUse(int iMeas, int nMeasUse)
{
	return ((nMeasUse >> iMeas) & 0x01) ? 1 : 0;
}

BOOL CDlgSubManual_AgingNew::MeasCellSave()	// LYS 2026-09-30 Hybrid 3c A13 BOOL: FALSE = not written (stale / overwrite declined)
{
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return TRUE;

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return TRUE;
	// LYS 2026-09-30 Hybrid 3c A13 section of the viewed segment: segment 0 = [MASECELL] (unchanged), k>=1 = [MASECELL_SEG{k+1}]
	CString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);
	// hybrid: the segment-switch save comes here directly -> own checks (after SaveAgingInfo both pass silently: same view, count cleared)
	if (IsSegmented() && (IsMaskViewStale() || ConfirmMaskOverwrite() == FALSE)) return FALSE;

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return TRUE; }

	std::bitset<3> bit;
	CString strCellData;
	for (int i = 0; i < nStick; i++) {
		strCellData.Empty();

		for (int j = 0; j < nCell; j++) {
			bit.reset();
			for (int k = 0; k < 3; k++) {	// WAD, VISION, ANGLE
				m_spreadAgingMeas->SetSheet(k + 1);
				bit[k] = (m_spreadAgingMeas->GetDataInt(j + 1, i + 1) != 0);
			}
			strCellData += dfcStr(_T("%d,"), (int)bit.to_ulong());
		}

		WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), strCellData, strEfaRcp);
	}

	// LYS 2026-09-30 Hybrid 3c A13 hybrid only: delete this section's rows beyond the segment's stick count
	//   (the loader stops at the first missing STICK_n; key by key - deleting the section would move it to the file end)
	if (IsSegmented())
	{
		TCHAR cOld[16];
		for (int i = nStick; i < DEFINE_AGING_CYCLE_STICK_MAX; i++)
		{
			ZeroMemory(cOld, sizeof(cOld));
			GetPrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), _T(""), cOld, _countof(cOld), strEfaRcp);
			if (cOld[0] == 0) break;
			WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), NULL, strEfaRcp);
		}
		CString strLog;
		strLog.Format(_T("[HYBRID] EFA rcp %s: [%s] saved %d stick(s) x %d cell(s)"), (LPCTSTR)GetRecipeID(), (LPCTSTR)strSec, nStick, nCell);
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
		m_nMaskMismatch = 0;
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
	return TRUE;
}

void CDlgSubManual_AgingNew::MeasCellLoad()
{
	// [MASECELL] -> 그리드. UpdateAgingCell이 시트를 만든 뒤에 호출할 것.
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return;
	m_nMaskMismatch = 0;	// LYS 2026-09-30
	CString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);	// LYS 2026-09-30 Hybrid 3c A13 segment 0 = MASECELL (unchanged)

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return; }

	TCHAR cCellData[EFA_CELLMASK_BUF];
	CArray<int, int> nCellChkArray;

	// LYS 2026-09-30 Hybrid 3c A13 (D3c-13) segment k>=1 without its section: the EFA engine measures every cell (D4),
	//   so show every cell checked - otherwise SAVE would turn "measure all" into "measure none". Segment 0: unchanged.
	if (IsSegmented() && m_nViewSeg >= 1)
	{
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(strSec, _T("STICK_1"), _T(""), cCellData, EFA_CELLMASK_BUF, strEfaRcp);
		if (cCellData[0] == 0)
		{
			for (int k = 0; k < 3; k++)
			{
				m_spreadAgingMeas->SetSheet(k + 1);
				for (int i = 0; i < nStick; i++)
					for (int j = 0; j < nCell; j++)
						m_spreadAgingMeas->SetData(j + 1, i + 1, _T("1"));
			}
			m_spreadAgingMeas->SetSheet(nKeepSheet);
			CString strNo;
			strNo.Format(_T("[HYBRID] EFA rcp [%s] not found - shown all checked (the engine measures every cell)"), (LPCTSTR)strSec);
			CLogWrapper::GetInstance()->KMLOG(OPER, strNo);
			return;
		}
	}

	for (int i = 0; i < nStick; i++) {
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), _T(""), cCellData, EFA_CELLMASK_BUF, strEfaRcp);	// LYS 2026-09-30 Hybrid 3c A13

		int pos = 0;
		nCellChkArray.RemoveAll();
		CString strCellData = cCellData;
		CString resToken = strCellData.Tokenize(_T(","), pos);
		while (resToken != _T("")) {
			nCellChkArray.Add(_ttoi(resToken.Trim()));
			resToken = strCellData.Tokenize(_T(","), pos);
		}

		if (nCellChkArray.GetSize() != nCell && nCellChkArray.GetSize() > 0) m_nMaskMismatch++;	// LYS 2026-09-30 present but wrong cell count
		if (nCellChkArray.GetSize() != nCell) continue;	// 행 없음/개수 불일치 -> 기본값 유지

		for (int j = 0; j < nCell; j++) {
			int nMask = nCellChkArray.GetAt(j);
			for (int k = 0; k < 3; k++) {	// WAD, VISION, ANGLE
				m_spreadAgingMeas->SetSheet(k + 1);
				m_spreadAgingMeas->SetData(j + 1, i + 1, ((nMask >> k) & 0x01) ? _T("1") : _T("0"));
			}
		}
	}

	if (m_nMaskMismatch > 0)	// LYS 2026-09-30 same warning as the AgingTest dialog
	{
		CString strLog;
		strLog.Format(_T("%sEFA rcp [%s]: %d row(s) with cell count != %d shown unchecked (%s)"), IsSegmented() ? _T("[HYBRID] ") : _T(""), (LPCTSTR)strSec, m_nMaskMismatch, nCell, (LPCTSTR)strEfaRcp);	// LYS 2026-09-30 Hybrid 3c A13 1-segment: same text
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);

		CString strWarn;
		if (IsSegmented())	// LYS 2026-09-30 Hybrid 3c A13
			strWarn.Format(_T("세그먼트 %d 셀 맵에 셀 수가 %d개가 아닌 행이 %d개 있습니다.\n해당 행은 미체크로 표시되며, 저장하면 화면 값으로 덮어씁니다."), m_nViewSeg + 1, nCell, m_nMaskMismatch);
		else
			strWarn.Format(_T("셀 맵에 셀 수가 %d개가 아닌 행이 %d개 있습니다.\n해당 행은 미체크로 표시되며, 저장하면 화면 값으로 덮어씁니다."), nCell, m_nMaskMismatch);
		AfxMessageBox(strWarn, MB_ICONWARNING);
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
}

// LYS 2026-09-30 the cell grid is not the current recipe's: never built (default 10 x 5 before LOAD / Recipe Open),
//   the recipe changed outside this dialog, or the size differs -> popup + TRUE. Same rule as the AgingTest dialog.
BOOL CDlgSubManual_AgingNew::IsMaskViewStale()
{
	int nStick = 0, nCell = 0;
	S_RecipeData* pRD = (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL) ? m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData() : NULL;
	if (IsSegmented())	// LYS 2026-09-30 Hybrid 3c A13 compare with the viewed segment (no SetActiveSegment); callers passed IsOfflineStatus
	{
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		S_RecipeMgr* pSegMgr = (pSegFunc != NULL && m_nViewSeg >= 0 && m_nViewSeg < pSegFunc->GetSegmentCount()) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL;
		pRD = (pSegMgr != NULL) ? &pSegMgr->m_sData : NULL;	// segment gone -> size 0 -> stale
	}
	if (pRD != NULL && pRD->m_vecStick.size() > 0)
	{
		nStick = (int)pRD->m_vecStick.size();
		nCell = (int)pRD->m_vecStick[0].m_vecCell.size();
	}

	BOOL bStale = (GetRecipeID() != m_strViewRcp) ? TRUE : FALSE;
	if (bStale == FALSE && m_spreadAgingMeas != NULL && m_spreadAgingMeas->GetSafeHwnd() != NULL)
	{
		short nKeepSheet = m_spreadAgingMeas->GetSheet();
		m_spreadAgingMeas->SetSheet(1);
		if (m_spreadAgingMeas->GetMaxRows() != nStick || m_spreadAgingMeas->GetMaxCols() != nCell) bStale = TRUE;
		m_spreadAgingMeas->SetSheet(nKeepSheet);
	}
	if (bStale == FALSE) return FALSE;
	AfxMessageBox(_T("셀 맵이 현재 레시피와 맞지 않습니다(불러온 뒤 레시피가 바뀌었거나 아직 불러오지 않음).\n레시피 열기 또는 이전값 불러오기 후 다시 저장하십시오."), MB_ICONWARNING);
	return TRUE;
}

// LYS 2026-09-30 rows whose cell count did not match were shown unchecked; ask once before they are overwritten (TRUE = go on)
BOOL CDlgSubManual_AgingNew::ConfirmMaskOverwrite()
{
	if (m_nMaskMismatch <= 0) return TRUE;
	CString strAsk;
	if (IsSegmented())	// LYS 2026-09-30 Hybrid 3c A13
		strAsk.Format(_T("세그먼트 %d 셀 맵에서 셀 수가 맞지 않던 행 %d개를 지금 화면 값으로 덮어씁니다.\n계속 하시겠습니까?"), m_nViewSeg + 1, m_nMaskMismatch);
	else
		strAsk.Format(_T("셀 맵에서 셀 수가 맞지 않던 행 %d개를 지금 화면 값으로 덮어씁니다.\n계속 하시겠습니까?"), m_nMaskMismatch);
	if (AfxMessageBox(strAsk, MB_YESNO | MB_ICONWARNING) != IDYES) return FALSE;
	m_nMaskMismatch = 0;
	return TRUE;
}

//=========================================================================
// LYS 2026-09-30 Hybrid 3c A13 segment selector for the EFA rcp cell mask (D6, same structure as the AgingTest dialog,
//   details as the WAD Multi dialog (D3c-12): R in the label, disabled while a hybrid run is busy, stale check before switching).
//   Hybrid only: grid = GetSegmentMgr(m_nViewSeg), save/load = CAgingCycleInfo::GetMaskSection(m_nViewSeg).
//   [COUNT]/[ITEM_n] (T0/T1) stay glass-common (A2). SetActiveSegment is never called here.
//=========================================================================
BOOL CDlgSubManual_AgingNew::IsSegmented()
{
	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return FALSE;
	return (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1) ? TRUE : FALSE;
}

BOOL CDlgSubManual_AgingNew::IsHybridRunBusy()
{
	// LYS 2026-09-30 Hybrid 3c A13 (S2 fix) raw run flags, no IsSegmented() precondition (the recipe step's reload makes it FALSE for
	//   a moment). Only called by the timer while the selector is visible, i.e. on a hybrid view.
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false) return TRUE;
	return CSystemParamShare::GetInstance()->IsCycleState() ? TRUE : FALSE;
}

// Callers: LoadAgingInfo and OnSelchangeSeg (after IsOfflineStatus, or once the recipe is no longer hybrid -> nSeg = 1, no segment
//   contents read). The timer hides the selector itself (S2 fix) and never calls this.
void CDlgSubManual_AgingNew::UpdateSegmentCombo()
{
	if (m_cmbSeg.GetSafeHwnd() == NULL) return;

	CString strRcpID = GetRecipeID();
	C_OutRecipeFunc* pFunc = (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL) ? m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr() : NULL;
	int nSeg = (IsSegmented() && pFunc != NULL) ? pFunc->GetSegmentCount() : 1;
	if (nSeg < 1) nSeg = 1;

	BOOL bWasShown = (m_cmbSeg.GetStyle() & WS_VISIBLE) ? TRUE : FALSE;
	BOOL bReset = (strRcpID != m_strViewRcp || m_nViewSeg < 0 || m_nViewSeg >= nSeg) ? TRUE : FALSE;	// LoadAgingInfo stamps m_strViewRcp right after this call
	if (bReset) m_nViewSeg = 0;
	// review F6: the grid on screen no longer matches m_nViewSeg (view reset, or a hybrid view whose recipe became 1-segment)
	//   -> clear the stamp, so SAVE / a segment switch is refused as stale until LOAD / Recipe Open rebuilds the grid
	if (bReset || (bWasShown && nSeg <= 1)) m_strViewRcp.Empty();

	m_cmbSeg.ResetContent();
	if (nSeg > 1)
	{
		CString str;
		for (int k = 0; k < nSeg; k++)
		{
			str.Format(_T("Seg %d/%d %s R%d (%d x %d)"), k + 1, nSeg, pFunc->GetSegmentName(k).GetString(), pFunc->GetSegmentRotate(k) * 90,
				pFunc->GetSegmentStickCount(k), pFunc->GetSegmentCellCount(k));
			m_cmbSeg.AddString(str);
		}
		m_cmbSeg.SetCurSel(m_nViewSeg);
	}
	m_cmbSeg.EnableWindow(TRUE);
	m_cmbSeg.ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
	if (GetDlgItem(IDC_STATIC_EFA_SEG) != NULL) GetDlgItem(IDC_STATIC_EFA_SEG)->ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
}

void CDlgSubManual_AgingNew::OnSelchangeSeg()
{
	int nSel = m_cmbSeg.GetCurSel();
	if (nSel < 0 || nSel == m_nViewSeg) return;

	if (IsSegmented() == FALSE)	// recipe became 1-segment outside this dialog: hide the selector, clear the stamp (the timer does the same)
	{
		UpdateSegmentCombo();
		return;
	}
	if (!IsOfflineStatus())	// also blocks a busy hybrid run (segment contents are not read while the measure thread may copy them)
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}
	C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	if (pSegFunc == NULL || nSel >= pSegFunc->GetSegmentCount())	// review F6: segment count changed since the combo was filled -> rebuild it, no switch
	{
		UpdateSegmentCombo();
		return;
	}
	if (IsMaskViewStale())	// recipe changed since LOAD / grid is not the viewed segment's (popup) - checked before asking, as the WAD Multi dialog does
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	CString strAsk;
	strAsk.Format(_T("세그먼트 %d 셀 맵을 먼저 저장하시겠습니까?\n예: 저장 후 전환 / 아니요: 저장하지 않고 전환 / 취소: 전환 취소"), m_nViewSeg + 1);
	int nAns = AfxMessageBox(strAsk, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (nAns == IDCANCEL || (nAns == IDYES && MeasCellSave() == FALSE))	// mask of the segment being left; the T0/T1 list is glass-common and saved by SAVE
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	CString strLog;
	strLog.Format(_T("[HYBRID] EFA view: segment %d -> %d/%d (%s)"), m_nViewSeg + 1, nSel + 1, pSegFunc->GetSegmentCount(), (nAns == IDYES) ? _T("saved first") : _T("not saved"));
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	m_nViewSeg = nSel;
	UpdateAgingCell();
	MeasCellLoad();
}

void CDlgSubManual_AgingNew::DblClickFpspreadCellStatus(long Col, long Row)
{
	// 헤더 더블클릭 = 일괄 토글 (코너: 시트 전체, 열 헤더: 그 열)
	int nCol = 0, nRow = 0, nMaxCol = 0, nMaxRow = 0;
	CString strValue;

	if (Col == 0 && Row == 0)
	{
		strValue = m_spreadAgingMeas->GetDataString(1, 1);

		nMaxCol = m_spreadAgingMeas->GetMaxCols();
		nMaxRow = m_spreadAgingMeas->GetMaxRows();

		for (nCol = 1; nCol <= nMaxCol; nCol++)
		{
			for (nRow = 1; nRow <= nMaxRow; nRow++)
			{
				m_spreadAgingMeas->SetData(nCol, nRow, strValue);
			}
		}
	}
	else if (Row == 0)
	{
		strValue = m_spreadAgingMeas->GetDataString(Col, 1);
		nMaxRow = m_spreadAgingMeas->GetMaxRows();

		for (nRow = 1; nRow <= nMaxRow; nRow++)
		{
			m_spreadAgingMeas->SetData(Col, nRow, strValue);
		}
	}
}

void CDlgSubManual_AgingNew::SheetChangedFsMeasArea(short OldSheet, short NewSheet)
{
	// OCX 시절 이벤트. CMySpreadDll이 시트를 직접 전환한다. EVENTSINK 맵 때문에 남겨 둔다.
}
