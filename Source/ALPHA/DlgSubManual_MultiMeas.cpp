// LYS 2026-07-23 WAD Multi Point measurement setting dialog (ported from OCP CDlgNaviManualMultiMeas)
// Phase 3: CMySpreadDll grids + CWadMultiMeasMgr binding.
// Grid init is done inline here (not in MySpreadDll.cpp) to avoid rebuilding
// every CoreFunction consumer project for a single-view layout.

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_MultiMeas.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
#include "SystemParamShare.h"				// LYS 2026-07-23 Recipe.ini path
#include "Manager\ShareMemoryMgr.h"
#include "Manager\SystemConfigMgr.h"		// LYS 2026-07-23 Phase 2
#include "Manager\WadMultiMeasMgr.h"		// LYS 2026-07-23 Phase 2
#include "Recipe\RecipeEditMgr.h"			// LYS 2026-07-23 Phase 3 (cell map size)

#include "..\CoreFunction\common\MySpreadDll.h"

// Fallback WAD Multi recipe folder (normally read from [OPT_PARA] WAD_MULTI_RCP_PATH)
#define DEF_WAD_MULTI_RCP_PATH	_T("D:\\HBS\\PARAM_OCP\\PARAM\\MultiMeasRcp\\")

// point list grid columns (11 data cols: No + 10 fields, OCP layout)
enum
{
	eMM_COL_NO = 1,			// LYS 2026-07-24 No as real first column (OCP format)
	eMM_COL_CLASS,
	eMM_COL_SUBCLASS,
	eMM_COL_X,
	eMM_COL_Y,
	eMM_COL_ENABLE,
	eMM_COL_RX,
	eMM_COL_GX,
	eMM_COL_BY,
	eMM_COL_WX,
	eMM_COL_WY,
	eMM_COL_LAST = eMM_COL_WY,
};

IMPLEMENT_DYNAMIC(CDlgSubManual_MultiMeas, CDialogEx)

CDlgSubManual_MultiMeas::CDlgSubManual_MultiMeas(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SUB_MANUAL_MULTI_MEAS, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_spreadPointList(nullptr)
	, m_spreadInspMode(nullptr)
	, m_spreadCellSize(nullptr)
	, m_spreadCellMap(nullptr)
	, m_hFont(nullptr)
{
	m_strMultiRcpPath = DEF_WAD_MULTI_RCP_PATH;
}

CDlgSubManual_MultiMeas::~CDlgSubManual_MultiMeas()
{
	if (m_spreadPointList)	{ delete m_spreadPointList;	m_spreadPointList = nullptr; }
	if (m_spreadInspMode)	{ delete m_spreadInspMode;	m_spreadInspMode = nullptr; }
	if (m_spreadCellSize)	{ delete m_spreadCellSize;	m_spreadCellSize = nullptr; }
	if (m_spreadCellMap)	{ delete m_spreadCellMap;	m_spreadCellMap = nullptr; }
}

void CDlgSubManual_MultiMeas::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgSubManual_MultiMeas, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_MM_OPEN,      &CDlgSubManual_MultiMeas::OnBnClickedButtonMmOpen)
	ON_BN_CLICKED(IDC_BUTTON_MM_SAVE,      &CDlgSubManual_MultiMeas::OnBnClickedButtonMmSave)
	ON_BN_CLICKED(IDC_BUTTON_MM_LIST_INIT, &CDlgSubManual_MultiMeas::OnBnClickedButtonMmListInit)
	ON_BN_CLICKED(IDC_BUTTON_MM_ADD_ROW,   &CDlgSubManual_MultiMeas::OnBnClickedButtonMmAddRow)
	ON_BN_CLICKED(IDC_BUTTON_MM_DEL_ROW,   &CDlgSubManual_MultiMeas::OnBnClickedButtonMmDelRow)
	ON_MESSAGE(SSM_DBLCLK, &CDlgSubManual_MultiMeas::OnSpreadDblClick)
END_MESSAGE_MAP()

CWadMultiMeasMgr* CDlgSubManual_MultiMeas::GetMgr()
{
	if (m_pAuroraMgr == nullptr)
		return nullptr;

	return m_pAuroraMgr->GetWadMultiMeasMgrPtr();
}

BOOL CDlgSubManual_MultiMeas::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// LYS 2026-07-23 Phase 2: recipe folder from SystemConfigMgr ([OPT_PARA] WAD_MULTI_RCP_PATH)
	if (m_pAuroraMgr != nullptr && m_pAuroraMgr->GetSystemConfigMgrPtr() != nullptr)
	{
		CString strPath = m_pAuroraMgr->GetSystemConfigMgrPtr()->GetWadMultiRcpPath();
		if (!strPath.IsEmpty())
			m_strMultiRcpPath = strPath;
	}

	GetDlgItem(IDC_EDIT_MM_RCP_PATH)->SetWindowText(m_strMultiRcpPath);

	//---------------------------------------------------------------
	// Farpoint spread create (over CTEXT placeholders)
	//---------------------------------------------------------------
	m_spreadPointList = new CMySpreadDll(IDC_FPS_MM_LIST,     FPS_RESOURCE_ID + 24, this, FALSE);
	m_spreadInspMode  = new CMySpreadDll(IDC_FPS_MM_MODE,     FPS_RESOURCE_ID + 25, this, FALSE);
	m_spreadCellSize  = new CMySpreadDll(IDC_FPS_MM_CELLSIZE, FPS_RESOURCE_ID + 26, this, FALSE);
	m_spreadCellMap   = new CMySpreadDll(IDC_FPS_MM_CELLMAP,  FPS_RESOURCE_ID + 27, this, TRUE);	// events for bulk toggle

	if (m_spreadPointList->GetSafeHwnd() != NULL)
		m_spreadPointList->MakeFont(&m_hFont, _T("Tahoma"), 12);

	InitSpreadPointList();
	InitSpreadInspMode();
	InitSpreadCellSize();
	InitSpreadCellMap();

	ResetPointGrid();

	return TRUE;
}

//---------------------------------------------------------------
// grid setup
//---------------------------------------------------------------
void CDlgSubManual_MultiMeas::InitSpreadPointList()
{
	if (m_spreadPointList == nullptr || m_spreadPointList->GetSafeHwnd() == NULL)
		return;

	m_spreadPointList->InitDefault();
	m_spreadPointList->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hFont, FALSE);

	// LYS 2026-07-24 custom header: header row + auto-text off + text into row 0
	m_spreadPointList->SetColHeaderRows(1);
	m_spreadPointList->SetColHeaderDisplay(SS_HEADERDISPLAY_BLANK);

	m_spreadPointList->SetMaxCols(eMM_COL_LAST);
	m_spreadPointList->SetMaxRows(1);

	m_spreadPointList->SetColWidth(SS_HEADER, 3);
	m_spreadPointList->SetColWidth(eMM_COL_NO, 4);
	m_spreadPointList->SetColWidth(eMM_COL_CLASS, 7);
	m_spreadPointList->SetColWidth(eMM_COL_SUBCLASS, 8);
	m_spreadPointList->SetColWidth(eMM_COL_X, 8);
	m_spreadPointList->SetColWidth(eMM_COL_Y, 8);
	m_spreadPointList->SetColWidth(eMM_COL_ENABLE, 5);
	m_spreadPointList->SetColWidth(eMM_COL_RX, 7);
	m_spreadPointList->SetColWidth(eMM_COL_GX, 7);
	m_spreadPointList->SetColWidth(eMM_COL_BY, 7);
	m_spreadPointList->SetColWidth(eMM_COL_WX, 7);
	m_spreadPointList->SetColWidth(eMM_COL_WY, 7);

	m_spreadPointList->SetRowHeight(SS_HEADER, 20);
	m_spreadPointList->SetRowHeight(SS_ALLROWS, 14);

	SetPointListHeaderText();
}

// LYS 2026-07-24 header labels via row 0 (OCP multi meas list format).
// Re-applied after grid rebuilds because ClearRange/SetMaxRows can reset header cells.
void CDlgSubManual_MultiMeas::SetPointListHeaderText()
{
	if (m_spreadPointList == nullptr || m_spreadPointList->GetSafeHwnd() == NULL)
		return;

	m_spreadPointList->SetData(eMM_COL_NO,       0, _T("No"));
	m_spreadPointList->SetData(eMM_COL_CLASS,    0, _T("CLASS"));
	m_spreadPointList->SetData(eMM_COL_SUBCLASS, 0, _T("SUB_CLASS"));
	m_spreadPointList->SetData(eMM_COL_X,        0, _T("X"));
	m_spreadPointList->SetData(eMM_COL_Y,        0, _T("Y"));
	m_spreadPointList->SetData(eMM_COL_ENABLE,   0, _T("ENABLE"));
	m_spreadPointList->SetData(eMM_COL_RX,       0, _T("RX"));
	m_spreadPointList->SetData(eMM_COL_GX,       0, _T("GX"));
	m_spreadPointList->SetData(eMM_COL_BY,       0, _T("BY"));
	m_spreadPointList->SetData(eMM_COL_WX,       0, _T("WX"));
	m_spreadPointList->SetData(eMM_COL_WY,       0, _T("WY"));
}

void CDlgSubManual_MultiMeas::InitSpreadInspMode()
{
	if (m_spreadInspMode == nullptr || m_spreadInspMode->GetSafeHwnd() == NULL)
		return;

	m_spreadInspMode->InitDefault();
	m_spreadInspMode->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hFont, FALSE);

	m_spreadInspMode->SetColHeaderRows(1);		// LYS 2026-07-24 proven header pattern
	m_spreadInspMode->SetColHeaderDisplay(SS_HEADERDISPLAY_BLANK);	// LYS 2026-07-24 auto-text off

	m_spreadInspMode->SetMaxCols(1);
	m_spreadInspMode->SetMaxRows(1);

	// LYS 2026-07-23 sized for Tahoma 12 font
	m_spreadInspMode->SetColWidth(SS_HEADER, 26);
	m_spreadInspMode->SetColWidth(1, 10);
	m_spreadInspMode->SetRowHeight(SS_HEADER, 14);
	m_spreadInspMode->SetRowHeight(SS_ALLROWS, 18);

	m_spreadInspMode->SetData(1, 0, _T("Value"));		// LYS 2026-07-24 col header via row 0
	m_spreadInspMode->SetData(SS_HEADER, 1, _T("Single(0) / Multi(1)"));

	SS_CELLTYPE cellType;
	m_spreadInspMode->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 2, NULL, SS_CASE_NOCASE);
	m_spreadInspMode->SetCellType(1, 1, &cellType);

	m_spreadInspMode->SetData(1, 1, _T("0"));
}

void CDlgSubManual_MultiMeas::InitSpreadCellSize()
{
	if (m_spreadCellSize == nullptr || m_spreadCellSize->GetSafeHwnd() == NULL)
		return;

	m_spreadCellSize->InitDefault();
	m_spreadCellSize->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hFont, FALSE);

	m_spreadCellSize->SetColHeaderRows(1);		// LYS 2026-07-24 proven header pattern
	m_spreadCellSize->SetColHeaderDisplay(SS_HEADERDISPLAY_BLANK);	// LYS 2026-07-24 auto-text off

	// LYS 2026-07-23 horizontal layout: 1 row x (CELL X | CELL Y) columns
	m_spreadCellSize->SetMaxCols(2);
	m_spreadCellSize->SetMaxRows(1);

	m_spreadCellSize->SetColWidth(SS_HEADER, 4);
	m_spreadCellSize->SetColWidth(1, 14);
	m_spreadCellSize->SetColWidth(2, 14);
	m_spreadCellSize->SetRowHeight(SS_HEADER, 14);
	m_spreadCellSize->SetRowHeight(SS_ALLROWS, 16);

	// LYS 2026-07-24 col header via row 0
	m_spreadCellSize->SetData(1, 0, _T("CELL X (mm)"));
	m_spreadCellSize->SetData(2, 0, _T("CELL Y (mm)"));

	SS_CELLTYPE cellType;
	m_spreadCellSize->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 12, NULL, SS_CASE_NOCASE);
	m_spreadCellSize->SetCellType(1, 1, &cellType);
	m_spreadCellSize->SetCellType(2, 1, &cellType);

	m_spreadCellSize->SetData(1, 1, _T("0"));
	m_spreadCellSize->SetData(2, 1, _T("0"));
}

void CDlgSubManual_MultiMeas::InitSpreadCellMap()
{
	if (m_spreadCellMap == nullptr || m_spreadCellMap->GetSafeHwnd() == NULL)
		return;

	m_spreadCellMap->InitDefault();
	m_spreadCellMap->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hFont, FALSE);

	m_spreadCellMap->SetColHeaderRows(1);
	// LYS 2026-07-24 500+ cells: use FarPoint auto letters (A..Z,AA..) for column headers
	m_spreadCellMap->SetColHeaderDisplay(SS_HEADERDISPLAY_LETTERS);

	// dimension is set by RebuildCellMap() after recipe/multi file load
	m_spreadCellMap->SetMaxCols(0);
	m_spreadCellMap->SetMaxRows(0);
	m_spreadCellMap->SetColWidth(SS_HEADER, 6);
	m_spreadCellMap->SetRowHeight(SS_HEADER, 14);
}

void CDlgSubManual_MultiMeas::SetPointRowType(int nRow)
{
	SS_CELLTYPE ctEdit, ctCheck, ctNo;

	m_spreadPointList->SetTypeEdit(&ctEdit, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
	m_spreadPointList->SetTypeCheckBox(&ctCheck, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	m_spreadPointList->SetTypeStaticText(&ctNo, SS_TEXT_VCENTER | SS_TEXT_CENTER);	// LYS 2026-07-24 No column

	for (int nCol = eMM_COL_NO; nCol <= eMM_COL_LAST; nCol++)
	{
		if (nCol == eMM_COL_NO)
			m_spreadPointList->SetCellType(nCol, nRow, &ctNo);
		else if (nCol == eMM_COL_ENABLE)
			m_spreadPointList->SetCellType(nCol, nRow, &ctCheck);
		else
			m_spreadPointList->SetCellType(nCol, nRow, &ctEdit);
	}
}

//---------------------------------------------------------------
// grid <-> data
//---------------------------------------------------------------
void CDlgSubManual_MultiMeas::GridToPoints(std::vector<S_WadMultiPoint>& outPoints)
{
	outPoints.clear();

	if (m_spreadPointList == nullptr || m_spreadPointList->GetSafeHwnd() == NULL)
		return;

	int nMaxRow = m_spreadPointList->GetMaxRows();

	for (int nRow = 1; nRow <= nMaxRow; nRow++)
	{
		S_WadMultiPoint pt;

		pt.strMainClass	= m_spreadPointList->GetDataString(eMM_COL_CLASS, nRow);
		pt.strSubClass	= m_spreadPointList->GetDataString(eMM_COL_SUBCLASS, nRow);
		pt.dPosX		= m_spreadPointList->GetDataDbl(eMM_COL_X, nRow);
		pt.dPosY		= m_spreadPointList->GetDataDbl(eMM_COL_Y, nRow);
		pt.bEnable		= (m_spreadPointList->GetDataInt(eMM_COL_ENABLE, nRow) != 0);
		pt.fOffsetRX	= m_spreadPointList->GetDataDbl(eMM_COL_RX, nRow);
		pt.fOffsetGX	= m_spreadPointList->GetDataDbl(eMM_COL_GX, nRow);
		pt.fOffsetBY	= m_spreadPointList->GetDataDbl(eMM_COL_BY, nRow);
		pt.fOffsetWX	= m_spreadPointList->GetDataDbl(eMM_COL_WX, nRow);
		pt.fOffsetWY	= m_spreadPointList->GetDataDbl(eMM_COL_WY, nRow);

		outPoints.push_back(pt);
	}
}

void CDlgSubManual_MultiMeas::PointsToGrid(const std::vector<S_WadMultiPoint>& vecPoints)
{
	if (m_spreadPointList == nullptr || m_spreadPointList->GetSafeHwnd() == NULL)
		return;

	int nCount = (int)vecPoints.size();

	m_spreadPointList->ClearRange(1, 1, -1, -1);
	m_spreadPointList->SetMaxRows(nCount);

	CString str;

	for (int i = 0; i < nCount; i++)
	{
		const S_WadMultiPoint& pt = vecPoints[i];
		int nRow = i + 1;

		SetPointRowType(nRow);

		str.Format(_T("%d"), nRow);				// LYS 2026-07-24 No column value
		m_spreadPointList->SetData(eMM_COL_NO, nRow, str);

		m_spreadPointList->SetData(eMM_COL_CLASS, nRow, pt.strMainClass);
		m_spreadPointList->SetData(eMM_COL_SUBCLASS, nRow, pt.strSubClass);

		str.Format(_T("%.4f"), pt.dPosX);		m_spreadPointList->SetData(eMM_COL_X, nRow, str);
		str.Format(_T("%.4f"), pt.dPosY);		m_spreadPointList->SetData(eMM_COL_Y, nRow, str);

		m_spreadPointList->SetData(eMM_COL_ENABLE, nRow, pt.bEnable ? _T("1") : _T("0"));

		str.Format(_T("%.4f"), pt.fOffsetRX);	m_spreadPointList->SetData(eMM_COL_RX, nRow, str);
		str.Format(_T("%.4f"), pt.fOffsetGX);	m_spreadPointList->SetData(eMM_COL_GX, nRow, str);
		str.Format(_T("%.4f"), pt.fOffsetBY);	m_spreadPointList->SetData(eMM_COL_BY, nRow, str);
		str.Format(_T("%.4f"), pt.fOffsetWX);	m_spreadPointList->SetData(eMM_COL_WX, nRow, str);
		str.Format(_T("%.4f"), pt.fOffsetWY);	m_spreadPointList->SetData(eMM_COL_WY, nRow, str);
	}

	SetPointListHeaderText();	// LYS 2026-07-24 restore headers after ClearRange/SetMaxRows
}

void CDlgSubManual_MultiMeas::ResetPointGrid()
{
	// one default row: point 0 = cell center (base measurement position)
	std::vector<S_WadMultiPoint> vecPoints;

	S_WadMultiPoint pt;
	pt.strMainClass = _T("C");
	pt.strSubClass = _T("C4");
	pt.bEnable = true;

	vecPoints.push_back(pt);

	PointsToGrid(vecPoints);
}

void CDlgSubManual_MultiMeas::LoadFromMgr()
{
	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr)
		return;

	// point list
	std::vector<S_WadMultiPoint> vecPoints;
	pMgr->GetPoints(vecPoints);

	if (vecPoints.empty())
		ResetPointGrid();
	else
		PointsToGrid(vecPoints);

	// insp mode
	if (m_spreadInspMode && m_spreadInspMode->GetSafeHwnd())
		m_spreadInspMode->SetData(1, 1, pMgr->IsMultiMode() ? _T("1") : _T("0"));

	// cell size (horizontal: col1 = X, col2 = Y)
	if (m_spreadCellSize && m_spreadCellSize->GetSafeHwnd())
	{
		CString str;
		str.Format(_T("%.4f"), pMgr->GetCellX());	m_spreadCellSize->SetData(1, 1, str);
		str.Format(_T("%.4f"), pMgr->GetCellY());	m_spreadCellSize->SetData(2, 1, str);
	}

	RebuildCellMap();
}

void CDlgSubManual_MultiMeas::SaveToMgr()
{
	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr)
		return;

	// point list
	std::vector<S_WadMultiPoint> vecPoints;
	GridToPoints(vecPoints);
	pMgr->SetPoints(vecPoints);

	// insp mode
	if (m_spreadInspMode && m_spreadInspMode->GetSafeHwnd())
		pMgr->SetInspMode(m_spreadInspMode->GetDataInt(1, 1) != 0);

	// cell size (horizontal: col1 = X, col2 = Y)
	if (m_spreadCellSize && m_spreadCellSize->GetSafeHwnd())
		pMgr->SetCellSize(m_spreadCellSize->GetDataDbl(1, 1), m_spreadCellSize->GetDataDbl(2, 1));

	// cell map
	if (m_spreadCellMap && m_spreadCellMap->GetSafeHwnd())
	{
		std::vector<std::vector<int>> vecCellUse;

		int nMaxRow = m_spreadCellMap->GetMaxRows();
		int nMaxCol = m_spreadCellMap->GetMaxCols();

		for (int nRow = 1; nRow <= nMaxRow; nRow++)
		{
			std::vector<int> vecRow;
			for (int nCol = 1; nCol <= nMaxCol; nCol++)
				vecRow.push_back(m_spreadCellMap->GetDataInt(nCol, nRow) != 0 ? 1 : 0);

			vecCellUse.push_back(vecRow);
		}

		pMgr->SetCellUseMap(vecCellUse);
	}
}

void CDlgSubManual_MultiMeas::RebuildCellMap()
{
	if (m_spreadCellMap == nullptr || m_spreadCellMap->GetSafeHwnd() == NULL)
		return;

	CWadMultiMeasMgr* pMgr = GetMgr();

	//-----------------------------------------------------------
	// dimension: current loaded recipe first, multi file second
	//-----------------------------------------------------------
	int nStick = 0, nCell = 0;

	S_RecipeData* pRD = nullptr;
	if (m_pAuroraMgr != nullptr && m_pAuroraMgr->GetRecipeEditMgrPtr() != nullptr)
		pRD = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();

	if (pRD != nullptr && pRD->m_vecStick.size() > 0)
	{
		nStick = (int)pRD->m_vecStick.size();
		nCell  = (int)pRD->m_vecStick[0].m_vecCell.size();
	}
	else if (pMgr != nullptr && pMgr->GetCellUseStickCount() > 0)
	{
		nStick = pMgr->GetCellUseStickCount();
		nCell  = pMgr->GetCellUseCellCount(0);
	}

	m_spreadCellMap->SetMaxRows(nStick);
	m_spreadCellMap->SetMaxCols(nCell);

	if (nStick <= 0 || nCell <= 0)
		return;

	m_spreadCellMap->SetColWidth(SS_ALLCOLS, 4);
	m_spreadCellMap->SetRowHeight(SS_ALLROWS, 13);

	CString str;
	SS_CELLTYPE ctCheck;
	m_spreadCellMap->SetTypeCheckBox(&ctCheck, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

	// LYS 2026-07-24 column headers: FarPoint auto letters (set in InitSpreadCellMap)

	for (int nRow = 1; nRow <= nStick; nRow++)
	{
		// LYS 2026-07-23 row header: S1,S2.. format (reverted per user request)
		str.Format(_T("S%d"), nRow);
		m_spreadCellMap->SetData(SS_HEADER, nRow, str);

		for (int nCol = 1; nCol <= nCell; nCol++)
		{
			m_spreadCellMap->SetCellType(nCol, nRow, &ctCheck);

			bool bUse = (pMgr != nullptr) ? pMgr->IsMultiCell(nRow - 1, nCol - 1) : false;
			m_spreadCellMap->SetData(nCol, nRow, bUse ? _T("1") : _T("0"));
		}
	}
}

//---------------------------------------------------------------
// status / interlock
//---------------------------------------------------------------
void CDlgSubManual_MultiMeas::UpdateStatus(BOOL bEnable)
{
	GetDlgItem(IDC_BUTTON_MM_OPEN)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_MM_SAVE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_MM_LIST_INIT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_MM_ADD_ROW)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_MM_DEL_ROW)->EnableWindow(bEnable);
}

BOOL CDlgSubManual_MultiMeas::IsOfflineStatus()
{
	// Same interlock concept as OCP isOffLineStatus(): allow edit only in offline mode
	if (m_pAuroraMgr != nullptr)
	{
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE)
		{
			AfxMessageBox(_T("Not offline mode.\nChange to offline mode and try again."), MB_ICONWARNING);
			return FALSE;
		}
	}

	return TRUE;
}

//---------------------------------------------------------------
// buttons
//---------------------------------------------------------------
void CDlgSubManual_MultiMeas::OnBnClickedButtonMmOpen()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_MultiMeas::OnBnClickedButtonMmOpen"));

	if (IsOfflineStatus() != TRUE)
		return;

	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr)
		return;

	// LYS 2026-07-23 file dialog open (B option reverted per user request; A create-new kept)
	CString strFileName;
	CString strFilter = _T("Recipe Files (*.rcp)|*.rcp|");

	// LYS 2026-07-23 initial dir = main recipe folder (Config\Recipe.ini [PATH] RECIPE_FOLDER)
	CString strInitialDir = m_strMultiRcpPath;
	{
		CString strRecipeIni = CSystemParamShare::GetInstance()->GetConfigFolderPath() + _T("Recipe.ini");

		TCHAR szFolder[MAX_PATH];
		ZeroMemory(szFolder, sizeof(szFolder));
		::GetPrivateProfileString(_T("PATH"), _T("RECIPE_FOLDER"), _T(""), szFolder, MAX_PATH, strRecipeIni);

		if (szFolder[0] != 0)
			strInitialDir = szFolder;
	}

	CFileDialog dlg(TRUE, _T("rcp"), NULL, OFN_FILEMUSTEXIST, strFilter);
	dlg.m_ofn.lpstrInitialDir = strInitialDir;

	if (dlg.DoModal() != IDOK)
		return;

	strFileName = dlg.GetFileName();
	if (strFileName.IsEmpty())
	{
		AfxMessageBox(_T("File is not a Recipe file or is locked and cannot be imported."));
		return;
	}

	int nExt = strFileName.Find(_T(".rcp"));
	if (nExt > 0)
		strFileName = strFileName.Left(nExt);

	// LYS 2026-07-24 also load the main recipe here (same mechanism as
	// Manual Meas recipe open: RecipeEditMgr::OpenRecipe) so the cell map
	// reflects the selected recipe without opening it elsewhere first.
	if (m_pAuroraMgr == nullptr || m_pAuroraMgr->GetRecipeEditMgrPtr() == nullptr)
		return;

	if (AfxMessageBox(_T("Load This Recipe : ") + strFileName, MB_OKCANCEL) != IDOK)
		return;

	if (m_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(strFileName) <= 0)
	{
		AfxMessageBox(_T("Fail : Load Recipe"));
		return;
	}

	if (pMgr->LoadMeasData(strFileName) == true)
	{
		LoadFromMgr();
		GetDlgItem(IDC_EDIT_MM_RECIPE)->SetWindowText(strFileName);

		AfxMessageBox(_T("WAD Multi Recipe imported."));
	}
	else
	{
		// LYS 2026-07-23 A option: create-new flow, file is written on [Save]
		CString strMsg;
		strMsg.Format(_T("WAD Multi Recipe not found for [%s].\nCreate a new one?"), (LPCTSTR)strFileName);

		if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONQUESTION) != IDYES)
			return;

		pMgr->Clear();
		LoadFromMgr();		// empty -> default 1 row + cell map from current recipe
		GetDlgItem(IDC_EDIT_MM_RECIPE)->SetWindowText(strFileName);
	}
}

void CDlgSubManual_MultiMeas::OnBnClickedButtonMmSave()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_MultiMeas::OnBnClickedButtonMmSave"));

	if (IsOfflineStatus() != TRUE)
		return;

	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr)
		return;

	CString strRecipe;
	GetDlgItem(IDC_EDIT_MM_RECIPE)->GetWindowText(strRecipe);

	if (strRecipe.IsEmpty() || strRecipe == _T("UNKNOWN"))
	{
		AfxMessageBox(_T("No recipe name.\nOpen a recipe first."), MB_ICONWARNING);
		return;
	}

	if (AfxMessageBox(_T("Save current settings?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	SaveToMgr();

	// cell range interlock (+-CellSize/2), OCP isCellRangeCheck
	int nNgIndex = -1;
	if (pMgr->IsCellRangeValid(nNgIndex) != true)
	{
		CString strMsg;
		strMsg.Format(_T("Point %d is out of cell range (+-Cell/2).\nSave canceled."), nNgIndex + 1);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return;
	}

	if (pMgr->SaveMeasData(strRecipe) != true)
	{
		AfxMessageBox(_T("Save WAD Multi Recipe failed."), MB_ICONWARNING);
		return;
	}

	// reload for verification (OCP Save->Load pattern)
	pMgr->LoadMeasData(strRecipe);
	LoadFromMgr();

	AfxMessageBox(_T("WAD Multi Recipe saved."));
}

void CDlgSubManual_MultiMeas::OnBnClickedButtonMmListInit()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_MultiMeas::OnBnClickedButtonMmListInit"));

	if (AfxMessageBox(_T("Reset point list?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	ResetPointGrid();
}

void CDlgSubManual_MultiMeas::OnBnClickedButtonMmAddRow()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_MultiMeas::OnBnClickedButtonMmAddRow"));

	std::vector<S_WadMultiPoint> vecPoints;
	GridToPoints(vecPoints);

	S_WadMultiPoint pt;
	pt.strMainClass = _T("C1");
	pt.bEnable = false;

	vecPoints.push_back(pt);

	PointsToGrid(vecPoints);
}

void CDlgSubManual_MultiMeas::OnBnClickedButtonMmDelRow()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubManual_MultiMeas::OnBnClickedButtonMmDelRow"));

	if (m_spreadPointList == nullptr || m_spreadPointList->GetSafeHwnd() == NULL)
		return;

	std::vector<S_WadMultiPoint> vecPoints;
	GridToPoints(vecPoints);

	if (vecPoints.size() <= 1)
		return;		// keep at least one row (OCP behavior)

	// delete active row; fall back to last row
	SS_COORD nCol = 0, nRow = 0;
	int nDelIndex = (int)vecPoints.size() - 1;

	if (m_spreadPointList->GetActiveCell(&nCol, &nRow))
	{
		if (nRow >= 1 && nRow <= (SS_COORD)vecPoints.size())
			nDelIndex = (int)nRow - 1;
	}

	if (nDelIndex == 0)
		return;		// row 1 (base point) is not deletable (OCP behavior)

	vecPoints.erase(vecPoints.begin() + nDelIndex);

	PointsToGrid(vecPoints);
}

//---------------------------------------------------------------
// spread events
//---------------------------------------------------------------
LRESULT CDlgSubManual_MultiMeas::OnSpreadDblClick(WPARAM wParam, LPARAM lParam)
{
	// forwarded by CMySpreadDll: wParam = component ID, lParam = LPSS_CELLCOORD
	if ((UINT)wParam != IDC_FPS_MM_CELLMAP)
		return 0;

	LPSS_CELLCOORD pCoord = (LPSS_CELLCOORD)lParam;
	if (pCoord == NULL)
		return 0;

	if (m_spreadCellMap == nullptr || m_spreadCellMap->GetSafeHwnd() == NULL)
		return 0;

	long nCol = (long)pCoord->Col;
	long nRow = (long)pCoord->Row;

	int nMaxCol = m_spreadCellMap->GetMaxCols();
	int nMaxRow = m_spreadCellMap->GetMaxRows();

	if (nMaxCol <= 0 || nMaxRow <= 0)
		return 0;

	CString strValue;

	if (nCol == 0 && nRow == 0)
	{
		// corner: copy (1,1) value to all cells (OCP DblClickFpspreadCellStatus)
		strValue = m_spreadCellMap->GetDataString(1, 1);

		for (int c = 1; c <= nMaxCol; c++)
			for (int r = 1; r <= nMaxRow; r++)
				m_spreadCellMap->SetData(c, r, strValue);
	}
	else if (nRow == 0)
	{
		// column header: copy (col,1) value to whole column
		strValue = m_spreadCellMap->GetDataString(nCol, 1);

		for (int r = 1; r <= nMaxRow; r++)
			m_spreadCellMap->SetData(nCol, r, strValue);
	}
	else if (nCol == 0)
	{
		// row header: copy (1,row) value to whole row
		strValue = m_spreadCellMap->GetDataString(1, nRow);

		for (int c = 1; c <= nMaxCol; c++)
			m_spreadCellMap->SetData(c, nRow, strValue);
	}

	return 0;
}
