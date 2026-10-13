// LYS 2026-07-23 WAD 다점 측정 설정 다이얼로그(OCP CDlgNaviManualMultiMeas 이식, Phase 3).
// 그리드 초기화는 이 화면 전용이라 MySpreadDll.cpp가 아닌 여기서 처리한다
// - CoreFunction을 쓰는 전 프로젝트 리빌드를 피하기 위함.

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
#include "Manager\SequenceManager.h"		// LYS 2026-09-30 Hybrid 3d busy guard

#include "..\CoreFunction\common\MySpreadDll.h"

// Fallback WAD Multi recipe folder (normally read from [OPT_PARA] WAD_MULTI_RCP_PATH)
#define DEF_WAD_MULTI_RCP_PATH	_T("D:\\HBS\\PARAM_OCP\\PARAM\\MultiMeasRcp\\")

#define MM_TIMER_SEG			1		// LYS 2026-09-30 Hybrid 3d segment view refresh (hybrid view only)

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

	// LYS 2026-09-30 Hybrid 3d
	m_nViewSeg = 0;
	m_nViewSegCount = 1;
	m_nViewStick = 0;
	m_nViewCell = 0;
	m_nMaskMismatch = 0;
	m_nMaskExtra = 0;
	m_bViewDataFound = false;
	m_bViewMaskFound = false;
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
	DDX_Control(pDX, IDC_COMBO_MM_SEG, m_cmbSeg);	// LYS 2026-09-30 Hybrid 3d
}

BEGIN_MESSAGE_MAP(CDlgSubManual_MultiMeas, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_MM_OPEN,      &CDlgSubManual_MultiMeas::OnBnClickedButtonMmOpen)
	ON_BN_CLICKED(IDC_BUTTON_MM_SAVE,      &CDlgSubManual_MultiMeas::OnBnClickedButtonMmSave)
	ON_BN_CLICKED(IDC_BUTTON_MM_LIST_INIT, &CDlgSubManual_MultiMeas::OnBnClickedButtonMmListInit)
	ON_BN_CLICKED(IDC_BUTTON_MM_ADD_ROW,   &CDlgSubManual_MultiMeas::OnBnClickedButtonMmAddRow)
	ON_BN_CLICKED(IDC_BUTTON_MM_DEL_ROW,   &CDlgSubManual_MultiMeas::OnBnClickedButtonMmDelRow)
	ON_MESSAGE(SSM_DBLCLK, &CDlgSubManual_MultiMeas::OnSpreadDblClick)
	ON_CBN_SELCHANGE(IDC_COMBO_MM_SEG, &CDlgSubManual_MultiMeas::OnSelchangeComboMmSeg)	// LYS 2026-09-30 Hybrid 3d
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
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

// LYS 2026-07-24 헤더 라벨은 row 0에 직접 기입한다(OCP multi meas 리스트 포맷).
// ClearRange/SetMaxRows가 헤더 셀을 지우므로 그리드 재구성 뒤 다시 적용한다.
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

	// LYS 2026-09-30 Hybrid 3d (D3d-4) hybrid view of a segment without its section: CELL 0/0 would refuse even the
	//   base point on save (IsCellRangeValid), so start from the segment's recipe cell size (same X/Y as the recipe map)
	m_bViewDataFound = pMgr->IsDataFound();
	m_bViewMaskFound = pMgr->IsMaskFound();
	if (IsViewSegmented() && m_bViewDataFound == false && m_spreadCellSize && m_spreadCellSize->GetSafeHwnd())
	{
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		S_RecipeMgr* pSegMgr = (pSegFunc != nullptr) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : nullptr;
		if (pSegMgr != nullptr)
		{
			CString str;
			str.Format(_T("%.4f"), fabs(pSegMgr->m_sMap.m_fCellSize.x));	m_spreadCellSize->SetData(1, 1, str);
			str.Format(_T("%.4f"), fabs(pSegMgr->m_sMap.m_fCellSize.y));	m_spreadCellSize->SetData(2, 1, str);
		}
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
	{
		// LYS 2026-09-30 Hybrid 3d hybrid view: the viewed segment's size (read only - SetActiveSegment is never called here)
		C_OutRecipeFunc* pSegFunc = IsViewSegmented() ? m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr() : nullptr;
		S_RecipeMgr* pSegMgr = (pSegFunc != nullptr) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : nullptr;
		pRD = (pSegMgr != nullptr) ? &pSegMgr->m_sData : m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	}

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
	m_nViewStick = nStick;	// LYS 2026-09-30 Hybrid 3d stale check
	m_nViewCell = nCell;
	m_nMaskMismatch = 0;
	m_nMaskExtra = 0;

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

	// LYS 2026-09-30 Hybrid 3d hybrid view: mask rows that do not fit this segment (shown cell by cell, like the runtime)
	if (IsViewSegmented() && pMgr != nullptr && pMgr->IsMaskFound())
	{
		int nRows = pMgr->GetCellUseStickCount();
		for (int i = 0; i < nRows && i < nStick; i++)
			if (pMgr->GetCellUseCellCount(i) != nCell) m_nMaskMismatch++;
		if (nRows > nStick) m_nMaskExtra = nRows - nStick;
		if (m_nMaskMismatch > 0 || m_nMaskExtra > 0)
		{
			CString strLog;
			strLog.Format(_T("[HYBRID] WAD Multi [%s]: %d row(s) with cell count != %d, %d row(s) past stick %d"),
				CWadMultiMeasMgr::GetMaskSection(m_nViewSeg).GetString(), m_nMaskMismatch, nCell, m_nMaskExtra, nStick);
			CLogWrapper::GetInstance()->KMLOG(WARN, strLog);

			CString strWarn;
			strWarn.Format(_T("세그먼트 %d WAD Multi 셀 맵에 셀 수가 %d개가 아닌 행 %d개, 스틱 수(%d)를 넘는 행 %d개가 있습니다.\n셀 단위로 표시되며, 저장하면 화면 값으로 덮어씁니다(넘는 행은 삭제)."),
				m_nViewSeg + 1, nCell, m_nMaskMismatch, nStick, m_nMaskExtra);
			AfxMessageBox(strWarn, MB_ICONWARNING);
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

	// LYS 2026-09-30 Hybrid 3d B16a no Recipe Open / Save / segment switch while a hybrid recipe is being measured
	//   (RecipeChange under the measure thread resets the segment vectors). 1-segment recipes: unchanged.
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

	// LYS 2026-07-24 메인 레시피도 여기서 로드한다(Manual Meas와 동일하게 RecipeEditMgr::OpenRecipe).
	// 다른 곳에서 먼저 열지 않아도 셀 맵이 선택한 레시피를 반영하도록.
	if (m_pAuroraMgr == nullptr || m_pAuroraMgr->GetRecipeEditMgrPtr() == nullptr)
		return;

	if (AfxMessageBox(_T("Load This Recipe : ") + strFileName, MB_OKCANCEL) != IDOK)
		return;

	// LYS 2026-10-13 Hybrid Phase 7 D4b-15 (b) / D7-11 (b): OpenRecipe() also returns JOB_RECIPE_NOT_EXIST (1013) when the .rcp load itself failed
	//   (load serial unchanged, the previous recipe stays loaded) and '<= 0' took that as success. Hybrid runtime recipe only (segment cache > 1
	//   or live DLL count > 1, judged before the open = the recipe that stays loaded when the load fails): refused with the existing message.
	//   1-segment recipe: the '<= 0' test alone, unchanged. Case (b) (.rcp read, only the .alg failed - load serial changed) passes as before.
	CRecipeEditMgr* pMmRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pMmFunc = pMmRcp->GetRecipeFuncPtr();
	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 1/2): busy (a run, a manual worker / recipe command, a recipe load) -> the segment-count cache int only,
	//   idle -> the live DLL count (a stale cache over a 1-segment reload is a 1-segment runtime, (14)-A). 1-segment recipe (cache 1, live 1): false either way
	bool bMmBusy = (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE || CSequenceManager::GetInstance()->IsEngineCompleteAll() == false
		|| CSystemParamShare::GetInstance()->IsCycleState() || pMmFunc == nullptr || pMmFunc->IsRcpLoadBusy() == TRUE);
	bool bMmHybrid = bMmBusy ? (pMmRcp->GetSegCountCache() > 1) : (pMmFunc->GetSegmentLoopCount() > 1);
	int nMmSeq = (bMmHybrid == true && pMmFunc != nullptr) ? pMmFunc->GetRcpLoadSeq() : 0;
	int nMmRtn = pMmRcp->OpenRecipe(strFileName);
	if (bMmHybrid == true && nMmRtn > 0 && pMmFunc != nullptr && pMmFunc->GetRcpLoadSeq() == nMmSeq)
	{
		CString strMmLog;
		strMmLog.Format(_T("[HYBRID] WAD Multi recipe open refused - %s returned %d but the .rcp was not loaded (load serial %d unchanged, previous recipe kept)"), (LPCTSTR)strFileName, nMmRtn, nMmSeq);
		CLogWrapper::GetInstance()->KMLOG(WARN, strMmLog);
		nMmRtn = 0;	// -> the existing failure message below
	}
	if (nMmRtn <= 0)
	{
		AfxMessageBox(_T("Fail : Load Recipe"));
		return;
	}

	if (pMgr->LoadMeasData(strFileName) == true)
	{
		UpdateSegmentCombo();	// LYS 2026-09-30 Hybrid 3d view = segment 0 of the recipe just opened (1-segment: selector stays hidden)
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
		UpdateSegmentCombo();	// LYS 2026-09-30 Hybrid 3d
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

	// LYS 2026-09-30 Hybrid 3d hybrid view: save the viewed segment only, with its own checks (the 1-segment path below is unchanged)
	if (IsViewSegmented())
	{
		SaveViewSegment(TRUE);
		return;
	}
	if (IsSegmented())	// a 1-segment view while a hybrid recipe is loaded (opened elsewhere): its grid is not this recipe's
	{
		AfxMessageBox(_T("WAD Multi 화면을 연 뒤 레시피가 바뀌었습니다.\nRecipe Open으로 다시 연 뒤 저장하십시오."), MB_ICONWARNING);
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

//---------------------------------------------------------------
// LYS 2026-09-30 Hybrid Phase 3d segment view (HybridGlass_Porting_Plan.md Phase 3d (5))
//   The grid shows GetSegmentMgr(m_nViewSeg) and saves to [DATA_SEG{k+1}] / [MASECELL_SEG{k+1}] (segment 0 = [DATA]/[MASECELL]).
//   SetActiveSegment is never called here - the measure thread owns the active segment.
//---------------------------------------------------------------
BOOL CDlgSubManual_MultiMeas::IsSegmented()
{
	if (m_pAuroraMgr == nullptr || m_pAuroraMgr->GetRecipeEditMgrPtr() == nullptr) return FALSE;
	return (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1) ? TRUE : FALSE;
}

BOOL CDlgSubManual_MultiMeas::IsViewSegmented()
{
	return (m_nViewSegCount > 1) ? TRUE : FALSE;
}

BOOL CDlgSubManual_MultiMeas::IsHybridRunBusy()
{
	if (IsSegmented() == FALSE) return FALSE;
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false) return TRUE;
	return CSystemParamShare::GetInstance()->IsCycleState() ? TRUE : FALSE;
}

void CDlgSubManual_MultiMeas::UpdateSegmentCombo()
{
	CRecipeEditMgr* pRcp = (m_pAuroraMgr != nullptr) ? m_pAuroraMgr->GetRecipeEditMgrPtr() : nullptr;
	C_OutRecipeFunc* pFunc = (pRcp != nullptr) ? pRcp->GetRecipeFuncPtr() : nullptr;
	int nSeg = (pRcp != nullptr && pFunc != nullptr) ? pRcp->GetSegmentLoopCount() : 1;
	if (nSeg < 1) nSeg = 1;

	m_nViewSeg = 0;
	m_nViewSegCount = nSeg;
	m_strViewSegRcp = (pRcp != nullptr) ? pRcp->GetRecipeID() : CString(_T(""));

	BOOL bShow = (nSeg > 1) ? TRUE : FALSE;
	if (m_cmbSeg.GetSafeHwnd() != NULL)
	{
		m_cmbSeg.ResetContent();
		if (bShow)
		{
			CString str;
			for (int k = 0; k < nSeg; k++)
			{
				str.Format(_T("Seg %d/%d %s R%d (%d x %d)"), k + 1, nSeg, pFunc->GetSegmentName(k).GetString(), pFunc->GetSegmentRotate(k) * 90,
					pFunc->GetSegmentStickCount(k), pFunc->GetSegmentCellCount(k));
				m_cmbSeg.AddString(str);
			}
			m_cmbSeg.SetCurSel(0);
		}
		m_cmbSeg.EnableWindow(TRUE);
		m_cmbSeg.ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}
	if (GetDlgItem(IDC_STATIC_MM_SEG) != NULL)		GetDlgItem(IDC_STATIC_MM_SEG)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	if (GetDlgItem(IDC_STATIC_MM_SEG_INFO) != NULL)	GetDlgItem(IDC_STATIC_MM_SEG_INFO)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);

	if (bShow)
	{
		CWadMultiMeasMgr* pMgr = GetMgr();
		if (pMgr != nullptr) pMgr->ResetRunState();	// B6: an idle load into the shared manager must not leave a run state behind
		SetTimer(MM_TIMER_SEG, 500, NULL);
	}
	else
		KillTimer(MM_TIMER_SEG);
}

void CDlgSubManual_MultiMeas::SetSegInfoText(LPCTSTR szText)
{
	CWnd* pInfo = GetDlgItem(IDC_STATIC_MM_SEG_INFO);
	if (pInfo == NULL) return;
	CString strOld;
	pInfo->GetWindowText(strOld);
	if (strOld != szText) pInfo->SetWindowText(szText);	// the 500 ms timer would flicker otherwise
}

void CDlgSubManual_MultiMeas::UpdateSegmentInfo()
{
	if (IsViewSegmented() == FALSE) return;

	CString strLine1, strLine2;
	C_OutRecipeFunc* pFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	S_RecipeMgr* pSegMgr = (pFunc != nullptr && m_nViewSeg < pFunc->GetSegmentCount()) ? pFunc->GetSegmentMgr(m_nViewSeg) : nullptr;
	if (pSegMgr != nullptr)
		strLine1.Format(_T("Multi(recipe) %s  cell %.2f x %.2f"), (pSegMgr->m_sInfo.m_sInspectItem.m_bUseSpec_Multi == TRUE) ? _T("ON") : _T("OFF"),
			fabs(pSegMgr->m_sMap.m_fCellSize.x), fabs(pSegMgr->m_sMap.m_fCellSize.y));

	CString strData = CWadMultiMeasMgr::GetDataSection(m_nViewSeg);
	if (m_bViewDataFound)
		strLine2.Format(_T("[%s] found"), strData.GetString());
	else
	{
		strLine2.Format(_T("[%s] none -> base point only"), strData.GetString());
		if (m_bViewMaskFound) strLine2 += _T(", [") + CWadMultiMeasMgr::GetMaskSection(m_nViewSeg) + _T("] present");
	}
	SetSegInfoText(strLine1 + _T("\n") + strLine2);
}

// hybrid view only. Short-circuit order: the segment vectors are read only when the counts still match.
BOOL CDlgSubManual_MultiMeas::IsMultiViewStale(BOOL bPopup)
{
	if (IsViewSegmented() == FALSE) return FALSE;

	BOOL bStale = FALSE;
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	CString strEdit, strRcpID = pRcp->GetRecipeID();
	GetDlgItem(IDC_EDIT_MM_RECIPE)->GetWindowText(strEdit);
	if (strEdit != strRcpID || m_strViewSegRcp != strRcpID || pRcp->GetSegmentLoopCount() != m_nViewSegCount)
		bStale = TRUE;
	else
	{
		C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
		if (pFunc == nullptr || m_nViewSeg >= pFunc->GetSegmentCount()
			|| pFunc->GetSegmentStickCount(m_nViewSeg) != m_nViewStick || pFunc->GetSegmentCellCount(m_nViewSeg) != m_nViewCell)
			bStale = TRUE;
	}
	if (bStale && bPopup)
		AfxMessageBox(_T("WAD Multi 화면을 연 뒤 레시피가 바뀌었습니다.\nRecipe Open으로 다시 연 뒤 저장하십시오."), MB_ICONWARNING);
	return bStale;
}

BOOL CDlgSubManual_MultiMeas::SaveViewSegment(BOOL bAsk)
{
	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr || IsViewSegmented() == FALSE) return FALSE;

	if (bAsk)
	{
		CString strAsk;
		strAsk.Format(_T("세그먼트 %d/%d WAD Multi 설정을 저장하시겠습니까?"), m_nViewSeg + 1, m_nViewSegCount);
		if (AfxMessageBox(strAsk, MB_YESNO | MB_ICONQUESTION) != IDYES) return FALSE;
	}
	if (IsMultiViewStale(TRUE)) return FALSE;

	CString strRecipe = m_strViewSegRcp;
	C_OutRecipeFunc* pFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();

	SaveToMgr();

	int nNgIndex = -1;
	if (pMgr->IsCellRangeValid(nNgIndex) != true)
	{
		CString strMsg;
		strMsg.Format(_T("포인트 %d가 셀 범위(±Cell/2) 밖에 있습니다.\n저장을 취소합니다."), nNgIndex + 1);
		AfxMessageBox(strMsg, MB_ICONWARNING);
		return FALSE;
	}

	S_RecipeMgr* pSegMgr = (pFunc != nullptr) ? pFunc->GetSegmentMgr(m_nViewSeg) : nullptr;
	if (pSegMgr != nullptr)
	{
		double dRX = fabs(pSegMgr->m_sMap.m_fCellSize.x), dRY = fabs(pSegMgr->m_sMap.m_fCellSize.y);
		if (fabs(pMgr->GetCellX() - dRX) > 0.5 || fabs(pMgr->GetCellY() - dRY) > 0.5)
		{
			CString strAsk;
			strAsk.Format(_T("Cell Size(%.4f x %.4f)가 세그먼트 %d 레시피 셀 크기(%.4f x %.4f)와 다릅니다.\n포인트 좌표는 이 세그먼트(R%d) 기준입니다. 그래도 저장하시겠습니까?"),
				pMgr->GetCellX(), pMgr->GetCellY(), m_nViewSeg + 1, dRX, dRY, pFunc->GetSegmentRotate(m_nViewSeg) * 90);
			if (AfxMessageBox(strAsk, MB_YESNO | MB_ICONWARNING) != IDYES) return FALSE;
		}
	}

	if (m_nMaskMismatch > 0 || m_nMaskExtra > 0)
	{
		CString strAsk;
		strAsk.Format(_T("셀 수가 맞지 않던 행 %d개와 스틱 수를 넘는 행 %d개를 지금 화면 값으로 덮어씁니다(넘는 행은 삭제).\n계속 하시겠습니까?"), m_nMaskMismatch, m_nMaskExtra);
		if (AfxMessageBox(strAsk, MB_YESNO | MB_ICONWARNING) != IDYES) return FALSE;
	}

	// lower-bound warning only: K comes from the AgingRcp (edited elsewhere); the auto start check refuses
	int nG = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetGlassRowBlock(0, 0);
	int nPk = pMgr->GetPointCount();
	if (pMgr->IsMultiMode() && nPk > 1 && nG * nPk > M_RESULT_AREA_MAX)
	{
		CString strWarn;
		strWarn.Format(_T("결과 행 수가 한도를 넘습니다: 셀 %d x 포인트 %d = %d > %d.\n자동 시작 시 거부됩니다."), nG, nPk, nG * nPk, M_RESULT_AREA_MAX);
		AfxMessageBox(strWarn, MB_ICONWARNING);
	}

	if (pMgr->SaveMeasSegment(strRecipe, m_nViewSeg, true) != true)
	{
		AfxMessageBox(_T("WAD Multi 레시피 저장에 실패했습니다."), MB_ICONWARNING);
		return FALSE;
	}

	pMgr->ResetRunState();
	pMgr->LoadMeasSegment(strRecipe, m_nViewSeg);	// reload for verification (OCP Save->Load pattern)
	LoadFromMgr();
	UpdateSegmentInfo();

	CString strLog;
	strLog.Format(_T("[HYBRID] WAD Multi saved %s [%s]/[%s] points %d rows %d"), strRecipe.GetString(),
		CWadMultiMeasMgr::GetDataSection(m_nViewSeg).GetString(), CWadMultiMeasMgr::GetMaskSection(m_nViewSeg).GetString(),
		pMgr->GetPointCount(), pMgr->GetCellUseStickCount());
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	if (bAsk) AfxMessageBox(_T("WAD Multi 레시피를 저장했습니다."));
	return TRUE;
}

void CDlgSubManual_MultiMeas::OnSelchangeComboMmSeg()
{
	int nSel = m_cmbSeg.GetCurSel();
	if (nSel < 0 || nSel == m_nViewSeg) return;

	if (IsViewSegmented() == FALSE || IsOfflineStatus() != TRUE || IsMultiViewStale(TRUE))	// IsOfflineStatus also blocks a busy hybrid run
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	CString strAsk;
	strAsk.Format(_T("세그먼트 %d WAD Multi 설정을 먼저 저장하시겠습니까?\n예: 저장 후 전환 / 아니요: 저장하지 않고 전환 / 취소: 전환 취소"), m_nViewSeg + 1);
	int nAns = AfxMessageBox(strAsk, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (nAns == IDCANCEL || (nAns == IDYES && SaveViewSegment(FALSE) == FALSE))
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	CWadMultiMeasMgr* pMgr = GetMgr();
	if (pMgr == nullptr)
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}

	m_nViewSeg = nSel;
	pMgr->ResetRunState();
	if (pMgr->LoadMeasSegment(m_strViewSegRcp, nSel) == false)
		pMgr->Clear();
	LoadFromMgr();
	UpdateSegmentInfo();
}

// The dialog is a WS_CHILD of CDlgSubManual: WM_SHOWWINDOW arrives only via the Manual sub-tab buttons, not when the
// Manual screen itself comes back from the top navigation - hence the timer (hybrid view only; none for 1-segment recipes).
void CDlgSubManual_MultiMeas::RefreshSegmentState()
{
	if (IsViewSegmented() == FALSE || m_cmbSeg.GetSafeHwnd() == NULL) return;

	if (IsHybridRunBusy())	// GetSegmentMgr is not read while the measure thread may be copying segments
	{
		m_cmbSeg.EnableWindow(FALSE);
		SetSegInfoText(_T("Hybrid run in progress"));
		return;
	}
	if (IsMultiViewStale(FALSE))
	{
		m_cmbSeg.EnableWindow(FALSE);
		SetSegInfoText(_T("Recipe changed - press Recipe Open"));
		return;
	}
	if (m_cmbSeg.IsWindowEnabled() == FALSE) m_cmbSeg.EnableWindow(TRUE);
	UpdateSegmentInfo();
}

void CDlgSubManual_MultiMeas::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == MM_TIMER_SEG)
		RefreshSegmentState();

	CDialogEx::OnTimer(nIDEvent);
}

void CDlgSubManual_MultiMeas::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	if (bShow)
		RefreshSegmentState();
}
