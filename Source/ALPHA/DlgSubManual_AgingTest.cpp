#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AgingTest.h"
#include "afxdialogex.h"

#include <iostream>
#include <bitset>

IMPLEMENT_DYNAMIC(CDlgSubManual_AgingTest, CDialogEx)

CDlgSubManual_AgingTest::CDlgSubManual_AgingTest(CWnd* pParent /*=NULL*/) 
	:CDialogEx(CDlgSubManual_AgingTest::IDD, pParent)
{
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
	for (int i = 0; i < MAX_AGING_STICK; i++) DDX_Control(pDX, IDC_CHECK_1 + i, m_chkStick[i]);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_AgingTest, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_COMMAND(IDC_BUTTON_1, OnStop)
	ON_COMMAND(IDC_BUTTON_2, OnRcpOpen)
	ON_COMMAND(IDC_BUTTON_3, OnItemClearAll)
	ON_COMMAND(IDC_BUTTON_4, OnItemAdd)
	ON_COMMAND(IDC_BUTTON_5, OnItemDel)
	ON_COMMAND(IDC_BUTTON_6, OnItemSave)
	ON_COMMAND(IDC_BUTTON_7, OnLoadAgingInfo)
	ON_COMMAND(IDC_BUTTON_8, OnAgingProcStrart)
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

	m_strAgingRcpPath = theApp.GetProfileString(_T("DIR"), _T("AGING_RCP"), _T("D:\\KMAC\\PARAM_ICP\\PARAM\\AgingRcp\\"));
	GetDlgItem(IDC_EDIT_2)->SetWindowText(m_strAgingRcpPath);

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

void CDlgSubManual_AgingTest::OnStop()
{
	//gLog(m_pSystem->hLog, "CDlgSubManual_AgingTest::OnStop()", LOG_LEVEL_O);
	//m_pJobProc->ActionStop();
}

void CDlgSubManual_AgingTest::OnRcpOpen()
{
	//if (!IsOfflineStatus()) return;
	//
	//gLog(m_pSystem->hLog, _T("CDlgSubManual_AgingTest::OnRcpOpen()"), LOG_LEVEL_O);
	//
	////CDlgNaviManualMeas::LoadRecipeDlg()
	//CString filename = _T(""), strRcpFile;
	//CString filter = _T("Recipe Files (*.rcp)|*.rcp|");
	//filename.Format(_T("%s%s%s*.rcp"), m_pSystem->m_strRootPath, STR_DIR_PARAM, dfRECIPE_DATA_FILE_PATH);
	//
	//CFileDialog dlg(TRUE, _T("rcp"), filename, OFN_FILEMUSTEXIST, filter); //OFN_FILEMUSTEXIST (존재하지 않는 파일명을 입력할 수 없음)
	//dlg.GetFileName().Format(_T("%s"), m_pSystem->stLotInfo.m_szRecipeID);
	//
	//if (dlg.DoModal() == IDOK)
	//{
	//	filename = dlg.GetFileName();
	//
	//	if (filename.IsEmpty()) MessageBox(_T("File is not an Recipe file or is locked and cannot be imported."));
	//	else
	//	{
	//		int j = 0;
	//		j = filename.Find(_T(".rcp"));
	//
	//		if (j > 0)
	//		{
	//			strRcpFile = filename.Left(j);		// remove ".rcp"
	//		}
	//		else
	//		{
	//			strRcpFile = filename;
	//		}
	//
	//		if (m_pDlgNaviRecipe->LoadPPID2(strRcpFile) == VTM_RV_SUCCESSED)
	//		{
	//			MessageBox(_T("Recipe file be imported complete."));
	//
	//			GetDlgItem(IDC_EDIT_1)->SetWindowText(m_pSystem->stLotInfo.m_szRecipeID);
	//
	//			UpdateAgingCell();
	//			LoadAgingInfo();
	//			return;
	//		}
	//		else
	//		{
	//			AfxMessageBox(_T("Load Recipe Error"));
	//		}
	//	}
	//}

	return;
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
	
	//UpdateListNum();
}

void CDlgSubManual_AgingTest::OnItemSave()
{
	CString strCurrentRcp = _T("");/*(CString)m_pSystem->stLotInfo.m_szRecipeID;*/
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

void CDlgSubManual_AgingTest::OnAgingProcStrart()
{
	//if (!IsOfflineStatus()) return;
	//if (SMEM_GetSys_ProcessStatus() == SPS_PROCESS) {
	//	AfxMessageBox(_T("프로세스 진행중 입니다.\n 프로세스 완료후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	////20180429 add by Hee
	//if (!m_pSystem->stInterLock.m_bSafetyPos_X) {
	//	AfxMessageBox(_T("Safety 위치가 아닙니다. \n\n안전위치 이동 후 진행하십시오."));
	//	return;
	//}
	//
	////OCP 는 적용 제외
	////if (m_pSystem->m_bProbeRotateInstall) {
	////	if (fabs (m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.GetCurPos (AXIS_R1)) > 0.01) {
	////		AfxMessageBox (_T("Safety 위치가 아닙니다. \n\n안전위치 이동 후 진행하십시오."));
	////		return;
	////	}
	////}
	//
	//if (AfxMessageBox(_T("Aging Proc 를 진행 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	//
	//CString strRcp;
	//GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	//
	//if (strRcp == _T("") || strRcp == _T("UNKNOWN")) {
	//	AfxMessageBox(_T("레시피 정보가 없습니다.\n 레시피 설정 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//int nMaxCount = m_spreadAgingList.GetMaxRows();
	//
	//if (nMaxCount <= 1) {
	//	AfxMessageBox(_T("리스트 정보가 없습니다.\n 리스트 추가 및 저장 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	////Cycle Count 정보
	//CString strTemp;
	//int nCycleCount = 1;
	//m_cmbCycle.GetWindowText(strTemp);
	//nCycleCount = atoi(strTemp);
	//
	//////스틱 정보
	////m_nStick.clear ();
	////for (int i = 0; i < MAX_AGING_STICK; i++) {
	////	if (m_chkStick[i].IsWindowEnabled ()) {
	////		m_nStick.push_back (m_chkStick[i].GetCheck ());
	////	}
	////	else break;
	////}
	//
	//////Aging 리스트 정보
	////m_vecAginList.clear ();
	////S_AgingTest sAgingList;
	////for (int i = 0; i < nMaxCount - 1; i++) {
	////	m_spreadAgingList.SetRow (i + 2);
	//
	////	for (int j = 2; j < MAX_AGING_ITEM + 1; j++) {
	////		m_spreadAgingList.SetCol (j);
	//
	////		if (j == 2) {
	////			strTemp = m_spreadAgingList.GetValue (); 
	////			sAgingList.m_bUseAging = atoi (strTemp);
	////		}
	////		else if (j == 3) {
	////			m_spreadAgingList.GetWindowText (strTemp);
	////			sAgingList.m_nPtnRGB = atoi (strTemp);
	////		}
	////		else if (j == 4) {
	////			m_spreadAgingList.GetWindowText (strTemp);
	////			sAgingList.m_nPtnSec = atoi (strTemp);
	////		}
	////		else if (j == 5) {
	////			strTemp = m_spreadAgingList.GetValue ();
	////			sAgingList.m_bUseProperty = atoi (strTemp);
	////		}
	////		else if (j == 6) {
	////			strTemp = m_spreadAgingList.GetValue ();
	////			sAgingList.m_bUseQuality = atoi (strTemp);
	////		}
	////	}
	//
	////	m_vecAginList.push_back (sAgingList);
	////}
	//
	//m_pSystem->m_bAutoAgingMode = TRUE;
	//m_pSystem->nJobCount = nCycleCount;
	//
	////진행 위치
	//m_pSystem->m_nAgingListRow = 0;
	//m_pSystem->m_nAgingListCol = 0;
	//
	//m_pJobProc->DoStartCycleTest();
}

LRESULT CDlgSubManual_AgingTest::OnSpreadDBClick(WPARAM wParam, LPARAM lParam)
{
	return 1;
}

void CDlgSubManual_AgingTest::UpdateListNum()
{
	CString str;
	int nMaxRow = m_spreadAgingList->GetMaxRows();
	//m_spreadAgingList.SetCol(1);

	for (int i = 1; i < nMaxRow; i++) {
		//m_spreadAgingList.SetRow(i + 1);
		//m_spreadAgingList.SetCellType(1);
		//m_spreadAgingList.SetTypeHAlign(2);
		//m_spreadAgingList.SetTypeVAlign(2);
		str.Format(_T("%d"), i);
		m_spreadAgingList->SetData(1, i + 1, str);
	}
}

BOOL CDlgSubManual_AgingTest::IsOfflineStatus()
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) {
	//	AfxMessageBox(_T("Offline 모드가 아닙니다..\n Offline 모드로 변경 후 진행 하십시오."), MB_ICONWARNING);
	//	return FALSE;
	//}

	return TRUE;
}

BOOL CDlgSubManual_AgingTest::LoadAgingInfo()
{
	CString strAgingRcpPath;
	//strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, (CString)m_pSystem->stLotInfo.m_szRecipeID);
		
	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, MAX_PATH);
	
	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
	int nMaxCount = _ttoi(cValue);
	
	AgingItemClearAll();
	if (nMaxCount <= 0) return FALSE;
	
	m_vecAginList.clear();
	S_AgingTest sAgingList;
	
	CString strItem;
	for (int i = 0; i < nMaxCount; i++) {
		strItem.Format(_T("ITEM_%d"), i + 1);
		AgingItemAdd();
	
		//m_spreadAgingList.SetRow(i + 2);
	
		for (int i = 2; i < MAX_AGING_ITEM + 1; i++) {
			//m_spreadAgingList.SetCol(i);
	
			if (i == 2) {
				GetPrivateProfileString(strItem, _T("USE_AGING"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgingList.SetValue(cValue);
				m_spreadAgingList->SetData(i, i + 2, cValue);
				sAgingList.m_bUseAging = _ttoi(cValue);
			}
			else if (i == 3) {
				GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgingList.SetTypeComboBoxCurSel(_ttoi(cValue));
				m_spreadAgingList->SetData(i, i + 2, cValue);
				sAgingList.m_nPtnRGB = _ttoi(cValue);
			}
			else if (i == 4) {
				GetPrivateProfileString(strItem, _T("PTN_SEC"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgingList.SetValue(cValue);
				m_spreadAgingList->SetData(i, i + 2, cValue);
				sAgingList.m_nPtnSec = _ttoi(cValue);
			}
			else if (i == 5) {
				GetPrivateProfileString(strItem, _T("USE_PROPERTY"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgingList.SetValue(cValue);
				m_spreadAgingList->SetData(i, i + 2, cValue);
				sAgingList.m_bUseProperty = _ttoi(cValue);
			}
			else if (i == 6) {
				GetPrivateProfileString(strItem, _T("USE_QUALITY"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgingList.SetValue(cValue);
				m_spreadAgingList->SetData(i, i + 2, cValue);
				sAgingList.m_bUseQuality = _ttoi(cValue);
			}
		}
	
		m_vecAginList.push_back(sAgingList);
	}
	
	MeasCellLoad();
	UpdateAgingCell();

	return TRUE;
}

void CDlgSubManual_AgingTest::SaveAgingInfo()
{
	CString strAgingRcpPath;
	//strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, (CString)m_pSystem->stLotInfo.m_szRecipeID);
	
	dfcCreateFolder(m_strAgingRcpPath);
	
	CString strValue, strItem;	
	int nMaxCount = m_spreadAgingList->GetMaxRows();
	if (nMaxCount <= 1) {
		AfxMessageBox(_T("Aging 레시피 항목 추가후 저장 하십시오."));
		return;
	}
	
	strValue.Format(_T("%d"), nMaxCount - 1);
	WritePrivateProfileString(_T("COUNT"), _T("MaxCount"), strValue, strAgingRcpPath);
	
	for (int i = 0; i < nMaxCount - 1; i++) {
		//m_spreadAgingList.SetRow(i + 2);
	
		strItem.Format(_T("ITEM_%d"), i + 1);
	
		for (int j = 1; j < MAX_AGING_ITEM; j++) {
			//m_spreadAgingList.SetCol(j);
	
			if (j == 1) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("USE_AGING"), strValue, strAgingRcpPath);
			}
			else if (j == 2) {
				//strValue.Format(_T("%d"), m_spreadAgingList->GetTypeComboBoxCurSel());
				strValue = m_spreadAgingList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("PTN_RGB"), strValue, strAgingRcpPath);
			}
			else if (j == 3) {
				//strValue.Format (_T("%d"), m_spreadAgingList.GetTypeComboBoxCurSel ());
				//m_spreadAgingList.GetWindowText (strValue);
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("PTN_SEC"), strValue, strAgingRcpPath);
			}
			else if (j == 4) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("USE_PROPERTY"), strValue, strAgingRcpPath);
			}
			else if (j == 5) {
				//strValue = m_spreadAgingList.GetValue();
				strValue = m_spreadAgingList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("USE_QUALITY"), strValue, strAgingRcpPath);
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
		if (i == 1 || i == 4 || i == 5) {
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
		
			sComboList = _T("1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t");
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
	
	//UpdateListNum();
}

void CDlgSubManual_AgingTest::UpdateAgingCell()
{
	//int iCol = 0, iRow = 0, iColMax = 0, iRowMax = 0;
	//CString strValue;
	//
	////if (m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2) {
	////	iColMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	////	iRowMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	////}
	////else {
	////	iColMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	////	iRowMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	////}
	//
	////iColMax = (int)m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.x;
	////iRowMax = (int)m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.y;
	//
	//TCHAR* szText[3] = { _T("특성"),_T("화질"),_T("색이상") };
	//
	//// BOTTOM
	////m_spreadAgingMeas.SetSheetCount(3);	// Set number of sheets
	//
	//for (int iSheet = 0; iSheet < 3; iSheet++)
	//{
	//	m_spreadAgingMeas.SetSheet(iSheet + 1);	// Set sheet names
	//
	//	m_spreadAgingMeas.SetMaxCols(iColMax);
	//	m_spreadAgingMeas.SetMaxRows(iRowMax);
	//
	//	m_spreadAgingMeas->ClearRange(1, 1, -1, -1);
	//
	//	strValue.Format(_T("%s"), szText[iSheet]);
	//	m_spreadAgingMeas.SetSheetName(strValue);
	//
	//	m_spreadAgingMeas.SetColHeadersShow(TRUE);
	//	m_spreadAgingMeas.SetRowHeadersShow(TRUE);
	//	m_spreadAgingMeas.SetScrollBars(3);
	//
	//	m_spreadAgingMeas->SetRowHeight(0, 12);
	//
	//	MEASGROUP stMeasArea;
	//	ZeroMemory(&stMeasArea, sizeof(stMeasArea));
	//	int iAreaMax = 0, iArea = 0;
	//	//iAreaMax = m_pDlgNaviRecipe->m_stRecipePath.m_aryMeasGroup.GetSize();
	//	//20190213 add by Hee (단변 행열 문자 변경)
	//	char cRow = 'A';
	//	int	 nCol = 1;
	//
	//	for (int i = 0; i < iRowMax; i++) {
	//		//m_spreadAgingMeas.SetRow(i + 1);
	//		m_spreadAgingMeas->SetRowHeight(i + 1, 15);
	//		//m_spreadAgingMeas.SetCol(0);
	//		if (m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2) {
	//			strValue.Format(_T("%c"), cRow + i);
	//			//m_spreadAgingMeas.SetValue(strValue);
	//			m_spreadAgingMeas->SetData(0, i + 1, strValue);
	//		}
	//		else {
	//			strValue.Format(_T("%d"), iRowMax - i);
	//			//m_spreadAgingMeas.SetValue(strValue);
	//			m_spreadAgingMeas->SetData(0, i + 1, strValue);
	//		}
	//
	//		for (int j = 0; j < iColMax; j++) {
	//			if (i != 0) continue;
	//			//m_spreadAgingMeas.SetRow(0);
	//			//m_spreadAgingMeas.SetCol(j + 1);
	//
	//			if (m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2) {
	//				strValue.Format(_T("%d"), j + 1);
	//				//m_spreadAgingMeas.SetValue(strValue);
	//				m_spreadAgingMeas->SetData(j + 1, 0, strValue);
	//			}
	//			else {
	//				strValue.Format(_T("%c"), cRow + j);
	//				//m_spreadAgingMeas.SetValue(strValue);
	//				m_spreadAgingMeas->SetData(j + 1, 0, strValue);
	//			}
	//		}
	//	}
	//
	//	for (iRow = 0; iRow < iRowMax; iRow++)
	//	{
	//		//m_spreadAgingMeas.SetRow(iRow + 1);
	//		m_spreadAgingMeas->SetRowHeight(iRow + 1, 15);
	//
	//		// jsyoon 20170111 add
	//		//m_spreadAgingMeas.SetCol(0);
	//		//strValue.Format("%d", iRowMax-iRow);
	//		//m_spreadAgingMeas.SetValue(strValue);
	//		//////////////////////////////////////////////////////////////////////////
	//
	//		for (iCol = 0; iCol < iColMax; iCol++)
	//		{
	//			iArea = iCol + iRow * iColMax;
	//
	//			if (iArea > iAreaMax)
	//				continue;
	//
	//			//stMeasArea = m_pDlgNaviRecipe->m_stRecipePath.m_aryMeasGroup.GetAt(iArea);
	//
	//			//m_spreadAgingMeas.SetCol(iCol + 1);
	//			m_spreadAgingMeas->SetColWidth(iCol + 1, 6);
	//			m_spreadAgingMeas.SetCellType(10);						// Define cell type as check box
	//			//m_spreadAgingMeas.SetTypeCheckCenter(TRUE);				// Make it a three state check box
	//			//m_spreadAgingMeas.SetTypeCheckType(0);					// Define the pictures used for each state of the check box
	//			//m_spreadAgingMeas.SetTypeCheckText(_T("USE"));
	//
	//			strValue.Format("%d", GetMeasUse(iSheet, stMeasArea.stProbe[0].m_iUsedMeas));
	//			//m_spreadAgingMeas.SetTypeHAlign(2);
	//			//m_spreadAgingMeas.SetTypeVAlign(2);
	//			//m_spreadAgingMeas.SetValue(strValue);
	//		}
	//
	//		//m_spreadAgingMeas.SetSelection(1, 1, -1, 1);
	//	}
	//}
	//
	//m_spreadAgingMeas.SetSheet(1);
	//m_spreadAgingMeas.SetActiveSheet(1);
	//m_spreadAgingMeas.Refresh();
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

void CDlgSubManual_AgingTest::MeasCellSave()
{
	//int nCellXMax = 0, nCellYMax = 0;
	//std::bitset<3> bit = 0;
	//
	//if (m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2) {
	//	nCellXMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	//	nCellYMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	//}
	//else {
	//	nCellXMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	//	nCellYMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	//}
	//
	////nCellXMax = m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.x;
	////nCellYMax = m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.y;
	//
	//if (nCellXMax < 0 || nCellYMax < 0) return;
	//
	//CString strCellData;
	//CString strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, (CString)m_pSystem->stLotInfo.m_szRecipeID);
	//
	//for (int i = 0; i < nCellYMax; i++) {
	//	strCellData.Empty();
	//
	//	for (int j = 0; j < nCellXMax; j++) {
	//
	//		for (int k = 0; k < 3; k++) { //특성, 화질, 색이상
	//			m_spreadAgingMeas.SetSheet(k + 1);
	//			m_spreadAgingMeas.SetRow(i + 1);
	//			m_spreadAgingMeas.SetCol(j + 1);
	//			bit[k] = atoi(m_spreadAgingMeas.GetValue());
	//		}
	//		strCellData += dfcStr(_T("%d,"), bit.to_ulong());
	//
	//	}
	//
	//	WritePrivateProfileString(_T("MASECELL"), dfcStr(_T("STICK_%d"), i + 1), strCellData, strAgingRcpPath);
	//
	//}
}

void CDlgSubManual_AgingTest::MeasCellLoad()
{
	//int nCellXMax = 0, nCellYMax = 0;
	//std::bitset<3> bit = 0;
	//
	//if (m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2) {
	//	nCellXMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	//	nCellYMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	//}
	//else {
	//	nCellXMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.x;
	//	nCellYMax = (int)m_pDlgNaviRecipe->m_stRecipePath.stCellInfo.CellCount.y;
	//}
	//
	////nCellXMax = m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.x;
	////nCellYMax = m_pDlgNaviRecipe->m_RecipePosICP.stCellInfo.CellCount.y;
	//
	//if (nCellXMax < 0 || nCellYMax < 0) return;
	//
	//CString strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, (CString)m_pSystem->stLotInfo.m_szRecipeID);
	//CString strID;
	//CArray <int, int> nCellChkArray;
	//
	//char cCellData[MAX_PATH];
	//ZeroMemory(cCellData, MAX_PATH);
	//MEASGROUP stMeasArea;
	//
	//int nArea = 0;
	//
	//for (int i = 0; i < nCellYMax; i++) {
	//
	//	GetPrivateProfileString(_T("MASECELL"), dfcStr(_T("STICK_%d"), i + 1), _T(""), cCellData, MAX_PATH, strAgingRcpPath);
	//
	//	int pos = 0;
	//	nCellChkArray.RemoveAll();
	//	CString strCellData = (CString)cCellData;
	//	CString resToken = strCellData.Tokenize(_T(","), pos);
	//	while (resToken != _T("")) {
	//		nCellChkArray.Add(atoi(resToken.Trim()));
	//		resToken = strCellData.Tokenize(_T(","), pos);
	//	}
	//
	//	if (nCellChkArray.GetSize() != nCellXMax) return;
	//
	//	ZeroMemory(&stMeasArea, sizeof(stMeasArea));
	//
	//	for (int j = 0; j < nCellXMax; j++) {
	//		nArea = j + i * nCellXMax;
	//		stMeasArea = m_pDlgNaviRecipe->m_stRecipePath.m_aryMeasGroup.GetAt(nArea);
	//		stMeasArea.stProbe[0].m_iUsedMeas = nCellChkArray.GetAt(j);
	//		m_pDlgNaviRecipe->m_stRecipePath.m_aryMeasGroup.SetAt(nArea, stMeasArea);
	//	}
	//
	//}
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
	//m_spreadAgingMeas.SetSheet(NewSheet);
}