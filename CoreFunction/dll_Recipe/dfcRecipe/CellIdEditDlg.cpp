#include "pch.h"
#include "dfcRecipe.h"
#include "CellIdEditDlg.h"
#include <map>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B Auto/Manual editor (docs/HybridGlass_Porting_Plan.md 5.7, plan p4b (5)).
//   Port of ICP C_CellIdEditDlg with the HMI differences: one grid per segment (no physical composite map), the rules come from
//   CellIdRule.* (same code as SaveFile and the ALPHA runtime), bulk lines follow the Auto letter axis for every GLASS ROTATE,
//   APPLY is refused when the In config generation moved (MakeCell APPLY / reload while the editor was open).
//   Reads g_inRcpInfo only (cell ID config, segment geometry / names, stick numbers). It never runs the page's grid flush
//   (C_RecipeMgrDlg::GetRecipeView would copy stick 0's HEAD Y/X into every stick - ICP 8d162da defect) and never writes a grid of the page.

namespace {
	const COLORREF CLR_CELL_AUTO		= RGB (204, 229, 255);	// Auto (derived B included)
	const COLORREF CLR_CELL_DUP			= RGB (255, 165, 0);	// A+B duplicate over all segments
	const COLORREF CLR_CELL_CHANGED		= RGB (255, 255, 190);	// Manual, differs from the pure Auto token of that panel
	const COLORREF CLR_CELL_INVALID		= COLOR_RED;			// empty, or a manual / derived token that is not [A-Z]+[0-9]+ within the cap
	const COLORREF CLR_CELL_OK			= COLOR_WHITE;			// Manual, same as Auto
	const COLORREF CLR_CELL_READONLY	= COLOR_GRAY_BRIGHT;

	enum { COL_CTRL_SEG, COL_CTRL_PANEL, COL_CTRL_MODE, COL_CTRL_MAX };
	enum { COL_STAT_MODE, COL_STAT_VALID, COL_STAT_ERROR, COL_STAT_FIRST, COL_STAT_MAX };
	enum { COL_SEL_STICK, COL_SEL_CELL, COL_SEL_AUTO, COL_SEL_ID, COL_SEL_MAX };
	enum { COL_BULK_LINE, COL_BULK_PREFIX, COL_BULK_APPLY_PREFIX, COL_BULK_START, COL_BULK_APPLY_NUM, COL_BULK_MAX };

	TCHAR	PanelChar (int nPanel) { return (nPanel == 1) ? _T('B') : _T('A'); }

	// one positive digit -> two digits ("1" -> "01"); anything else is returned trimmed, unchanged
	CString	NormalizeBulkStart (CString str)
	{
		str.Trim ();
		if (str.GetLength () == 1 && str[0] >= _T('1') && str[0] <= _T('9')) str.Insert (0, _T('0'));
		return str;
	}

	// leading [A-Z] run / trailing [0-9] run; empty when the ID has none
	CString	CellIdLetters (const CString &str)
	{
		int n = 0;
		while (n < str.GetLength () && str[n] >= _T('A') && str[n] <= _T('Z')) n++;
		return str.Left (n);
	}
	CString	CellIdDigits (const CString &str)
	{
		int n = str.GetLength ();
		while (n > 0 && str[n - 1] >= _T('0') && str[n - 1] <= _T('9')) n--;
		return str.Mid (n);
	}

	// exactly [A-Z]+[0-9]+ (no blanks, upper case) and at most nMax chars = what CellId_Validate accepts for a manual / derived token
	BOOL	IsCanonicalToken (const CString &str, int nMax)
	{
		int nLen = str.GetLength ();
		if (nLen <= 0 || nLen > nMax) return FALSE;
		int nLet = CellIdLetters (str).GetLength ();
		if (nLet == 0 || nLet == nLen) return FALSE;
		return (CellIdDigits (str).GetLength () == nLen - nLet) ? TRUE : FALSE;
	}

	// range-checked [seg][stick][cell] read ("" outside)
	CString	GridAt (const CCellIdGrid &grid, int k, int i, int j)
	{
		if (k < 0 || k >= (int)grid.size ()) return CString ();
		if (i < 0 || i >= (int)grid[k].size ()) return CString ();
		if (j < 0 || j >= (int)grid[k][i].size ()) return CString ();
		return grid[k][i][j];
	}

	// physical stick number of the recipe stick (display only), -1 when unknown
	int		StickNumOf (int nSeg, int nStick)
	{
		S_RecipeMgr *pSeg = g_inRcpInfo.GetSegmentMgr (nSeg);
		if (pSeg == NULL && nSeg == g_inRcpInfo.GetActiveSegmentIndex ()) pSeg = &g_inRcpInfo.m_recipeMgr;
		if (pSeg == NULL || nStick < 0 || nStick >= (int)pSeg->m_sData.m_vecStick.size ()) return -1;
		return pSeg->m_sData.m_vecStick[nStick].m_nStickNum;
	}
}

IMPLEMENT_DYNAMIC (C_CellIdEditDlg, C_BaseDialog)

C_CellIdEditDlg::C_CellIdEditDlg (CWnd* pParent /*=NULL*/) : C_BaseDialog (IDD_CELL_ID_EDIT_DLG, pParent)
{
	m_nCfgSeq	= -1;
	m_nSeg		= 0;
	m_nPanel	= 0;
	m_nSelStick	= -1;
	m_nSelCell	= -1;
}

C_CellIdEditDlg::~C_CellIdEditDlg ()
{
}

void	C_CellIdEditDlg::DoDataExchange (CDataExchange* pDX)
{
	C_BaseDialog::DoDataExchange (pDX);
	DDX_Control (pDX, IDC_GRID_1, m_gridCtrlHead);
	DDX_Control (pDX, IDC_GRID_2, m_gridCtrlData);
	DDX_Control (pDX, IDC_GRID_3, m_gridStatHead);
	DDX_Control (pDX, IDC_GRID_4, m_gridStatData);
	DDX_Control (pDX, IDC_GRID_5, m_gridMap);
	DDX_Control (pDX, IDC_GRID_6, m_gridSelHead);
	DDX_Control (pDX, IDC_GRID_7, m_gridSelData);
	DDX_Control (pDX, IDC_GRID_8, m_gridBulkHead);
	DDX_Control (pDX, IDC_GRID_9, m_gridBulkData);
}

BEGIN_MESSAGE_MAP (C_CellIdEditDlg, C_BaseDialog)
	ON_NOTIFY (GVN_ENDLABELEDIT, IDC_GRID_2, &C_CellIdEditDlg::OnCtrlEndEdit)
	ON_NOTIFY (NM_CLICK,         IDC_GRID_5, &C_CellIdEditDlg::OnMapClick)
	ON_NOTIFY (GVN_ENDLABELEDIT, IDC_GRID_7, &C_CellIdEditDlg::OnSelEndEdit)
	ON_NOTIFY (NM_CLICK,         IDC_GRID_9, &C_CellIdEditDlg::OnBulkClick)
	ON_NOTIFY (GVN_ENDLABELEDIT, IDC_GRID_9, &C_CellIdEditDlg::OnBulkEndEdit)
	ON_COMMAND (IDC_BUTTON_1, &C_CellIdEditDlg::OnBtnApply)
END_MESSAGE_MAP ()

BOOL	C_CellIdEditDlg::OnInitDialog ()
{
	m_nCfgSeq = g_inRcpInfo.GetCellIdCfgSeq ();		// first: MakeCell APPLY / a reload after this point makes APPLY refuse (review C1/K1)

	C_BaseDialog::OnInitDialog ();

	m_work = g_inRcpInfo.GetCellIdConfig ();
	CellId_BuildGeo (g_inRcpInfo, m_geo);
	CellId_EnsureShape (m_work, m_geo);
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) CellId_BuildAuto (m_geo, p, m_auto[p]);	// out-parameter form (CellIdRule.h)
	if (m_geo.empty ()) {		// guard only - RecipeMgrDlg refuses to open without cells
		EndDialog (IDCANCEL);
		return TRUE;
	}

	m_nSeg = g_inRcpInfo.GetActiveSegmentIndex ();
	if (m_nSeg < 0 || m_nSeg >= (int)m_geo.size ()) m_nSeg = 0;
	m_nPanel	= 0;
	m_nSelStick	= -1;
	m_nSelCell	= -1;

	InitControlGrid ();
	InitStatusGrid ();
	InitSelectGrid ();
	InitBulkGrid ();
	BuildMap ();
	RefreshView ();

	return TRUE;
}

void	C_CellIdEditDlg::OnOK ()
{
	// Enter must not close / apply; APPLY is IDC_BUTTON_1 (OnBtnApply calls C_BaseDialog::OnOK)
}

int		C_CellIdEditDlg::StickCount (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_geo.size () || m_geo[nSeg].m_nStick < 0) return 0;
	return m_geo[nSeg].m_nStick;
}

int		C_CellIdEditDlg::CellCount (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_geo.size () || m_geo[nSeg].m_nCell < 0) return 0;
	return m_geo[nSeg].m_nCell;
}

CString*	C_CellIdEditDlg::ManualId (int nSeg, int nStick, int nCell)
{
	if (m_nPanel < 0 || m_nPanel >= M_CELLID_PANEL_MAX) return NULL;
	CCellIdGrid &grid = m_work.m_vecManual[m_nPanel];
	if (nSeg < 0 || nSeg >= (int)grid.size ()) return NULL;
	if (nStick < 0 || nStick >= (int)grid[nSeg].size ()) return NULL;
	if (nCell < 0 || nCell >= (int)grid[nSeg][nStick].size ()) return NULL;
	return &grid[nSeg][nStick][nCell];
}

void	C_CellIdEditDlg::InitControlGrid ()
{
	InitGridUI (m_gridCtrlHead, IDC_GRID_1, 1, COL_CTRL_MAX, TRUE, TRUE);
	m_gridCtrlHead.SetItemText (0, COL_CTRL_SEG,   _T("SEGMENT"));
	m_gridCtrlHead.SetItemText (0, COL_CTRL_PANEL, _T("PANEL"));
	m_gridCtrlHead.SetItemText (0, COL_CTRL_MODE,  _T("MODE"));
	m_gridCtrlHead.EnableWindow (FALSE);

	InitGridUI (m_gridCtrlData, IDC_GRID_2, 1, COL_CTRL_MAX, TRUE);
	m_gridCtrlData.SetFixedRowCol (0, 0);		// every column is a value, not a label

	CRect rc;
	m_gridCtrlHead.GetClientRect (&rc);
	const int nWidth = (rc.Width () - 4) / COL_CTRL_MAX;
	for (int i = 0; i < COL_CTRL_MAX; i++) {
		m_gridCtrlHead.SetColumnWidth (i, nWidth);
		m_gridCtrlData.SetColumnWidth (i, nWidth);
	}

	// segment options carry the 1-based index like the page's segment combo: CGridCellCombo::GetCurSel matches by text, names may repeat
	CStringArray arrSeg, arrPanel, arrMode;
	for (int k = 0; k < (int)m_geo.size (); k++) {
		CString strName = g_inRcpInfo.GetSegmentName (k);
		if (strName.IsEmpty ()) strName.Format (_T("Segment#%d"), k + 1);
		arrSeg.Add (dfcStr (_T("%d. %s"), k + 1, (LPCTSTR)strName));
	}
	arrPanel.Add (_T("A"));
	arrPanel.Add (_T("B"));
	arrMode.Add (_T("AUTO"));
	arrMode.Add (_T("MANUAL"));

	const CStringArray *pOptions[COL_CTRL_MAX] = { &arrSeg, &arrPanel, &arrMode };
	for (int c = 0; c < COL_CTRL_MAX; c++) {
		m_gridCtrlData.SetCellType (0, c, RUNTIME_CLASS (CGridCellCombo));
		CGridCellCombo *pCell = (CGridCellCombo*)m_gridCtrlData.GetCell (0, c);
		if (pCell) {
			pCell->SetStyle (CBS_DROPDOWNLIST);
			pCell->SetOptions (*pOptions[c]);
		}
	}
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_SEG,   m_nSeg);
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_PANEL, m_nPanel);
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_MODE,  IsManual () ? 1 : 0);
}

void	C_CellIdEditDlg::InitStatusGrid ()
{
	InitGridUI (m_gridStatHead, IDC_GRID_3, 1, COL_STAT_MAX, TRUE, TRUE);
	m_gridStatHead.SetItemText (0, COL_STAT_MODE,  _T("PANEL MODE"));
	m_gridStatHead.SetItemText (0, COL_STAT_VALID, _T("VALID (A)"));		// current panel letter, rewritten by RefreshView
	m_gridStatHead.SetItemText (0, COL_STAT_ERROR, _T("ERROR (A+B)"));	// both panels (ICP X15 label fix, review K10)
	m_gridStatHead.SetItemText (0, COL_STAT_FIRST, _T("FIRST ERROR"));
	m_gridStatHead.EnableWindow (FALSE);

	InitGridUI (m_gridStatData, IDC_GRID_4, 1, COL_STAT_MAX, TRUE);
	m_gridStatData.SetFixedRowCol (0, 0);
	m_gridStatData.SetEditable (FALSE);
	m_gridStatData.EnableTitleTips (TRUE);		// the first error text is usually longer than the cell

	CRect rc;
	m_gridStatHead.GetClientRect (&rc);
	const int nSmall = 110;
	const int nFirst = max (nSmall, (int)rc.Width () - 4 - nSmall * 3);
	const int nWidth[COL_STAT_MAX] = { nSmall, nSmall, nSmall, nFirst };
	for (int i = 0; i < COL_STAT_MAX; i++) {
		m_gridStatHead.SetColumnWidth (i, nWidth[i]);
		m_gridStatData.SetColumnWidth (i, nWidth[i]);
	}
	m_gridStatData.SetItemFormat (0, COL_STAT_FIRST, DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

void	C_CellIdEditDlg::InitSelectGrid ()
{
	InitGridUI (m_gridSelHead, IDC_GRID_6, 1, COL_SEL_MAX, TRUE, TRUE);
	m_gridSelHead.SetItemText (0, COL_SEL_STICK, _T("STICK"));
	m_gridSelHead.SetItemText (0, COL_SEL_CELL,  _T("CELL"));
	m_gridSelHead.SetItemText (0, COL_SEL_AUTO,  _T("AUTO"));
	m_gridSelHead.SetItemText (0, COL_SEL_ID,    _T("CELL ID"));
	m_gridSelHead.EnableWindow (FALSE);

	InitGridUI (m_gridSelData, IDC_GRID_7, 1, COL_SEL_MAX, TRUE);
	m_gridSelData.SetFixedRowCol (0, 0);

	CRect rc;
	m_gridSelHead.GetClientRect (&rc);
	const int nWidth[COL_SEL_MAX] = { 90, 60, 90, max (90, (int)rc.Width () - 4 - 240) };
	for (int i = 0; i < COL_SEL_MAX; i++) {
		m_gridSelHead.SetColumnWidth (i, nWidth[i]);
		m_gridSelData.SetColumnWidth (i, nWidth[i]);
	}
	const int nReadOnly[] = { COL_SEL_STICK, COL_SEL_CELL, COL_SEL_AUTO };
	for (int i = 0; i < (int)_countof (nReadOnly); i++) {
		m_gridSelData.SetItemState (0, nReadOnly[i], m_gridSelData.GetItemState (0, nReadOnly[i]) | GVIS_READONLY);
	}
}

void	C_CellIdEditDlg::InitBulkGrid ()
{
	InitGridUI (m_gridBulkHead, IDC_GRID_8, 1, COL_BULK_MAX, TRUE, TRUE);
	m_gridBulkHead.SetItemText (0, COL_BULK_LINE,		  _T("LINE"));
	m_gridBulkHead.SetItemText (0, COL_BULK_PREFIX,		  _T("PREFIX"));
	m_gridBulkHead.SetItemText (0, COL_BULK_APPLY_PREFIX, _T(""));
	m_gridBulkHead.SetItemText (0, COL_BULK_START,		  _T("START NO."));
	m_gridBulkHead.SetItemText (0, COL_BULK_APPLY_NUM,	  _T(""));
	m_gridBulkHead.EnableWindow (FALSE);

	InitGridUI (m_gridBulkData, IDC_GRID_9, 1, COL_BULK_MAX, TRUE);
	m_gridBulkData.SetFixedRowCol (0, 0);

	CRect rc;
	m_gridBulkHead.GetClientRect (&rc);
	const int nEdit = 80, nButton = 120;
	const int nLine = max (80, (int)rc.Width () - 4 - nEdit * 2 - nButton * 2);
	const int nWidth[COL_BULK_MAX] = { nLine, nEdit, nButton, nEdit, nButton };
	for (int i = 0; i < COL_BULK_MAX; i++) {
		m_gridBulkHead.SetColumnWidth (i, nWidth[i]);
		m_gridBulkData.SetColumnWidth (i, nWidth[i]);
	}
	m_gridBulkData.SetItemState (0, COL_BULK_LINE, m_gridBulkData.GetItemState (0, COL_BULK_LINE) | GVIS_READONLY);

	// read-only button cells still get the click (NM_CLICK); read-only only blocks the stray in-place editor
	const int nButtonCol[] = { COL_BULK_APPLY_PREFIX, COL_BULK_APPLY_NUM };
	const TCHAR *szButton[] = { _T("APPLY PREFIX"), _T("APPLY NUMBERS") };
	for (int i = 0; i < (int)_countof (nButtonCol); i++) {
		m_gridBulkData.SetCellType (0, nButtonCol[i], RUNTIME_CLASS (CGridCellButton));
		CGridCellButton *pCell = (CGridCellButton*)m_gridBulkData.GetCell (0, nButtonCol[i]);
		if (pCell) pCell->SetText (szButton[i]);
		m_gridBulkData.SetItemState (0, nButtonCol[i], m_gridBulkData.GetItemState (0, nButtonCol[i]) | GVIS_READONLY);
	}
}

// cell map of the current segment: row = stick (recipe index order), column = cell (SelectCellDlg::SetViewSelectCell direction)
void	C_CellIdEditDlg::BuildMap ()
{
	const int nStick = StickCount (m_nSeg), nCell = CellCount (m_nSeg);

	m_gridMap.SetFocusCell (-1, -1);			// the previous segment may have had more rows / columns
	InitGridUI (m_gridMap, IDC_GRID_5, nStick + 1, nCell + 1, TRUE, TRUE);	// fixed header row C1..Cn + fixed column S1..Sn
	m_gridMap.EnableToolTips (FALSE);
	m_gridMap.EnableTitleTips (TRUE);
	m_gridMap.SetEditable (FALSE);
	m_gridMap.EnableSelection (FALSE);

	// never shrink below a readable size; a larger map scrolls inside the grid
	CRect rc;
	m_gridMap.GetClientRect (&rc);
	const int nColW = (nCell > 0) ? max (60, ((int)rc.Width () - 64) / nCell) : 60;
	const int nRowH = max (22, ((int)rc.Height () - 4) / (nStick + 1));
	m_gridMap.SetColumnWidth (0, 60);
	for (int j = 1; j <= nCell; j++) m_gridMap.SetColumnWidth (j, nColW);
	for (int i = 0; i <= nStick; i++) m_gridMap.SetRowHeight (i, nRowH);

	m_gridMap.SetItemText (0, 0, _T(""));
	for (int j = 0; j < nCell; j++) m_gridMap.SetItemText (0, j + 1, dfcStr (_T("C%d"), j + 1));
	for (int i = 0; i < nStick; i++) m_gridMap.SetItemText (i + 1, 0, dfcStr (_T("S%d"), i + 1));
}

void	C_CellIdEditDlg::RefreshView ()
{
	const BOOL bManual = IsManual ();
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_SEG,   m_nSeg);
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_PANEL, m_nPanel);
	m_gridCtrlData.SetItemCurSel (0, COL_CTRL_MODE,  bManual ? 1 : 0);
	m_gridCtrlData.Invalidate ();

	// resolved tokens of both panels (B Auto may be derived from a manual A, so the pure Auto preview is not enough)
	//   + one combined A+B count over all segments = CellId_Validate step 3
	CCellIdGrid vecRes[M_CELLID_PANEL_MAX];
	std::map<CString, int> mapCount;
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
		CellId_BuildResolved (m_work, m_geo, p, vecRes[p]);
		for (size_t k = 0; k < vecRes[p].size (); k++)
			for (size_t i = 0; i < vecRes[p][k].size (); i++)
				for (size_t j = 0; j < vecRes[p][k][i].size (); j++)
					if (!vecRes[p][k][i][j].IsEmpty ()) mapCount[vecRes[p][k][i][j]]++;
	}
	const int  nTokenMax = CellId_TokenMax (-1);
	const BOOL bOverride = (bManual || (m_nPanel == 1 && m_work.m_nMode[0] == M_CELLID_MODE_MANUAL)) ? TRUE : FALSE;	// derived B = override too

	const int nStick = StickCount (m_nSeg), nCell = CellCount (m_nSeg);
	for (int i = 0; i < nStick; i++) {
		for (int j = 0; j < nCell; j++) {
			CString str = GridAt (vecRes[m_nPanel], m_nSeg, i, j);
			COLORREF clrBk = CLR_CELL_AUTO;
			if (str.IsEmpty () || (bOverride && !IsCanonicalToken (str, nTokenMax)))	clrBk = CLR_CELL_INVALID;
			else if (mapCount[str] > 1)													clrBk = CLR_CELL_DUP;
			else if (bManual)	clrBk = (str != GridAt (m_auto[m_nPanel], m_nSeg, i, j)) ? CLR_CELL_CHANGED : CLR_CELL_OK;	// B: pure Auto B
			m_gridMap.SetItemText (i + 1, j + 1, str);
			m_gridMap.SetItemBkColour (i + 1, j + 1, clrBk);
			m_gridMap.SetItemFgColour (i + 1, j + 1, (i == m_nSelStick && j == m_nSelCell) ? COLOR_BLUE : COLOR_BLACK);
		}
	}
	m_gridMap.Invalidate ();

	// VALID = current panel, every segment: not empty, override -> canonical within the cap, no A+B duplicate
	int nTotal = 0, nValid = 0;
	for (int k = 0; k < (int)m_geo.size (); k++) {
		for (int i = 0; i < StickCount (k); i++) {
			for (int j = 0; j < CellCount (k); j++) {
				nTotal++;
				CString str = GridAt (vecRes[m_nPanel], k, i, j);
				if (str.IsEmpty ()) continue;
				if (bOverride && !IsCanonicalToken (str, nTokenMax)) continue;
				if (mapCount[str] > 1) continue;
				nValid++;
			}
		}
	}
	std::vector<S_CellIdError> vecErr;
	CellId_Validate (m_work, m_geo, nTokenMax, vecErr, NULL);		// ERROR = both panels (Auto/Auto -> 0, never checked)

	m_gridStatHead.SetItemText (0, COL_STAT_VALID, dfcStr (_T("VALID (%c)"), PanelChar (m_nPanel)));
	m_gridStatHead.Invalidate ();
	m_gridStatData.SetItemText (0, COL_STAT_MODE,  dfcStr (_T("%c / %s"), PanelChar (m_nPanel), bManual ? _T("MANUAL") : _T("AUTO")));
	m_gridStatData.SetItemText (0, COL_STAT_VALID, dfcStr (_T("%d/%d"), nValid, nTotal));
	m_gridStatData.SetItemText (0, COL_STAT_ERROR, dfcStr (_T("%d"), (int)vecErr.size ()));
	m_gridStatData.SetItemText (0, COL_STAT_FIRST, vecErr.empty () ? CString (_T("-")) : FormatError (vecErr[0]));
	const COLORREF clrStatus = vecErr.empty () ? COLOR_WHITE : CLR_CELL_INVALID;
	m_gridStatData.SetItemBkColour (0, COL_STAT_ERROR, clrStatus);
	m_gridStatData.SetItemBkColour (0, COL_STAT_FIRST, clrStatus);
	m_gridStatData.Invalidate ();

	RefreshSelectGrid ();
}

void	C_CellIdEditDlg::RefreshSelectGrid ()
{
	const int  nStick = StickCount (m_nSeg), nCell = CellCount (m_nSeg);
	const BOOL bHasSel = (m_nSelStick >= 0 && m_nSelStick < nStick && m_nSelCell >= 0 && m_nSelCell < nCell) ? TRUE : FALSE;
	const BOOL bEditable = (bHasSel && IsManual ()) ? TRUE : FALSE;

	if (bHasSel) {
		// STICK = recipe stick index + 1 (the key) and the physical stick number (display only)
		int nStickNum = StickNumOf (m_nSeg, m_nSelStick);
		if (nStickNum >= 0) m_gridSelData.SetItemText (0, COL_SEL_STICK, dfcStr (_T("%d (#%d)"), m_nSelStick + 1, nStickNum));
		else				m_gridSelData.SetItemText (0, COL_SEL_STICK, dfcStr (_T("%d"), m_nSelStick + 1));
		m_gridSelData.SetItemText (0, COL_SEL_CELL, dfcStr (_T("%d"), m_nSelCell + 1));
		m_gridSelData.SetItemText (0, COL_SEL_AUTO, GridAt (m_auto[m_nPanel], m_nSeg, m_nSelStick, m_nSelCell));

		CString strId;
		if (IsManual ()) {
			CString *pId = ManualId (m_nSeg, m_nSelStick, m_nSelCell);
			if (pId) strId = *pId;
		}
		else CellId_Resolve (m_work, m_geo, m_nSeg, m_nSelStick, m_nSelCell, m_nPanel, strId, TRUE);	// Auto or derived B
		m_gridSelData.SetItemText (0, COL_SEL_ID, strId);
	}
	else {
		m_gridSelData.SetItemText (0, COL_SEL_STICK, _T("-"));
		m_gridSelData.SetItemText (0, COL_SEL_CELL,  _T("-"));
		m_gridSelData.SetItemText (0, COL_SEL_AUTO,  _T("-"));
		m_gridSelData.SetItemText (0, COL_SEL_ID,    _T(""));
	}

	UINT nState = m_gridSelData.GetItemState (0, COL_SEL_ID);
	m_gridSelData.SetItemState (0, COL_SEL_ID, bEditable ? (nState & ~GVIS_READONLY) : (nState | GVIS_READONLY));
	m_gridSelData.SetItemBkColour (0, COL_SEL_ID, bEditable ? COLOR_WHITE : CLR_CELL_READONLY);
	m_gridSelData.Invalidate ();

	RefreshBulkGrid ();
}

void	C_CellIdEditDlg::RefreshBulkGrid ()
{
	std::vector< std::pair<int, int> > vecCells;
	CString strLine, strError;
	const BOOL bLine   = GetBulkLineCells (vecCells, strLine, strError);
	const BOOL bEnable = (bLine && IsManual ()) ? TRUE : FALSE;

	m_gridBulkData.SetItemText (0, COL_BULK_LINE, bLine ? strLine : CString (_T("-")));
	const int nEditCol[] = { COL_BULK_PREFIX, COL_BULK_START };
	for (int i = 0; i < (int)_countof (nEditCol); i++) {
		UINT nState = m_gridBulkData.GetItemState (0, nEditCol[i]);
		m_gridBulkData.SetItemState (0, nEditCol[i], bEnable ? (nState & ~GVIS_READONLY) : (nState | GVIS_READONLY));
		m_gridBulkData.SetItemBkColour (0, nEditCol[i], bEnable ? COLOR_WHITE : CLR_CELL_READONLY);
	}
	m_gridBulkData.Invalidate ();
}

// cells that share the selected cell's Auto letter in this segment, in rising Auto number order (ICP Deg0 row / Deg90 column, every
//   GLASS ROTATE - ICP's Deg180/-90 refusal is not ported, X5). START 1 then equals the Auto numbers.
BOOL	C_CellIdEditDlg::GetBulkLineCells (std::vector< std::pair<int, int> > &vecCells, CString &strLine, CString &strError)
{
	vecCells.clear ();
	strLine.Empty ();
	strError.Empty ();
	const int nStick = StickCount (m_nSeg), nCell = CellCount (m_nSeg);
	if (m_nSelStick < 0 || m_nSelStick >= nStick || m_nSelCell < 0 || m_nSelCell >= nCell) {
		strError = _T("대상 Cell을 먼저 선택하십시오.");
		return FALSE;
	}

	if (m_geo[m_nSeg].m_nRotate == 1) {		// MakeCellID rotate 1: letter = cell, number = stick + 1 -> the map column, top to bottom
		for (int i = 0; i < nStick; i++) vecCells.push_back (std::make_pair (i, m_nSelCell));
		strLine.Format (_T("CELL %d : STICK 1->%d (%d)"), m_nSelCell + 1, nStick, (int)vecCells.size ());
	}
	else {									// other rotates: letter = stick, number = cell + 1 -> the map row, left to right
		for (int j = 0; j < nCell; j++) vecCells.push_back (std::make_pair (m_nSelStick, j));
		strLine.Format (_T("STICK %d : CELL 1->%d (%d)"), m_nSelStick + 1, nCell, (int)vecCells.size ());
	}
	return TRUE;
}

void	C_CellIdEditDlg::ApplyBulkPrefix ()
{
	// LYS 2026-10-01 Hybrid Phase 4b review fix: the target line first - with no cell selected the PREFIX / START cells are read-only
	//   (usually empty), so their format popup would hide the real reason (plan 5-3, OOS O13)
	std::vector< std::pair<int, int> > vecCells;
	CString strLine, strError;
	if (!GetBulkLineCells (vecCells, strLine, strError)) { AfxMessageBox (strError, MB_ICONWARNING); return; }

	CString strPrefix = m_gridBulkData.GetItemText (0, COL_BULK_PREFIX);
	strPrefix.Trim ();
	strPrefix.MakeUpper ();
	if (strPrefix.IsEmpty () || CellIdLetters (strPrefix) != strPrefix) {
		AfxMessageBox (_T("Prefix는 영문 대문자만 입력할 수 있습니다."), MB_ICONWARNING);
		return;
	}
	m_gridBulkData.SetItemText (0, COL_BULK_PREFIX, strPrefix);

	for (size_t n = 0; n < vecCells.size (); n++) {
		CString *pId = ManualId (m_nSeg, vecCells[n].first, vecCells[n].second);
		if (pId == NULL) continue;
		CString strDigits = CellIdDigits (*pId);
		// blank / malformed target: the number comes from this panel's pure Auto token (B -> B Auto number, ICP handoff 2.7)
		if (strDigits.IsEmpty ()) strDigits = CellIdDigits (GridAt (m_auto[m_nPanel], m_nSeg, vecCells[n].first, vecCells[n].second));
		*pId = strPrefix + strDigits;
	}
	RefreshView ();
}

void	C_CellIdEditDlg::ApplyBulkNumbers ()
{
	// LYS 2026-10-01 Hybrid Phase 4b review fix: the target line first - with no cell selected the PREFIX / START cells are read-only
	//   (usually empty), so their format popup would hide the real reason (plan 5-3, OOS O13)
	std::vector< std::pair<int, int> > vecCells;
	CString strLine, strError;
	if (!GetBulkLineCells (vecCells, strLine, strError)) { AfxMessageBox (strError, MB_ICONWARNING); return; }

	CString strStart = NormalizeBulkStart (m_gridBulkData.GetItemText (0, COL_BULK_START));
	if (strStart.IsEmpty () || strStart.GetLength () > 9 || CellIdDigits (strStart) != strStart || _ttoi64 (strStart) <= 0) {
		AfxMessageBox (_T("Start Number는 9자리 이하의 양의 정수만 입력할 수 있습니다."), MB_ICONWARNING);
		return;
	}
	m_gridBulkData.SetItemText (0, COL_BULK_START, strStart);

	// refuse up front so a line is never half-numbered
	const __int64 nStart = _ttoi64 (strStart);
	if (nStart + (__int64)vecCells.size () - 1 > 999999999) {
		AfxMessageBox (_T("마지막 번호가 9자리를 초과합니다."), MB_ICONWARNING);
		return;
	}

	// minimum pad only - %0*d never truncates a number that outgrows it (09 -> 10, 99 -> 100)
	const int nWidth = max (2, strStart.GetLength ());
	for (size_t n = 0; n < vecCells.size (); n++) {
		CString *pId = ManualId (m_nSeg, vecCells[n].first, vecCells[n].second);
		if (pId == NULL) continue;
		CString strLetters = CellIdLetters (*pId);
		// blank / malformed target: the letters come from this panel's pure Auto token
		if (strLetters.IsEmpty ()) strLetters = CellIdLetters (GridAt (m_auto[m_nPanel], m_nSeg, vecCells[n].first, vecCells[n].second));
		pId->Format (_T("%s%0*d"), (LPCTSTR)strLetters, nWidth, (int)(nStart + (__int64)n));
	}
	RefreshView ();
}

void	C_CellIdEditDlg::OnBulkClick (NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pItem = reinterpret_cast<NM_GRIDVIEW*> (pNMHDR);
	*pResult = 0;
	if (pItem->iRow != 0) return;
	if (pItem->iColumn != COL_BULK_APPLY_PREFIX && pItem->iColumn != COL_BULK_APPLY_NUM) return;

	if (!IsManual ()) {
		AfxMessageBox (_T("일괄 입력은 Manual 모드에서만 사용할 수 있습니다."), MB_ICONWARNING);
		return;
	}
	if (pItem->iColumn == COL_BULK_APPLY_PREFIX) ApplyBulkPrefix ();
	else										 ApplyBulkNumbers ();
}

void	C_CellIdEditDlg::OnBulkEndEdit (NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pItem = reinterpret_cast<NM_GRIDVIEW*> (pNMHDR);
	*pResult = 0;
	if (pItem->iRow != 0 || pItem->iColumn != COL_BULK_START) return;

	m_gridBulkData.SetItemText (0, COL_BULK_START, NormalizeBulkStart (m_gridBulkData.GetItemText (0, COL_BULK_START)));
}

// ICP SetPanelMode: Auto -> Manual seeds every segment from what the panel resolves to now (B may be derived from a manual A),
//   so only the exceptions need editing; Manual -> Auto drops that panel's manual IDs (Auto keeps no inactive draft)
void	C_CellIdEditDlg::SetPanelMode (int nMode)
{
	CCellIdGrid vecSeed;
	CellId_BuildResolved (m_work, m_geo, m_nPanel, vecSeed);	// before the mode flips
	m_work.m_nMode[m_nPanel] = (nMode == M_CELLID_MODE_MANUAL) ? M_CELLID_MODE_MANUAL : M_CELLID_MODE_AUTO;
	CellId_EnsureShape (m_work, m_geo);							// Manual -> [seg][stick][cell], Auto -> empty

	if (m_work.m_nMode[m_nPanel] == M_CELLID_MODE_MANUAL) {
		for (int k = 0; k < (int)m_geo.size (); k++) {
			for (int i = 0; i < StickCount (k); i++) {
				for (int j = 0; j < CellCount (k); j++) {
					CString *pId = ManualId (k, i, j);
					if (pId == NULL) continue;
					CString strSeed = GridAt (vecSeed, k, i, j);
					if (strSeed.IsEmpty ()) strSeed = GridAt (m_auto[m_nPanel], k, i, j);	// underivable B -> pure Auto
					*pId = strSeed;
				}
			}
		}
	}
	RefreshView ();
}

void	C_CellIdEditDlg::OnCtrlEndEdit (NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pItem = reinterpret_cast<NM_GRIDVIEW*> (pNMHDR);
	*pResult = 0;
	if (pItem->iRow != 0) return;

	const int nSel = m_gridCtrlData.GetItemCurSel (0, pItem->iColumn);
	if (nSel < 0) return;

	switch (pItem->iColumn) {
		case COL_CTRL_SEG:
			if (nSel == m_nSeg || nSel >= (int)m_geo.size ()) return;
			m_nSeg = nSel;
			m_nSelStick = m_nSelCell = -1;
			BuildMap ();				// the segment's stick / cell counts differ
			RefreshView ();
			break;

		case COL_CTRL_PANEL:
			if (nSel >= M_CELLID_PANEL_MAX) return;
			m_nPanel = nSel;			// selection kept
			RefreshView ();
			break;

		case COL_CTRL_MODE: {
			const int nMode = (nSel == 1) ? M_CELLID_MODE_MANUAL : M_CELLID_MODE_AUTO;
			if (nMode == m_work.m_nMode[m_nPanel]) return;
			if (nMode == M_CELLID_MODE_AUTO &&
				AfxMessageBox (dfcStr (_T("Panel %c의 Manual Cell ID가 모든 Segment에서 삭제됩니다.\n계속 하시겠습니까?"), PanelChar (m_nPanel)),
							   MB_YESNO | MB_ICONQUESTION) != IDYES) {
				*pResult = -1;			// negative result: dfcGrid OnEndEditCell restores the previous mode text
				return;
			}
			SetPanelMode (nMode);
			break;
		}
	}
}

void	C_CellIdEditDlg::OnMapClick (NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pItem = reinterpret_cast<NM_GRIDVIEW*> (pNMHDR);
	*pResult = 0;

	const int nStick = pItem->iRow - 1, nCell = pItem->iColumn - 1;	// fixed header row / column
	if (nStick < 0 || nStick >= StickCount (m_nSeg) || nCell < 0 || nCell >= CellCount (m_nSeg)) return;

	m_nSelStick = nStick;
	m_nSelCell	= nCell;
	RefreshView ();
}

void	C_CellIdEditDlg::OnSelEndEdit (NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_GRIDVIEW *pItem = reinterpret_cast<NM_GRIDVIEW*> (pNMHDR);
	*pResult = 0;
	if (pItem->iRow != 0 || pItem->iColumn != COL_SEL_ID || !IsManual ()) return;

	CString *pId = ManualId (m_nSeg, m_nSelStick, m_nSelCell);
	if (pId == NULL) return;

	// invalid text is kept (trimmed / upper case) so the map can mark it red; APPLY / SAVE refuse it
	CString str = m_gridSelData.GetItemText (0, COL_SEL_ID);
	str.Trim ();
	str.MakeUpper ();
	*pId = str;
	RefreshView ();
}

void	C_CellIdEditDlg::SelectError (const S_CellIdError &e)
{
	if (e.m_nSeg < 0 || e.m_nSeg >= (int)m_geo.size ()) return;
	if (e.m_nSeg != m_nSeg) {
		m_nSeg = e.m_nSeg;
		BuildMap ();
	}
	if (e.m_nPanel >= 0 && e.m_nPanel < M_CELLID_PANEL_MAX) m_nPanel = e.m_nPanel;	// ICP X6: the panel follows the error too
	m_nSelStick = e.m_nStick;
	m_nSelCell	= e.m_nCell;
	RefreshView ();				// combos follow m_nSeg / m_nPanel

	if (e.m_nStick >= 0 && e.m_nStick < StickCount (m_nSeg) && e.m_nCell >= 0 && e.m_nCell < CellCount (m_nSeg))
		m_gridMap.EnsureVisible (e.m_nStick + 1, e.m_nCell + 1);
}

void	C_CellIdEditDlg::OnBtnApply ()
{
	// review C1/K1 line 2: MakeCell RECIPE APPLY (modeless, still clickable) or a reload changed the cells / config under the editor.
	//   A geometry compare alone cannot catch a same-size rebuild, so the generation decides; the compare is an extra guard.
	CCellIdGeo geoNow;
	CellId_BuildGeo (g_inRcpInfo, geoNow);
	BOOL bSameGeo = (geoNow.size () == m_geo.size ()) ? TRUE : FALSE;
	for (size_t k = 0; bSameGeo && k < geoNow.size (); k++) {
		if (geoNow[k].m_nRotate != m_geo[k].m_nRotate || geoNow[k].m_nStick != m_geo[k].m_nStick || geoNow[k].m_nCell != m_geo[k].m_nCell) bSameGeo = FALSE;
	}
	if (g_inRcpInfo.GetCellIdCfgSeq () != m_nCfgSeq || !bSameGeo) {
		AfxMessageBox (_T("편집 중에 레시피 셀 구성 또는 Cell ID 설정이 바뀌었습니다. (MAKE CELL 적용 등)\n편집 내용을 적용하지 않고 닫습니다. CELL ID 편집기를 다시 여십시오."), MB_ICONWARNING);
		EndDialog (IDCANCEL);
		return;
	}

	std::vector<S_CellIdError> vecErr;
	if (CellId_Validate (m_work, m_geo, CellId_TokenMax (-1), vecErr, NULL) != TRUE && !vecErr.empty ()) {
		SelectError (vecErr[0]);
		AfxMessageBox (dfcStr (_T("Cell ID 설정이 올바르지 않습니다. (%d건)\n%s"), (int)vecErr.size (), (LPCTSTR)FormatError (vecErr[0])), MB_ICONWARNING);
		return;
	}

	g_inRcpInfo.SetCellIdConfig (m_work);	// Auto panels emptied, Manual panels shaped, generation bumped. The file is not saved (SAVE re-validates)
	C_BaseDialog::OnOK ();
}

// "Segment %d Stick %d Cell %d (%c): <text>" - 1-based, Stick = recipe stick index + 1
CString	C_CellIdEditDlg::FormatError (const S_CellIdError &e)
{
	CString strMsg;
	switch (e.m_nCode) {
		case M_CELLID_ERR_EMPTY:
			strMsg = _T("Cell ID가 비어 있습니다.");
			break;
		case M_CELLID_ERR_LENGTH:
			strMsg.Format (_T("Cell ID '%s' 길이가 최대값(%d)을 초과했습니다."), (LPCTSTR)e.m_strToken, e.m_nLimit);
			break;
		case M_CELLID_ERR_FORMAT:
			strMsg.Format (_T("Cell ID 형식이 올바르지 않습니다. (영문 대문자 + 숫자: '%s')"), (LPCTSTR)e.m_strToken);
			break;
		case M_CELLID_ERR_DERIVE:
			strMsg.Format (_T("A Manual ID '%s'로부터 B Cell ID를 만들 수 없습니다."), (LPCTSTR)e.m_strToken);
			break;
		case M_CELLID_ERR_DUP:
			strMsg.Format (_T("Cell ID '%s' 중복 (Segment %d Stick %d Cell %d (%c))"), (LPCTSTR)e.m_strToken,
						   e.m_nSeg2 + 1, e.m_nStick2 + 1, e.m_nCell2 + 1, PanelChar (e.m_nPanel2));
			break;
		default:
			strMsg.Format (_T("Cell ID 오류 (code %d)"), e.m_nCode);
			break;
	}
	CString strOut;
	strOut.Format (_T("Segment %d Stick %d Cell %d (%c): %s"), e.m_nSeg + 1, e.m_nStick + 1, e.m_nCell + 1, PanelChar (e.m_nPanel), (LPCTSTR)strMsg);
	return strOut;
}

// first nMax errors, one per line, then one "... (n more)" line in Korean for the rest (SaveFile popup uses nMax 5)
CString	C_CellIdEditDlg::FormatErrors (const std::vector<S_CellIdError> &vec, int nMax)
{
	CString strOut;
	int nShow = (int)vec.size ();
	if (nMax >= 0 && nShow > nMax) nShow = nMax;
	for (int n = 0; n < nShow; n++) {
		if (n > 0) strOut += _T("\n");
		strOut += FormatError (vec[n]);
	}
	if ((int)vec.size () > nShow) {
		CString strMore;
		strMore.Format (_T("... 외 %d건"), (int)vec.size () - nShow);
		if (!strOut.IsEmpty ()) strOut += _T("\n");
		strOut += strMore;
	}
	return strOut;
}
