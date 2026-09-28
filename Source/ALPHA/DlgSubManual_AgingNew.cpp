// DlgSubManual_AgingNew.cpp
// LYS 2026-09-28 EFA: EFA (2-pass aging) test view - port of OCP_EFA CDlgNaviManualAgingNew.
// Copied from CDlgSubManual_AgingTest (2026-08-06 port) and reduced to the EFA deltas:
//   - separate mode flag  : VTM_SYSTEM::m_bNewAgingMode (OCP m_bNewAgingMode) instead of m_bAutoAgingMode
//   - separate rcp folder : registry DIR\EFA_RCP (OCP reuses DIR\AGING_RCP by mistake -> file collision)
//   - list fixed to 2 rows: T0 (before aging) / T1 (after aging); ITEM add/del hidden
//   - cycle combo 1..25, pattern combo 0..25
// File format is identical to AgingTest ({rcp}.rcp INI: [COUNT] [ITEM_n] [MASECELL]) so CAgingCycleInfo can parse it.
// ASCII only: this file is compiled as CP949 without /utf-8 (see HMI/CLAUDE.md rule 7).
#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AgingNew.h"
#include "AuroraMgr.h"
#include "Manager\ShareMemoryMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Manager\SequenceManager.h"
#include "Manager\SystemConfigMgr.h"
#include "afxdialogex.h"

#include <iostream>
#include <bitset>

#define EFA_TIMER_HIGHLIGHT		1
#define EFA_CELLMASK_BUF		1024	// [MASECELL] line buffer (MAX_PATH is too small for many cells)

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
}

CDlgSubManual_AgingNew::~CDlgSubManual_AgingNew()
{
	// spreads: same lifetime handling as CDlgSubManual_AgingTest (child windows die with the dialog)
}

void CDlgSubManual_AgingNew::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_BUTTON_1, m_chkStop);
	DDX_Control(pDX, IDC_COMBO_1, m_cmbCycle);
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
	ON_MESSAGE(SSM_DBLCLK, OnSpreadDBClick)	// DLL spread forwards dblclick (OCX EVENTSINK does not fire)
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
// helpers
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
// init
//=========================================================================
BOOL CDlgSubManual_AgingNew::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	m_chkStop.SetImage(IDB_CHECKNO32);
	m_chkStop.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_chkStop.ShowWindow(FALSE);

	// FarPoint spreads (DLL). Resource IDs +28/+29 are unused by the other manual views.
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

		m_spreadAgingList->InitMySpread_AGINGTEST_LIST();	// same 7-col layout as AgingTest
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
	for (int i = 0; i < 25; i++) {	// OCP_EFA: cycle count 1..25 (AgingTest 1..10)
		str.Format(_T("%d"), i + 1);
		m_cmbCycle.AddString(str);
	}
	m_cmbCycle.SetCurSel(0);

	m_strEfaRcpPath = GetEfaRcpPath();
	GetDlgItem(IDC_EDIT_2)->SetWindowText(m_strEfaRcpPath);
	GetDlgItem(IDC_EDIT_1)->SetWindowText(GetRecipeID());

	// EFA list is fixed to 2 rows (T0/T1) - ITEM add/del stay hidden (rc: NOT WS_VISIBLE)
	GetDlgItem(IDC_BUTTON_4)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_5)->ShowWindow(SW_HIDE);

	SetTimer(EFA_TIMER_HIGHLIGHT, 500, NULL);	// progress highlight poll

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
	// EFA pass progress highlight. Engine writes m_nAgingListRow (1-base pass = T0/T1 row) and
	// m_nAgingListCol (1=Aging 2=PTN 3=SEC 4=WAD 5=VISION 6=ANGLE), 0/0 when idle.
	if (nIDEvent == EFA_TIMER_HIGHLIGHT)
	{
		do
		{
			if (m_spreadAgingList == NULL || m_spreadAgingList->GetSafeHwnd() == NULL) break;
			if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL) break;
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() == NULL) break;

			int nRow = 0, nCol = 0;
			if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_NewAgingMode())
			{
				nRow = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow;
				nCol = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol;
			}

			if (nRow == m_nHlRow && nCol == m_nHlCol) break;	// no change

			if (m_nHlRow > 0 && m_nHlCol > 0 &&
				m_nHlRow <= m_spreadAgingList->GetMaxRows() && m_nHlCol <= m_spreadAgingList->GetMaxCols())
			{
				m_spreadAgingList->SetColor(m_nHlCol, m_nHlRow, RGB_WHITE, RGB_BLACK);	// FarPoint: (Col, Row)
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
// buttons
//=========================================================================
void CDlgSubManual_AgingNew::OnStop()
{
	// HMI equivalent of OCP m_pJobProc->ActionStop()
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
	// .rcp select -> RecipeChange (OCP LoadPPID2) -> rebuild cell map + load EFA info for the new recipe
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

				GetDlgItem(IDC_EDIT_1)->SetWindowText(GetRecipeID());

				CString strEfaRcp = GetEfaRcpFile();
				if (strEfaRcp.IsEmpty() || ::GetFileAttributes(strEfaRcp) == INVALID_FILE_ATTRIBUTES)
				{
					CString strMsg;
					strMsg.Format(_T("No EFA recipe in %s.\nSave the EFA list first and reopen."), (LPCTSTR)m_strEfaRcpPath);
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

void CDlgSubManual_AgingNew::OnItemClearAll()
{
	// OCP_EFA: "reset the whole list" -> rebuild the 2 fixed rows
	if (AfxMessageBox(_T("Reset the EFA list to 2 empty items (T0 / T1).\nContinue?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	AgingItemClearAll(TRUE);
}

void CDlgSubManual_AgingNew::OnItemAdd()
{
	return;	// OCP_EFA: list fixed to 2 rows (button hidden)
}

void CDlgSubManual_AgingNew::OnItemDel()
{
	return;	// OCP_EFA: list fixed to 2 rows (button hidden)
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

void CDlgSubManual_AgingNew::OnAgingProcStrart()
{
	// EFA cycle test start via the HMI cycle-inspection path
	// (OCP_EFA: m_bNewAgingMode + nJobCount + JOB_Proc::DoStartCycleTest -> DoWorkStartMeasICP_EFA).
	// The engine side (2-pass loop) is Phase 2 - until then the flag is set but the sequence runs standard.
	if (!IsOfflineStatus()) return;

	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetShareMemoryMgrPtr() == NULL ||
		m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return;

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_PROCESS)
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

	//=====================================================================
	// pre-checks (mirror of AgingTest START): OCP mode, recipe EFA flag, EFA rcp file with items.
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

	int nMaxCount = m_spreadAgingList->GetMaxRows();	// HMI grid: MaxRows = item count
	if (nMaxCount < 1)
	{
		AfxMessageBox(_T("No list information.\nLoad or save the list first."), MB_ICONWARNING);
		return;
	}

	// cycle count from combo
	CString strTemp;
	int nCycleCount = 1;
	m_cmbCycle.GetWindowText(strTemp);
	nCycleCount = _ttoi(strTemp);
	if (nCycleCount < 1) nCycleCount = 1;

	// collect grid -> m_vecAginList (cols 1..6 = Aging/PTN/Sec/WAD/VISION/ANGLE)
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

	// shared state: EFA and AgingTest modes are mutually exclusive (OCP resets both at every start)
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_AutoAgingMode(false);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_NewAgingMode(true);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 0;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nJobCount = nCycleCount;
	}

	// HMI cycle-inspection start (same path as DlgSubOperate cycle button / AgingTest START)
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
	// forwarded by CMySpreadDll (wParam = component ID, lParam = LPSS_CELLCOORD)
	if ((UINT)wParam != IDC_FPSPREAD_AGING_CELL_STATUS) return 0;

	LPSS_CELLCOORD pCoord = (LPSS_CELLCOORD)lParam;
	if (pCoord == NULL) return 0;

	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return 0;

	DblClickFpspreadCellStatus((long)pCoord->Col, (long)pCoord->Row);
	return 0;
}

//=========================================================================
// list grid
//=========================================================================
void CDlgSubManual_AgingNew::UpdateListNum()
{
	// row numbers into header col 0. Rows 1/2 are labelled T0/T1 to match the OCP_EFA meaning.
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

	return TRUE;
}

BOOL CDlgSubManual_AgingNew::LoadAgingInfo()
{
	// {EFA_RCP}\{RecipeID}.rcp -> list grid + m_vecAginList, then cell map.
	// HMI grid coords: header = SS_HEADER row, data rows 1..MaxRows, cols 1..6
	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return FALSE;

	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, sizeof(cValue));

	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strEfaRcp);
	int nMaxCount = _ttoi(cValue);

	AgingItemClearAll();
	if (nMaxCount <= 0) return FALSE;

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

	UpdateAgingCell();	// build 3-sheet map first
	MeasCellLoad();		// then fill from [MASECELL]

	return TRUE;
}

void CDlgSubManual_AgingNew::SaveAgingInfo()
{
	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return;

	dfcCreateFolder(m_strEfaRcpPath);

	CString strValue, strItem;
	int nMaxCount = m_spreadAgingList->GetMaxRows();
	if (nMaxCount < 1) {
		AfxMessageBox(_T("Add EFA items before saving."));
		return;
	}

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

	// OCP_EFA AgingItemClearAll(TRUE): "EFA list is fixed to 2 rows"
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
		if (i == 1 || i == 4 || i == 5 || i == 6) {	// Aging / WAD / VISION / ANGLE check boxes
			m_spreadAgingList->SetTypeCheckBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}

		if (i == 2) {	// pattern 0..25 (OCP_EFA 20260403: 10 -> 25)
			sComboList.Empty();
			for (int p = 0; p <= 25; p++) sComboList += dfcStr(_T("%d\t"), p);
			m_spreadAgingList->SetTypeComboBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, sComboList);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}

		if (i == 3) {	// hold seconds
			m_spreadAgingList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
			m_spreadAgingList->SetCellType(i, nMaxRow + 1, &cellType);
		}
	}

	UpdateListNum();
}

//=========================================================================
// cell-use map (3 sheets WAD/VISION/ANGLE, [MASECELL] STICK_n = 3bit mask CSV) - same as AgingTest
//=========================================================================
void CDlgSubManual_AgingNew::UpdateAgingCell()
{
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	int nStick = 0, nCell = 0;
	S_RecipeData* pRD = NULL;
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
		pRD = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();

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

void CDlgSubManual_AgingNew::MeasCellSave()
{
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return;

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return; }

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

		WritePrivateProfileString(_T("MASECELL"), dfcStr(_T("STICK_%d"), i + 1), strCellData, strEfaRcp);
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
}

void CDlgSubManual_AgingNew::MeasCellLoad()
{
	// [MASECELL] -> grid. Call AFTER UpdateAgingCell built the sheets.
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return;

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return;

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return; }

	TCHAR cCellData[EFA_CELLMASK_BUF];
	CArray<int, int> nCellChkArray;

	for (int i = 0; i < nStick; i++) {
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(_T("MASECELL"), dfcStr(_T("STICK_%d"), i + 1), _T(""), cCellData, EFA_CELLMASK_BUF, strEfaRcp);

		int pos = 0;
		nCellChkArray.RemoveAll();
		CString strCellData = cCellData;
		CString resToken = strCellData.Tokenize(_T(","), pos);
		while (resToken != _T("")) {
			nCellChkArray.Add(_ttoi(resToken.Trim()));
			resToken = strCellData.Tokenize(_T(","), pos);
		}

		if (nCellChkArray.GetSize() != nCell) continue;	// row missing/mismatched -> keep defaults

		for (int j = 0; j < nCell; j++) {
			int nMask = nCellChkArray.GetAt(j);
			for (int k = 0; k < 3; k++) {	// WAD, VISION, ANGLE
				m_spreadAgingMeas->SetSheet(k + 1);
				m_spreadAgingMeas->SetData(j + 1, i + 1, ((nMask >> k) & 0x01) ? _T("1") : _T("0"));
			}
		}
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
}

void CDlgSubManual_AgingNew::DblClickFpspreadCellStatus(long Col, long Row)
{
	// header dblclick = bulk toggle (corner: whole sheet, column header: that column)
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
	// OCX-era event; CMySpreadDll switches sheets itself. Kept for the EVENTSINK map.
}
