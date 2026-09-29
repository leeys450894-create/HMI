#include "pch.h"
#include "dfcRecipe.h"
#include "SelectCellDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC (C_SelectCellDlg, CDialogEx)

C_SelectCellDlg::C_SelectCellDlg (CWnd* pParent /*=NULL*/) : C_BaseDialog (IDD_SELECT_CELL_DLG, pParent)
{
	m_hWnd = pParent;
	////m_pRecipeMem = NULL;
	////m_pRecipeMem = g_pSysMapView;
}

C_SelectCellDlg::~C_SelectCellDlg ()
{
}

void C_SelectCellDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control (pDX, IDC_GRID_1,   m_gridSelSpec);
	for (int i = 0; i < M_MODE_MAX; i++) DDX_Control (pDX, IDC_RADIO_1 + i, m_rdMode[i]);
}

BEGIN_MESSAGE_MAP (C_SelectCellDlg, CDialogEx)
	ON_WM_ERASEBKGND ()
	ON_WM_DESTROY ()
	ON_COMMAND (IDC_BUTTON_1, OnUpdateCell)
	ON_COMMAND (IDC_BUTTON_2, OnSelectAllCell)	// LYS 2026-08-18
	ON_COMMAND (IDC_BUTTON_3, OnClearAllCell)	// LYS 2026-08-18
	ON_NOTIFY (NM_CLICK,  IDC_GRID_1, OnSelItem)
	ON_COMMAND_RANGE (IDC_RADIO_1, IDC_RADIO_4, OnSelMode)

END_MESSAGE_MAP ()

void C_SelectCellDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL C_SelectCellDlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();

	for (int i = 0; i < M_MODE_MAX; i++) InitButtons (m_rdMode[i]);
	m_rdMode[0].SetCheck (TRUE);
	m_nMode = M_SPEC;

	return TRUE;
}

BOOL	C_SelectCellDlg::ShowWindow (int nCmdShow)
{
	if (nCmdShow == SW_SHOW) {
		SetViewSelectCell ();
	}

	return CDialogEx::ShowWindow (nCmdShow);
}

void	C_SelectCellDlg::OnUpdateCell ()
{
	CString str;

	if		(m_nMode == M_SPEC)		str = _T("특성");
	else if (m_nMode == M_QUALITY)  str = _T("화질");
	else if (m_nMode == M_COLOR)    str = _T("면계측");
	else if (m_nMode == M_ANGLE)	str = _T("시야각");

	if (AfxMessageBox (dfcStr (_T("%s Cell 선택 값을 업데이트 합니다.\n 계속 하시겠습니까?"), str), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	UpdateSelectCell ();
	
}

void	C_SelectCellDlg::InitSelectGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, int nAng)
{
	CRect rc;
	GetDlgItem (nDlgItem)->GetWindowRect (&rc);
	ScreenToClient (&rc);
	DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE;
	
	Grid.SetFont (dfcGetFontName (), 12, FW_BOLD);

	Grid.SetColumnCount (nCol);
	Grid.SetRowCount (nRow);
	
	Grid.SetFixedRowCol (0, 0);

	Grid.EnableToolTips (FALSE);
	Grid.EnableTitleTips (FALSE);
	Grid.SetEditable (FALSE);
	Grid.EnableSelection (FALSE);
	Grid.GetWindowRect (&rc);

	int	nWidth = 0;
	int nHeight = 0;

	nWidth = (rc.Width () - 4) / nCol;
	for (int i = 0; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);

	nHeight = (rc.Height () - 4) / nRow;
	for (int i = 0; i < nRow; i++) Grid.SetRowHeight (i, nHeight);

	for (int i = 0; i < nRow; i++) {
		for (int j = 0; j < nCol; j++) {
			Grid.SetItemFormat (i, j, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}
	//Grid.SetFixedTextColor (COLOR_WHITE);
	//Grid.SetFixedBkColor (COLOR_FRAME);
}

void	C_SelectCellDlg::OnSelItem (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	int nRow = (int) pNMLV->iRow;
	int nCol = (int) pNMLV->iColumn;

	if (m_gridSelSpec.GetItemBkColour (nRow, nCol) == COLOR_SKYBLUE_BRIGHT) {
		m_gridSelSpec.SetItemBkColour (nRow, nCol, COLOR_WHITE);
	}
	else {
		m_gridSelSpec.SetItemBkColour (nRow, nCol, COLOR_SKYBLUE_BRIGHT);
	}

	Invalidate (FALSE);
}

void	C_SelectCellDlg::SetViewSelectCell (/*m_nMode*/)
{
	int nStick = 0, nCell = 0;
	double fRatio = 0.41;

	nStick = g_inRcpInfo.GetStickCount ();
	if (nStick < 0) return;

	nCell  = g_inRcpInfo.GetCellCount (0);

	// LYS 2026-07-20 Support both 0 and 90 degree glass rotation
	// LYS 2026-08-18 rotate 180은 0도 레이아웃을 재사용한다(0/180 종횡비 동일, 90/270만 뒤바뀜).
	// 없으면 90 레시피 뒤 180을 열 때 가로 박스로 그려진다. 표시 전용이라 선택 데이터는 무관.
	if (g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_0 || g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_180) {
		double fRatio0 = min(550.0 / RCP_GLASS_SIZE_X, 380.0 / RCP_GLASS_SIZE_Y);
		m_gridSelSpec.MoveWindow (30, 20, (int)(RCP_GLASS_SIZE_Y * fRatio0), (int)(RCP_GLASS_SIZE_X * fRatio0));
	}
	else if (g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_90 || g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_270) {	// LYS 2026-09-29 Hybrid Phase 2 270은 90 배치 공유(세그먼트 전환 시 빈 창 방지)
		m_gridSelSpec.MoveWindow (180, 100, (int)(RCP_GLASS_SIZE_X * fRatio), (int)(RCP_GLASS_SIZE_Y * fRatio));
	}

	InitSelectGridUI (m_gridSelSpec, IDC_GRID_1, nStick, nCell, 0);

	for (int i = 0; i < nStick; i++) {
		for (int j = 0; j < nCell; j++) {
			m_gridSelSpec.SetItemText (i, j, g_inRcpInfo.GetCellID (i, j));

			if (g_inRcpInfo.GetSelectCell (i, j, m_nMode)) {
				m_gridSelSpec.SetItemBkColour (i, j, COLOR_SKYBLUE_BRIGHT);
			}
			else {
				m_gridSelSpec.SetItemBkColour (i, j, COLOR_WHITE);
			}
		}
	}
}

void	C_SelectCellDlg::OnSelMode (UINT nID)
{
	switch (nID) {
		case IDC_RADIO_1:	m_nMode = M_SPEC;	break;
		case IDC_RADIO_2:	m_nMode = M_QUALITY;break;
		case IDC_RADIO_3:	m_nMode = M_COLOR;	break;
		case IDC_RADIO_4:	m_nMode = M_ANGLE;	break;
	}

	if (nID >= IDC_RADIO_1 && nID <= IDC_RADIO_4) SetViewSelectCell ();
}

//=========================================================================
// LYS 2026-08-18 현재 모드의 전체 선택 / 전체 해제.
// 그리드 색만 바뀌고 레시피 반영은 Cell Update 버튼(UpdateSelectCell)이 담당한다.
// 범위는 그리드가 아니라 레시피에서 가져온다(SetViewSelectCell와 동일).
//=========================================================================
void	C_SelectCellDlg::SetAllCell (BOOL bSelect)
{
	int nStick = 0, nCell = 0;

	nStick = g_inRcpInfo.GetStickCount ();
	if (nStick <= 0) return;

	nCell = g_inRcpInfo.GetCellCount (0);
	if (nCell <= 0) return;

	for (int i = 0; i < nStick; i++) {
		for (int j = 0; j < nCell; j++) {
			m_gridSelSpec.SetItemBkColour (i, j, bSelect ? COLOR_SKYBLUE_BRIGHT : COLOR_WHITE);
		}
	}

	Invalidate (FALSE);
}

void	C_SelectCellDlg::OnSelectAllCell ()
{
	SetAllCell (TRUE);
}

void	C_SelectCellDlg::OnClearAllCell ()
{
	SetAllCell (FALSE);
}

void	C_SelectCellDlg::UpdateSelectCell ()
{
	int nStick = 0, nCell = 0;

	nStick = g_inRcpInfo.GetStickCount ();
	if (nStick < 0) return;

	nCell  = g_inRcpInfo.GetCellCount (0);

	for (int i = 0; i < nStick; i++) {
		for (int j = 0; j < nCell; j++) {
			if (m_gridSelSpec.GetItemBkColour (i, j) == COLOR_SKYBLUE_BRIGHT) {
				g_inRcpInfo.SetSelectCell (i, j, m_nMode, TRUE);
			}
			else {
				g_inRcpInfo.SetSelectCell (i, j, m_nMode, FALSE);
			}
		}
	}
}