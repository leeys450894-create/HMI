#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AgingTest.h"
#include "AuroraMgr.h"					// LYS 2026-08-06 Phase 1
#include "Manager\ShareMemoryMgr.h"	// LYS 2026-08-06 Phase 1
#include "Recipe\RecipeEditMgr.h"		// LYS 2026-08-06 Phase 1
#include "SystemParamShare.h"			// LYS 2026-08-06 Phase 4
#include "Manager\SequenceManager.h"	// LYS 2026-08-06 Phase 4
#include "Manager\SystemConfigMgr.h"	// LYS 2026-08-18 START pre-check (IsOCP)
#include "DoWork\AgingCycleInfo.h"		// LYS 2026-08-18 START pre-check (engine loader)
#include "ResumePrompt.h"				// LYS 2026-10-08 Hybrid Phase 5 resume prompt on START (D5-11 a)
#include "afxdialogex.h"

#include <iostream>
#include <bitset>

IMPLEMENT_DYNAMIC(CDlgSubManual_AgingTest, CDialogEx)

CDlgSubManual_AgingTest::CDlgSubManual_AgingTest(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=NULL*/) 
	:CDialogEx(CDlgSubManual_AgingTest::IDD, pParent)
	, m_pAuroraMgr(pAuroraMgr)	// LYS 2026-08-06 Phase 1
{
	m_spreadAgingList = NULL;	// LYS 2026-08-06 Phase 1
	m_spreadAgingMeas = NULL;	// LYS 2026-08-06 Phase 1
	m_hfont = NULL;				// LYS 2026-08-06 Phase 1
	m_nHlRow = 0;			// LYS 2026-08-06 E4
	m_nHlCol = 0;
	m_nViewSeg = 0;			// LYS 2026-09-29 Hybrid 3b A13
	m_nMaskMismatch = 0;
	//m_pSystem = NULL;
	//m_pSystem = SMEM_GetSharedMemory();
}

CDlgSubManual_AgingTest::~CDlgSubManual_AgingTest()
{
}

void CDlgSubManual_AgingTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_BUTTON_1, m_chkStop);
	DDX_Control(pDX, IDC_COMBO_1, m_cmbCycle);
	DDX_Control(pDX, IDC_COMBO_AGING_SEG, m_cmbSeg);	// LYS 2026-09-29 Hybrid 3b A13
	for (int i = 0; i < MAX_AGING_STICK; i++) DDX_Control(pDX, IDC_CHECK_1 + i, m_chkStick[i]);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_AgingTest, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()	// LYS 2026-08-06 E4
	ON_COMMAND(IDC_BUTTON_1, OnStop)
	ON_COMMAND(IDC_BUTTON_2, OnRcpOpen)
	ON_COMMAND(IDC_BUTTON_3, OnItemClearAll)
	ON_COMMAND(IDC_BUTTON_4, OnItemAdd)
	ON_COMMAND(IDC_BUTTON_5, OnItemDel)
	ON_COMMAND(IDC_BUTTON_6, OnItemSave)
	ON_COMMAND(IDC_BUTTON_7, OnLoadAgingInfo)
	ON_COMMAND(IDC_BUTTON_8, OnAgingProcStrart)
	ON_CBN_SELCHANGE(IDC_COMBO_AGING_SEG, OnSelchangeSeg)	// LYS 2026-09-29 Hybrid 3b A13
	ON_MESSAGE(SSM_DBLCLK, OnSpreadDBClick)	// LYS 2026-08-06 Phase 3: DLL spread forwards dblclick
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CDlgSubManual_AgingTest, CDialog)
	ON_EVENT(CDlgSubManual_AgingTest, IDC_FPSPREAD_AGING_CELL_STATUS, 11, CDlgSubManual_AgingTest::DblClickFpspreadCellStatus, VTS_I4 VTS_I4)	
	ON_EVENT(CDlgSubManual_AgingTest, IDC_FPSPREAD_AGING_CELL_STATUS, 122, CDlgSubManual_AgingTest::SheetChangedFsMeasArea, VTS_I2 VTS_I2)
	//ON_MESSAGE(SSM_CLICK, OnSpreadDBClick)
	//ON_EVENT(CDlgSubManual_AgingTest, IDC_FPS_AGING_LIST, 3, CDlgSubManual_AgingTest::ButtonClickedFsMeasAreaBottom, VTS_I4 VTS_I4 VTS_I2)
END_EVENTSINK_MAP()

BOOL CDlgSubManual_AgingTest::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CDlgSubManual_AgingTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	m_chkStop.SetImage(IDB_CHECKNO32);
	m_chkStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_chkStop.ShowWindow(FALSE);

	//---------------------------------------------------------------------
	// Farpoint Spread 생성 
	//---------------------------------------------------------------------
	m_spreadAgingList = new CMySpreadDll(IDC_FPS_AGING_LIST, FPS_RESOURCE_ID + 20, this, TRUE);
	m_spreadAgingMeas = new CMySpreadDll(IDC_FPSPREAD_AGING_CELL_STATUS, FPS_RESOURCE_ID + 21, this, TRUE);
	
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

		m_spreadAgingList->InitMySpread_AGINGTEST_LIST();
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

	for (int i = 0; i < MAX_AGING_STICK; i++) {
		m_chkStick[i].SetImage(IDB_CHECKNO32);
		m_chkStick[i].SetCheckedImage(IDB_CHECK32);
		m_chkStick[i].m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
		m_chkStick[i].SetCheck(FALSE);
	}

	CString str;
	for (int i = 0; i < 10; i++) {
		str.Format(_T("%d"), i + 1);
		m_cmbCycle.AddString(str);
	}
	m_cmbCycle.SetCurSel(0);

	m_strAgingRcpPath = theApp.GetProfileString(_T("DIR"), _T("AGING_RCP"), _T("D:\\HBS\\PARAM_OCP\\PARAM\\AgingRcp\\"));
	GetDlgItem(IDC_EDIT_2)->SetWindowText(m_strAgingRcpPath);	// LYS 2026-08-06 default path D:\HBS\PARAM_OCP

	// LYS 2026-08-06 Phase 1: show current recipe name
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());

	SetTimer(1, 500, NULL);	// LYS 2026-08-06 E4: aging progress highlight poll

	return TRUE;
}

void CDlgSubManual_AgingTest::UpdateStatus(BOOL bEnable)
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) m_chkStop.ShowWindow(FALSE);
	//else m_chkStop.ShowWindow(!bEnable);
	
	for (int i = IDC_BUTTON_2; i < IDC_BUTTON_8 + 1; i++) {
		GetDlgItem(i)->EnableWindow(bEnable);
	}
		
	//static BOOL bOdd = TRUE;
	//if (m_pSystem->m_bAutoAgingMode && bOdd) {
	//	int nMaxCount = m_spreadAgingList->GetMaxRows();
	//	if (nMaxCount <= 1) return;
	//
	//	for (int i = 0; i < nMaxCount - 1; i++) {
	//		//m_spreadAgingList.SetRow(i + 2);
	//
	//		for (int j = 2; j < MAX_AGING_ITEM + 1; j++) {
	//			//m_spreadAgingList.SetCol(j);
	//			//m_spreadAgingList.SetBackColor(dfCOLOR_WHITE);
	//		}
	//	}
	//
	//	if (m_pSystem->m_nAgingListRow != 0) {
	//		//m_spreadAgingList.SetRow(m_pSystem->m_nAgingListRow + 1);
	//		//m_spreadAgingList.SetCol(m_pSystem->m_nAgingListCol + 1);
	//		m_spreadAgingList.SetBackColor(dfCOLOR_YELLOW);
	//	}
	//}
	//bOdd = !bOdd;
}

void CDlgSubManual_AgingTest::OnTimer(UINT_PTR nIDEvent)
{
	// LYS 2026-08-06 E4: 에이징 사이클 진행 하이라이트.
	// 엔진이 m_nAgingListRow(1기준 항목 행) / m_nAgingListCol
	// (1=Aging 2=PTN 3=SEC 4=WAD 5=VISION 6=ANGLE)을 쓰고 유휴 시 0/0.
	if (nIDEvent == 1)
	{
		// LYS 2026-09-29 Hybrid 3b A13 recipe changed outside this dialog to a 1-segment recipe -> hide the segment selector
		if (m_cmbSeg.GetSafeHwnd() != NULL && (m_cmbSeg.GetStyle() & WS_VISIBLE) && IsSegmented() == FALSE) UpdateSegmentCombo();

		do
		{
			if (m_spreadAgingList == NULL || m_spreadAgingList->GetSafeHwnd() == NULL) break;
			if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL) break;
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() == NULL) break;

			int nRow = 0, nCol = 0;
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_bAutoAgingMode)
			{
				nRow = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow;
				nCol = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol;
			}

			if (nRow == m_nHlRow && nCol == m_nHlCol) break;	// no change

			if (m_nHlRow > 0 && m_nHlCol > 0 &&
				m_nHlRow <= m_spreadAgingList->GetMaxRows() && m_nHlCol <= m_spreadAgingList->GetMaxCols())
			{
				m_spreadAgingList->SetColor(m_nHlCol, m_nHlRow, RGB_WHITE, RGB_BLACK);
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

void CDlgSubManual_AgingTest::OnStop()
{
	// LYS 2026-08-06 Phase 4: stop aging cycle test (HMI equivalent of OCP m_pJobProc->ActionStop)
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_AgingTest::OnStop"));

	CSystemParamShare::GetInstance()->SetCycleState(false);

	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetShareMemoryMgrPtr() != NULL &&
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_bAutoAgingMode = FALSE;
	}

	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false)
	{
		CSequenceManager::GetInstance()->EngineStop();
	}

	m_chkStop.ShowWindow(FALSE);
}

void CDlgSubManual_AgingTest::OnRcpOpen()
{
	// LYS 2026-08-06 Phase 3: .rcp 선택 -> RecipeChange(OCP LoadPPID2 대응)
	//                        -> 셀 맵 재구성 + 새 레시피의 aging 정보 로드
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

			if (j > 0)	strRcpFile = filename.Left(j);		// remove ".rcp"
			else		strRcpFile = filename;

			if (m_pAuroraMgr->GetRecipeEditMgrPtr()->RecipeChange(strRcpFile) == VTM_RV_SUCCESSED)
			{
				MessageBox(_T("Recipe file be imported complete."));

				GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());

				// LYS 2026-08-06 warn if no matching aging recipe exists in AgingRcp path
				CString strAgingRcp = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());
				if (::GetFileAttributes(strAgingRcp) == INVALID_FILE_ATTRIBUTES)
				{
					CString strMsg;
					strMsg.Format(_T("No Recipe in %s.\nSave the aging recipe first and reopen."), m_strAgingRcpPath);
					AfxMessageBox(strMsg, MB_ICONWARNING);
				}

				LoadAgingInfo();	// UpdateAgingCell + MeasCellLoad run inside
				return;
			}
			else
			{
				AfxMessageBox(_T("Load Recipe Error"));
			}
		}
	}
}

void CDlgSubManual_AgingTest::OnItemClearAll()
{
	if (AfxMessageBox(_T("전체 리스트를 삭제합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	AgingItemClearAll();
}

void CDlgSubManual_AgingTest::OnItemAdd()
{
	AgingItemAdd();
}

void CDlgSubManual_AgingTest::OnItemDel()
{
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	if (nMaxRow <= 0) return;
	
	m_spreadAgingList->SetMaxRows(nMaxRow - 1);
	
	UpdateListNum();	// LYS 2026-08-06 Phase 1 restore
}

void CDlgSubManual_AgingTest::OnItemSave()
{
	CString strCurrentRcp = _T("");	// LYS 2026-08-06 Phase 1: was hardcoded empty (save always blocked)
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		strCurrentRcp = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	GetDlgItem(IDC_EDIT_1)->SetWindowText(strCurrentRcp);
	if (strCurrentRcp == _T("") || strCurrentRcp == _T("UNKNOWN")) {
		AfxMessageBox(_T("기본 레시피가 없습니다.\n 기본 레시피 Open 후 진행 하십시오."), MB_ICONWARNING);
		return;
	}
	
	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("현재값을 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	
	SaveAgingInfo();
}

void CDlgSubManual_AgingTest::OnLoadAgingInfo()
{
	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("이전값을 불러 옵니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	
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
	strLog.Format(_T("[HYBRID] AgingTest START refused - process status %d is not idle (manual worker / recipe command running)"), nSps);
	CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
	return true;
}

void CDlgSubManual_AgingTest::OnAgingProcStrart()
{
	// LYS 2026-08-06 Phase 4: HMI 사이클 검사 경로로 에이징 사이클 테스트 시작
	// (OCP: m_bAutoAgingMode + nJobCount + JOB_Proc::DoStartCycleTest).
	// 엔진이 측정 시퀀스를 CSystemParamShare::GetCycleCount()회 반복한다.
	// TODO: AGINGTEST_CONFIRM - 에이징 리스트 기반 측정 엔진(OCP DoWorkStartMeasOCP_A,
	//       6,743줄: 패턴별 점등 + 셀별 WAD/VISION/Color/ANGLE)은 미이식.
	//       m_bAutoAgingMode 소비처가 없어 사이클은 표준 시퀀스로 돈다.
	if (!IsOfflineStatus()) return;

	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL ||
		m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return;

	// LYS 2026-10-13 Hybrid Phase 7 D7-15 (b): hybrid runtime recipe -> START also needs SPS_IDLE (IsHybridStartNotIdle); 1-segment recipe: SPS_PROCESS only, unchanged
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_PROCESS || IsHybridStartNotIdle(m_pAuroraMgr))
	{
		AfxMessageBox(_T("Process is running.\nTry again after it completes."), MB_ICONWARNING);
		return;
	}

	if (AfxMessageBox(_T("Start Aging Proc.\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strRcp = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	if (strRcp.IsEmpty() || strRcp == _T("UNKNOWN"))
	{
		AfxMessageBox(_T("No recipe information.\nOpen a recipe first."), MB_ICONWARNING);
		return;
	}

	// LYS 2026-09-30 user request: the cycle measures CSystemParamShare's recipe (the recipe step calls OpenRecipe with it), not the one
	//   this dialog opened with RecipeChange -> START is refused unless both are the same recipe (1-segment and hybrid alike), otherwise
	//   the aging flag / file / mask checks below would describe another recipe. Run recipe "UNKNOWN" (nothing opened for a run) is refused.
	if (strRcp.CompareNoCase(CSystemParamShare::GetInstance()->GetRecipeID()) != 0)
	{
		CString strRunRcp = CSystemParamShare::GetInstance()->GetRecipeID();
		CString strRefuse;
		strRefuse.Format(_T("AgingTest START refused - dialog recipe %s != run recipe %s (the cycle measures the run recipe)"), (LPCTSTR)strRcp, (LPCTSTR)strRunRcp);
		CLogWrapper::GetInstance()->KMLOG(OPER, strRefuse);
		CString strMsg;
		strMsg.Format(_T("이 화면의 레시피(%s)와 운전 레시피(%s)가 다릅니다.\nSTART는 운전 레시피를 측정하므로 이 화면의 셀 맵과 사전 검사가 맞지 않습니다.\n운전 레시피를 이 레시피로 연 뒤(운전 화면 CYCLE 또는 수동 측정 화면의 레시피 열기) 다시 시도하십시오."), (LPCTSTR)strRcp, (LPCTSTR)strRunRcp);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return;
	}

	//=====================================================================
	// LYS 2026-08-18 START 사전 검사.
	// 엔진은 m_bAutoAgingMode를 보지 않는다. 에이징 사이클은 AgingCycleBegin()이 통과할
	// 때만 켜진다(IsOCP + 레시피 m_bUseAging_Test + USE_AGING 항목이 있는 aging rcp).
	// 검사가 없으면 패턴 점등 없는 일반 시퀀스가 조용히 돌아 이유를 알 수 없었다.
	//=====================================================================
	if (m_pAuroraMgr->GetSystemConfigMgrPtr() == NULL ||
		m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP() == false)
	{
		AfxMessageBox(_T("Aging cycle runs in OCP mode only.\nCheck SystemConfig.ini [EQUIP_TYPE] MACHINE_TYPE."), MB_ICONWARNING);
		return;
	}

	S_RecipeInfo* pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo == NULL || pRecipeInfo->m_sInspectItem.m_bUseAging_Test == FALSE)
	{
		AfxMessageBox(_T("AGING TEST is not enabled in the current recipe.\nTurn it on in RECIPE - Inspection Item, then retry."), MB_ICONWARNING);
		return;
	}

	// same loader the sequence uses, so this cannot drift from the engine gate
	CAgingCycleInfo cycleInfo;
	if (cycleInfo.Load(strRcp) == false)
	{
		CString strChkMsg;
		strChkMsg.Format(_T("Aging recipe not found.\n%s%s.rcp\n\nSave the list first."), (LPCTSTR)m_strAgingRcpPath, (LPCTSTR)strRcp);
		AfxMessageBox(strChkMsg, MB_ICONWARNING);
		return;
	}

	int nUseItem = 0;
	for (int iChk = 0; iChk < cycleInfo.GetItemCount(); iChk++)
	{
		const S_AgingTest* pChk = cycleInfo.GetItem(iChk);
		if (pChk != NULL && pChk->m_bUseAging) nUseItem++;
	}

	if (nUseItem < 1)
	{
		AfxMessageBox(_T("The saved aging list has no item with Aging checked.\nCheck at least one item and save."), MB_ICONWARNING);
		return;
	}

	int nMaxCount = m_spreadAgingList->GetMaxRows();	// HMI grid: MaxRows = item count
	if (nMaxCount < 1)
	{
		AfxMessageBox(_T("No list information.\nAdd and save the list first."), MB_ICONWARNING);
		return;
	}

	// cycle count from combo
	CString strTemp;
	int nCycleCount = 1;
	m_cmbCycle.GetWindowText(strTemp);
	nCycleCount = _ttoi(strTemp);
	if (nCycleCount < 1) nCycleCount = 1;

	// collect grid -> m_vecAginList (cols 1..6 = Aging/PTN/Sec/Property/Quality/ANGLE)
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

	// LYS 2026-10-08 Hybrid Phase 5 (D5-11 a) journal bookmark -> the same resume prompt as CYCLE (EFA mode checked for this START);
	//   otherwise only the stale resume flag is cleared (D5-10 a). CANCEL = nothing changed.
	if (ResumeDialogEntry(m_pAuroraMgr, RESUME_ENTRY_AGINGTEST) == RESUME_PROMPT_CANCEL)
		return;

	// shared state (OCP parity: JOB_Proc reads these in auto aging mode)
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_bAutoAgingMode = TRUE;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_NewAgingMode(false);	// LYS 2026-09-28 EFA: EFA 모드와 동시에 켜지지 않게 한다 (EFA)
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nJobCount = nCycleCount;
	}

	// HMI cycle-inspection start (same path as DlgSubOperate cycle button)
	CSystemParamShare::GetInstance()->SetCycleCount(nCycleCount);
	CSystemParamShare::GetInstance()->SetCycleState(true);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);

	m_chkStop.ShowWindow(TRUE);

	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_AgingTest::OnAgingProcStrart - cycle test start"));

	CSequenceManager::GetInstance()->EngineStart();
}

LRESULT CDlgSubManual_AgingTest::OnSpreadDBClick(WPARAM wParam, LPARAM lParam)
{
	// LYS 2026-08-06 Phase 3: forwarded by CMySpreadDll (wParam = component ID, lParam = LPSS_CELLCOORD)
	if ((UINT)wParam != IDC_FPSPREAD_AGING_CELL_STATUS) return 0;

	LPSS_CELLCOORD pCoord = (LPSS_CELLCOORD)lParam;
	if (pCoord == NULL) return 0;

	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return 0;

	DblClickFpspreadCellStatus((long)pCoord->Col, (long)pCoord->Row);
	return 0;
}

void CDlgSubManual_AgingTest::UpdateListNum()
{
	// LYS 2026-08-06 Phase 1: No goes to row header col 0 (old code wrote into col 1 = Aging)
	CString str;
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	for (int i = 1; i <= nMaxRow; i++) {
		str.Format(_T("%d"), i);
		m_spreadAgingList->SetDataString(0, i, str);
	}
}

BOOL CDlgSubManual_AgingTest::IsOfflineStatus()
{
	// LYS 2026-08-06 Phase 1: restore offline check via ShareMemoryMgr
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetShareMemoryMgrPtr() != NULL) {
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE) {
			AfxMessageBox(_T("Offline 모드가 아닙니다..\n Offline 모드로 변경 후 진행 하십시오."), MB_ICONWARNING);
			return FALSE;
		}
	}

	// LYS 2026-09-29 Hybrid 3b A13 no recipe open / save / reload / START / segment switch while a hybrid recipe is being measured
	//   (RecipeChange mid-run resets the segments under the measure thread). 1-segment recipes: unchanged.
	if (IsHybridRunBusy())
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

BOOL CDlgSubManual_AgingTest::LoadAgingInfo()
{
	// LYS 2026-08-06 Phase 1: rcp 경로(현재 레시피) 복원 + 그리드 인덱싱 수정
	// HMI 그리드 좌표: 헤더 = SS_HEADER 행, 데이터 1..MaxRows, 열 1..6
	CString strRcpID = _T("");
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		strRcpID = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	if (strRcpID.IsEmpty()) return FALSE;
	UpdateSegmentCombo();	// LYS 2026-09-29 Hybrid 3b A13 reset/clamp the viewed segment before the grid is rebuilt

	CString strAgingRcpPath;
	strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcpID);

	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, sizeof(cValue));

	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
	int nMaxCount = _ttoi(cValue);

	AgingItemClearAll();
	if (nMaxCount <= 0) {
		// LYS 2026-09-30 no AgingRcp yet: build this recipe's cell map for every recipe (3b did it for hybrid only).
		//   A 1-segment recipe used to keep the dialog's default 10 x 5 one-sheet grid, and SAVE wrote it
		//   (2026-09-30 10:29: 5 x 20 recipe saved as 10-cell rows with 7/0 values -> cells K..T measured)
		m_strViewSegRcp = strRcpID;
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

		GetPrivateProfileString(strItem, _T("USE_AGING"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(1, nRow, cValue);
		sAgingList.m_bUseAging = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(2, nRow, cValue);
		sAgingList.m_nPtnRGB = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("PTN_SEC"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(3, nRow, cValue);
		sAgingList.m_nPtnSec = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_PROPERTY"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(4, nRow, cValue);
		sAgingList.m_bUseProperty = _ttoi(cValue);

		GetPrivateProfileString(strItem, _T("USE_QUALITY"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(5, nRow, cValue);
		sAgingList.m_bUseQuality = _ttoi(cValue);

		// LYS 2026-08-06 Phase 2: ANGLE col (HMI: replaces OCP USE_LCR)
		GetPrivateProfileString(strItem, _T("USE_ANGLE"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
		m_spreadAgingList->SetData(6, nRow, cValue);
		sAgingList.m_bUseAngle = _ttoi(cValue);

		m_vecAginList.push_back(sAgingList);
	}

	m_strViewSegRcp = strRcpID;	// LYS 2026-09-29 Hybrid 3b A13 the grid below is built for this recipe
	UpdateAgingCell();	// LYS 2026-08-06 Phase 3: build 3-sheet map first
	MeasCellLoad();		// LYS 2026-08-06 Phase 3: then fill from [MASECELL]

	return TRUE;
}

void CDlgSubManual_AgingTest::SaveAgingInfo()
{
	CString strAgingRcpPath;
	CString strRcpID = _T("");	// LYS 2026-08-06 Phase 1: current recipe from RecipeEditMgr
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		strRcpID = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	if (strRcpID.IsEmpty()) return;
	strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcpID);
	if (IsMaskViewStale(strRcpID)) return;	// LYS 2026-09-29 Hybrid 3b A13 check before [COUNT]/[ITEM_n] are written (MeasCellSave repeats it)
	// LYS 2026-09-30 IsMaskViewStale checks 1-segment recipes too (default 10 x 5 grid before LOAD / Recipe Open)
	
	dfcCreateFolder(m_strAgingRcpPath);
	
	CString strValue, strItem;	
	int nMaxCount = m_spreadAgingList->GetMaxRows();
	if (nMaxCount < 1) {	// LYS 2026-08-06 Phase 1 (HMI: MaxRows = item count)
		AfxMessageBox(_T("Aging 레시피 항목 추가후 저장 하십시오."));
		return;
	}
	
	if (ConfirmMaskOverwrite() == FALSE) return;	// LYS 2026-09-30 ask before [COUNT]/[ITEM_n] are written

	strValue.Format(_T("%d"), nMaxCount);	// LYS 2026-08-06 Phase 1
	WritePrivateProfileString(_T("COUNT"), _T("MaxCount"), strValue, strAgingRcpPath);
	
	for (int i = 0; i < nMaxCount; i++) {	// LYS 2026-08-06 Phase 1
		//m_spreadAgingList.SetRow(i + 2);
	
		strItem.Format(_T("ITEM_%d"), i + 1);
	
		for (int j = 1; j < MAX_AGING_ITEM; j++) {
			//m_spreadAgingList.SetCol(j);
	
			if (j == 1) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("USE_AGING"), strValue, strAgingRcpPath);
			}
			else if (j == 2) {
				//strValue.Format(_T("%d"), m_spreadAgingList->GetTypeComboBoxCurSel());
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("PTN_RGB"), strValue, strAgingRcpPath);
			}
			else if (j == 3) {
				//strValue.Format (_T("%d"), m_spreadAgingList.GetTypeComboBoxCurSel ());
				//m_spreadAgingList.GetWindowText (strValue);
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("PTN_SEC"), strValue, strAgingRcpPath);
			}
			else if (j == 4) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("USE_PROPERTY"), strValue, strAgingRcpPath);
			}
			else if (j == 5) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("USE_QUALITY"), strValue, strAgingRcpPath);
			}
			// LYS 2026-08-06 Phase 2: ANGLE col (HMI: replaces OCP USE_LCR)
			else if (j == 6) {
				strValue = m_spreadAgingList->GetDataString(j, i + 1);
				WritePrivateProfileString(strItem, _T("USE_ANGLE"), strValue, strAgingRcpPath);
			}
		}
	}
	
	MeasCellSave();
}

void CDlgSubManual_AgingTest::AgingItemClearAll()
{
	m_spreadAgingList->InitMySpread_AGINGTEST_LIST();
}

void CDlgSubManual_AgingTest::AgingItemAdd()
{
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	if (nMaxRow < 0) return;
	
	CString str, sComboList;
	SS_CELLTYPE cellType;

	m_spreadAgingList->SetMaxRows(nMaxRow + 1);
	//m_spreadAgingList.SetRow(nMaxRow + 1);
	
	for (int i = 1; i < MAX_AGING_ITEM; i++) {
		//m_spreadAgingList.SetCol(i);
		//if (i == 2 || i == 5 || i == 6) {
		if (i == 1 || i == 4 || i == 5 || i == 6) {	// LYS 2026-08-06 Phase 2 +ANGLE col
			m_spreadAgingList->SetTypeCheckBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			m_spreadAgingList->SetCellType(i, nMaxRow
				+ 1, &cellType);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
			//m_spreadAgingList.SetCellType(10);			//Check Box
			//m_spreadAgingList.SetTypeCheckCenter(TRUE);
			//m_spreadAgingList.SetTypeCheckType(0);
		}
	
		if (i == 2) {
		
			sComboList = _T("0\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t");	// LYS 2026-08-06 Phase 2: add missing pattern 0
			m_spreadAgingList->SetTypeComboBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, sComboList);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
			//m_spreadAgingList.SetCellType(8);			//Combo Box
			//m_spreadAgingList.SetTypeHAlign(2);
			//m_spreadAgingList.SetTypeVAlign(2);
			//for (int j = 10; j >= 0; j--) {
			//	str.Format(_T("%d"), j);
			//	//m_spreadAgingList.SetTypeComboBoxString(str);
			//	//m_spreadAgingList->SetTypeComboBox
			//}
			//m_spreadAgingList.SetTypeComboBoxCurSel(0);
		}
	
		if (i == 3) {
			m_spreadAgingList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
			//m_spreadAgingList.SetCellType(1);			//Edit
			//m_spreadAgingList.SetTypeHAlign(2);
			//m_spreadAgingList.SetTypeVAlign(2);
			//m_spreadAgingList.SetWindowText(_T("0"));
			//m_spreadAgingList->SetData(i, )
		}
	}
	
	UpdateListNum();	// LYS 2026-08-06 Phase 1 restore
}

void CDlgSubManual_AgingTest::UpdateAgingCell()
{
	// LYS 2026-08-06 Phase 3: 3시트 셀 사용 맵 재구성(WAD/VISION/ANGLE - HMI엔 COLOR 없음).
	// 행 = stick, 열 = 현재 RecipeData의 셀. 값은 AgingRcp의 [MASECELL]에만 저장하고
	// 레시피 MEASGROUP은 건드리지 않는다(OCP는 m_iUsedMeas에 미러링했음).
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	int nStick = 0, nCell = 0;
	S_RecipeData* pRD = NULL;
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
	{
		// LYS 2026-09-29 Hybrid 3b A13 hybrid: dimensions of the selected segment (no SetActiveSegment - the measure thread owns it)
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		S_RecipeMgr* pSegMgr = (IsSegmented() && pSegFunc != NULL) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL;
		pRD = (pSegMgr != NULL) ? &pSegMgr->m_sData : m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	}

	if (pRD != NULL && pRD->m_vecStick.size() > 0)
	{
		nStick = (int)pRD->m_vecStick.size();
		nCell = (int)pRD->m_vecStick[0].m_vecCell.size();
	}

	//static const TCHAR* szSheet[4] = { _T("WAD"), _T("VISION"), _T("Color"), _T("ANGLE") };
	static const TCHAR* szSheet[3] = { _T("WAD"), _T("VISION"),_T("ANGLE") };

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

int CDlgSubManual_AgingTest::GetMeasUse(int iMeas, int nMeasUse)
{
	int iRet = 0;
	BYTE nByte = 0x00;

	nByte = (nMeasUse >> iMeas) & 0x01;
	if (nByte == 0)
		iRet = 0;
	else
		iRet = 1;

	return iRet;
}

BOOL CDlgSubManual_AgingTest::MeasCellSave()	// LYS 2026-09-29 Hybrid 3b A13 FALSE = not written
{
	// LYS 2026-08-06 Phase 3: 그리드 -> [MASECELL] STICK_n = 3bit 마스크 CSV
	// bit0..2 = WAD / VISION / ANGLE (HMI엔 COLOR 없음, OCP는 4종)
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return TRUE;

	CString strRcpID = _T("");
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		strRcpID = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	if (strRcpID.IsEmpty()) return TRUE;

	CString strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcpID);
	CString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);	// LYS 2026-09-29 Hybrid 3b A13 segment 0 = MASECELL (unchanged)
	// LYS 2026-09-30 stale/size check and overwrite confirmation for every recipe (1-segment too)
	if (IsMaskViewStale(strRcpID)) return FALSE;
	if (ConfirmMaskOverwrite() == FALSE) return FALSE;

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

		WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), strCellData, strAgingRcpPath);
	}

	// LYS 2026-09-29 Hybrid 3b A13 delete this section's rows beyond the segment's stick count (the loader reads until the first missing key)
	if (IsSegmented())
	{
		TCHAR cOld[16];
		for (int i = nStick; i < DEFINE_AGING_CYCLE_STICK_MAX; i++)
		{
			ZeroMemory(cOld, sizeof(cOld));
			GetPrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), _T(""), cOld, _countof(cOld), strAgingRcpPath);
			if (cOld[0] == 0) break;
			WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), NULL, strAgingRcpPath);
		}
	}
	m_nMaskMismatch = 0;

	m_spreadAgingMeas->SetSheet(nKeepSheet);
	return TRUE;
}

void CDlgSubManual_AgingTest::MeasCellLoad()
{
	// LYS 2026-08-06 Phase 3: [MASECELL] -> grid. Call AFTER UpdateAgingCell built the sheets.
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	CString strRcpID = _T("");
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		strRcpID = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
	if (strRcpID.IsEmpty()) return;

	CString strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcpID);
	CString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);	// LYS 2026-09-29 Hybrid 3b A13
	m_nMaskMismatch = 0;

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return; }

	TCHAR cCellData[1024];	// OCP used MAX_PATH - too small for many cells
	CArray<int, int> nCellChkArray;

	// LYS 2026-09-29 Hybrid 3b A13 segment k>=1 without its section: the engine measures every cell, so show every cell checked
	if (IsSegmented() && m_nViewSeg >= 1)
	{
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(strSec, _T("STICK_1"), _T(""), cCellData, 1024, strAgingRcpPath);
		if (cCellData[0] == 0)
		{
			for (int k = 0; k < 3; k++) {
				m_spreadAgingMeas->SetSheet(k + 1);
				for (int i = 0; i < nStick; i++)
					for (int j = 0; j < nCell; j++)
						m_spreadAgingMeas->SetData(j + 1, i + 1, _T("1"));
			}
			m_spreadAgingMeas->SetSheet(nKeepSheet);
			return;
		}
	}

	for (int i = 0; i < nStick; i++) {
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), _T(""), cCellData, 1024, strAgingRcpPath);

		int pos = 0;
		nCellChkArray.RemoveAll();
		CString strCellData = cCellData;
		CString resToken = strCellData.Tokenize(_T(","), pos);
		while (resToken != _T("")) {
			nCellChkArray.Add(_ttoi(resToken.Trim()));
			resToken = strCellData.Tokenize(_T(","), pos);
		}

		if (nCellChkArray.GetSize() != nCell) {	// row missing/mismatched -> keep defaults
			if (nCellChkArray.GetSize() > 0) m_nMaskMismatch++;	// LYS 2026-09-29 Hybrid 3b A13 present but wrong cell count
			continue;
		}

		for (int j = 0; j < nCell; j++) {
			int nMask = nCellChkArray.GetAt(j);
			for (int k = 0; k < 3; k++) {	// WAD, VISION, ANGLE
				m_spreadAgingMeas->SetSheet(k + 1);
				m_spreadAgingMeas->SetData(j + 1, i + 1, ((nMask >> k) & 0x01) ? _T("1") : _T("0"));
			}
		}
	}

	// LYS 2026-09-30 warn for every recipe (1-segment too): rows with another cell count stay unchecked and SAVE overwrites them
	if (m_nMaskMismatch > 0)	// LYS 2026-09-29 Hybrid 3b A13 (2026-09-28 16:12 case: rows kept unchecked, then saved as zeros)
	{
		CString strLog;
		strLog.Format(_T("%sAgingRcp %s: %d row(s) with cell count != %d shown unchecked (%s)"), IsSegmented() ? _T("[HYBRID] ") : _T(""), strSec.GetString(), m_nMaskMismatch, nCell, strAgingRcpPath.GetString());
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);

		CString strWarn;
		if (IsSegmented())
			strWarn.Format(_T("세그먼트 %d 셀 맵에 셀 수가 %d개가 아닌 행이 %d개 있습니다.\n해당 행은 미체크로 표시되며, 저장하면 화면 값으로 덮어씁니다."), m_nViewSeg + 1, nCell, m_nMaskMismatch);
		else
			strWarn.Format(_T("셀 맵에 셀 수가 %d개가 아닌 행이 %d개 있습니다.\n해당 행은 미체크로 표시되며, 저장하면 화면 값으로 덮어씁니다."), nCell, m_nMaskMismatch);
		AfxMessageBox(strWarn, MB_ICONWARNING);
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
}

// LYS 2026-09-29 Hybrid 3b A13 segment selector for the AgingRcp cell mask.
//   Hybrid recipes only: the grid shows GetSegmentMgr(m_nViewSeg) and saves to CAgingCycleInfo::GetMaskSection(m_nViewSeg).
//   SetActiveSegment is never called here (the measure thread owns the active segment).
BOOL CDlgSubManual_AgingTest::IsSegmented()
{
	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return FALSE;
	return (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1) ? TRUE : FALSE;
}

BOOL CDlgSubManual_AgingTest::IsHybridRunBusy()
{
	if (IsSegmented() == FALSE) return FALSE;
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false) return TRUE;
	return CSystemParamShare::GetInstance()->IsCycleState() ? TRUE : FALSE;
}

void CDlgSubManual_AgingTest::UpdateSegmentCombo()
{
	if (m_cmbSeg.GetSafeHwnd() == NULL) return;

	CString strRcpID = _T("");
	C_OutRecipeFunc* pFunc = NULL;
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
	{
		strRcpID = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
		pFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	}
	int nSeg = (IsSegmented() && pFunc != NULL) ? pFunc->GetSegmentCount() : 1;

	if (strRcpID != m_strViewSegRcp || m_nViewSeg < 0 || m_nViewSeg >= nSeg) m_nViewSeg = 0;	// m_strViewSegRcp is stamped by LoadAgingInfo right before the grid is rebuilt

	m_cmbSeg.ResetContent();
	if (nSeg > 1)
	{
		CString str;
		for (int k = 0; k < nSeg; k++)
		{
			str.Format(_T("Seg %d/%d %s (%d x %d)"), k + 1, nSeg, pFunc->GetSegmentName(k).GetString(), pFunc->GetSegmentStickCount(k), pFunc->GetSegmentCellCount(k));
			m_cmbSeg.AddString(str);
		}
		m_cmbSeg.SetCurSel(m_nViewSeg);
	}
	m_cmbSeg.ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
	if (GetDlgItem(IDC_STATIC_AGING_SEG) != NULL) GetDlgItem(IDC_STATIC_AGING_SEG)->ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
}

// LYS 2026-09-29 Hybrid 3b A13 the grid/combo were built for another recipe (changed outside this dialog, or LOAD not done yet)
// LYS 2026-09-30 every recipe, and the grid size must be the recipe's: the default 10 x 5 grid shown before LOAD / Recipe Open
//   has no recipe stamp, so saving it is refused (it wrote 10-cell rows for a 20-cell recipe on 2026-09-30)
BOOL CDlgSubManual_AgingTest::IsMaskViewStale(CString strRcpID)
{
	int nStick = 0, nCell = 0;
	BOOL bStale = (strRcpID != m_strViewSegRcp) ? TRUE : FALSE;
	if (bStale == FALSE)
	{
		S_RecipeData* pRD = NULL;
		if (IsSegmented())
		{
			C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
			S_RecipeMgr* pSegMgr = (pSegFunc != NULL && m_nViewSeg < pSegFunc->GetSegmentCount()) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL;
			if (pSegMgr == NULL) bStale = TRUE;
			else pRD = &pSegMgr->m_sData;
		}
		else
			pRD = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();

		if (pRD != NULL && pRD->m_vecStick.size() > 0)
		{
			nStick = (int)pRD->m_vecStick.size();
			nCell = (int)pRD->m_vecStick[0].m_vecCell.size();
		}
	}
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

// LYS 2026-09-30 rows whose cell count did not match were shown unchecked by MeasCellLoad; ask once before they are overwritten.
//   TRUE = go on (nothing to ask, or accepted - then the count is cleared so MeasCellSave does not ask again)
BOOL CDlgSubManual_AgingTest::ConfirmMaskOverwrite()
{
	if (m_nMaskMismatch <= 0) return TRUE;
	CString strAsk;
	if (IsSegmented())
		strAsk.Format(_T("세그먼트 %d 셀 맵에서 셀 수가 맞지 않던 행 %d개를 지금 화면 값으로 덮어씁니다.\n계속 하시겠습니까?"), m_nViewSeg + 1, m_nMaskMismatch);
	else
		strAsk.Format(_T("셀 맵에서 셀 수가 맞지 않던 행 %d개를 지금 화면 값으로 덮어씁니다.\n계속 하시겠습니까?"), m_nMaskMismatch);
	if (AfxMessageBox(strAsk, MB_YESNO | MB_ICONWARNING) != IDYES) return FALSE;
	m_nMaskMismatch = 0;
	return TRUE;
}

void CDlgSubManual_AgingTest::OnSelchangeSeg()
{
	int nSel = m_cmbSeg.GetCurSel();
	if (nSel < 0 || nSel == m_nViewSeg) return;

	if (IsSegmented() == FALSE || !IsOfflineStatus())	// IsOfflineStatus also blocks while a hybrid run is busy
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	CString strAsk;
	strAsk.Format(_T("세그먼트 %d 셀 맵을 먼저 저장하시겠습니까?\n예: 저장 후 전환 / 아니요: 저장하지 않고 전환 / 취소: 전환 취소"), m_nViewSeg + 1);
	int nAns = AfxMessageBox(strAsk, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (nAns == IDCANCEL || (nAns == IDYES && MeasCellSave() == FALSE))	// mask of the segment being left; the item list is glass-common and saved by SAVE
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	m_nViewSeg = nSel;
	UpdateAgingCell();
	MeasCellLoad();
}

void CDlgSubManual_AgingTest::DblClickFpspreadCellStatus(long Col, long Row)
{
	int nCol = 0, nRow = 0, nMaxCol = 0, nMaxRow = 0;
	CString strValue;
	
	if (Col == 0 && Row == 0)
	{
		//m_spreadAgingMeas.SetCol(1);
		//m_spreadAgingMeas.SetRow(1);
		//strValue = m_spreadAgingMeas.GetValue();
		strValue = m_spreadAgingMeas->GetDataString(1,1);
	
		nMaxCol = m_spreadAgingMeas->GetMaxCols();
		nMaxRow = m_spreadAgingMeas->GetMaxRows();
	
		for (nCol = 1; nCol <= nMaxCol; nCol++)
		{
			//m_spreadAgingMeas.SetCol(nCol);
	
			for (nRow = 1; nRow <= nMaxRow; nRow++)
			{
				//m_spreadAgingMeas.SetRow(nRow);
				//m_spreadAgingMeas.SetValue(strValue);
				m_spreadAgingMeas->SetData(nCol, nRow, strValue);
			}
		}
	}
	else if (Row == 0)
	{
		//m_spreadAgingMeas.SetCol(Col);
		//m_spreadAgingMeas.SetRow(1);
		//strValue = m_spreadAgingMeas.GetValue();
		strValue = m_spreadAgingMeas->GetDataString(Col, 1);
		nMaxRow = m_spreadAgingMeas->GetMaxRows();
	
		for (nRow = 1; nRow <= nMaxRow; nRow++)
		{
			//m_spreadAgingMeas.SetRow(nRow);
			//m_spreadAgingMeas.SetValue(strValue);
			m_spreadAgingMeas->SetData(Col, nRow, strValue);
		}
	}
}

void CDlgSubManual_AgingTest::SheetChangedFsMeasArea(short OldSheet, short NewSheet)
{
	// LYS 2026-08-06 Phase 3: OCX 시절 이벤트라 CMySpreadDll에서는 발생하지 않는다
	// (DLL spread가 탭 클릭 시 스스로 시트를 갱신. EVENTSINK 맵 유지용).
}