#include "pch.h"
#include "dfcRecipe.h"
#include "MakeCellDlg.h"
#include "afxdialogex.h"

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

END_MESSAGE_MAP ()

void C_MakeCellDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL C_MakeCellDlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();

	m_cmbAngle.AddString (_T("0˚"));
	m_cmbAngle.AddString (_T("90˚"));
	m_cmbAngle.SetCurSel (0);

	InitCoordInfo ();
	InitCADInfo ();

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


	// 2022.12.20 add
	CString strTemp;
	if (_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) == 0)
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
	if (AfxMessageBox (_T("현재 정보로 레시피를 생성 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strRcpName;
	m_cmbCellInfoRcp.GetWindowText (strRcpName);

	if (strRcpName.IsEmpty ()) {
		AfxMessageBox (_T("선택된 레시피가 없습니다."));
		return;
	}

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
	else if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_90) {
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

	// LYS 2026-08-18 rotate 180 shares the 0-degree basis (same physical aspect).
	// NOTE: left/right review offsets are mirrored at 180 vs 0 - sign needs a check
	// against the mechanical drawing before real-machine use.
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
	else if (g_inRcpInfo.GetGlsRotate () == RCP_CELL_ANG_90) {
		rcp->m_sMap.m_fContactOffset = fX1 - fE - fI + fM;
		fReviewOffseet	= fX2 - fF - fJ + fN;

		//rcp->m_sMap.m_fAlignKey.x = rcp->m_sMap.m_fGlobalMark.x;
		//rcp->m_sMap.m_fAlignKey.y = rcp->m_sMap.m_fGlobalMark.y;
	}

	// jsyoon 2022.08.25 add
	fReviewOffseet2 = rcp->m_sMap.m_fContactOffset;

	// LYS 2026-08-18 rotate 180 needs its OWN value - it must NOT share the 0-degree
	// branch. Under the 2026-08-07 scheme the measurement angle comes from the stick
	// m_fStageR itself, so the USER OFFSET is 0. Before this branch existed, rotate=2
	// fell through with no assignment and kept a stale 90 from a previously edited
	// recipe - the root cause of the short-side 270-degree misposition fixed by hand
	// on 2026-08-07 (it would have come back on the next MakeCell run).
	if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_0) {
		g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate = 90.;
	}
	else if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_90) {
		g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate = 0;
	}
	else if (g_inRcpInfo.GetGlsRotate() == RCP_CELL_ANG_180) {
		g_inRcpInfo.m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate = 0;
	}

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
		sCell.m_fCellX = rcp->m_sMap.m_fAlignKey.x + rcp->m_sMap.m_fCellCenter.x - fCellOffset - fBlockOffset;	// jsyoon 2022.08.25 change	// LYS 2026-08-07 rotation-change: cells advance toward -Y from A01 anchor (mirrored arrangement)

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
		if (_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) != nAngle)
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
	else if (nAngle == RCP_CELL_ANG_90) 
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
		if (_wtoi(m_strArrCoordData.GetAt(CELL_ROW_00)) != nAngle)
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