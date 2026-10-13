#include "pch.h"
#include "dfcRecipe.h"
#include "MakeCellDlg.h"
#include "CellIdRule.h"	// LYS 2026-10-12 Hybrid Phase 6 CellId_AutoToken for the overlay IDs
#include "afxdialogex.h"
#include <memory>	// LYS 2026-09-29 unique_ptr

IMPLEMENT_DYNAMIC (C_MakeCellDlg, CDialogEx)

C_MakeCellDlg::C_MakeCellDlg (CWnd* pParent /*=NULL*/) : C_BaseDialog (IDD_MAKE_CELL_DLG, pParent)
{
	m_hWnd = pParent;
	//m_pRecipeMem = NULL;
	//m_pRecipeMem = g_pSysMapView;

	CString strCellInfo = theApp.m_iniFile.GetProfileString (_T("FILE"), _T("CELLINFO"));
	m_iniCellInfo = C_IniFile (dfcIniFolder () + strCellInfo);
}

C_MakeCellDlg::~C_MakeCellDlg ()
{
}

void C_MakeCellDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control (pDX, IDC_COMBO_1, m_cmbCellInfoRcp);
	DDX_Control (pDX, IDC_COMBO_2, m_cmbAngle);
	DDX_Control (pDX, IDC_GRID_1,   m_gridCoordHead);
	DDX_Control (pDX, IDC_GRID_2,   m_gridCoordData);
	DDX_Control (pDX, IDC_GRID_3,   m_gridCADData);
	DDX_Control (pDX, IDC_CHECK_1,  m_chkHybrid);	// LYS 2026-09-29 Hybrid Phase 2
	DDX_Control (pDX, IDC_COMBO_3,  m_cmbSegCount);
	DDX_Control (pDX, IDC_STATIC_CELL_OVERLAY, m_wndCellOverlay);	// LYS 2026-09-30 ICP IDC_STATIC_2

}

BEGIN_MESSAGE_MAP (C_MakeCellDlg, CDialogEx)
	ON_WM_ERASEBKGND ()
	ON_WM_DESTROY ()
	ON_CBN_DROPDOWN (IDC_COMBO_1,	OnCmbChangeRcp)
	ON_COMMAND (IDC_BUTTON_1, OnCellInfoOpen)
	ON_COMMAND (IDC_BUTTON_2, OnCellInfoSave)
	ON_COMMAND (IDC_BUTTON_3, OnCellMapUpdate)
	ON_COMMAND (IDC_BUTTON_4, OnRecipeUpdate)
	ON_COMMAND (IDC_BUTTON_5, OnCalcUpdate)
	ON_BN_CLICKED (IDC_CHECK_1, OnHybridCheck)	// LYS 2026-09-29 Hybrid Phase 2
	ON_COMMAND (IDC_BUTTON_6, OnSegCountApply)
	ON_COMMAND (IDC_BUTTON_7, OnSegPrev)
	ON_COMMAND (IDC_BUTTON_8, OnSegNext)

END_MESSAGE_MAP ()

void C_MakeCellDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL C_MakeCellDlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();

	// LYS 2026-09-29 콤보 인덱스 == m_nGlsRotate (0/90/180/270), RecipeMgrDlg GLASS ROTATE와 같은 선택지
	m_cmbAngle.AddString (_T("0˚"));
	m_cmbAngle.AddString (_T("90˚"));
	m_cmbAngle.AddString (_T("180˚"));
	m_cmbAngle.AddString (_T("270˚"));
	m_cmbAngle.SetCurSel (0);

	InitCoordInfo ();
	InitCADInfo ();
	InitHybridUI ();	// LYS 2026-09-29 Hybrid Phase 2
	UpdateCellOverlay ();	// LYS 2026-09-30

	return TRUE;
}

void	C_MakeCellDlg::InitCoordInfo ()
{
	InitGridUI (m_gridCoordHead, IDC_GRID_1, 1, 3, TRUE, TRUE, PIX_COORD_ITEM);
	m_gridCoordHead.SetItemText (0, 0, _T("ITEM"));
	m_gridCoordHead.SetItemText (0, 1, _T("DATA"));
	m_gridCoordHead.SetItemText (0, 2, _T("DESCRIPTION"));

	InitGridUI (m_gridCoordData, IDC_GRID_2, 16, 3, TRUE, FALSE, PIX_COORD_ITEM);
	m_gridCoordData.SetItemText (0,  0, _T("GLASS ROTATE"));
	m_gridCoordData.SetItemText (1,  0, _T("1.GLASS WIDTH(X)"));
	m_gridCoordData.SetItemText (2,  0, _T("2.GLASS HEIGHT(Y)"));
	m_gridCoordData.SetItemText (3,  0, _T("3.CELL SIZE X"));
	m_gridCoordData.SetItemText (4,  0, _T("4.CELL SIZE Y"));
	m_gridCoordData.SetItemText (5,  0, _T("5.CELL STICK COUNT(Y)"));
	m_gridCoordData.SetItemText (6,  0, _T("6.CELL COUNT(X)"));
	m_gridCoordData.SetItemText (7,  0, _T("7.BLOCK DISTANCE"));
	m_gridCoordData.SetItemText (8,  0, _T("8.GLOBAL MARK X"));
	m_gridCoordData.SetItemText (9,  0, _T("9.GLOBAL MARK Y"));
	m_gridCoordData.SetItemText (10, 0, _T("10.CELL CENTER X"));
	m_gridCoordData.SetItemText (11, 0, _T("11.CELL CENTER Y"));
	m_gridCoordData.SetItemText (12, 0, _T("12.CELL to CELL X"));
	m_gridCoordData.SetItemText (13, 0, _T("13.CELL to CELL Y"));
	m_gridCoordData.SetItemText (14, 0, _T("14.PROBE WIDTH"));
	m_gridCoordData.SetItemText (15, 0, _T("15.PROBE HEIGHT"));

	//콤보박스
	m_gridCoordData.SetCellType (0, 1, RUNTIME_CLASS(CGridCellCombo));
	m_gridCoordData.SetItemState(0, 1, m_gridCoordData.GetItemState(0, 1) & GVIS_READONLY);

	//콤보 데이터
	CStringArray ang;
	ang.Add (_T("0˚"));
	ang.Add (_T("90˚"));
	ang.Add (_T("180˚"));	// LYS 2026-09-29 m_nGlsRotate=2
	ang.Add (_T("270˚"));	// LYS 2026-09-29 m_nGlsRotate=3

	CGridCellCombo* pCell = NULL;
	pCell = (CGridCellCombo*)m_gridCoordData.GetCell (0, 1);
	pCell->SetStyle (CBS_DROPDOWNLIST);	
	pCell->SetOptions (ang);
	m_gridCoordData.SetItemCurSel (0, 1, 0);
}

void	C_MakeCellDlg::InitCADInfo ()
{
	InitGridUI (m_gridCADData, IDC_GRID_3, 42, 3, TRUE, FALSE, PIX_COORD_ITEM);
	//m_gridCADData.SetItemText (0,  0, _T("A"));
	//m_gridCADData.SetItemText (1,  0, _T("B"));
	//m_gridCADData.SetItemText (2,  0, _T("C"));
	//m_gridCADData.SetItemText (3,  0, _T("D"));
	//m_gridCADData.SetItemText (4,  0, _T("E"));
	//m_gridCADData.SetItemText (5,  0, _T("F"));
	//m_gridCADData.SetItemText (6,  0, _T("G"));
	//m_gridCADData.SetItemText (7,  0, _T("H"));
	//m_gridCADData.SetItemText (8,  0, _T("I"));
	//m_gridCADData.SetItemText (9,  0, _T("J"));
	//m_gridCADData.SetItemText (10, 0, _T("K"));
	//m_gridCADData.SetItemText (11, 0, _T("L"));
	//m_gridCADData.SetItemText (12, 0, _T("M"));
	//m_gridCADData.SetItemText (13, 0, _T("N"));
	//m_gridCADData.SetItemText (14, 0, _T("O"));
	//m_gridCADData.SetItemText (15, 0, _T("P"));
	//m_gridCADData.SetItemText (16, 0, _T("Q"));
	//m_gridCADData.SetItemText (17, 0, _T("R"));
	//m_gridCADData.SetItemText (18, 0, _T("S"));
	//m_gridCADData.SetItemText (19, 0, _T("T"));
	//m_gridCADData.SetItemText (20, 0, _T("U"));
	//m_gridCADData.SetItemText (21, 0, _T("V"));
	//m_gridCADData.SetItemText (22, 0, _T("W"));
	//m_gridCADData.SetItemText (23, 0, _T("X"));
	//m_gridCADData.SetItemText (24, 0, _T("Y"));
	//m_gridCADData.SetItemText (25, 0, _T("X1"));
	//m_gridCADData.SetItemText (26, 0, _T("X2"));
	//m_gridCADData.SetItemText (27, 0, _T("X3"));
	//m_gridCADData.SetItemText (28, 0, _T("X4"));
	//m_gridCADData.SetItemText (29, 0, _T("X5"));
	//m_gridCADData.SetItemText (30, 0, _T("X6"));
	//m_gridCADData.SetItemText (31, 0, _T("X7"));

	//m_gridCADData.SetItemText (32, 0, _T("PROBEID_1(1)"));
	//m_gridCADData.SetItemText (33, 0, _T("PROBEID_1(2)"));
	//m_gridCADData.SetItemText (34, 0, _T("PROBEID_2(1)"));
	//m_gridCADData.SetItemText (35, 0, _T("PROBEID_2(2)"));
	//m_gridCADData.SetItemText (36, 0, _T("PROBEID_3(1)"));
	//m_gridCADData.SetItemText (37, 0, _T("PROBEID_3(2)"));
	//m_gridCADData.SetItemText (38, 0, _T("PROBEID_4(1)"));
	//m_gridCADData.SetItemText (39, 0, _T("PROBEID_4(2)"));
	//m_gridCADData.SetItemText (40, 0, _T("PROBEID_5(1)"));
	//m_gridCADData.SetItemText (41, 0, _T("PROBEID_5(2)"));

	CString str;
	for (int i = 0; i < CAD_ROW_X1; i++) {
		str.Format (_T("%c"), 65 + i);
		m_gridCADData.SetItemText (CAD_ROW_A + i,  0, str);
	}
	for (int i = 0; i < CAD_ROW_P1_1 - CAD_ROW_X1; i++) {
		str.Format (_T("X%d"), i + 1);
		m_gridCADData.SetItemText (CAD_ROW_X1 + i,  0, str);
	}

	int nCount = 0;
	for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) 
	{
		//for (int j = 0; j < 2; j++)
		//{
		//	str.Format(_T("PROBEID_%d(%d)"), i + 1, j + 1);
		//	m_gridCADData.SetItemText(CAD_ROW_P1_1 + nCount, 0, str);
		//	nCount++;
		//}

		// LYS 2026-07-14 Make Cell P1~5 -> P5
		if (i == 4)
		{
			for (int j = 0; j < 2; j++)
			{
				str.Format(_T("PROBEID_%d(%d)"), i + 1, j + 1);
				m_gridCADData.SetItemText(CAD_ROW_P1_1 + nCount, 0, str);
				nCount++;
			}
		}
		else
		{
			for (int j = 0; j < 2; j++)
			{
				str.Format(_T("-"));
				m_gridCADData.SetItemText(CAD_ROW_P1_1 + nCount, 0, str);
				nCount++;
			}
		}
	}
}

void	C_MakeCellDlg::OnCmbChangeRcp ()
{
	CStringArray arr;
	m_iniCellInfo.GetProfileSectionNames (arr);

	m_cmbCellInfoRcp.ResetContent ();

	for (int i = 0; i < arr.GetSize (); i++) {
		m_cmbCellInfoRcp.AddString (arr.GetAt (i));
	}
}

void	C_MakeCellDlg::OnCellInfoOpen ()
{
	CString strRcpName, str;
	m_cmbCellInfoRcp.GetWindowText (strRcpName);

	if (strRcpName.IsEmpty ()) {
		AfxMessageBox (_T("선택된 레시피가 없습니다."));
		return;
	}

	//파일에서 데이터 가져오기
	//CStringArray strArrCoordData;
	m_strArrCoordData.RemoveAll ();
	m_strArrCoordData.SetSize (CELL_ROW_END);
	m_strArrCoordData.SetAt (CELL_ROW_00, dfcStr (m_iniCellInfo.GetProfileInt		(strRcpName, _T("ROTATE"), 0)));
	m_strArrCoordData.SetAt (CELL_ROW_01, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("GLASSX"), 1500.00)));
	m_strArrCoordData.SetAt (CELL_ROW_02, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("GLASSY"), 925.00)));
	m_strArrCoordData.SetAt (CELL_ROW_03, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLX"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_04, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLY"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_05,			m_iniCellInfo.GetProfileString	(strRcpName, _T("CELLSTICK"), _T("1")));
	m_strArrCoordData.SetAt (CELL_ROW_06,           m_iniCellInfo.GetProfileString  (strRcpName, _T("CELLROW"), _T("1")));		// jsyoon 2022.08.25 change
//	m_strArrCoordData.SetAt (CELL_ROW_06, dfcStr (m_iniCellInfo.GetProfileInt		(strRcpName, _T("CELLROW"), 1)));
	m_strArrCoordData.SetAt (CELL_ROW_07,			m_iniCellInfo.GetProfileString	(strRcpName, _T("BLOCKDX"), _T("")));
	m_strArrCoordData.SetAt (CELL_ROW_08, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("GLOBALX"), -733.96)));
	m_strArrCoordData.SetAt (CELL_ROW_09, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("GLOBALY"), -443.00)));
	m_strArrCoordData.SetAt (CELL_ROW_10, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLOX"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_11, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLOY"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_12, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLDX"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_13, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("CELLDY"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_14,	dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("PROBEY"), 0.0)));
	m_strArrCoordData.SetAt (CELL_ROW_15,	dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, _T("PROBEX"), 0.0)));

	CStringArray strArrCADData;
	strArrCADData.RemoveAll ();
	strArrCADData.SetSize (CAD_ROW_END);
	for (int i = 0; i < CAD_ROW_X1; i++) {
		str.Format (_T("%c"), 65 + i);
		strArrCADData.SetAt (CAD_ROW_A + i, dfcStr (m_iniCellInfo.GetProfileDouble (strRcpName, str, 0.0)));
	}
	for (int i = 0; i < CAD_ROW_P1_1 - CAD_ROW_X1; i++) {
		str.Format (_T("X%d"), i + 1);
		strArrCADData.SetAt (CAD_ROW_X1 + i, dfcStr (m_iniCellInfo.GetProfileDouble	(strRcpName, str, 0.0)));
	}
	for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {
		str.Format (_T("PROBEID%d"), i + 1);
		strArrCADData.SetAt (CAD_ROW_P1_1 + i, m_iniCellInfo.GetProfileString (strRcpName, str, _T("")));
	}

	//UI 데이터 표시
	for (int i = 0; i < CELL_ROW_END; i++) {
		if		(i == CELL_ROW_00) m_gridCoordData.SetItemCurSel (i, 1, _wtoi (m_strArrCoordData.GetAt (i)));
	//	else if (i == CELL_ROW_06) m_gridCoordData.SetItemTextFmt (i, 1, _T("%d"), _wtoi (m_strArrCoordData.GetAt (i)));	// jsyoon 2022.08.25 change
		else if (i == CELL_ROW_05 || i == CELL_ROW_06 || i == CELL_ROW_07) m_gridCoordData.SetItemTextFmt (i, 1, m_strArrCoordData.GetAt (i));
		else		m_gridCoordData.SetItemTextFmt (i, 1, _T("%.4lf"), _wtof (m_strArrCoordData.GetAt (i)));
	}
	for (int i = 0; i < CAD_ROW_END; i++) {
		if (i > CAD_ROW_X7) m_gridCADData.SetItemText (i,  1, strArrCADData.GetAt (i));
		else				m_gridCADData.SetItemTextFmt (i, 1, _T("%.4lf"), _wtof (strArrCADData.GetAt (i)));
	}


	// LYS 2026-09-29 CALC 각도 콤보를 불러온 ROTATE에 맞춘다 (OnCalcUpdate는 그리드가 아니라 콤보를 읽는다)
	{
		int nRot = _wtoi (m_strArrCoordData.GetAt (CELL_ROW_00));
		if (nRot >= 0 && nRot < m_cmbAngle.GetCount ()) m_cmbAngle.SetCurSel (nRot);
	}

	// 2022.12.20 add
	CString strTemp;
	if ((_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) % 2) == 0)	// LYS 2026-09-29 0/180은 0도 기준을 공유
	{
		m_gridCoordData.SetItemText(CELL_ROW_03, 2, _T("X6"));
		m_gridCoordData.SetItemText(CELL_ROW_04, 2, _T("X7"));

		//strTemp.Format(_T("-(%.4lf / 2 - F)"), RCP_GLASS_SIZE_Y);
		strTemp.Format(_T("-(%.4lf / 2 - E)"), RCP_GLASS_SIZE_Y); 
		m_gridCoordData.SetItemText(CELL_ROW_08, 2, strTemp);

		//strTemp.Format(_T("-(%.4lf / 2 - E)"), RCP_GLASS_SIZE_X);
		strTemp.Format(_T("-(%.4lf / 2 - F)"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_09, 2, strTemp);

		
		m_gridCoordData.SetItemText(CELL_ROW_10, 2, _T("X2-F"));
		m_gridCoordData.SetItemText(CELL_ROW_11, 2, _T("X1-E"));
		m_gridCoordData.SetItemText(CELL_ROW_12, 2, _T("T"));
		m_gridCoordData.SetItemText(CELL_ROW_13, 2, _T("S"));
	}
	else
	{
		m_gridCoordData.SetItemText(CELL_ROW_03, 2, _T("X7"));
		m_gridCoordData.SetItemText(CELL_ROW_04, 2, _T("X6"));

		strTemp.Format(_T("-(%.4lf / 2 - F)"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_08, 2, strTemp);

		strTemp.Format(_T("-(%.4lf / 2 - E)"), RCP_GLASS_SIZE_Y);
		m_gridCoordData.SetItemText(CELL_ROW_09, 2, strTemp);

		m_gridCoordData.SetItemText(CELL_ROW_10, 2, _T("X2-F"));
		m_gridCoordData.SetItemText(CELL_ROW_11, 2, _T("X1-E"));
		m_gridCoordData.SetItemText(CELL_ROW_12, 2, _T("T"));
		m_gridCoordData.SetItemText(CELL_ROW_13, 2, _T("S"));
	}

	// LYS 2026-09-29 Hybrid Phase 2 OPEN 한 슬롯만 하이브리드 RECIPE APPLY에서 재생성된다
	if (m_nSegView >= 0 && m_nSegView < (int)m_vecSegInput.size ()) m_vecSegInput[m_nSegView].bOpened = TRUE;
	UpdateHybridUI ();
	UpdateCellOverlay ();	// LYS 2026-09-30 ICP OnCellInfoOpen -> UpdateGlassMap

	Invalidate (FALSE);
}

void	C_MakeCellDlg::OnCellInfoSave ()
{
	if (AfxMessageBox (_T("Cell 정보를 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strRcpName;
	m_cmbCellInfoRcp.GetWindowText (strRcpName);

	if (strRcpName.IsEmpty ()) {
		AfxMessageBox (_T("선택된 레시피가 없습니다."));
		return;
	}

	int iRotate = m_gridCoordData.GetItemCurSel(CELL_ROW_00, 1);
	m_iniCellInfo.WriteProfileInt(strRcpName, _T("ROTATE"), iRotate );

//	m_iniCellInfo.WriteProfileInt	 (strRcpName, _T("ROTATE"), _wtoi (m_gridCoordData.GetItemText (CELL_ROW_00, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("GLASSX"), _wtof (m_gridCoordData.GetItemText (CELL_ROW_01, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("GLASSY"), _wtof (m_gridCoordData.GetItemText (CELL_ROW_02, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLX"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_03, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLY"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_04, 1)));
	m_iniCellInfo.WriteProfileString (strRcpName, _T("CELLSTICK"), m_gridCoordData.GetItemText (CELL_ROW_05, 1));
	m_iniCellInfo.WriteProfileString (strRcpName, _T("CELLROW"),   m_gridCoordData.GetItemText (CELL_ROW_06, 1));	// jsyoon 2022.08.25 change
//	m_iniCellInfo.WriteProfileInt	 (strRcpName, _T("CELLROW"), _wtoi (m_gridCoordData.GetItemText (CELL_ROW_06, 1)));
	m_iniCellInfo.WriteProfileString (strRcpName, _T("BLOCKDX"), m_gridCoordData.GetItemText (CELL_ROW_07, 1));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("GLOBALX"), _wtof (m_gridCoordData.GetItemText (CELL_ROW_08, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("GLOBALY"), _wtof (m_gridCoordData.GetItemText (CELL_ROW_09, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLOX"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_10, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLOY"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_11, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLDX"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_12, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("CELLDY"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_13, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("PROBEY"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_14, 1)));
	m_iniCellInfo.WriteProfileDouble (strRcpName, _T("PROBEX"),  _wtof (m_gridCoordData.GetItemText (CELL_ROW_15, 1)));

	CString str;
	for (int i = 0; i < CAD_ROW_X1; i++) {
		str.Format (_T("%c"), 65 + i);
		m_iniCellInfo.WriteProfileDouble (strRcpName, str, _wtof (m_gridCADData.GetItemText (CAD_ROW_A + i, 1)));
	}
	for (int i = 0; i < CAD_ROW_P1_1 - CAD_ROW_X1; i++) {
		str.Format (_T("X%d"), i + 1);
		m_iniCellInfo.WriteProfileDouble (strRcpName, str, _wtof (m_gridCADData.GetItemText (CAD_ROW_X1 + i, 1)));
	}
	for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {

		str.Format(_T("PROBEID%d"), i + 1);
		m_iniCellInfo.WriteProfileString(strRcpName, str, m_gridCADData.GetItemText(CAD_ROW_P1_1 + i, 1));
		// LYS 2026-07-14 Make Cell P1~5 -> P5
		//if (i == CAD_ROW_END - CAD_ROW_P5_1 || CAD_ROW_END - CAD_ROW_P5_2)
		//{
		//	str.Format(_T("PROBEID%d"), i + 1);
		//	m_iniCellInfo.WriteProfileString(strRcpName, str, m_gridCADData.GetItemText(CAD_ROW_P1_1 + i, 1));
		//}
		//else
		//{
		//	str.Format(_T("-"));
		//	m_iniCellInfo.WriteProfileString(strRcpName, str, m_gridCADData.GetItemText(CAD_ROW_P1_1 + i, 1));
		//}

	}

}

void	C_MakeCellDlg::OnCellMapUpdate ()
{

}

void	C_MakeCellDlg::OnRecipeUpdate ()
{
	// LYS 2026-09-29 Hybrid Phase 2 하이브리드 모드는 세그먼트별 입력으로 레시피 전체를 만든다
	if (IsHybridEdit ()) {
		ApplyHybridRecipe ();
		return;
	}

	if (AfxMessageBox (_T("현재 정보로 레시피를 생성 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strRcpName;
	m_cmbCellInfoRcp.GetWindowText (strRcpName);

	if (strRcpName.IsEmpty ()) {
		AfxMessageBox (_T("선택된 레시피가 없습니다."));
		return;
	}

	// LYS 2026-09-29 Hybrid Phase 2 하이브리드 해제 후 적용: 해제할 때 보고 있던 세그먼트를 기준으로 1세그먼트 레시피를 만든다
	int  nOldActive = g_inRcpInfo.GetActiveSegmentIndex ();	// LYS 2026-09-29
	BOOL bCollapse = (g_inRcpInfo.GetSegmentCount () > 1);
	if (bCollapse) g_inRcpInfo.SetActiveSegment (m_nSingleSrcSeg);

	std::unique_ptr<S_RecipeMgr> pBefore (new S_RecipeMgr (g_inRcpInfo.m_recipeMgr));	// LYS 2026-09-29 좌표 검사 실패 시 되돌린다 (힙 - 스택 오버플로 방지)
	BuildRecipeFromGrid (strRcpName);

	// LYS 2026-09-29 셀/스틱 좌표가 글라스 밖이면 적용하지 않는다 (셀 정보 오류로 헤드 Y SW 리밋에 걸린 사고 재발 방지)
	{
		CString strLayoutErr = g_inRcpInfo.CheckRecipeLayout (&g_inRcpInfo.m_recipeMgr, _T("레시피"));
		if (!strLayoutErr.IsEmpty ()) {
			g_inRcpInfo.m_recipeMgr = *pBefore;
			if (bCollapse) g_inRcpInfo.SetActiveSegment (nOldActive);
			AfxMessageBox (_T("레시피 생성 불가 (셀 좌표 검사)\n\n") + strLayoutErr, MB_ICONWARNING);
			return;
		}
	}

	if (bCollapse) g_inRcpInfo.CollapseToActiveSegment ();
	// LYS 2026-10-01 Hybrid Phase 4b rebuilt cells are not proven to be the same physical cells (ICP 4.13, plan 5.7) -> Cell ID back to Auto/Auto
	if (g_inRcpInfo.ResetCellIdConfig ()) AfxMessageBox (_T("셀 구성이 바뀌어 Cell ID Manual 설정(A/B 모드·수동 ID)을 초기화했습니다.\nCELL ID 편집기에서 다시 입력하십시오."), MB_ICONINFORMATION);

	::SendMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_MAKE_CELL_SET, NULL);

	ShowWindow (SW_HIDE);
}

void	C_MakeCellDlg::OnCalcUpdate ()
{
	if (AfxMessageBox (_T("CAD 설계값을 계산 하여 업데이트 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strTemp;
	double fTemp1 = 0.0, fTemp2 = 0.0;
	int nAngle = 0;

	nAngle = m_cmbAngle.GetCurSel ();
	if (nAngle < 0) {	// LYS 2026-09-29 CBS_DROPDOWN은 직접 입력이 가능 -> 선택 없음 처리
		AfxMessageBox (_T("GLASS ROTATE 각도를 선택하세요."));
		return;
	}

	// LYS 2026-09-29 Hybrid Phase 2 OPEN 없이 CAD APPLY 하면 m_strArrCoordData가 비어 GetAt 예외 - 새 세그먼트 슬롯에서 쉽게 밟는다
	if (m_strArrCoordData.GetSize () < CELL_ROW_END) {
		AfxMessageBox (_T("먼저 셀 정보를 OPEN 하세요."));
		return;
	}

	m_gridCoordData.SetItemCurSel (CELL_ROW_00, 1, nAngle);


	m_gridCoordData.SetItemText (CELL_ROW_05,  1, _T(""));
	m_gridCoordData.SetItemText (CELL_ROW_06,  1, _T(""));
	m_gridCoordData.SetItemText (CELL_ROW_07,  1, _T(""));

	// LYS 2026-08-18 rotate 180 shares the 0-degree basis (same physical aspect)
	if (nAngle == RCP_CELL_ANG_0 || nAngle == RCP_CELL_ANG_180) {

		strTemp.Format(_T("%.4lf"), RCP_GLASS_SIZE_Y); 
		m_gridCoordData.SetItemText(CELL_ROW_01, 1, strTemp);
		strTemp.Format(_T("%.4lf"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_02, 1, strTemp);

	//	fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));
		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_E, 1));
		strTemp.Format(_T("%.4lf"), -((RCP_GLASS_SIZE_Y / 2.0) - fTemp1));
		m_gridCoordData.SetItemText(CELL_ROW_08, 1, strTemp);

		strTemp.Format(_T("-(%.4lf / 2 - E)"), RCP_GLASS_SIZE_Y);
		m_gridCoordData.SetItemText(CELL_ROW_08, 2, strTemp);


	//	fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_E, 1));
		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));
		strTemp.Format(_T("%.4lf"), -((RCP_GLASS_SIZE_X / 2.0) - fTemp1));
		m_gridCoordData.SetItemText(CELL_ROW_09, 1, strTemp);

		strTemp.Format(_T("-(%.4lf / 2 - F)"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_09, 2, strTemp);

		m_gridCoordData.SetItemText(CELL_ROW_03, 1, m_gridCADData.GetItemText(CAD_ROW_X6, 1));	m_gridCoordData.SetItemText(CELL_ROW_03, 2, _T("X6"));
		m_gridCoordData.SetItemText(CELL_ROW_04, 1, m_gridCADData.GetItemText(CAD_ROW_X7, 1));  m_gridCoordData.SetItemText(CELL_ROW_04, 2, _T("X7"));
		/*m_gridCoordData.SetItemText (CELL_ROW_03,  1, m_gridCADData.GetItemText (CAD_ROW_Q, 1));
		m_gridCoordData.SetItemText (CELL_ROW_04,  1, m_gridCADData.GetItemText (CAD_ROW_R, 1));*/

		//
		if ((_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) % 2) != (nAngle % 2))	// LYS 2026-09-29 종횡비가 바뀔 때만 스틱/셀을 맞바꾼다
		{
			m_gridCoordData.SetItemText(CELL_ROW_05, 1, m_strArrCoordData.GetAt(CELL_ROW_06));
			m_gridCoordData.SetItemText(CELL_ROW_06, 1, m_strArrCoordData.GetAt(CELL_ROW_05));
			m_gridCoordData.SetItemText(CELL_ROW_07, 1, m_strArrCoordData.GetAt(CELL_ROW_07));
		}
		else
		{
			m_gridCoordData.SetItemText(CELL_ROW_05, 1, m_strArrCoordData.GetAt(CELL_ROW_05));
			m_gridCoordData.SetItemText(CELL_ROW_06, 1, m_strArrCoordData.GetAt(CELL_ROW_06));
			m_gridCoordData.SetItemText(CELL_ROW_07, 1, m_strArrCoordData.GetAt(CELL_ROW_07));
		}
		//

	/*	fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_X2, 1));
		fTemp2 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));
		strTemp.Format(_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText(CELL_ROW_10, 1, strTemp);
		m_gridCoordData.SetItemText(CELL_ROW_10, 2, _T("X2-F"));

		fTemp1 = _wtof (m_gridCADData.GetItemText (CAD_ROW_X1, 1));
		fTemp2 = _wtof (m_gridCADData.GetItemText (CAD_ROW_E, 1));
		strTemp.Format (_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText (CELL_ROW_11,  1, strTemp);
		m_gridCoordData.SetItemText(CELL_ROW_11, 2, _T("X1-E"));*
		
		m_gridCoordData.SetItemText (CELL_ROW_12,  1, m_gridCADData.GetItemText (CAD_ROW_T, 1));  
		m_gridCoordData.SetItemText(CELL_ROW_12, 2, _T("T"));

		m_gridCoordData.SetItemText (CELL_ROW_13,  1, m_gridCADData.GetItemText (CAD_ROW_S, 1));
		m_gridCoordData.SetItemText(CELL_ROW_13, 2, _T("S")); */

		// 2023.02.03 change
		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_X2, 1));
		fTemp2 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));
		strTemp.Format(_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText(CELL_ROW_11, 1, strTemp);
		m_gridCoordData.SetItemText(CELL_ROW_11, 2, _T("X2-F"));

		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_X1, 1));
		fTemp2 = _wtof(m_gridCADData.GetItemText(CAD_ROW_E, 1));
		strTemp.Format(_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText(CELL_ROW_10, 1, strTemp);
		m_gridCoordData.SetItemText(CELL_ROW_10, 2, _T("X1-E"));


		m_gridCoordData.SetItemText (CELL_ROW_13,  1, m_gridCADData.GetItemText (CAD_ROW_T, 1));  
		m_gridCoordData.SetItemText(CELL_ROW_13, 2, _T("T"));

		m_gridCoordData.SetItemText (CELL_ROW_12,  1, m_gridCADData.GetItemText (CAD_ROW_S, 1));
		m_gridCoordData.SetItemText(CELL_ROW_12, 2, _T("S"));

		m_gridCoordData.SetItemText(CELL_ROW_14, 1, m_gridCADData.GetItemText(CAD_ROW_A, 1));
		m_gridCoordData.SetItemText(CELL_ROW_14, 2, _T("A"));
	}
	else if (nAngle == RCP_CELL_ANG_90 || nAngle == RCP_CELL_ANG_270)	// LYS 2026-09-29 270은 90 기준을 공유
	{
		strTemp.Format(_T("%.4lf"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_01, 1, strTemp);
		

		strTemp.Format(_T("%.4lf"), RCP_GLASS_SIZE_Y);
		m_gridCoordData.SetItemText(CELL_ROW_02, 1, strTemp);

	//	fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_E, 1));
		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));   // 2022.12.20 change
		strTemp.Format(_T("%.4lf"), -((RCP_GLASS_SIZE_X / 2.0) - fTemp1));
		m_gridCoordData.SetItemText(CELL_ROW_08, 1, strTemp);

		strTemp.Format(_T("-(%.4lf / 2 - F)"), RCP_GLASS_SIZE_X);
		m_gridCoordData.SetItemText(CELL_ROW_08, 2, strTemp);

	//	fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_F, 1));
		fTemp1 = _wtof(m_gridCADData.GetItemText(CAD_ROW_E, 1));	// 2022.12.20 change
		strTemp.Format(_T("%.4lf"), -((RCP_GLASS_SIZE_Y / 2.0) - fTemp1));
		m_gridCoordData.SetItemText(CELL_ROW_09, 1, strTemp);

		strTemp.Format(_T("-(%.4lf / 2 - E)"), RCP_GLASS_SIZE_Y);
		m_gridCoordData.SetItemText(CELL_ROW_09, 2, strTemp);


		// CELL SIZE
		m_gridCoordData.SetItemText(CELL_ROW_03, 1, m_gridCADData.GetItemText(CAD_ROW_X7, 1));	m_gridCoordData.SetItemText(CELL_ROW_03, 2, _T("X7"));

		m_gridCoordData.SetItemText(CELL_ROW_04, 1, m_gridCADData.GetItemText(CAD_ROW_X6, 1));	m_gridCoordData.SetItemText(CELL_ROW_04, 2, _T("X6"));

		//m_gridCoordData.SetItemText (CELL_ROW_03,  1, m_gridCADData.GetItemText (CAD_ROW_R, 1));
		//m_gridCoordData.SetItemText (CELL_ROW_04,  1, m_gridCADData.GetItemText (CAD_ROW_Q, 1));

		//
		if ((_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) % 2) != (nAngle % 2))	// LYS 2026-09-29 종횡비가 바뀔 때만 스틱/셀을 맞바꾼다
		{
			m_gridCoordData.SetItemText(CELL_ROW_05, 1, m_strArrCoordData.GetAt(CELL_ROW_06));	// STICK
			m_gridCoordData.SetItemText(CELL_ROW_06, 1, m_strArrCoordData.GetAt(CELL_ROW_05));  // CELL 
			m_gridCoordData.SetItemText(CELL_ROW_07, 1, m_strArrCoordData.GetAt(CELL_ROW_07));  // BLOCK
		}
		else
		{
			m_gridCoordData.SetItemText(CELL_ROW_05, 1, m_strArrCoordData.GetAt(CELL_ROW_05));	// STICK
			m_gridCoordData.SetItemText(CELL_ROW_06, 1, m_strArrCoordData.GetAt(CELL_ROW_06));  // CELL 
			m_gridCoordData.SetItemText(CELL_ROW_07, 1, m_strArrCoordData.GetAt(CELL_ROW_07));  // BLOCK
		}
		//

		fTemp1 = _wtof (m_gridCADData.GetItemText (CAD_ROW_X2, 1));
		fTemp2 = _wtof (m_gridCADData.GetItemText (CAD_ROW_F, 1));
		strTemp.Format (_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText(CELL_ROW_10,  1, strTemp);			// CELL CENTOR

		m_gridCoordData.SetItemText(CELL_ROW_10, 2, _T("X2-F"));

		fTemp1 = _wtof (m_gridCADData.GetItemText (CAD_ROW_X1, 1));
		fTemp2 = _wtof (m_gridCADData.GetItemText (CAD_ROW_E, 1));
		strTemp.Format (_T("%.4lf"), fTemp1 - fTemp2);
		m_gridCoordData.SetItemText(CELL_ROW_11,  1, strTemp);			// CELL CENTOR

		m_gridCoordData.SetItemText(CELL_ROW_11, 2, _T("X1-E"));

		//m_gridCoordData.SetItemText(CELL_ROW_12, 1, m_gridCADData.GetItemText(CAD_ROW_S, 1));	// CELL to CELL
		//m_gridCoordData.SetItemText(CELL_ROW_13, 1, m_gridCADData.GetItemText(CAD_ROW_T, 1));	// CELL to CELL

		m_gridCoordData.SetItemText (CELL_ROW_12,  1, m_gridCADData.GetItemText (CAD_ROW_T, 1));	// CELL to CELL		
		m_gridCoordData.SetItemText(CELL_ROW_12, 2, _T("T"));

		m_gridCoordData.SetItemText (CELL_ROW_13,  1, m_gridCADData.GetItemText (CAD_ROW_S, 1));	// CELL to CELL
		m_gridCoordData.SetItemText(CELL_ROW_13, 2, _T("S"));

		m_gridCoordData.SetItemText(CELL_ROW_14, 1, m_gridCADData.GetItemText(CAD_ROW_B, 1));
		m_gridCoordData.SetItemText(CELL_ROW_14, 2, _T("B"));
	}

	m_gridCoordData.SetItemText(CELL_ROW_15, 1, _T("20.0000"));

	UpdateCellOverlay ();	// LYS 2026-09-30 ICP OnCalcUpdate -> UpdateGlassMap

	Invalidate (FALSE);
}

void	C_MakeCellDlg::GetCadInfo ()
{
	S_RecipeMgr *rcp = &g_inRcpInfo.m_recipeMgr;

	//CAD_ROW_A ~ CAD_ROW_X7
	for (int i = 0; i < CAD_ROW_P1_1; i++) {
		rcp->m_sMap.m_fCadData[i] = _wtof (m_gridCADData.GetItemText (i, 1));
	}

	//CAD_ROW_P1_1 ~ CAD_ROW_P5_2
	for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {
		wcscpy_s (rcp->m_sMap.m_chCadData[i], m_gridCADData.GetItemText (CAD_ROW_P1_1 + i, 1));
	}
}

// LYS 2026-09-29 Hybrid Phase 2 레시피 생성 본문 - OnRecipeUpdate에서 코드 이동(논리 무변경). 그리드 입력으로 g_inRcpInfo.m_recipeMgr(작업 슬롯)를 만든다.
//   맵/스틱/셀/CAD/회전/오프셋만 새로 쓰고 옵션(검사 항목, 램프, 컨택 등)은 작업 슬롯 값을 그대로 둔다(기존 동작).
void	C_MakeCellDlg::BuildRecipeFromGrid (CString strRcpName)
{
	g_inRcpInfo.SetRcpVer (0);
	g_inRcpInfo.ClearRcpMap ();
	g_inRcpInfo.SetModelInfoName (strRcpName);
	
	GetCadInfo (); //CAD 정보 저장해놓기 PPID 전송용

	S_RecipeMgr *rcp = &g_inRcpInfo.m_recipeMgr;
	//g_RecipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate	= m_gridCoordData.GetItemCurSel (CELL_ROW_00, 1);
	g_inRcpInfo.SetGlsRotate (m_gridCoordData.GetItemCurSel (CELL_ROW_00, 1));
	rcp->m_sMap.m_fCellSize.x				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_03, 1));
	rcp->m_sMap.m_fCellSize.y				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_04, 1));
//	rcp->m_sMap.m_fCellCount.y				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_06, 1));
//	rcp->m_sMap.m_fCellCount.y				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_05, 1));		// Stick 
	rcp->m_sMap.m_fGlobalMark.x				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_08, 1));
	rcp->m_sMap.m_fGlobalMark.y				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_09, 1));
	rcp->m_sMap.m_fCellCenter.x				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_10, 1));
	rcp->m_sMap.m_fCellCenter.y				= _wtof (m_gridCoordData.GetItemText (CELL_ROW_11, 1));
	rcp->m_sMap.m_fCellDistance.x			= _wtof (m_gridCoordData.GetItemText (CELL_ROW_12, 1));
	rcp->m_sMap.m_fCellDistance.y			= _wtof (m_gridCoordData.GetItemText (CELL_ROW_13, 1));
	rcp->m_sInfo.m_sProbeInfo.m_fProbeH		= _wtof (m_gridCoordData.GetItemText (CELL_ROW_14, 1));
	rcp->m_sInfo.m_sProbeInfo.m_fProbeW		= _wtof (m_gridCoordData.GetItemText (CELL_ROW_15, 1));
	
	// LYS 2026-08-18 rotate 180 shares the 0-degree basis (same physical aspect)
	if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_0 || g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_180) {
		wcscpy_s (rcp->m_sMap.m_chCellCount,	m_gridCoordData.GetItemText(CELL_ROW_05, 1));
	}
	else if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_90 || g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_270) {	// LYS 2026-09-29
		wcscpy_s(rcp->m_sMap.m_chCellCount, m_gridCoordData.GetItemText(CELL_ROW_06, 1));	// CELL 
	}

	wcscpy_s (rcp->m_sMap.m_chCellDistance, m_gridCoordData.GetItemText(CELL_ROW_07, 1));

	double fReviewOffseet = 0.0, fReviewOffseet2 = 0.0;

	double fX1 = 0.0, fX2 = 0.0, fE = 0.0, fF = 0.0, fI = 0.0, fJ = 0.0, fM = 0.0, fN = 0.0;
	fX1 = _wtof (m_gridCADData.GetItemText (CAD_ROW_X1, 1));
	fX2 = _wtof (m_gridCADData.GetItemText (CAD_ROW_X2, 1));
	fE  = _wtof (m_gridCADData.GetItemText (CAD_ROW_E, 1));
	fF  = _wtof (m_gridCADData.GetItemText (CAD_ROW_F, 1));
	fI  = _wtof (m_gridCADData.GetItemText (CAD_ROW_I, 1));
	fJ  = _wtof (m_gridCADData.GetItemText (CAD_ROW_J, 1));
	fM  = _wtof (m_gridCADData.GetItemText (CAD_ROW_M, 1));
	fN  = _wtof (m_gridCADData.GetItemText (CAD_ROW_N, 1));

	rcp->m_sMap.m_fAlignKey.x = rcp->m_sMap.m_fGlobalMark.x;
	rcp->m_sMap.m_fAlignKey.y = rcp->m_sMap.m_fGlobalMark.y;

	// LYS 2026-08-18 rotate 180은 0도와 같은 기준을 쓴다(물리적 종횡비 동일).
	// 주의: 좌/우 리뷰 오프셋은 180에서 반전되므로 실기 전 기구도 대조 필요.
	if (g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_0 || g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_180) { 
		// 2023.02.03 change
		//rcp->m_sMap.m_fContactOffset = fX1 - fE - fI + fM;
		//fReviewOffseet = fX2 - fF - fJ + fN;
		fReviewOffseet = fX1 - fE - fI + fM;
		rcp->m_sMap.m_fContactOffset = fX2 - fF - fJ + fN;
		/*rcp->m_sMap.m_fContactOffset = fX1 - fF - fJ + fN;
		fReviewOffseet  = fX1 - fE - fI + fM;*/

		//rcp->m_sMap.m_fAlignKey.x = rcp->m_sMap.m_fGlobalMark.y;
		//rcp->m_sMap.m_fAlignKey.y = rcp->m_sMap.m_fGlobalMark.x;
	}
	else if (g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_90 || g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_270) {	// LYS 2026-09-29
		rcp->m_sMap.m_fContactOffset = fX1 - fE - fI + fM;
		fReviewOffseet	= fX2 - fF - fJ + fN;

		//rcp->m_sMap.m_fAlignKey.x = rcp->m_sMap.m_fGlobalMark.x;
		//rcp->m_sMap.m_fAlignKey.y = rcp->m_sMap.m_fGlobalMark.y;
	}

	// jsyoon 2022.08.25 add
	fReviewOffseet2 = rcp->m_sMap.m_fContactOffset;

	// LYS 2026-08-18 rotate 180은 0도 분기를 공유하면 안 되고 자체 값이 필요하다.
	// 2026-08-07 체계에서 측정 각도는 stick m_fStageR에서 오므로 USER OFFSET은 0.
	// 이 분기가 없어 rotate=2가 대입 없이 통과, 이전 90 값이 남던 것이 단변 270도 오위치의 원인.
	// LYS 2026-09-29 측정 각도 = 스틱 m_fStageR (= rotate*90, 아래에서 설정). USER OFFSET은 모든 각도에서 0.
	// 예전 0도 값 90 때문에 rotate 0이 90에서 측정됐고 장변/단변 기준 레시피와 맞지 않았다.
	g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate = 0;

	g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewY = fReviewOffseet2;
	///

	g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewLeft  = -fReviewOffseet;
	g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewRight = fReviewOffseet;

	//프로브 ID
	// LYS 2026-07-21 P5 only (index 4)
	wcscpy_s (rcp->m_sInfo.m_sProbeInfo.m_chProbeFirst[4],  m_gridCADData.GetItemText (CAD_ROW_P5_1, 1));
	wcscpy_s (rcp->m_sInfo.m_sProbeInfo.m_chProbeSecond[4], m_gridCADData.GetItemText (CAD_ROW_P5_2, 1));

	CString str;
	CStringArray strArrayBlock, strArrayStick, strArrayOffset;
	int nBegin = 0, nEnd = 0;

	//Block Distance
	str = m_gridCoordData.GetItemText (CELL_ROW_07, 1);
	strArrayBlock.RemoveAll ();
	for (int i =0; i <= str.GetLength (); i++) {
		if (str.GetAt (i) == ',' || i == str.GetLength ()) {
			nEnd = i - nBegin;
			strArrayBlock.Add (str.Mid (nBegin, nEnd).Trim ());
			nBegin = i + 1;
		}
	}

	// Stick
	nBegin = 0;
	str = m_gridCoordData.GetItemText(CELL_ROW_05, 1);
	strArrayStick.RemoveAll();
	for (int i = 0; i <= str.GetLength(); i++) {
		if (str.GetAt(i) == ',' || i == str.GetLength()) {
			nEnd = i - nBegin;
			strArrayStick.Add(str.Mid(nBegin, nEnd).Trim());
			nBegin = i + 1;
		}
	}

	//Stick 개수
	int nStickCnt = 0;
	for (int i = 0; i < strArrayStick.GetSize(); i++) nStickCnt += _wtoi(strArrayStick.GetAt(i));

	rcp->m_sMap.m_fCellCount.y = (double)nStickCnt;			// Stick


	//Cell Offset
	nBegin = 0;
//	str = m_gridCoordData.GetItemText (CELL_ROW_05, 1);
	str = m_gridCoordData.GetItemText(CELL_ROW_06, 1);		// CELL
	strArrayOffset.RemoveAll ();
	for (int i =0; i <= str.GetLength (); i++) {
		if (str.GetAt (i) == ',' || i == str.GetLength ()) {
			nEnd = i - nBegin;
			strArrayOffset.Add (str.Mid (nBegin, nEnd).Trim ());
			nBegin = i + 1;
		}
	}

	//Cell 개수
	int nCellCnt = 0;
	for (int i = 0; i < strArrayOffset.GetSize (); i++) nCellCnt += _wtoi (strArrayOffset.GetAt(i));

	rcp->m_sMap.m_fCellCount.x = (double)nCellCnt;			// Cell


	//Recipe.stCellInfo.CellCount.x = (float)iX; 
	//Recipe.stCellInfo.CellNum = nGroupCnt;

	//Direction
	//int nOldCllX = 0;
	//double fBlockDistance = 0.0;

	//double fShiftX = 0.0, fShiftY = 0.0;
	//int iDX = 1, iDY = 1;
	//if (g_RecipeMgr.m_sMap.m_fGlobalMark.x > 0) {
	//	iDX = -1;
	//	fShiftX = g_RecipeMgr.m_sMap.m_fCellSize.x;
	//}

	//if (g_RecipeMgr.m_sMap.m_fGlobalMark.y > 0) {
	//	iDY = -1;
	//	fShiftY = g_RecipeMgr.m_sMap.m_fCellSize.y;
	//}

	//좌표 넣기
	g_inRcpInfo.ClearRcpData ();
	S_StickInfo		sStick;		//임시 저장
	S_CellInfo		sCell;		//임시 저장


	double fTemp = 0.0;
	int nOffsetPos = 0;
	int nBlockCnt = (int)strArrayBlock.GetSize();
	nStickCnt = (int)strArrayStick.GetSize();

	int nAdd = 0;
	BOOL bOffsetStick = FALSE;
	if (nStickCnt > 0)
	{
		bOffsetStick = TRUE;
		nOffsetPos = _wtof(strArrayStick.GetAt(nAdd));
	}
		
	double fBlockOffset = 0.0;
	double fCellOffset = 0.0;

	//Stage 분배
	double fStagePos = 0.0;
	
	int nMaxStick = 0;
	nMaxStick = rcp->m_sMap.m_fCellCount.y;

	double* pStageY = nullptr;
	pStageY = new double[nMaxStick];

	for (int i = 0; i < nMaxStick; i++) {
		if (i == nOffsetPos) {
			if (nAdd < nBlockCnt) fBlockOffset = fBlockOffset + _wtof(strArrayBlock.GetAt(nAdd));
			nAdd++;
			if (nAdd < nStickCnt) nOffsetPos = nOffsetPos + _wtoi(strArrayStick.GetAt(nAdd));
		}
		else if (i > 0) {
			fCellOffset = ((i - nAdd) * rcp->m_sMap.m_fCellDistance.y);
		}

		pStageY[i] = rcp->m_sMap.m_fAlignKey.y + rcp->m_sMap.m_fCellCenter.y + fCellOffset - fReviewOffseet2 + fBlockOffset;
	}

	//Stage 분배
	int nCount = 1;
	for (int i = (int)nMaxStick - 1; i >= 0; i--) {
		sStick.Clear ();
		sStick.m_fStageY = pStageY[i];
		sStick.m_fStageR = g_inRcpInfo.GetGlsRotate () * 90.0;	// LYS 2026-09-29 스틱 R = m_nGlsRotate*90 (장변 90, 단변 180)
		sStick.m_nStickNum = nCount++;
		
		g_inRcpInfo.AddStick (sStick);
	}

	if (pStageY != nullptr) delete pStageY;

	////Stage 분배
	//double fStagePos = 0.0; 
	//int nCount = 1; 
	//for (int i = (int)rcp->m_sMap.m_fCellCount.y - 1; i >= 0; i--) {
	//	sStick.Clear ();
	////	sStick.m_fStageY = rcp->m_sMap.m_fGlobalMark.y + rcp->m_sMap.m_fCellCenter.y + (i * rcp->m_sMap.m_fCellDistance.y);
	//	sStick.m_fStageY = rcp->m_sMap.m_fAlignKey.y + rcp->m_sMap.m_fCellCenter.y + (i * rcp->m_sMap.m_fCellDistance.y) - fReviewOffseet2 + fBlockOffset;
	//	sStick.m_nStickNum = nCount++;
	//	
	//	g_inRcpInfo.AddStick (sStick);
	//}

	//Cell 분배
	//double fTemp = 0.0;
	//int nOffsetPos = 0;
	//int nBlockCnt  = (int)strArrayBlock.GetSize ();
	int nOffsetCnt = (int)strArrayOffset.GetSize ();

	nAdd = 0;

	BOOL bOffset = FALSE;
	if (nOffsetCnt > 0) bOffset = TRUE;

	fBlockOffset = 0.0;
	fCellOffset = 0.0;

	for (int i = 0; i < rcp->m_sMap.m_fCellCount.x; i++) {
		sCell.Clear ();
		if (i == 0 && bOffset) {
			nOffsetPos = _wtof (strArrayOffset.GetAt (nAdd));
		}

		if (i == nOffsetPos) {
			if (nAdd < nBlockCnt) fBlockOffset = fBlockOffset + _wtof (strArrayBlock.GetAt (nAdd));
			nAdd++;
			if (nAdd < nOffsetCnt) nOffsetPos = nOffsetPos + _wtoi (strArrayOffset.GetAt (nAdd));
		}
		else if (i > 0) {
			fCellOffset = ((i - nAdd) * rcp->m_sMap.m_fCellDistance.x);
		}
		
	//	sCell.m_fCellX = rcp->m_sMap.m_fGlobalMark.x + rcp->m_sMap.m_fCellCenter.x + fCellOffset + fBlockOffset;
		// LYS 2026-09-07 앵커 부호 반전: GLOBALX/CELLOX(ini, 자동채움 -(size/2-F), X2-F)는 -Y 코너 기준이고, 2026-08-07 이후 셀은 A01(+Y측)에서 -Y로 진행하므로
		//   A01 = -(GLOBALX + CELLOX). 구 코드(+앵커 -진행)는 -690에서 더 -로 나가 셀이 글래스 밖에 생성됨(OCP_TEST_LONG_GLASS 2026-09-07). ROTATE_0의 수작업 반전 레시피(+641→-641)와 동일 결과
		sCell.m_fCellX = -(rcp->m_sMap.m_fAlignKey.x + rcp->m_sMap.m_fCellCenter.x) - fCellOffset - fBlockOffset;	// jsyoon 2022.08.25 change	// LYS 2026-08-07 rotation-change: cells advance toward -Y from A01 anchor (mirrored arrangement)

		for (int i = 0; i < g_inRcpInfo.GetStickCount (); i++) {
			g_inRcpInfo.AddCell (i, sCell);
		}
	}

	//Cell ID, Area
	CString strCellID;
	int nArea = 0;
	// LYS 2026-07-16 Add rotate branching for Cell ID to match sequence MakeCellID
	int nGlsRotate = g_inRcpInfo.GetGlsRotate();
	nMaxStick = g_inRcpInfo.GetStickCount();

	for (int i = 0; i < nMaxStick; i++) {
		
		for (int j = 0; j < g_inRcpInfo.GetCellCount (i); j++) {

			if (nGlsRotate == RCP_CELL_ANG_90) {
				// rotate=1 (YY, BT): Letter=CellIdx, Num=StickIdx+1
				strCellID.Format (_T("%c%02d"), 65 + j, i + 1);
			}
			else {
				// rotate=0 (DG): Letter=MaxStick-StickIdx-1, Num=CellIdx+1
				strCellID.Format (_T("%c%02d"), 65 + nMaxStick - i - 1, j + 1);
			}
			
			g_inRcpInfo.SetCellID (i, j, strCellID);
			g_inRcpInfo.SetCellArea (i, j, (i * g_inRcpInfo.GetCellCount (i)) + j);
		}
	}

}

// LYS 2026-09-29 Hybrid Phase 2 MakeCell 하이브리드 UI (docs/HybridGlass_Porting_Plan.md 5.10)
//   세그먼트마다 MakeCell 입력(셀 정보 이름, 좌표 그리드, CAD 그리드, OPEN 원본)을 슬롯에 보관하고 < > 로 오간다.
//   레시피는 RECIPE APPLY 때만 바뀐다(유일한 커밋 지점). OPEN 한 슬롯만 재생성하고 나머지 세그먼트는 그대로 둔다.
//   1세그먼트(체크 해제) 레시피의 RECIPE APPLY는 지금 동작 그대로.
void	C_MakeCellDlg::InitHybridUI ()
{
	for (int i = 1; i <= M_SEGMENT_MAX; i++) m_cmbSegCount.AddString (dfcStr (_T("%d"), i));
	m_cmbSegCount.SetCurSel (0);
	m_chkHybrid.SetCheck (BST_UNCHECKED);

	m_vecSegInput.assign (1, S_MakeSegInput ());
	m_nSegView		= 0;
	m_nSingleSrcSeg = 0;
	UpdateHybridUI ();
}

BOOL	C_MakeCellDlg::ShowWindow (int nCmdShow)
{
	if (nCmdShow == SW_SHOW) {
		SyncHybridFromRecipe ();
		UpdateCellOverlay ();	// LYS 2026-09-30
	}

	return CDialogEx::ShowWindow (nCmdShow);
}

// 창을 열 때 레시피 세그먼트 수에 슬롯 수를 맞춘다. 같은 인덱스 슬롯의 입력은 유지.
void	C_MakeCellDlg::SyncHybridFromRecipe ()
{
	int nRcpSeg = g_inRcpInfo.GetSegmentLoopCount ();

	if ((int)m_vecSegInput.size () != nRcpSeg) {
		SaveSegInput (m_nSegView);
		m_vecSegInput.resize (nRcpSeg);
		if (m_nSegView >= nRcpSeg) {
			m_nSegView = nRcpSeg - 1;
			LoadSegInput (m_nSegView);
		}
	}

	m_chkHybrid.SetCheck (nRcpSeg > 1 ? BST_CHECKED : BST_UNCHECKED);
	m_cmbSegCount.SetCurSel (nRcpSeg - 1);
	m_nSingleSrcSeg = 0;
	UpdateHybridUI ();
}

void	C_MakeCellDlg::UpdateHybridUI ()
{
	BOOL bHybrid = (m_chkHybrid.GetCheck () == BST_CHECKED);
	int	 nSeg	 = (int)m_vecSegInput.size ();

	GetDlgItem (IDC_COMBO_3)->EnableWindow (bHybrid);
	GetDlgItem (IDC_BUTTON_6)->EnableWindow (bHybrid);
	GetDlgItem (IDC_BUTTON_7)->EnableWindow (bHybrid && m_nSegView > 0);
	GetDlgItem (IDC_BUTTON_8)->EnableWindow (bHybrid && m_nSegView < nSeg - 1);

	CString strName;
	if (m_nSegView < g_inRcpInfo.GetSegmentCount ()) strName = g_inRcpInfo.GetSegmentName (m_nSegView);
	else if (m_nSegView > 0)						 strName = _T("(new)");

	CString str;
	str.Format (_T("Seg %d/%d  %s%s"), m_nSegView + 1, nSeg, strName.GetString (),
				(m_nSegView < nSeg && m_vecSegInput[m_nSegView].bOpened) ? _T("  [OPEN]") : _T(""));
	SetDlgItemText (IDC_STATIC_1, str);
}

BOOL	C_MakeCellDlg::IsHybridEdit ()
{
	return (m_chkHybrid.GetCheck () == BST_CHECKED) && ((int)m_vecSegInput.size () > 1);
}

void	C_MakeCellDlg::SaveSegInput (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegInput.size ()) return;
	S_MakeSegInput &sIn = m_vecSegInput[nSeg];

	m_cmbCellInfoRcp.GetWindowText (sIn.strCellInfo);
	sIn.nAngle	   = m_cmbAngle.GetCurSel ();
	sIn.nRotateSel = m_gridCoordData.GetItemCurSel (CELL_ROW_00, 1);

	sIn.vecOpenData.clear ();
	for (int i = 0; i < m_strArrCoordData.GetSize (); i++) sIn.vecOpenData.push_back (m_strArrCoordData.GetAt (i));

	sIn.vecCoord.assign (CELL_ROW_END, CString ());
	sIn.vecCoordDesc.assign (CELL_ROW_END, CString ());
	for (int i = 0; i < CELL_ROW_END; i++) {
		sIn.vecCoord[i]		= m_gridCoordData.GetItemText (i, 1);
		sIn.vecCoordDesc[i] = m_gridCoordData.GetItemText (i, 2);
	}

	sIn.vecCad.assign (CAD_ROW_END, CString ());
	for (int i = 0; i < CAD_ROW_END; i++) sIn.vecCad[i] = m_gridCADData.GetItemText (i, 1);
}

// 입력이 없는 슬롯(새 세그먼트)은 빈 그리드로 보여준다 - OPEN 부터 해야 한다
void	C_MakeCellDlg::LoadSegInput (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegInput.size ()) return;
	const S_MakeSegInput &sIn = m_vecSegInput[nSeg];

	m_cmbCellInfoRcp.SetWindowText (sIn.strCellInfo);
	m_cmbAngle.SetCurSel ((sIn.nAngle >= 0 && sIn.nAngle < m_cmbAngle.GetCount ()) ? sIn.nAngle : 0);

	m_strArrCoordData.RemoveAll ();
	for (size_t i = 0; i < sIn.vecOpenData.size (); i++) m_strArrCoordData.Add (sIn.vecOpenData[i]);

	m_gridCoordData.SetItemCurSel (CELL_ROW_00, 1, (sIn.nRotateSel >= 0) ? sIn.nRotateSel : 0);
	for (int i = CELL_ROW_01; i < CELL_ROW_END; i++) {
		m_gridCoordData.SetItemText (i, 1, (i < (int)sIn.vecCoord.size ())	   ? sIn.vecCoord[i]	 : CString ());
		m_gridCoordData.SetItemText (i, 2, (i < (int)sIn.vecCoordDesc.size ()) ? sIn.vecCoordDesc[i] : CString ());
	}
	for (int i = 0; i < CAD_ROW_END; i++) {
		m_gridCADData.SetItemText (i, 1, (i < (int)sIn.vecCad.size ()) ? sIn.vecCad[i] : CString ());
	}

	Invalidate (FALSE);
}

void	C_MakeCellDlg::MoveSegView (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegInput.size () || nSeg == m_nSegView) return;

	RefreshViewPreview ();	// LYS 2026-09-30 outgoing slot: its grid as it is now (hand edits) stays in the overlay
	SaveSegInput (m_nSegView);
	m_nSegView = nSeg;
	LoadSegInput (m_nSegView);
	UpdateHybridUI ();
	UpdateCellOverlay ();	// LYS 2026-09-30
}

void	C_MakeCellDlg::OnSegPrev ()
{
	MoveSegView (m_nSegView - 1);
}

void	C_MakeCellDlg::OnSegNext ()
{
	MoveSegView (m_nSegView + 1);
}

// 해제는 확인 후 - ICP는 체크를 풀자마자 세그먼트를 버렸다(비이식)
void	C_MakeCellDlg::OnHybridCheck ()
{
	BOOL bHybrid = (m_chkHybrid.GetCheck () == BST_CHECKED);

	if (!bHybrid && (int)m_vecSegInput.size () > 1) {
		CString strMsg;
		strMsg.Format (_T("하이브리드 모드를 해제하시겠습니까?\n\nRECIPE APPLY 시 지금 보고 있는 세그먼트(Seg %d) 하나로 레시피가 만들어집니다."), m_nSegView + 1);
		if (AfxMessageBox (strMsg, MB_YESNO | MB_ICONQUESTION) != IDYES) {
			m_chkHybrid.SetCheck (BST_CHECKED);
			return;
		}

		SaveSegInput (m_nSegView);
		S_MakeSegInput sKeep = m_vecSegInput[m_nSegView];
		m_nSingleSrcSeg = (m_nSegView < g_inRcpInfo.GetSegmentCount ()) ? m_nSegView : 0;
		m_vecSegInput.assign (1, sKeep);
		m_nSegView = 0;
		m_cmbSegCount.SetCurSel (0);
	}

	UpdateHybridUI ();
	UpdateCellOverlay ();	// LYS 2026-09-30
}

// 슬롯 수만 바꾼다. 레시피는 RECIPE APPLY 때 바뀐다.
void	C_MakeCellDlg::OnSegCountApply ()
{
	if (m_chkHybrid.GetCheck () != BST_CHECKED) return;

	int nNew = m_cmbSegCount.GetCurSel () + 1;
	int nOld = (int)m_vecSegInput.size ();
	if (nNew < 1 || nNew > M_SEGMENT_MAX || nNew == nOld) return;

	if (nNew < nOld) {
		CString strMsg;
		strMsg.Format (_T("세그먼트 수를 %d개 -> %d개로 줄이시겠습니까?\n\nRECIPE APPLY 시 세그먼트 %d~%d가 삭제됩니다."), nOld, nNew, nNew + 1, nOld);
		if (AfxMessageBox (strMsg, MB_YESNO | MB_ICONQUESTION) != IDYES) {
			m_cmbSegCount.SetCurSel (nOld - 1);
			return;
		}
	}

	SaveSegInput (m_nSegView);
	m_vecSegInput.resize (nNew);
	if (m_nSegView >= nNew) m_nSegView = nNew - 1;
	if (nNew == 1) m_nSingleSrcSeg = 0;
	LoadSegInput (m_nSegView);
	UpdateHybridUI ();
	UpdateCellOverlay ();	// LYS 2026-09-30
}

// 하이브리드 RECIPE APPLY: 세그먼트 전체를 새로 조립해서 한 번에 교체한다
//   기존 세그먼트(k < 기존 수)는 그 세그먼트 값을 기준으로, OPEN 한 슬롯만 재생성 - 옵션 보존(ICP 7단계 보존 재조정 대신 단순화, D-11)
//   새 세그먼트는 세그먼트 1 전체 복사 또는 기본값(D-11 2택) 위에 생성. CAD도 세그먼트마다 저장(ICP I17 결함 비이식)
void	C_MakeCellDlg::ApplyHybridRecipe ()
{
	SaveSegInput (m_nSegView);

	int nNew = (int)m_vecSegInput.size ();
	int nOld = g_inRcpInfo.GetSegmentLoopCount ();

	CString strRebuild, strKeep, strErr, strNo;
	int nOpened = 0;
	for (int k = 0; k < nNew && strErr.IsEmpty (); k++) {
		strNo.Format (_T("%d "), k + 1);
		if (m_vecSegInput[k].bOpened) {
			if (m_vecSegInput[k].strCellInfo.IsEmpty ()) strErr.Format (_T("Seg %d: 셀 정보 이름이 비어 있습니다."), k + 1);
			strRebuild += strNo;
			nOpened++;
		}
		else if (k >= nOld) strErr.Format (_T("Seg %d는 새 세그먼트입니다 - 먼저 셀 정보를 OPEN 하세요."), k + 1);
		else				strKeep += strNo;
	}
	if (strErr.IsEmpty () && nOpened == 0) strErr = _T("OPEN 한 세그먼트가 없습니다.\n다시 만들 세그먼트마다 셀 정보를 OPEN 하세요.");
	if (!strErr.IsEmpty ()) {
		AfxMessageBox (strErr, MB_ICONWARNING);
		return;
	}

	CString strMsg, strRemove;
	if (nNew < nOld) strRemove.Format (_T("\n삭제   : %d~%d"), nNew + 1, nOld);
	if (strKeep.IsEmpty ()) strKeep = _T("-");
	strMsg.Format (_T("하이브리드 레시피를 생성합니다 (세그먼트 %d개)\n\n재생성 : %s\n유지   : %s%s\n\n계속 하시겠습니까?"),
				   nNew, strRebuild.GetString (), strKeep.GetString (), strRemove.GetString ());
	if (AfxMessageBox (strMsg, MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	BOOL bCopy = TRUE;
	if (nNew > nOld) {
		int nRet = AfxMessageBox (_T("새 세그먼트의 옵션(검사 항목, 램프, 컨택 등) 초기값을 선택하세요.\n\n예 = 세그먼트 1에서 복사\n아니요 = 기본값"), MB_YESNOCANCEL | MB_ICONQUESTION);
		if (nRet == IDCANCEL) return;
		bCopy = (nRet == IDYES);
	}

	// 기존 세그먼트 사본 - GetSegmentMgr(활성)은 작업 슬롯을 가리키고 아래 루프가 작업 슬롯을 덮어쓰므로 먼저 복사한다
	g_inRcpInfo.CommitActiveSegment ();
	std::vector<S_RecipeMgr>	 vecOld;
	std::vector<S_SegmentHeader> vecOldHdr;
	vecOld.resize (nOld);	// LYS 2026-09-29 스택 오버플로 방지 - S_RecipeMgr는 고정 문자 배열만 약 88KB라 스택에 두지 않는다
	for (int k = 0; k < nOld; k++) {
		S_RecipeMgr *pSeg = g_inRcpInfo.GetSegmentMgr (k);
		if (pSeg) vecOld[k] = *pSeg;
		vecOldHdr.push_back (g_inRcpInfo.GetSegmentHeader (k));
	}
	std::unique_ptr<S_RecipeMgr> pStage (new S_RecipeMgr (g_inRcpInfo.m_recipeMgr));

	std::vector<S_RecipeMgr>	 vecNew;
	vecNew.reserve (nNew);
	std::vector<S_SegmentHeader> vecNewHdr;
	for (int k = 0; k < nNew; k++) {
		vecNew.resize (k + 1);	// LYS 2026-09-29 새 세그먼트는 벡터 원소(힙)에서 바로 만든다 - 스택 사본/임시 객체 없음
		S_RecipeMgr		&sSeg = vecNew[k];
		if (k < nOld)	sSeg = vecOld[k];
		else if (bCopy) sSeg = vecOld[0];
		S_SegmentHeader sHdr = (k < nOld) ? vecOldHdr[k] : S_SegmentHeader ();

		if (k >= nOld) {	// 새 이름 "Segment#n" - 남는 세그먼트 이름과 겹치지 않게
			for (int n = k + 1; n < k + 1 + 100; n++) {
				CString strCand;
				strCand.Format (_T("Segment#%d"), n);
				BOOL bDup = FALSE;
				for (int j = 0; j < nOld && j < nNew; j++)	 if (strCand.CompareNoCase (vecOldHdr[j].m_chName) == 0) bDup = TRUE;
				for (size_t j = 0; j < vecNewHdr.size (); j++) if (strCand.CompareNoCase (vecNewHdr[j].m_chName) == 0) bDup = TRUE;
				if (!bDup) {
					wcsncpy_s (sHdr.m_chName, strCand, _TRUNCATE);
					break;
				}
			}
		}

		if (m_vecSegInput[k].bOpened) {
			LoadSegInput (k);
			g_inRcpInfo.m_recipeMgr = sSeg;
			BuildRecipeFromGrid (m_vecSegInput[k].strCellInfo);

			// LYS 2026-09-29 셀/스틱 좌표 글라스 범위 검사 - 하나라도 실패하면 레시피를 바꾸지 않는다
			CString strWhat;
			strWhat.Format (_T("Seg %d"), k + 1);
			CString strLayoutErr = g_inRcpInfo.CheckRecipeLayout (&g_inRcpInfo.m_recipeMgr, strWhat);
			if (!strLayoutErr.IsEmpty ()) {
				g_inRcpInfo.m_recipeMgr = *pStage;
				LoadSegInput (m_nSegView);
				UpdateHybridUI ();
				AfxMessageBox (_T("하이브리드 레시피 생성 불가 (셀 좌표 검사)\n\n") + strLayoutErr, MB_ICONWARNING);
				return;
			}
			sSeg = g_inRcpInfo.m_recipeMgr;
		}

		vecNewHdr.push_back (sHdr);
	}

	g_inRcpInfo.m_recipeMgr = *pStage;
	if (!g_inRcpInfo.ReplaceSegments (vecNew, vecNewHdr)) {
		AfxMessageBox (_T("하이브리드 레시피 생성에 실패했습니다."), MB_ICONWARNING);
		LoadSegInput (m_nSegView);
		return;
	}

	// LYS 2026-10-01 Hybrid Phase 4b rebuilt cells are not proven to be the same physical cells (ICP 4.13, plan 5.7) -> Cell ID back to Auto/Auto
	if (g_inRcpInfo.ResetCellIdConfig ()) AfxMessageBox (_T("셀 구성이 바뀌어 Cell ID Manual 설정(A/B 모드·수동 ID)을 초기화했습니다.\nCELL ID 편집기에서 다시 입력하십시오."), MB_ICONINFORMATION);
	for (int k = 0; k < nNew; k++) m_vecSegInput[k].bOpened = FALSE;	// 다음 APPLY가 지난 입력으로 재생성하지 않도록
	LoadSegInput (m_nSegView);
	UpdateHybridUI ();

	::SendMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_MAKE_CELL_SET, NULL);

	ShowWindow (SW_HIDE);
}

// LYS 2026-09-30 IDC_STATIC_CELL_OVERLAY - ICP MakeCellDlg IDC_STATIC_2 (C_GlassMap::UpdateGlassMap): every segment on one glass.
//   Source per slot = what the next RECIPE APPLY uses for it:
//     single mode (1 slot)   : the grid - the non-hybrid APPLY always builds from it; the recipe while the grid does not build
//     hybrid, OPEN-ed slot   : its grid, built while the slot is on view and kept in the slot (MoveSegView refreshes the
//                              outgoing slot first, so a hand edit made before the move is shown)
//     hybrid, other slots    : the recipe as it is (g_inRcpInfo, the editor copy) - hybrid APPLY keeps them and clears bOpened
//   Refreshed on OPEN / CAD CALC / segment move / hybrid check / count apply / show (ICP: OPEN and CALC); a hand edit in the
//   grids of the slot on view shows after the next of these.
void	C_MakeCellDlg::RefreshViewPreview ()
{
	int nSlot = (int)m_vecSegInput.size ();
	if (m_nSegView < 0 || m_nSegView >= nSlot) return;

	S_MakeSegInput &sIn = m_vecSegInput[m_nSegView];
	if (!sIn.bOpened && nSlot != 1) {
		sIn.bPreview = FALSE;
		return;
	}

	S_GlassMapSeg sGrid;
	sIn.bPreview = BuildGridPreview (sGrid);
	sIn.sPreview = sGrid;
}

void	C_MakeCellDlg::UpdateCellOverlay ()
{
	if (m_wndCellOverlay.GetSafeHwnd () == NULL) return;

	RefreshViewPreview ();

	int nSlot	= (int)m_vecSegInput.size ();
	int nRcpSeg	= g_inRcpInfo.GetSegmentCount ();
	std::vector<S_GlassMapSeg> vecSeg (nSlot);
	for (int k = 0; k < nSlot; k++) {
		const S_MakeSegInput &sIn = m_vecSegInput[k];
		if (sIn.bOpened || nSlot == 1) {
			if (sIn.bPreview) {
				vecSeg[k] = sIn.sPreview;
				continue;
			}
			if (sIn.bOpened) continue;	// an OPEN-ed grid that does not build draws nothing, not the old cells
		}

		if (nRcpSeg <= 1) {
			if (k == 0) C_GlassMap::BuildSeg (&g_inRcpInfo.m_recipeMgr, vecSeg[k]);
			continue;
		}
		int nSrc = (nSlot == 1) ? m_nSingleSrcSeg : k;	// hybrid unchecked: the non-hybrid APPLY starts from that segment
		if (nSrc < nRcpSeg) C_GlassMap::BuildSeg (g_inRcpInfo.GetSegmentMgr (nSrc), vecSeg[k]);
	}

	// LYS 2026-10-12 Hybrid Phase 6 IDs the next hybrid RECIPE APPLY leaves (D6-8 (a)): slot k becomes segment k and APPLY resets any manual
	//   Cell ID config, so every slot shows the Auto token with the Phase 4 letter carry in slot order (CellId_AutoToken = the runtime MakeCellID
	//   copy). One slot (a 1-segment recipe after APPLY): m_chCellID as before
	if (nSlot > 1) {
		CCellIdGeo geo;
		for (int k = 0; k < nSlot; k++) {
			S_CellIdSegGeo g = { vecSeg[k].nRotate, vecSeg[k].nStick, vecSeg[k].nCell0 };	// a slot that draws nothing adds no letters
			geo.push_back (g);
		}
		for (int k = 0; k < nSlot; k++) {
			for (size_t c = 0; c < vecSeg[k].vecCell.size (); c++) {
				S_GlassMapCell &sCell = vecSeg[k].vecCell[c];
				CString strTok = CellId_AutoToken (geo, k, sCell.nStick, sCell.nCell, 0);
				if (!strTok.IsEmpty ()) _tcsncpy_s (sCell.szID, _countof (sCell.szID), strTok, _TRUNCATE);
			}
		}
	}
	m_wndCellOverlay.UpdateGlassMap (vecSeg, m_nSegView);
}

// "5" or "3,2" -> 5, the way BuildRecipeFromGrid adds up the stick / cell count lists
static int SumMakeCellCount (const CString &str, BOOL &bBad)
{
	int nSum = 0, nPos = 0;
	CString strTok = str.Tokenize (_T(","), nPos);
	while (nPos != -1) {
		int n = _wtoi (strTok.Trim ());
		if (n < 0 || n > 10000) bBad = TRUE;
		else					nSum += n;
		strTok = str.Tokenize (_T(","), nPos);
	}
	return nSum;
}

// LYS 2026-09-30 Grid -> overlay cells without changing the recipe. BuildRecipeFromGrid writes only g_inRcpInfo.m_recipeMgr
//   (the editor's working slot), so it runs there and the slot is put back - the stage / restore ApplyHybridRecipe uses.
//   Counts are checked first: BuildRecipeFromGrid allocates new double[stick count] without a check.
BOOL	C_MakeCellDlg::BuildGridPreview (S_GlassMapSeg &sOut)
{
	sOut = S_GlassMapSeg ();

	BOOL bBad	= FALSE;
	int	 nStick	= SumMakeCellCount (m_gridCoordData.GetItemText (CELL_ROW_05, 1), bBad);
	int	 nCell	= SumMakeCellCount (m_gridCoordData.GetItemText (CELL_ROW_06, 1), bBad);
	if (bBad || nStick < 1 || nStick > 200 || nCell < 1 || nCell > 500) return FALSE;

	CString strRcpName;
	m_cmbCellInfoRcp.GetWindowText (strRcpName);

	std::unique_ptr<S_RecipeMgr> pKeep (new S_RecipeMgr (g_inRcpInfo.m_recipeMgr));	// heap - S_RecipeMgr is about 88 KB
	BOOL bOk = TRUE;
	try {
		BuildRecipeFromGrid (strRcpName);
		C_GlassMap::BuildSeg (&g_inRcpInfo.m_recipeMgr, sOut);
	}
	catch (...) {
		bOk = FALSE;
	}
	g_inRcpInfo.m_recipeMgr = *pKeep;

	return bOk && !sOut.vecCell.empty ();
}
