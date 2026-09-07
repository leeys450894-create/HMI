#include "pch.h"
#include "dfcRecipe.h"
#include "AlignRcpDlg.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC (C_AlignRcpDlg, CDialogEx)

C_AlignRcpDlg::C_AlignRcpDlg (CWnd* pParent /*=NULL*/) : C_BaseDialog (IDD_ALIGN_RECIPE_DLG, pParent)
{
	m_hWnd = pParent;
	////m_pRecipeMem = NULL;
	////m_pRecipeMem = g_pSysMapView;
}

C_AlignRcpDlg::~C_AlignRcpDlg ()
{
}

void C_AlignRcpDlg::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control (pDX, IDC_STATIC_1, m_stcView);
	for (int i = 0; i < 3; i++) DDX_Control (pDX, IDC_BUTTON_1 + i, m_btnStyle[i]);
	DDX_Control (pDX, IDC_GRID_1,   m_gridGlassAlignHead);
	DDX_Control (pDX, IDC_GRID_2,   m_gridGlassAlignData);
	DDX_Control (pDX, IDC_GRID_3,   m_gridProbeAlignHead);
	DDX_Control (pDX, IDC_GRID_4,   m_gridProbeAlignData);
	DDX_Control (pDX, IDC_GRID_5,   m_gridCcdSetMntHead);
	DDX_Control (pDX, IDC_GRID_6,   m_gridCcdSetMntData);
	DDX_Control (pDX, IDC_GRID_7,   m_gridRefImageHead);
	DDX_Control (pDX, IDC_GRID_8,   m_gridRefImageData);
	DDX_Control (pDX, IDC_GRID_9,   m_gridAlignName);
	DDX_Control (pDX, IDC_GRID_10,  m_gridMarginHead);
	DDX_Control (pDX, IDC_GRID_11,  m_gridMarginData);
	DDX_Control (pDX, IDC_GRID_12,  m_gridAlignProcData);
	DDX_Control (pDX, IDC_GRID_13,  m_gridAlignPosHead);
	DDX_Control (pDX, IDC_GRID_14,  m_gridAlignPosData);
}

BEGIN_MESSAGE_MAP (C_AlignRcpDlg, CDialogEx)
	ON_WM_ERASEBKGND ()
	ON_WM_DESTROY ()
	//ON_WM_PAINT ()
	ON_COMMAND (IDC_BUTTON_1, OnAlignLoad)
	ON_COMMAND (IDC_BUTTON_2, OnAlignSave)
	ON_COMMAND (IDC_BUTTON_3, OnAlignSaveAs)
	ON_COMMAND (IDC_BUTTON_4, OnCcdSetUpdate)
	ON_COMMAND (IDC_BUTTON_5, OnShowRefImageSet)
	ON_COMMAND (IDC_BUTTON_6, OnPatternSearch)
	ON_COMMAND (IDC_BUTTON_7, OnAlignCcdSet)
	ON_NOTIFY (NM_CLICK,  IDC_GRID_8, OnSelCcdCurrent)
	ON_NOTIFY (NM_DBLCLK, IDC_GRID_8, OnSelRefImg)
	ON_NOTIFY (NM_CLICK, IDC_GRID_2, OnSelGlassAlign)
	ON_NOTIFY (NM_CLICK, IDC_GRID_4, OnSelProbeAlign)
	ON_NOTIFY (NM_CLICK, IDC_GRID_12, OnAlignProc)


END_MESSAGE_MAP ()

void C_AlignRcpDlg::OnDestroy()
{
	if (!m_image.IsNull ()) m_image.Destroy ();

	CDialogEx::OnDestroy();
}

//void	C_AlignRcpDlg::OnPaint ()
//{
//	CPaintDC	dc (this);
//
//	CMemDC	 memDC (dc, m_rcDraw);
//	CDC		*pmemDC = &memDC.GetDC();
//
//	CRect rc2 = dfcClientRect (this);
//	pmemDC->FillSolidRect (&rc2, COLOR_FRAME_BRIGHT);
//
//	if (!m_image.IsNull ()) {
//		m_image.Draw (*pmemDC,m_rcDraw, Gdiplus::InterpolationModeInvalid);
//	}
//}

BOOL C_AlignRcpDlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();
	
	m_stcView.GetWindowRect (&m_rcDraw);
	ScreenToClient (&m_rcDraw);

	for (int i = 0; i < 3; i++) InitButtons (m_btnStyle[i]);

	m_nSelCcd = -1;
	m_nSelProbe = 0;
	m_nRefRow = -1;
	m_nRefCol = -1;
	InitGlassAlign ();
	InitProbeAlign ();
	InitCcdSetMonitor ();
	InitRefImage ();
	InitAlignName ();
	InitMargin ();
	InitAlignProc ();
	InitAlignPos ();

	return TRUE;
}

void	C_AlignRcpDlg::ShowRefImage ()
{
	CPaintDC	dc (this);

	CMemDC	 memDC (dc, m_rcDraw);
	CDC		*pmemDC = &memDC.GetDC();

	CRect rc2 = dfcClientRect (this);
	pmemDC->FillSolidRect (&rc2, COLOR_FRAME_BRIGHT);

	if (!m_image.IsNull ()) {

		m_stcView.GetWindowRect(&m_rcDraw);
		m_image.Draw (*pmemDC,m_rcDraw, Gdiplus::InterpolationModeInvalid);
	}
}

void	C_AlignRcpDlg::SetAlignName (CString str)
{
	m_gridAlignName.SetItemText (0, 1, str);
	Invalidate (FALSE);
}

CString	C_AlignRcpDlg::GetAlignName ()
{
	return m_gridAlignName.GetItemText (0, 1);
}

void	C_AlignRcpDlg::OnAlignLoad ()
{
	CString strfilePath, strName;

	strfilePath.Format (_T("%s*.alg"), theApp.m_strAlignFolder);
	CFileDialog dlg (TRUE, _T("alg"), strfilePath, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, _T("Recipe Files (*.alg)|*.alg|"));

	if (dlg.DoModal() == IDOK)
	{			
		strName = dlg.GetFileName ();

		if (!strName.IsEmpty ()) {
			int nFind = strName.Find (_T(".alg"));

			if (nFind > 0)	strName = strName.Left (nFind);
			else			strName = strName;

			//if (g_inRcpInfo.LoadAlignRcpFile (strName)) {
			//	SetAlignView ();
			//	SetAlignName (strName);
			//}
			//else {
			//	AfxMessageBox (_T("Align 레시피 열기 실패!!."));
			//	return;
			//}
			
			if (!LoadAlignRecipe (strName)) return;

		}
		else {
			AfxMessageBox (_T("선택된 레시피가 없습니다."));
			return;
		}
	}	
}

BOOL	C_AlignRcpDlg::LoadAlignRecipe (CString strName)
{
	if (g_inRcpInfo.LoadAlignRcpFile (strName)) {
		SetAlignView ();
		SetAlignName (strName);
	}
	else {
		AfxMessageBox (_T("Align 레시피 열기 실패!!."));
		return FALSE;
	}

	return TRUE;
}

void	C_AlignRcpDlg::OnAlignSave ()
{
	if (AfxMessageBox (_T("레시피를 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strRcpName = GetAlignName ();
	if (strRcpName.IsEmpty ()) {
		AfxMessageBox (_T("현재 레시피 이름이 없습니다."));
		return;
	}

	//CString strOldPath, strNewPath, strTime;
	//SYSTEMTIME st;
	//::GetLocalTime(&st);
	//strTime.Format (_T("%04d%02d%02d-%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	//strOldPath = theApp.m_strAlignFolder + strRcpName + _T(".rcp");
	//strNewPath = theApp.m_strRcpBackup + strRcpName + _T("\\") + strTime + _T(".bak");

	//dfcCreateDirectory (strNewPath);
	//CopyFile (strOldPath, strNewPath, FALSE);

	GetAlignView ();
	SaveAlignFile (strRcpName);
}

void	C_AlignRcpDlg::OnAlignSaveAs ()
{
	if (AfxMessageBox (_T("새이름으로 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	CString strRcpName = GetAlignName ();

	CString strFile, strName;

	strFile.Format (_T("%s%s"), theApp.m_strAlignFolder, _T("default.alg") );
	CFileDialog dlg (FALSE, _T("alg"), strFile, OFN_OVERWRITEPROMPT, _T("Recipe Files (*.alg)|*.alg|"));

	if (dlg.DoModal () == IDOK)
	{
		strName = dlg.GetFileName ();

		if (!strName.IsEmpty ()) {
			int nFind = strName.Find (_T(".alg"));

			if (nFind > 0)	strName = strName.Left (nFind);
			else			strName = strName;

			if (strRcpName == strName) {
				AfxMessageBox (_T("동일한 이름으로 저장 할 수 없습니다."));
				return;
			}

			////Backup
			//CFileFind finder;
			//BOOL bFind = finder.FindFile (dfcStr (_T("%s%s%s"), theApp.m_strAlignFolder, strName, _T(".rcp")));

			//CString strOldPath, strNewPath, strTime;
			//SYSTEMTIME st;
			//::GetLocalTime(&st);
			//strTime.Format (_T("%04d%02d%02d-%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

			//strOldPath = theApp.m_strAlignFolder + strName + _T(".rcp");
			//strNewPath = theApp.m_strRcpBackup + strName + _T("\\") + strTime + _T(".bak");

			//dfcCreateDirectory (strNewPath);
			//CopyFile (strOldPath, strNewPath, FALSE);
			//

			GetAlignView ();
			SaveAlignFile (strName);
			SetAlignName (strName);
		}
		else {
			AfxMessageBox (_T("레시피 이름이 없습니다."));
			return;
		}
	}
}

void	C_AlignRcpDlg::InitAlignName ()
{
	InitGridUI (m_gridAlignName, IDC_GRID_9, 1, 2);
	m_gridAlignName.SetItemText	 (0, 0, _T("ALIGN NAME"));
	m_gridAlignName.SetItemState	(0, 1, m_gridAlignName.GetItemState (0, 1) | GVIS_READONLY);
	m_gridAlignName.SetItemFgColour (0, 1, COLOR_GRAY);
}

void	C_AlignRcpDlg::InitGlassAlign ()
{
	InitGridUI (m_gridGlassAlignHead, IDC_GRID_1, 1, 1, TRUE);
	m_gridGlassAlignHead.SetItemText (0, 0, _T("GLASS ALIGN"));
	m_gridGlassAlignHead.EnableWindow	 (FALSE);

	//InitGridUI (m_gridGlassAlignData, IDC_GRID_2, 4, 8, TRUE, FALSE, PIX_UNIFORMITY);
	InitAlignGridUI (m_gridGlassAlignData, IDC_GRID_2, 4, 9, 0);
	//m_gridGlassAlignData.SetItemText (0, 0, _T("BASE STAGE Y"));
	//m_gridGlassAlignData.SetItemText (0, 1, _T("BASE FRONT LEFT"));
	//m_gridGlassAlignData.SetItemText (0, 2, _T("BASE FRONT RIGHT"));
	//
	//m_gridGlassAlignData.SetItemText (2, 0, _T("STAGE Y"));
	//m_gridGlassAlignData.SetItemText (2, 1, _T("STAGE ROTATE"));
	//m_gridGlassAlignData.SetItemText (2, 2, _T("FRONT Y"));
	//m_gridGlassAlignData.SetItemText (2, 3, _T("FRONT LEFT"));
	//m_gridGlassAlignData.SetItemText (2, 4, _T("FRONT RIGHT"));
	//m_gridGlassAlignData.SetItemText (2, 5, _T("FRONT Z"));

	// LYS 2026-07-01 Change Name FRONT -> BACK
	m_gridGlassAlignData.SetItemText(0, 0, _T("BASE STAGE Y"));
	m_gridGlassAlignData.SetItemText(0, 1, _T("BASE BACK LEFT"));
	m_gridGlassAlignData.SetItemText(0, 2, _T("BASE BACK RIGHT"));

	m_gridGlassAlignData.SetItemText(2, 0, _T("STAGE Y"));
	m_gridGlassAlignData.SetItemText(2, 1, _T("STAGE ROTATE"));
	m_gridGlassAlignData.SetItemText(2, 2, _T("BACK Y"));
	m_gridGlassAlignData.SetItemText(2, 3, _T("BACK RIGHT"));
	m_gridGlassAlignData.SetItemText(2, 4, _T("BACK LEFT"));
	m_gridGlassAlignData.SetItemText(2, 5, _T("SUB Z RIGHT"));
	m_gridGlassAlignData.SetItemText(2, 6, _T("SUB Z LEFT"));

	for (int i = 0; i < 4; i++) {
		if (i == 3) continue;

		// jsyoon 2022.09.19 change
		if (i == 1)
		{
			for (int j = 3; j < 8; j++) {
				m_gridGlassAlignData.SetItemState(i, j, m_gridGlassAlignData.GetItemState(i, j) | GVIS_READONLY);
			}
		}
		else
		{
			for (int j = 0; j < 8; j++) {
				m_gridGlassAlignData.SetItemState(i, j, m_gridGlassAlignData.GetItemState(i, j) | GVIS_READONLY);
			}
		}

		/*for (int j = 0; j < 8; j++) {
			m_gridGlassAlignData.SetItemState (i, j, m_gridGlassAlignData.GetItemState (i, j) | GVIS_READONLY);
		}*/
	}

	////버튼
	m_gridGlassAlignData.SetCellType  (3, 7, RUNTIME_CLASS(CGridCellButton));
	m_gridGlassAlignData.SetItemState (3, 7, m_gridGlassAlignData.GetItemState (3, 7) | GVIS_READONLY);
	m_gridGlassAlignData.SetCellType  (3, 8, RUNTIME_CLASS(CGridCellButton));
	m_gridGlassAlignData.SetItemState (3, 8, m_gridGlassAlignData.GetItemState (3, 8) | GVIS_READONLY);

	//버튼 이름
	CGridCellButton* pCell = NULL;
	pCell = (CGridCellButton*)m_gridGlassAlignData.GetCell (3, 7);
	pCell->SetText (_T("GET"));
	pCell = (CGridCellButton*)m_gridGlassAlignData.GetCell (3, 8);
	pCell->SetText (_T("MOVE"));
}

void	C_AlignRcpDlg::SetGlassAlignData ()
{
	m_gridGlassAlignData.SetItemText (1, 0, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_StageY));
	m_gridGlassAlignData.SetItemText (1, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontLeft));
	m_gridGlassAlignData.SetItemText (1, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontRight));

	m_gridGlassAlignData.SetItemText (3, 0, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageY));
	m_gridGlassAlignData.SetItemText (3, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageRotate));
	m_gridGlassAlignData.SetItemText (3, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontY));
	m_gridGlassAlignData.SetItemText (3, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontLeft));
	m_gridGlassAlignData.SetItemText (3, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontRight));
	m_gridGlassAlignData.SetItemText (3, 5, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZRight));
	m_gridGlassAlignData.SetItemText (3, 6, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZLeft));
}

BOOL	C_AlignRcpDlg::GetGlassAlignData (BOOL bShare, S_GlsAlign *pAlignShare)
{
	S_GlsAlign *pAlign;
	if (bShare) pAlign = pAlignShare;
	else		pAlign = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign;

	pAlign->m_fBase_StageY		= _wtof (m_gridGlassAlignData.GetItemText (1, 0));
	pAlign->m_fBase_FrontLeft	= _wtof (m_gridGlassAlignData.GetItemText (1, 1));
	pAlign->m_fBase_FrontRight	= _wtof (m_gridGlassAlignData.GetItemText (1, 2));

	pAlign->m_fStageY			= _wtof (m_gridGlassAlignData.GetItemText (3, 0));
	pAlign->m_fStageRotate		= _wtof (m_gridGlassAlignData.GetItemText (3, 1));
	pAlign->m_fFrontY			= _wtof (m_gridGlassAlignData.GetItemText (3, 2));
	pAlign->m_fFrontLeft		= _wtof (m_gridGlassAlignData.GetItemText (3, 3));
	pAlign->m_fFrontRight		= _wtof (m_gridGlassAlignData.GetItemText (3, 4));
	pAlign->m_fSubZRight			= _wtof (m_gridGlassAlignData.GetItemText (3, 5));
	pAlign->m_fSubZLeft			= _wtof (m_gridGlassAlignData.GetItemText (3, 6));

	return TRUE;
}

void	C_AlignRcpDlg::SetGlassAlignKeyPos ()
{
	m_gridGlassAlignData.SetItemText (1, 0, dfcStr (fabs (g_inRcpInfo.m_recipeMgr.m_sMap.m_fAlignKey.y))); 
	m_gridGlassAlignData.SetItemText (1, 1, dfcStr (-fabs (g_inRcpInfo.m_recipeMgr.m_sMap.m_fAlignKey.x)));
	m_gridGlassAlignData.SetItemText (1, 2, dfcStr (fabs (g_inRcpInfo.m_recipeMgr.m_sMap.m_fAlignKey.x)));

	Invalidate(FALSE);
}

void	C_AlignRcpDlg::InitProbeAlign ()
{
	InitGridUI (m_gridProbeAlignHead, IDC_GRID_3, 1, 1, TRUE);
	m_gridProbeAlignHead.SetItemText (0, 0, _T("PROBE ALIGN"));
	m_gridProbeAlignHead.EnableWindow	 (FALSE);

	InitAlignGridUI (m_gridProbeAlignData, IDC_GRID_4, 4, 10, 1);
	m_gridProbeAlignData.SetItemText (0, 0, _T("BASE PROBE Y"));
	m_gridProbeAlignData.SetItemText (0, 1, _T("BASE BACK LEFT"));
	m_gridProbeAlignData.SetItemText (0, 2, _T("BASE BACK RIGHT"));

	m_gridProbeAlignData.SetItemText (2, 0, _T("PROBE Y"));
	m_gridProbeAlignData.SetItemText (2, 1, _T("PROBE X"));
	m_gridProbeAlignData.SetItemText (2, 2, _T("BACK Y"));
	m_gridProbeAlignData.SetItemText (2, 3, _T("BACK RIGHT"));
	m_gridProbeAlignData.SetItemText (2, 4, _T("BACK LEFT"));
	m_gridProbeAlignData.SetItemText (2, 5, _T("SUB Z RIGHT"));
	m_gridProbeAlignData.SetItemText (2, 6, _T("SUB Z LEFT"));
	m_gridProbeAlignData.SetItemText (2, 7, _T("STAGE Y"));

	for (int i = 0; i < 4; i++) {
		if (i == 3) continue;

		for (int j = 0; j < 9; j++) {
			if (i == 1) {
				if (j == 0 || j == 1 || j == 2) continue;
			}
			m_gridProbeAlignData.SetItemState (i, j, m_gridProbeAlignData.GetItemState (i, j) | GVIS_READONLY);
		}
	}

	////버튼
	m_gridProbeAlignData.SetCellType  (3, 8, RUNTIME_CLASS(CGridCellButton));
	m_gridProbeAlignData.SetItemState (3, 8, m_gridProbeAlignData.GetItemState (3, 8) | GVIS_READONLY);
	m_gridProbeAlignData.SetCellType  (3, 9, RUNTIME_CLASS(CGridCellButton));
	m_gridProbeAlignData.SetItemState (3, 9, m_gridProbeAlignData.GetItemState (3, 9) | GVIS_READONLY);

	//버튼 이름
	CGridCellButton* pCell = NULL;
	pCell = (CGridCellButton*)m_gridProbeAlignData.GetCell (3, 8);
	pCell->SetText (_T("GET"));
	pCell = (CGridCellButton*)m_gridProbeAlignData.GetCell (3, 9);
	pCell->SetText (_T("MOVE"));
}

void	C_AlignRcpDlg::SetProbeAlignData ()
{
	m_gridProbeAlignData.SetItemText (1, 0, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_ProbeY));
	m_gridProbeAlignData.SetItemText (1, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackLeft));
	m_gridProbeAlignData.SetItemText (1, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackRight));

	m_gridProbeAlignData.SetItemText (3, 0, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeY));
	m_gridProbeAlignData.SetItemText (3, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeX));
	m_gridProbeAlignData.SetItemText (3, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackY));
	m_gridProbeAlignData.SetItemText (3, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackLeft));
	m_gridProbeAlignData.SetItemText (3, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackRight));
	m_gridProbeAlignData.SetItemText (3, 5, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZRight));
	m_gridProbeAlignData.SetItemText (3, 6, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZLeft));
	m_gridProbeAlignData.SetItemText (3, 7, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fStageY));
}

BOOL	C_AlignRcpDlg::GetProbeAlignData (BOOL bShare, S_ProbeAlign *pProbeShare)
{
	S_ProbeAlign *pProbe;
	if (bShare) pProbe = pProbeShare;
	else		pProbe = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign;

	pProbe->m_fBase_ProbeY	  = _wtof (m_gridProbeAlignData.GetItemText (1, 0));
	pProbe->m_fBase_BackLeft  = _wtof (m_gridProbeAlignData.GetItemText (1, 1));
	pProbe->m_fBase_BackRight = _wtof (m_gridProbeAlignData.GetItemText (1, 2));

	pProbe->m_fProbeY		  = _wtof (m_gridProbeAlignData.GetItemText (3, 0));
	pProbe->m_fProbeX		  = _wtof (m_gridProbeAlignData.GetItemText (3, 1));
	pProbe->m_fBackY	      = _wtof (m_gridProbeAlignData.GetItemText (3, 2));
	pProbe->m_fBackLeft		  = _wtof (m_gridProbeAlignData.GetItemText (3, 3));
	pProbe->m_fBackRight	  = _wtof (m_gridProbeAlignData.GetItemText (3, 4));
	pProbe->m_fSubZRight		  = _wtof (m_gridProbeAlignData.GetItemText (3, 5));
	pProbe->m_fSubZLeft		  = _wtof (m_gridProbeAlignData.GetItemText (3, 6));
	pProbe->m_fStageY		  = _wtof (m_gridProbeAlignData.GetItemText (3, 7));

	return TRUE;
}

void	C_AlignRcpDlg::InitCcdSetMonitor ()
{
	InitAlignGridUI (m_gridCcdSetMntHead, IDC_GRID_5, 1, 5, 2);
	m_gridCcdSetMntHead.SetItemText (0, 0, _T("CCD"));
	m_gridCcdSetMntHead.SetItemText (0, 1, _T("EXP. TIME"));
	m_gridCcdSetMntHead.SetItemText (0, 2, _T("GAIN"));
	m_gridCcdSetMntHead.SetItemText (0, 3, _T("LED"));
	m_gridCcdSetMntHead.SetItemText (0, 4, _T("MAT. SCORE"));

	InitAlignGridUI (m_gridCcdSetMntData, IDC_GRID_6, 5, 5, 3);
	//m_gridCcdSetMntData.SetItemText (0, 0, _T("FRONT LEFT"));
	//m_gridCcdSetMntData.SetItemText (1, 0, _T("FRONT RIGHT"));
	//m_gridCcdSetMntData.SetItemText (2, 0, _T("BACK LEFT"));
	//m_gridCcdSetMntData.SetItemText (3, 0, _T("BACK RIGHT"));

	// LYS 2026-08-24 Align CCD Name 수정
	m_gridCcdSetMntData.SetItemText(0, 0, _T("GLASS LEFT"));
	m_gridCcdSetMntData.SetItemText(1, 0, _T("GLASS RIGHT"));
	m_gridCcdSetMntData.SetItemText(2, 0, _T("PROBE LEFT"));
	m_gridCcdSetMntData.SetItemText(3, 0, _T("PROBE RIGHT"));
	m_gridCcdSetMntData.SetItemText (4, 0, _T("CURRENT"));

	for (int i = 0; i < 4; i++) {
		m_gridCcdSetMntData.SetItemState (i, 1, m_gridCcdSetMntData.GetItemState (i, 1) | GVIS_READONLY);
		m_gridCcdSetMntData.SetItemState (i, 2, m_gridCcdSetMntData.GetItemState (i, 2) | GVIS_READONLY);
		m_gridCcdSetMntData.SetItemState (i, 3, m_gridCcdSetMntData.GetItemState (i, 3) | GVIS_READONLY);
		m_gridCcdSetMntData.SetItemState (i, 4, m_gridCcdSetMntData.GetItemState (i, 4) | GVIS_READONLY);
	}
}

void	C_AlignRcpDlg::SetCcdSetMntData ()
{
	m_gridCcdSetMntData.SetItemText (0, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nExposureTime));
	m_gridCcdSetMntData.SetItemText (1, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nExposureTime));
	m_gridCcdSetMntData.SetItemText (2, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nExposureTime));
	m_gridCcdSetMntData.SetItemText (3, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nExposureTime));

	m_gridCcdSetMntData.SetItemText (0, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nGain));
	m_gridCcdSetMntData.SetItemText (1, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nGain));
	m_gridCcdSetMntData.SetItemText (2, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nGain));
	m_gridCcdSetMntData.SetItemText (3, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nGain));

	m_gridCcdSetMntData.SetItemText (0, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nLed));
	m_gridCcdSetMntData.SetItemText (1, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nLed));
	m_gridCcdSetMntData.SetItemText (2, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nLed));
	m_gridCcdSetMntData.SetItemText (3, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nLed));

	m_gridCcdSetMntData.SetItemText (0, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nMatchingScore));
	m_gridCcdSetMntData.SetItemText (1, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nMatchingScore));
	m_gridCcdSetMntData.SetItemText (2, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nMatchingScore));
	m_gridCcdSetMntData.SetItemText (3, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nMatchingScore));
}

BOOL	C_AlignRcpDlg::GetCcdSetMntData (BOOL bShare, S_AlignCcdSet *pCcdShare)
{
	S_AlignCcdSet *pCcd;
	if (bShare) pCcd = pCcdShare;
	else		pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet;

	pCcd->m_sFrontLeft.m_nExposureTime	 = _wtoi (m_gridCcdSetMntData.GetItemText (0, 1));
	pCcd->m_sFrontRight.m_nExposureTime  = _wtoi (m_gridCcdSetMntData.GetItemText (1, 1));
	pCcd->m_sBackLeft.m_nExposureTime	 = _wtoi (m_gridCcdSetMntData.GetItemText (2, 1));
	pCcd->m_sBackRight.m_nExposureTime	 = _wtoi (m_gridCcdSetMntData.GetItemText (3, 1));

	pCcd->m_sFrontLeft.m_nGain			 = _wtoi (m_gridCcdSetMntData.GetItemText (0, 2));
	pCcd->m_sFrontRight.m_nGain			 = _wtoi (m_gridCcdSetMntData.GetItemText (1, 2));
	pCcd->m_sBackLeft.m_nGain			 = _wtoi (m_gridCcdSetMntData.GetItemText (2, 2));
	pCcd->m_sBackRight.m_nGain			 = _wtoi (m_gridCcdSetMntData.GetItemText (3, 2));

	pCcd->m_sFrontLeft.m_nLed			 = _wtoi (m_gridCcdSetMntData.GetItemText (0, 3));
	pCcd->m_sFrontRight.m_nLed			 = _wtoi (m_gridCcdSetMntData.GetItemText (1, 3));
	pCcd->m_sBackLeft.m_nLed			 = _wtoi (m_gridCcdSetMntData.GetItemText (2, 3));
	pCcd->m_sBackRight.m_nLed			 = _wtoi (m_gridCcdSetMntData.GetItemText (3, 3));

	pCcd->m_sFrontLeft.m_nMatchingScore  = _wtoi (m_gridCcdSetMntData.GetItemText (0, 4));
	pCcd->m_sFrontRight.m_nMatchingScore = _wtoi (m_gridCcdSetMntData.GetItemText (1, 4));
	pCcd->m_sBackLeft.m_nMatchingScore	 = _wtoi (m_gridCcdSetMntData.GetItemText (2, 4));
	pCcd->m_sBackRight.m_nMatchingScore  = _wtoi (m_gridCcdSetMntData.GetItemText (3, 4));

	return TRUE;
}

void	C_AlignRcpDlg::InitRefImage ()
{
	InitAlignGridUI (m_gridRefImageHead, IDC_GRID_7, 1, 3, 2);
	m_gridRefImageHead.SetItemText (0, 0, _T("ITEM"));
	m_gridRefImageHead.SetItemText (0, 1, _T("LEFT"));
	m_gridRefImageHead.SetItemText (0, 2, _T("RIGHT"));

	InitAlignGridUI (m_gridRefImageData, IDC_GRID_8, 10, 3, 3);
	m_gridRefImageData.SetItemText (0, 0, _T("GLASS_1"));
	m_gridRefImageData.SetItemText (1, 0, _T("GLASS_2"));
	m_gridRefImageData.SetItemText (2, 0, _T("GLASS_3"));
	m_gridRefImageData.SetItemText (3, 0, _T("GLASS_4"));
	m_gridRefImageData.SetItemText (4, 0, _T("GLASS_5"));
	m_gridRefImageData.SetItemText (5, 0, _T("PROBE_1"));
	m_gridRefImageData.SetItemText (6, 0, _T("PROBE_2"));
	m_gridRefImageData.SetItemText (7, 0, _T("PROBE_3"));
	m_gridRefImageData.SetItemText (8, 0, _T("PROBE_4"));
	m_gridRefImageData.SetItemText (9, 0, _T("PROBE_5"));

	for (int i = 0; i < 10; i++) {
		m_gridRefImageData.SetItemState (i, 1, m_gridRefImageData.GetItemState (i, 1) | GVIS_READONLY);
		m_gridRefImageData.SetItemState (i, 2, m_gridRefImageData.GetItemState (i, 2) | GVIS_READONLY);
	}
}

void	C_AlignRcpDlg::SetRefImageData ()
{
	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		m_gridRefImageData.SetItemText (0 + i, 1, g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnFrontLeft[i]);
		m_gridRefImageData.SetItemText (0 + i, 2, g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnFrontRight[i]);

		m_gridRefImageData.SetItemText (5 + i, 1, g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnBackLeft[i]);
		m_gridRefImageData.SetItemText (5 + i, 2, g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnBackRight[i]);
	}
}

BOOL	C_AlignRcpDlg::GetRefImageData (BOOL bShare, S_Align *pPatternShare)
{
	S_Align *pPattern;
	if (bShare) pPattern = pPatternShare;
	else		pPattern = &g_inRcpInfo.m_recipeMgr.m_sAlign;

	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		wcscpy_s (pPattern->m_chPtnFrontLeft[i],  m_gridRefImageData.GetItemText (0 + i, 1));
		wcscpy_s (pPattern->m_chPtnFrontRight[i], m_gridRefImageData.GetItemText (0 + i, 2));
	
		wcscpy_s (pPattern->m_chPtnBackLeft[i],  m_gridRefImageData.GetItemText (5 + i, 1));
		wcscpy_s (pPattern->m_chPtnBackRight[i], m_gridRefImageData.GetItemText (5 + i, 2));
	}

	return TRUE;
}

void	C_AlignRcpDlg::InitMargin ()
{
	InitAlignGridUI (m_gridMarginHead, IDC_GRID_10, 1, 6, 4);
	m_gridMarginHead.SetItemText (0, 0, _T("ITEM"));
	m_gridMarginHead.SetItemText (0, 1, _T("MARGIN X"));
	m_gridMarginHead.SetItemText (0, 2, _T("MARGIN Y"));
	m_gridMarginHead.SetItemText (0, 3, _T("MARGIN R"));
	m_gridMarginHead.SetItemText (0, 4, _T("CENTERING"));
	m_gridMarginHead.SetItemText (0, 5, _T("FIND PTN."));

	InitAlignGridUI (m_gridMarginData, IDC_GRID_11, 2, 6, 5);
	m_gridMarginData.SetItemText (0, 0, _T("GLASS"));
	m_gridMarginData.SetItemText (1, 0, _T("PROBE"));

	m_gridMarginData.SetItemState (0, 5, m_gridMarginData.GetItemState (0, 5) | GVIS_READONLY);
}

void	C_AlignRcpDlg::SetMarginData ()
{
	m_gridMarginData.SetItemText (0, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fX));
	m_gridMarginData.SetItemText (0, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fY));
	m_gridMarginData.SetItemText (0, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fR));
	m_gridMarginData.SetItemText (0, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_nCentering));

	m_gridMarginData.SetItemText (1, 1, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fX));
	m_gridMarginData.SetItemText (1, 2, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fY));
	m_gridMarginData.SetItemText (1, 3, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fR));
	m_gridMarginData.SetItemText (1, 4, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nCentering));
	m_gridMarginData.SetItemText (1, 5, dfcStr (g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nFindPtn));
}

BOOL	C_AlignRcpDlg::GetMarginData (BOOL bShare, S_AlignMargin *pMarginShare)
{
	S_AlignMargin *pMargin;
	if (bShare) pMargin = pMarginShare;
	else		pMargin = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin;

	pMargin->m_sGlassMargin.m_fX		 = _wtof (m_gridMarginData.GetItemText (0, 1));
	pMargin->m_sGlassMargin.m_fY		 = _wtof (m_gridMarginData.GetItemText (0, 2));
	pMargin->m_sGlassMargin.m_fR		 = _wtof (m_gridMarginData.GetItemText (0, 3));
	pMargin->m_sGlassMargin.m_nCentering = _wtoi (m_gridMarginData.GetItemText (0, 4));

	pMargin->m_sProbeMargin.m_fX		 = _wtof (m_gridMarginData.GetItemText (1, 1));
	pMargin->m_sProbeMargin.m_fY		 = _wtof (m_gridMarginData.GetItemText (1, 2));
	pMargin->m_sProbeMargin.m_fR		 = _wtof (m_gridMarginData.GetItemText (1, 3));
	pMargin->m_sProbeMargin.m_nCentering = _wtoi (m_gridMarginData.GetItemText (1, 4));
	pMargin->m_sProbeMargin.m_nFindPtn   = _wtoi (m_gridMarginData.GetItemText (1, 5));

	return TRUE;
}

void	C_AlignRcpDlg::InitAlignProc ()
{
	InitAlignGridUI (m_gridAlignProcData, IDC_GRID_12, 4, 1, 6);
	m_gridAlignProcData.SetFont (dfcGetFontName (), 13, FW_SEMIBOLD);

	////버튼
	for (int i = 0; i < 4; i++) {
		if (i == 1) continue;
		m_gridAlignProcData.SetCellType (i, 0, RUNTIME_CLASS(CGridCellButton));
		m_gridAlignProcData.SetItemState (i, 0, m_gridAlignProcData.GetItemState (i, 0) | GVIS_READONLY);
	}

	//버튼 이름
	CGridCellButton* pCell = NULL;
	pCell = (CGridCellButton*)m_gridAlignProcData.GetCell (0, 0);
	pCell->SetText (_T("GLASS ALIGN"));
	pCell = (CGridCellButton*)m_gridAlignProcData.GetCell (2, 0);
	pCell->SetText (_T("PROBE ALIGN"));
	pCell = (CGridCellButton*)m_gridAlignProcData.GetCell (3, 0);
	pCell->SetText (_T("ALIGN RESET"));

	//콤보박스
	m_gridAlignProcData.SetCellType  (1, 0, RUNTIME_CLASS(CGridCellCombo));
	m_gridAlignProcData.SetItemState (1, 0, m_gridAlignProcData.GetItemState (1, 0) & GVIS_READONLY);

	//콤보 데이터
	CStringArray probe;
	probe.Add (_T("PROBE 1"));
	probe.Add (_T("PROBE 2"));
	probe.Add (_T("PROBE 3"));
	probe.Add (_T("PROBE 4"));
	probe.Add (_T("PROBE 5"));

	CGridCellCombo* pCell2 = NULL;
	pCell2 = (CGridCellCombo*)m_gridAlignProcData.GetCell (1, 0);
	pCell2->SetStyle (CBS_DROPDOWNLIST);	
	pCell2->SetOptions (probe);
	m_gridAlignProcData.SetItemCurSel (1, 0, 4);	// jsyoon 2022.08.17 chnage 0 to 4
}

void	C_AlignRcpDlg::InitAlignPos ()
{
	InitAlignGridUI (m_gridAlignPosHead, IDC_GRID_13, 1, 4, 7);
	m_gridAlignPosHead.SetItemText (0, 0, _T("X"));
	m_gridAlignPosHead.SetItemText (0, 1, _T("Y"));
	m_gridAlignPosHead.SetItemText (0, 2, _T("X"));
	m_gridAlignPosHead.SetItemText (0, 3, _T("Y"));

	InitAlignGridUI (m_gridAlignPosData, IDC_GRID_14, 2, 4, 6);

	for (int i = 0; i < 4; i++) {
		m_gridAlignPosData.SetItemState (0, i, m_gridAlignPosData.GetItemState (0, i) | GVIS_READONLY);
		m_gridAlignPosData.SetItemState (1, i, m_gridAlignPosData.GetItemState (1, i) | GVIS_READONLY);
		m_gridAlignPosData.SetItemText  (0, i, _T("0.0000"));
		m_gridAlignPosData.SetItemText  (1, i, _T("0.0000"));
	}
}

void	C_AlignRcpDlg::InitAlignGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, int nType)
{
	CRect rc;
	GetDlgItem (nDlgItem)->GetWindowRect (&rc);
	ScreenToClient (&rc);
	DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE;
	
	Grid.SetFont (dfcGetFontName (), 12, FW_BOLD);

	Grid.SetColumnCount (nCol);
	Grid.SetRowCount (nRow);
	
	Grid.EnableToolTips (FALSE);
	Grid.EnableTitleTips (FALSE);
	Grid.GetWindowRect (&rc);

	int	nWidth = 0;
	int nHeight = 0;

	if (nType == 0 || nType == 1) {
		Grid.SetFixedRowCol (0, 0);
		for (int i = 0; i < 6; i++) Grid.SetColumnWidth (i, 90);

		if (nType == 0) {					// LYS 2026-09-04 GLASS: data 0-6, buttons 7-8
			Grid.SetColumnWidth (6, 90);		// SUB Z LEFT
			Grid.SetColumnWidth (7, 60);
			Grid.SetColumnWidth (8, 60);
		}
		else {								// LYS 2026-09-04 PROBE: data 0-7, buttons 8-9
			Grid.SetColumnWidth (6, 90);		// SUB Z LEFT
			Grid.SetColumnWidth (7, 90);		// STAGE Y
			Grid.SetColumnWidth (8, 60);
			Grid.SetColumnWidth (9, 60);
		}
	}
	else  if (nType == 2 || nType == 3) {
		if (nType == 2) Grid.SetFixedRowCol (1, 1);
		else			Grid.SetFixedRowCol (0, 1);

		nWidth = (rc.Width () - 4) / 4;
		Grid.SetColumnWidth (0, nWidth);
		nWidth = ((rc.Width () - 4) - nWidth) / (nCol - 1);
		for (int i = 1; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);
	}
	else  if (nType == 4 || nType == 5) {
		if (nType == 4) Grid.SetFixedRowCol (1, 1);
		else			Grid.SetFixedRowCol (0, 1);

		nWidth = (rc.Width () - 4) / 5;
		Grid.SetColumnWidth (0, nWidth);
		nWidth = ((rc.Width () - 4) - nWidth) / (nCol - 1);
		for (int i = 1; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);
	}
	else if (nType == 6 || nType == 7) {
		if (nType == 6) Grid.SetFixedRowCol (0, 0);
		else			Grid.SetFixedRowCol (1, 1);

		nWidth = (rc.Width () - 4) / nCol;
		for (int i = 0; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);
	}

	nHeight = (rc.Height () - 4) / nRow;
	for (int i = 0; i < nRow; i++) Grid.SetRowHeight (i, nHeight);


	for (int i = 0; i < nRow; i++) {
		for (int j = 0; j < nCol; j++) {
			Grid.SetItemFormat (i, j, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}
	Grid.SetFixedTextColor (COLOR_WHITE);
	Grid.SetFixedBkColor (COLOR_FRAME);
}

void	C_AlignRcpDlg::SetAlignView ()
{
	SetGlassAlignData ();	//Glass Align Info
	SetProbeAlignData ();	//Probe Align Info
	SetCcdSetMntData ();	//CCD Set Info
	SetRefImageData ();		//Refernce Image
	SetMarginData ();		//Marign Info

	Invalidate (FALSE);
}

BOOL	C_AlignRcpDlg::GetAlignView ()
{
	if (!GetGlassAlignData ()) return FALSE; //Glass Align Info
	if (!GetProbeAlignData ()) return FALSE; //Probe Align Info
	if (!GetCcdSetMntData ())  return FALSE; //CCD Set Info
	if (!GetRefImageData ())   return FALSE; //Refernce Image
	if (!GetMarginData ())     return FALSE; //Marign Info

	return TRUE;
}

BOOL	C_AlignRcpDlg::SaveAlignFile (CString strRcpName)
{
	CString strPath;
	strPath = theApp.m_strAlignFolder + strRcpName + _T(".alg");

	dfcCreateDirectory (strPath);

	CStdioFile File;
	if(!File.Open (strPath, CFile::modeCreate | CFile::modeWrite)) return FALSE; // file 생성 오류

	CString str, strTemp, strTemp2, strTemp3, strTemp4;
	int nTemp = 0;

	////Version Header
	//WriteString (&File, _T("_VERSION_"));
	////Version Data
	//WriteInt (&File, RECIPE_FILE_END);

	//Glass Align Header
	WriteString (&File, _T("_GLASS_ALIGN_"));
	//Glass Align Data
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_StageY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontLeft);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontRight);

	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageRotate);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontLeft);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontRight);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontZ);

	//Probe Align Header
	WriteString (&File, _T("_PROBE_ALIGN_"));
	//Probe Align Data
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_ProbeY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackLeft);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackRight);

	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeX);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackLeft);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackRight);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackZ);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fStageY);

	//CCD Set Header
	WriteString (&File, _T("_CCD_SET_"));
	//CCD Set Data
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nExposureTime);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nExposureTime);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nExposureTime);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nExposureTime);

	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nGain);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nGain);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nGain);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nGain);

	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nLed);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nLed);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nLed);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nLed);

	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nMatchingScore);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nMatchingScore);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nMatchingScore);
	WriteInt (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nMatchingScore);

	//Reference Image Header
	WriteString (&File, _T("_CCD_SET_"));
	//Reference Image Data
	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		str.Format (_T("%s,"), g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnFrontLeft[i]);
		strTemp += str;

		str.Format (_T("%s,"), g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnFrontRight[i]);
		strTemp2 += str;

		str.Format (_T("%s,"), g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnBackLeft[i]);
		strTemp3 += str;

		str.Format (_T("%s,"), g_inRcpInfo.m_recipeMgr.m_sAlign.m_chPtnBackRight[i]);
		strTemp4 += str;
	}
	WriteString (&File, strTemp);
	WriteString (&File, strTemp2);
	WriteString (&File, strTemp3);
	WriteString (&File, strTemp4);

	//Margin Header
	WriteString (&File, _T("_CENTERING_MARGIN_"));
	//Margin Data
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fX);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fR);
	WriteInt    (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_nCentering);

	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fX);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fY);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fR);
	WriteInt    (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nCentering);
	WriteInt    (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nFindPtn);

	// LYS 2026-09-04 SUB Z trailing block (versionless-safe) : Z3 RIGHT / Z4 LEFT
	WriteString (&File, _T("_SUB_Z_"));
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZRight);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZLeft);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZRight);
	WriteDouble (&File, g_inRcpInfo.m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZLeft);

	File.Close ();

	return TRUE;
}

void	C_AlignRcpDlg::GetCcdCurrentView (int nHEAD)
{
	S_AlignCcdItem *pCcd;
	if		(nHEAD == ALG_FRONT_LEFT)  pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft;
	else if (nHEAD == ALG_FRONT_RIGHT) pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight;
	else if (nHEAD == ALG_BACK_LEFT)   pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft;
	else if (nHEAD == ALG_BACK_RIGHT)  pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight;

	m_gridCcdSetMntData.SetItemText (4, 1, dfcStr (pCcd->m_nExposureTime));
	m_gridCcdSetMntData.SetItemText (4, 2, dfcStr (pCcd->m_nGain));
	m_gridCcdSetMntData.SetItemText (4, 3, dfcStr (pCcd->m_nLed));
	m_gridCcdSetMntData.SetItemText (4, 4, dfcStr (pCcd->m_nMatchingScore));

	Invalidate (FALSE);
}

void	C_AlignRcpDlg::OnSelCcdCurrent (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	if (pNMLV->iColumn == 0) {
		m_nSelCcd = -1;
		return;
	}
	
	if (pNMLV->iColumn == 1) {
		if (pNMLV->iRow >= 0 && pNMLV->iRow <= 4) {
			m_nSelCcd = ALG_FRONT_LEFT;
			GetCcdCurrentView (m_nSelCcd);
		}
		else if (pNMLV->iRow >= 5 && pNMLV->iRow <= 9) {
			m_nSelCcd = ALG_BACK_LEFT;
			GetCcdCurrentView (m_nSelCcd);
		}
	}
	else if (pNMLV->iColumn == 2) {
		if (pNMLV->iRow >= 0 && pNMLV->iRow <= 4) {
			m_nSelCcd = ALG_FRONT_RIGHT;
			GetCcdCurrentView (m_nSelCcd);
		}
		else if (pNMLV->iRow >= 5 && pNMLV->iRow <= 9) {
			m_nSelCcd = ALG_BACK_RIGHT;
			GetCcdCurrentView (m_nSelCcd);
		}
	}

	m_nRefRow = pNMLV->iRow;
	m_nRefCol = pNMLV->iColumn;

	CString strPath, strName;
	strName = m_gridRefImageData.GetItemText (pNMLV->iRow, pNMLV->iColumn);

	if (!m_image.IsNull ()) m_image.Destroy ();
	if (!strName.IsEmpty ()) {
		strPath.Format (_T("%s%s"), theApp.m_strRefImgFolder, strName);
		m_image.Load (strPath);
	}

	ShowRefImage ();
}

void	C_AlignRcpDlg::OnSelRefImg (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	if (pNMLV->iColumn == 0) return;

	CString strfilePath, strName;
	
	if (pNMLV->iRow >= 0 && pNMLV->iRow <= 4) {
		strfilePath.Format (_T("%s*.png"), theApp.m_strRefImgFolder);
	}
	else if (pNMLV->iRow >= 5 && pNMLV->iRow <= 9) {
		strfilePath.Format (_T("%s*.png"), theApp.m_strRefImgFolder);
	}

	CFileDialog dlg (TRUE, _T("png"), strfilePath, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, _T("Image Files (*.png)|*.png|"));

	if (dlg.DoModal() == IDOK)
	{			
		strName = dlg.GetFileName ();

		if (!strName.IsEmpty ()) {
			m_gridRefImageData.SetItemText (pNMLV->iRow, pNMLV->iColumn, strName);
			Invalidate (FALSE);

			strfilePath.Format (_T("%s%s"), theApp.m_strRefImgFolder, strName);
			if (!m_image.IsNull ()) m_image.Destroy ();
			m_image.Load (strfilePath);

			ShowRefImage ();
		}
		else {
			AfxMessageBox (_T("선택된 이미지가 없습니다."));
			return;
		}
	}	
}

void	C_AlignRcpDlg::OnSelGlassAlign (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	if (pNMLV->iRow != 3 || pNMLV->iColumn < 7) return;
	if (pNMLV->iColumn == 7) { //Glass Pos Get
		if (AfxMessageBox (_T("Glass Align 현재 위치 값을 가져 옵니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
		::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_GLASS_GET_POS, NULL);
	}
	else {					   //Glass Pos Move
		if (AfxMessageBox (_T("Glass Align 위치로 이동 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
		::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_GLASS_MOVE, NULL);
	}
}

void	C_AlignRcpDlg::OnSelProbeAlign (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	if (pNMLV->iRow != 3 || pNMLV->iColumn < 8) return;
	if (pNMLV->iColumn == 8) { //Probe Pos Get
		if (AfxMessageBox (_T("Probe Align 현재 위치 값을 가져 옵니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
		::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_PROBE_GET_POS, NULL);
	}
	else {					   //Probe Pos Move
		if (AfxMessageBox (dfcStr (_T("Probe Align (%d번) 위치로 이동 합니다.\n 계속 하시겠습니까?"), GetProbeSel () + 1), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
		::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_PROBE_MOVE, NULL);
	}
}

void	C_AlignRcpDlg::OnAlignProc (NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_GRIDVIEW*	pNMLV = reinterpret_cast<NM_GRIDVIEW*>(pNMHDR);

	if (pNMLV->iRow == 0 || pNMLV->iRow == 2) { //Align
		//m_pRecipeMem->m_sAlign.Clear ();
		
		if (pNMLV->iRow == 0) {
			if (AfxMessageBox (_T("Glass Align 을 진행 합니다.\n 계속 하시겠습니까?"), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
			//GetGlassAlignData (TRUE); //Glass Align Info
			::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_GLASS_PROC, NULL);
		}
		else if (pNMLV->iRow == 2) {
			if (AfxMessageBox (dfcStr (_T("%d번 Probe Align 을 진행 합니다.\n 계속 하시겠습니까?"), GetProbeSel () + 1), MB_YESNO|MB_ICONQUESTION) != IDYES) return;
			//GetProbeAlignData (TRUE); //Probe Align Info
			::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_PROBE_PROC, NULL);
		}

		//GetCcdSetMntData (TRUE); //CCD Set Info
		//GetRefImageData (TRUE);	 //Refernce Image
		//GetMarginData (TRUE);	 //Marign Info
	}
	else if (pNMLV->iRow == 3) { //Align Reset
		::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_RESET, NULL);
	}
}

CString		C_AlignRcpDlg::GetCcdItemText (int m_nSelCcd)
{
	CString strCcd;

	//if		(m_nSelCcd == ALG_FRONT_LEFT)  strCcd = _T("Front Left");
	//else if (m_nSelCcd == ALG_FRONT_RIGHT) strCcd = _T("Front Right");
	//else if (m_nSelCcd == ALG_BACK_LEFT)   strCcd = _T("Back Left");
	//else if (m_nSelCcd == ALG_BACK_RIGHT)  strCcd = _T("Back Right");

	// LYS 2026-08-24 Align CCD Name 수정
	if (m_nSelCcd == ALG_FRONT_LEFT)	   strCcd = _T("Glass Align Left");
	else if (m_nSelCcd == ALG_FRONT_RIGHT) strCcd = _T("Glass Align Right");
	else if (m_nSelCcd == ALG_BACK_LEFT)   strCcd = _T("Probe Align Left");
	else if (m_nSelCcd == ALG_BACK_RIGHT)  strCcd = _T("Probe Align Right");

	return strCcd;
}

void	C_AlignRcpDlg::OnCcdSetUpdate ()
{
	if (m_nSelCcd == -1) {
		AfxMessageBox (_T("선택된 항목이 없습니다."));
		return;
	}

	S_AlignCcdItem *pCcd;
	if		(m_nSelCcd == ALG_FRONT_LEFT)  pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft;
	else if (m_nSelCcd == ALG_FRONT_RIGHT) pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight;
	else if (m_nSelCcd == ALG_BACK_LEFT)   pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft;
	else if (m_nSelCcd == ALG_BACK_RIGHT)  pCcd = &g_inRcpInfo.m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight;

	if (AfxMessageBox (dfcStr (_T("%s 값을 현재값으로 업데이트 합니다.\n 계속 하시겠습니까?"), GetCcdItemText (m_nSelCcd)), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	pCcd->m_nExposureTime  = _wtoi (m_gridCcdSetMntData.GetItemText (4, 1));
	pCcd->m_nGain		   = _wtoi (m_gridCcdSetMntData.GetItemText (4, 2));
	pCcd->m_nLed		   = _wtoi (m_gridCcdSetMntData.GetItemText (4, 3));
	pCcd->m_nMatchingScore = _wtoi (m_gridCcdSetMntData.GetItemText (4, 4));

	//if (pCcd->m_nExposureTime < 0 || pCcd->m_nExposureTime > 30000) {
	//	AfxMessageBox (_T("ExposureTime 값은 0 ~ 30000 사이값으로 가능합니다."));
	//	return;
	//}
	if (pCcd->m_nGain < 0 || pCcd->m_nGain > 1000) {
		AfxMessageBox (_T("Gain 값은 0 ~ 1000 사이값으로 가능합니다."));
		return;
	}
	if (pCcd->m_nLed < 0 || pCcd->m_nLed > 255) {
		AfxMessageBox (_T("Led 값은 0 ~ 255 사이값으로 가능합니다."));
		return;
	}
	if (pCcd->m_nMatchingScore < 0 || pCcd->m_nMatchingScore > 1000) {
		AfxMessageBox (_T("Matching Score 값은 0 ~ 1000 사이값으로 가능합니다."));
		return;
	}

	m_gridCcdSetMntData.SetItemText (m_nSelCcd, 1, dfcStr (pCcd->m_nExposureTime));
	m_gridCcdSetMntData.SetItemText (m_nSelCcd, 2, dfcStr (pCcd->m_nGain));
	m_gridCcdSetMntData.SetItemText (m_nSelCcd, 3, dfcStr (pCcd->m_nLed));
	m_gridCcdSetMntData.SetItemText (m_nSelCcd, 4, dfcStr (pCcd->m_nMatchingScore));

	Invalidate (FALSE);
}

void	C_AlignRcpDlg::OnShowRefImageSet ()
{
	if (m_nSelCcd == -1) {
		AfxMessageBox (_T("선택된 항목이 없습니다."));
		return;
	}

	if (AfxMessageBox (dfcStr (_T("%s 패턴 설정을 진행 합니다.\n 계속 하시겠습니까?"), GetCcdItemText (m_nSelCcd)), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_SHOW_REFIMG, (LPARAM)m_nSelCcd);
}

void	C_AlignRcpDlg::OnPatternSearch ()
{
	if (m_nSelCcd == -1) {
		AfxMessageBox (_T("선택된 항목이 없습니다."));
		return;
	}

	if (AfxMessageBox (dfcStr (_T("%s 패턴 찾기를 진행 합니다.\n 계속 하시겠습니까?"), GetCcdItemText (m_nSelCcd)), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_PATTERN_SEARCH, m_nSelCcd); 
}

void	C_AlignRcpDlg::OnAlignCcdSet ()
{
	if (m_nSelCcd == -1) {
		AfxMessageBox (_T("선택된 항목이 없습니다."));
		return;
	}

	if (AfxMessageBox (dfcStr (_T("%s 의 Current 값으로 설정 합니다.\n 계속 하시겠습니까?"), GetCcdItemText (m_nSelCcd)), MB_YESNO|MB_ICONQUESTION) != IDYES) return;

	::PostMessage (m_hWnd->m_hWnd, WM_RECIPE_EVENT, (WPARAM)M_ALIGN_CCD_SET, m_nSelCcd);
}

void	C_AlignRcpDlg::GetCcdCurrentVal (int *nData)
{
	nData[0] = _wtoi (m_gridCcdSetMntData.GetItemText (4, 1)); //Exp. Time
	nData[1] = _wtoi (m_gridCcdSetMntData.GetItemText (4, 2)); //Gain
	nData[2] = _wtoi (m_gridCcdSetMntData.GetItemText (4, 3)); //Led
	nData[3] = _wtoi (m_gridCcdSetMntData.GetItemText (4, 4)); //Mat. Score
}

void	C_AlignRcpDlg::GetAlignPos (double *fData)
{
	fData[0] = _wtof (m_gridGlassAlignData.GetItemText (3, 0)); //Stage Y
	fData[1] = _wtof (m_gridGlassAlignData.GetItemText (3, 1)); //Stage Rotate
	fData[2] = _wtof (m_gridGlassAlignData.GetItemText (3, 2)); //Front Y
	fData[3] = _wtof (m_gridGlassAlignData.GetItemText (3, 3)); //Front Left
	fData[4] = _wtof (m_gridGlassAlignData.GetItemText (3, 4)); //Front Right
	fData[5] = _wtof (m_gridGlassAlignData.GetItemText (3, 5)); //SUB Z RIGHT
	fData[6] = _wtof (m_gridGlassAlignData.GetItemText (3, 6)); //SUB Z LEFT
}

void	C_AlignRcpDlg::SetAlignPos (double *fData)
{
	m_gridGlassAlignData.SetItemText (3, 0, dfcStr (fData[0])); //Stage Y
	m_gridGlassAlignData.SetItemText (3, 1, dfcStr (fData[1])); //Stage Rotate
	m_gridGlassAlignData.SetItemText (3, 2, dfcStr (fData[2])); //Front Y
	m_gridGlassAlignData.SetItemText (3, 3, dfcStr (fData[3])); //Front Left
	m_gridGlassAlignData.SetItemText (3, 4, dfcStr (fData[4])); //Front Right
	m_gridGlassAlignData.SetItemText (3, 5, dfcStr (fData[5])); //SUB Z RIGHT
	m_gridGlassAlignData.SetItemText (3, 6, dfcStr (fData[6])); //SUB Z LEFT
}

int		C_AlignRcpDlg::GetProbeSel ()
{
	return m_gridAlignProcData.GetItemCurSel (1, 0);
}

void	C_AlignRcpDlg::GetProbePos (double *fData)
{
	fData[0] = _wtof (m_gridProbeAlignData.GetItemText (3, 0)); //Probe Y
	fData[1] = _wtof (m_gridProbeAlignData.GetItemText (3, 1)); //Probe X
	fData[2] = _wtof (m_gridProbeAlignData.GetItemText (3, 2)); //Back Y
	fData[3] = _wtof (m_gridProbeAlignData.GetItemText (3, 3)); //Back Left
	fData[4] = _wtof (m_gridProbeAlignData.GetItemText (3, 4)); //Back Right
	fData[5] = _wtof (m_gridProbeAlignData.GetItemText (3, 5)); //Back Z
	fData[6] = _wtof (m_gridProbeAlignData.GetItemText (3, 6)); //SUB Z LEFT
	fData[7] = _wtof (m_gridProbeAlignData.GetItemText (3, 7)); //Stage Y
}

void	C_AlignRcpDlg::SetProbePos (double *fData)
{
	m_gridProbeAlignData.SetItemText (3, 0, dfcStr (fData[0])); //Probe Y
	m_gridProbeAlignData.SetItemText (3, 1, dfcStr (fData[1])); //Probe X
	m_gridProbeAlignData.SetItemText (3, 2, dfcStr (fData[2])); //Back Y
	m_gridProbeAlignData.SetItemText (3, 3, dfcStr (fData[3])); //Back Left
	m_gridProbeAlignData.SetItemText (3, 4, dfcStr (fData[4])); //Back Right
	m_gridProbeAlignData.SetItemText (3, 5, dfcStr (fData[5])); //Back Z
	m_gridProbeAlignData.SetItemText (3, 6, dfcStr (fData[6])); //SUB Z LEFT
	m_gridProbeAlignData.SetItemText (3, 7, dfcStr (fData[7])); //Stage Y
}

void	C_AlignRcpDlg::SetAlignEndPos (double *fData, int nItem)
{
	if (nItem == 0 || nItem == 1) {
		m_gridAlignPosData.SetItemText (nItem, 0, dfcStr (fData[0])); //Glass Left X	/ Probe Left X
		m_gridAlignPosData.SetItemText (nItem, 1, dfcStr (fData[1])); //Glass Left Y	/ Probe Left Y
		m_gridAlignPosData.SetItemText (nItem, 2, dfcStr (fData[2])); //Glass Right X	/ Probe Right X
		m_gridAlignPosData.SetItemText (nItem, 3, dfcStr (fData[3])); //Glass Right Y	/ Probe Right Y
	}
}