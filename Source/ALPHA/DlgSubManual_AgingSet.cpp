#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_AgingSet.h"
#include "afxdialogex.h"

#include <iostream>
#include <bitset>

#include <fstream>
#include <string>
using namespace std;

IMPLEMENT_DYNAMIC(CDlgSubManual_AgingSet, CDialogEx)

CDlgSubManual_AgingSet::CDlgSubManual_AgingSet(CWnd* pParent /*=NULL*/)
	:CDialogEx(CDlgSubManual_AgingSet::IDD, pParent)
{
	//m_pSystem = NULL;
	//m_pSystem = SMEM_GetSharedMemory();
}

CDlgSubManual_AgingSet::~CDlgSubManual_AgingSet()
{
}

void CDlgSubManual_AgingSet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_CHECK_1, m_btnAgingAlarmUse);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_AgingSet, CDialogEx)
	ON_WM_ERASEBKGND()
	//ON_COMMAND (IDC_BUTTON_1, OnStop)
	ON_COMMAND(IDC_BUTTON_2, OnRcpOpen)
	ON_COMMAND(IDC_BUTTON_3, OnItemClearAll)
	ON_COMMAND(IDC_BUTTON_4, OnItemAdd)
	ON_COMMAND(IDC_BUTTON_5, OnItemDel)
	ON_COMMAND(IDC_BUTTON_6, OnItemSave)
	ON_COMMAND(IDC_BUTTON_7, OnLoadAgingInfo)
	ON_COMMAND(IDC_BUTTON_8, OnAgingProcStrart)

	ON_COMMAND(IDC_CHECK_1, OnAgingAlarmUse)
END_MESSAGE_MAP()

BOOL CDlgSubManual_AgingSet::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CDlgSubManual_AgingSet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	//---------------------------------------------------------------------
	// Farpoint Spread 생성 
	//---------------------------------------------------------------------
	m_spreadAgAlarmList = new CMySpreadDll(IDC_FPS_AGING_LIST, FPS_RESOURCE_ID + 19, this, TRUE);

	if (m_spreadAgAlarmList->GetSafeHwnd() != NULL)
	{
		m_spreadAgAlarmList->InitDefault();

		SS_CELLTYPE cellType;
		m_spreadAgAlarmList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
		m_spreadAgAlarmList->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

		m_spreadAgAlarmList->MakeFont(&m_hfont);
		m_spreadAgAlarmList->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

		m_spreadAgAlarmList->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_spreadAgAlarmList->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_spreadAgAlarmList->SetColHeaderRows(1);

		m_spreadAgAlarmList->InitMySpread_AGINGSET();
	}

	//CString strPath;
	//if (m_pSystem->stEquip.m_nEQType == EQU_TYPE_ICP) strPath.Format(_T("D:\\KMAC\\%s\\PARAM\\AgingTargetRcp\\"), _T("PARAM_ICP"));
	//else if (m_pSystem->stEquip.m_nEQType == EQU_TYPE_OCP) strPath.Format(_T("D:\\KMAC\\%s\\PARAM\\AgingTargetRcp\\"), _T("PARAM_OCP"));
	
	//m_strAgingRcpPath = theApp.GetProfileString(_T("DIR"), _T("AGING_TARGET_RCP"), strPath);
	//GetDlgItem(IDC_EDIT_2)->SetWindowText(m_strAgingRcpPath);
	
	m_strOldRcp = _T("");
	m_strPtn = _T("");
	m_nCell = 0;
	
	m_strAgingFilePath = theApp.GetProfileString(_T("DIR"), _T("AGING_FILE"), _T("T:\\AGING.txt"));
	GetDlgItem(IDC_EDIT_3)->SetWindowText(m_strAgingFilePath);
	
	m_bAgingAlarmUse = theApp.GetProfileInt(_T("CONFIG"), _T("AGING_ALARM_USE"), 1);
	
	m_btnAgingAlarmUse.SetImage(IDB_CHECKNO32);
	m_btnAgingAlarmUse.SetCheckedImage(IDB_CHECK32);
	m_btnAgingAlarmUse.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	m_btnAgingAlarmUse.SetCheck(m_bAgingAlarmUse);

	return TRUE;
}

void CDlgSubManual_AgingSet::UpdateStatus(BOOL bEnable)
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) m_chkStop.ShowWindow (FALSE);
	//else m_chkStop.ShowWindow (!bEnable);

	for (int i = IDC_BUTTON_2; i < IDC_BUTTON_8 + 1; i++) GetDlgItem(i)->EnableWindow(bEnable);

	GetDlgItem(IDC_CHECK_1)->EnableWindow(bEnable);
}

//void CDlgSubManual_AgingSet::OnStop ()
//{
//	gLog(m_pSystem->hLog, "CDlgSubManual_AgingSet::OnStop()", LOG_LEVEL_O);
//	m_pJobProc->ActionStop();
//}

void CDlgSubManual_AgingSet::OnRcpOpen()
{
	if (!IsOfflineStatus()) return;
	
	//gLog(m_pSystem->hLog, _T("CDlgSubManual_AgingSet::OnRcpOpen()"), LOG_LEVEL_O);
	
	//CDlgNaviManualMeas::LoadRecipeDlg()
	CString strFileName = _T("");
	CString filter = _T("Recipe Files (*.rcp)|*.rcp|");
	//strFileName.Format(_T("%s%s%s*.rcp"), m_pSystem->m_strRootPath, STR_DIR_PARAM, dfRECIPE_DATA_FILE_PATH);
	
	CFileDialog dlg(TRUE, _T("rcp"), strFileName, OFN_FILEMUSTEXIST, filter); //OFN_FILEMUSTEXIST (존재하지 않는 파일명을 입력할 수 없음)
	//dlg.GetFileName().Format(_T("%s"), m_pSystem->stLotInfo.m_szRecipeID);
	
	if (dlg.DoModal() == IDOK)
	{
		strFileName = dlg.GetFileName();
	
		if (strFileName.IsEmpty()) MessageBox(_T("File is not an Recipe file or is locked and cannot be imported."));
		else
		{
			int j = 0;
			j = strFileName.Find(_T(".rcp"));
	
			if (j > 0)
			{
				strFileName = strFileName.Left(j);		// remove ".rcp"
			}
			else
			{
				strFileName = strFileName;
			}
	
			MessageBox(_T("Recipe file be imported complete."));
	
			GetDlgItem(IDC_EDIT_1)->SetWindowText(strFileName/*m_pSystem->stLotInfo.m_szRecipeID*/);
	
			m_strOldRcp = strFileName;
			LoadAgingInfo(strFileName);	
		}
	}

	//return;
}

void CDlgSubManual_AgingSet::OnItemClearAll()
{
	if (AfxMessageBox(_T("전체 리스트를 삭제합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	AgingItemClearAll();
}

void CDlgSubManual_AgingSet::OnItemAdd()
{
	AgingItemAdd();
}

void CDlgSubManual_AgingSet::OnItemDel()
{
	int nMaxRow = m_spreadAgAlarmList->GetMaxRows();
	if (nMaxRow <= 1) return;
	
	m_spreadAgAlarmList->SetMaxRows(nMaxRow - 1);
	
	UpdateListNum();
}

void CDlgSubManual_AgingSet::OnItemSave()
{
	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("현재값을 저장 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strCurrentRcp;
	GetDlgItem(IDC_EDIT_1)->GetWindowText(strCurrentRcp);
	if (strCurrentRcp.IsEmpty() || strCurrentRcp == _T("UNKNOWN")) {
		AfxMessageBox(_T("기본 레시피가 없습니다.\n 기본 레시피 Open 후 진행 하십시오."), MB_ICONWARNING);
		return;
	}

	SaveAgingInfo(strCurrentRcp);
}

void CDlgSubManual_AgingSet::OnLoadAgingInfo()
{
	if (!IsOfflineStatus()) return;
	if (AfxMessageBox(_T("이전값을 불러 옵니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	CString strCurrentRcp;
	GetDlgItem(IDC_EDIT_1)->GetWindowText(strCurrentRcp);
	if (strCurrentRcp.IsEmpty() || strCurrentRcp == _T("UNKNOWN")) {
		AfxMessageBox(_T("기본 레시피가 없습니다.\n 기본 레시피 Open 후 진행 하십시오."), MB_ICONWARNING);
		return;
	}

	LoadAgingInfo(strCurrentRcp);
}

void CDlgSubManual_AgingSet::OnAgingProcStrart()
{
	//if (!IsOfflineStatus()) return;
	//if (SMEM_GetSys_ProcessStatus() == SPS_PROCESS) {
	//	AfxMessageBox(_T("프로세스 진행중 입니다.\n 프로세스 완료후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//////20180429 add by Hee
	////if (!m_pSystem->stInterLock.m_bSafetyPos_X) {
	////	AfxMessageBox (_T("Safety 위치가 아닙니다. \n\n안전위치 이동 후 진행하십시오."));
	////	return;
	////}
	//
	////if (m_pSystem->m_bProbeRotateInstall) {
	////	if (fabs (m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.GetCurPos (AXIS_R1)) > 0.01) {
	////		AfxMessageBox (_T("Safety 위치가 아닙니다. \n\n안전위치 이동 후 진행하십시오."));
	////		return;
	////	}
	////}
	//
	//if (AfxMessageBox(_T("Aging 결과 파일 에서 전류값을 확인 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	//
	//CString strRcp;
	//GetDlgItem(IDC_EDIT_1)->GetWindowText(strRcp);
	//
	//if (strRcp == _T("") && strRcp == _T("UNKNOWN")) {
	//	AfxMessageBox(_T("레시피 정보가 없습니다.\n 레시피 설정 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//int nMaxCount = m_spreadAgAlarmList.GetMaxRows();
	//
	//if (nMaxCount <= 1) {
	//	AfxMessageBox(_T("리스트 정보가 없습니다.\n 리스트 추가 및 저장 후 진행 하십시오."), MB_ICONWARNING);
	//	return;
	//}
	//
	//////Cycle Count 정보
	//CString strTemp;
	////int nCycleCount = 1;
	////m_cmbCycle.GetWindowText (strTemp);
	////nCycleCount = atoi (strTemp);
	//
	////////스틱 정보
	//////m_nStick.clear ();
	//////for (int i = 0; i < MAX_AGING_STICK; i++) {
	//////	if (m_chkStick[i].IsWindowEnabled ()) {
	//////		m_nStick.push_back (m_chkStick[i].GetCheck ());
	//////	}
	//////	else break;
	//////}
	//
	////Aging 리스트 정보
	//m_vecAgTgAlarm.clear();
	//S_AgingTargetAlarm sAgingAlarm;
	//for (int i = 0; i < nMaxCount - 1; i++) {
	//	m_spreadAgAlarmList.SetRow(i + 2);
	//
	//	for (int j = 2; j < MAX_AGING_ITEM + 1; j++) {
	//		m_spreadAgAlarmList.SetCol(j);
	//
	//		if (j == 2) {
	//			strTemp = m_spreadAgAlarmList.GetValue();
	//			sAgingAlarm.m_bUseAlarm = atoi(strTemp);
	//		}
	//		else if (j == 3) {
	//			m_spreadAgAlarmList.GetWindowText(strTemp);
	//			sAgingAlarm.m_nPtnRGB = atoi(strTemp);
	//		}
	//		else if (j == 4) {
	//			m_spreadAgAlarmList.GetWindowText(strTemp);
	//			sAgingAlarm.m_fPtnBase = atof(strTemp);
	//		}
	//		else if (j == 5) {
	//			strTemp = m_spreadAgAlarmList.GetValue();
	//			sAgingAlarm.m_fPtnUp = atof(strTemp);
	//		}
	//		else if (j == 6) {
	//			strTemp = m_spreadAgAlarmList.GetValue();
	//			sAgingAlarm.m_fPtnLo = atof(strTemp);
	//		}
	//	}
	//
	//	m_vecAgTgAlarm.push_back(sAgingAlarm);
	//}
	//
	////m_pSystem->m_bAutoAgingMode = TRUE;
	////m_pSystem->nJobCount = nCycleCount;
	//
	//////진행 위치
	////m_pSystem->m_nAgingListRow = 0;
	////m_pSystem->m_nAgingListCol = 0;
	//
	////m_pJobProc->DoStartCycleTest ();
	//
	//
	//
////////////////////////////////
	//
	////DeleteAgingFile ();
	//
	////LoadAgingInfo (); //레시피 확인
	////if (!LoadAgingInfo ())
	//
	////AGING_NOT_FILE    //Aging 생성된 파일 없을때
	////AGING_FIND_ALARM  //Aging Spec Out 찾았을때
	//
	//int nAlarm = 0;
	//CString str;
	//nAlarm = FindAgingAlarm();
	//
	//if (nAlarm == AGING_NOT_FILE) {
	//	AfxMessageBox(_T("Aging 파일을 찾을수 없습니다. \n Aging 파일이 생성되었는지 확인 하십시오."));
	//}
	//else if (nAlarm == AGING_FIND_ALARM) {
	//	str.Format(_T("Aging Spec Out : Ptn(%s) Cell(%d)"), GetPtn(), GetCell());
	//	AfxMessageBox(str);
	//}
}

void CDlgSubManual_AgingSet::UpdateListNum()
{
	CString str;
	int nMaxRow = m_spreadAgAlarmList->GetMaxRows();
	//m_spreadAgAlarmList->SetCol(1);
	for (int i = 1; i < nMaxRow; i++) {
		//m_spreadAgAlarmList->SetRow(i + 1);
		//m_spreadAgAlarmList->SetCellType(1);
		//m_spreadAgAlarmList->SetTypeHAlign(2);
		//m_spreadAgAlarmList->SetTypeVAlign(2);
		str.Format(_T("%d"), i);
		m_spreadAgAlarmList->SetDataString(1, i+1, str);
	}
}

BOOL CDlgSubManual_AgingSet::IsOfflineStatus()
{
	//if (m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE) {
	//	AfxMessageBox(_T("Offline 모드가 아닙니다..\n Offline 모드로 변경 후 진행 하십시오."), MB_ICONWARNING);
	//	return FALSE;
	//}
	//
	return TRUE;
}

BOOL CDlgSubManual_AgingSet::LoadAgingInfo(CString strRcp)
{
	CString strAgingRcpPath;
	//strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcp/*(CString)m_pSystem->stLotInfo.m_szRecipeID*/);
	
	//CString strPath = gf_GetFilePath(dfDEFAULT_AGING_FILE_PATH);
	
	TCHAR cValue[MAX_PATH];
	ZeroMemory(cValue, MAX_PATH);
	
	GetPrivateProfileString(_T("COUNT"), _T("MaxCount"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
	int nMaxCount = _ttoi(cValue);
	
	AgingItemClearAll();
	if (nMaxCount <= 0) return FALSE;
	
	//
	m_vecAgTgAlarm.clear();
	S_AgingTargetAlarm sAgingAlarm;
	//
	
	CString strItem;
	for (int i = 0; i < nMaxCount; i++) {
		strItem.Format(_T("ITEM_%d"), i + 1);
		AgingItemAdd();
	
		//m_spreadAgAlarmList.SetRow(i + 2);
	
		for (int i = 2; i < MAX_AGING_ITEM + 1; i++) {
			//m_spreadAgAlarmList.SetCol(i);
	
			if (i == 2) {
				GetPrivateProfileString(strItem, _T("USE_ALARM"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgAlarmList.SetValue(cValue);
				m_spreadAgAlarmList->SetData(i, i + 2, cValue);
				sAgingAlarm.m_bUseAlarm = _ttoi(cValue);
			}
			else if (i == 3) {
				GetPrivateProfileString(strItem, _T("PTN_RGB"), _T("0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgAlarmList->SetTypeComboBoxCurSel(_ttoi(cValue));
				sAgingAlarm.m_nPtnRGB = _ttoi(cValue);
			}
			else if (i == 4) {
				GetPrivateProfileString(strItem, _T("BASE_VAL"), _T("0.0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgAlarmList.SetValue(cValue);
				m_spreadAgAlarmList->SetData(i, i + 2, cValue);
				sAgingAlarm.m_fPtnBase = _ttoi(cValue);
			}
			else if (i == 5) {
				GetPrivateProfileString(strItem, _T("UP_VAL"), _T("0.0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgAlarmList.SetValue(cValue);
				m_spreadAgAlarmList->SetData(i, i + 2, cValue);
				sAgingAlarm.m_fPtnUp = _ttoi(cValue);
			}
			else if (i == 6) {
				GetPrivateProfileString(strItem, _T("LO_VAL"), _T("0.0"), cValue, MAX_PATH, strAgingRcpPath);
				//m_spreadAgAlarmList.SetValue(cValue);
				m_spreadAgAlarmList->SetData(i, i + 2, cValue);
				sAgingAlarm.m_fPtnLo = _ttoi(cValue);
			}
		}
	
		m_vecAgTgAlarm.push_back(sAgingAlarm);
	}
	
	return TRUE;
}

void CDlgSubManual_AgingSet::SaveAgingInfo(CString strRcp)
{
	CString strAgingRcpPath;
	//strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcp/*(CString)m_pSystem->stLotInfo.m_szRecipeID*/);
	
	//dfcCreateFolder(m_strAgingRcpPath);
	
	//CString strPath = gf_GetFilePath(dfDEFAULT_AGING_FILE_PATH);
	CString strValue, strItem;
	
	int nMaxCount = m_spreadAgAlarmList->GetMaxRows();
	if (nMaxCount <= 1) {
		AfxMessageBox(_T("Aging 레시피 항목 추가후 저장 하십시오."));
		return;
	}
	
	strValue.Format(_T("%d"), nMaxCount - 1);
	WritePrivateProfileString(_T("COUNT"), _T("MaxCount"), strValue, strAgingRcpPath);
	
	for (int i = 0; i < nMaxCount - 1; i++) {
		//m_spreadAgAlarmList.SetRow(i + 2);
	
		strItem.Format(_T("ITEM_%d"), i + 1);
	
		for (int j = 1; j < MAX_AGING_ITEM; j++) {
			//m_spreadAgAlarmList.SetCol(j);
	
			if (j == 1) {
				strValue = m_spreadAgAlarmList->GetDataString(j, i + 2);
				WritePrivateProfileString(strItem, _T("USE_ALARM"), strValue, strAgingRcpPath);
			}
			else if (j == 2) {
				strValue.Format(_T("%d"), m_spreadAgAlarmList->GetDataString(j, i + 1));
				WritePrivateProfileString(strItem, _T("PTN_RGB"), strValue, strAgingRcpPath);
			}
			else if (j == 3) {
				strValue.Format(_T("%.4lf"), _ttof(m_spreadAgAlarmList->GetDataString(j, i + 1)));
				WritePrivateProfileString(strItem, _T("BASE_VAL"), strValue, strAgingRcpPath);
			}
			else if (j == 4) {
				strValue.Format(_T("%.4lf"), _ttof(m_spreadAgAlarmList->GetDataString(j, i + 1)));
				WritePrivateProfileString(strItem, _T("UP_VAL"), strValue, strAgingRcpPath);
			}
			else if (j == 5) {
				strValue.Format(_T("%.4lf"), _ttof(m_spreadAgAlarmList->GetDataString(j, i + 1)));
				WritePrivateProfileString(strItem, _T("LO_VAL"), strValue, strAgingRcpPath);
			}
		}
	}
}

void CDlgSubManual_AgingSet::AgingItemClearAll()
{
	m_spreadAgAlarmList->ClearRange(1, 1, -1, -1);
	m_spreadAgAlarmList->SetMaxRows(1);
	
	CStringArray strArr;
	strArr.Add(_T("No"));
	strArr.Add(_T("Use"));
	strArr.Add(_T("점등패턴"));
	strArr.Add(_T("기준값(mA)"));
	strArr.Add(_T("상한값(mA)"));
	strArr.Add(_T("하한값(mA)"));
	
	//m_spreadAgAlarmList.SetRow(1);
	for (int i = 1; i < m_spreadAgAlarmList->GetMaxCols() + 1; i++) {
		//m_spreadAgAlarmList.SetCol(i);
	
		//m_spreadAgAlarmList.SetCellType(5);
		//m_spreadAgAlarmList.SetTypeHAlign(2);
		//m_spreadAgAlarmList.SetTypeVAlign(2);
		//m_spreadAgAlarmList.SetValue(strArr.GetAt(i - 1));
		//
		//m_spreadAgAlarmList->SetBackColor(dfCOLOR_BACKCOLOR_DARK);
		//m_spreadAgAlarmList->SetForeColor(dfCOLOR_FONTCOLOR_YELLOW);
	}
}

void CDlgSubManual_AgingSet::AgingItemAdd()
{
	//int nMaxRow = m_spreadAgAlarmList.GetMaxRows();
	//if (nMaxRow < 1) return;
	//
	//CString str;
	//
	//m_spreadAgAlarmList.SetMaxRows(nMaxRow + 1);
	////m_spreadAgAlarmList.InsertRows (nMaxRow, 1);
	//m_spreadAgAlarmList.SetRow(nMaxRow + 1);
	//
	//for (int i = 2; i < MAX_AGING_ITEM + 1; i++) {
	//	m_spreadAgAlarmList.SetCol(i);
	//	if (i == 2 /*|| i == 5 || i == 6*/) {
	//		m_spreadAgAlarmList.SetCellType(10);			//Check Box
	//		m_spreadAgAlarmList.SetTypeCheckCenter(TRUE);
	//		m_spreadAgAlarmList.SetTypeCheckType(0);
	//	}
	//
	//	if (i == 3) {
	//		m_spreadAgAlarmList.SetCellType(8);			//Combo Box
	//		m_spreadAgAlarmList.SetTypeHAlign(2);
	//		m_spreadAgAlarmList.SetTypeVAlign(2);
	//		for (int j = 20; j >= 0; j--) {
	//			str.Format(_T("%d"), j);
	//			m_spreadAgAlarmList.SetTypeComboBoxString(str);
	//		}
	//		m_spreadAgAlarmList.SetTypeComboBoxCurSel(0);
	//	}
	//
	//	if (i == 4 || i == 5 || i == 6) {
	//		m_spreadAgAlarmList.SetCellType(1);			//Edit
	//		m_spreadAgAlarmList.SetTypeHAlign(2);
	//		m_spreadAgAlarmList.SetTypeVAlign(2);
	//		m_spreadAgAlarmList.SetWindowText(_T("0.0000"));
	//	}
	//}
	//
	//UpdateListNum();
}

BOOL CDlgSubManual_AgingSet::CmAgingFileFindTimeOut(CString strFile, int nTimeOut)
{
	BOOL bTimeOut = FALSE;

	DWORD dwTimeout = nTimeOut;
	DWORD dwBeginTick = GetTickCount();
	DWORD dwRecheck = 1000; //1Sec
	DWORD dwCurrentTick = 0;
	DWORD dwTemp = 0;

	if (!CmAgingFileFind(strFile)) {
		while (1) {
			dwCurrentTick = GetTickCount();

			if ((dwCurrentTick - dwBeginTick) < 0) dwBeginTick = dwTemp; //음수로 나오면 기존 지난 시간 사용
			else dwTemp = (dwCurrentTick - dwBeginTick);

			if ((dwCurrentTick - dwBeginTick) > dwTimeout) {
				bTimeOut = TRUE;
				break;
			}
			else if ((dwCurrentTick - dwBeginTick) > dwRecheck) {
				if (CmAgingFileFind(strFile)) break;
				dwRecheck += 1000;
			}
		}
	}

	return bTimeOut;
}

BOOL CDlgSubManual_AgingSet::CmAgingFileFind(CString strFile)
{
	//파일 이름만 가져오기
	int pos = 0;
	CString strID;
	CString resToken = strFile.Tokenize(_T("\\"), pos);
	while (resToken != _T("")) {
		strID = resToken.Trim();
		resToken = strFile.Tokenize(_T("\\"), pos);
	}

	//파일 찾기
	BOOL bExist = FALSE;
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strFile);
	while (bWorking) {
		bWorking = finder.FindNextFile();
		if (finder.IsDots() || finder.IsDirectory()) continue;
		if (strID == finder.GetFileName()) {
			bExist = TRUE;
			break;
		}

	}
	finder.Close();

	return bExist;
}

int CDlgSubManual_AgingSet::FindAgingAlarm()
{
	CString strTemp = m_strAgingFilePath; //Aging 후 만들어지는 파일
	CString strPtn;
	
	BOOL bSearchFile = FALSE;
	bSearchFile = !CmAgingFileFindTimeOut(strTemp, 10000);
	
	m_strPtn = _T("");
	m_nCell = 0;
	
	//BOOL bAlarm = FALSE;
	
	char cLineRead[1024];
	ZeroMemory(cLineRead, sizeof(cLineRead));
	
	int nBegin = 0, nEnd = 0;
	CStringArray strArrData;
	
	if (bSearchFile) {
		ifstream fin;
		fin.open(strTemp);
	
		if (fin.is_open()) {
			while (!fin.eof()) {
				fin.getline(cLineRead, 1024, '\n');
				strTemp = cLineRead;
	
				if (strTemp.IsEmpty()) {
					//bAlarm = TRUE;
					break;
				}
	
				if (strTemp.Find(_T("(")) < 0) continue;
	
				nBegin = 0; nEnd = 0; m_nCell = 0;
				int nLength = strTemp.GetLength();
	
				strArrData.RemoveAll();
				for (int i = 0; i < nLength; i++) {
					if (strTemp.GetAt(i) == ',' || i == nLength - 1) {
	
						if (i == nLength - 1) {
							nEnd = i;
						}
						else {
							nEnd = i - nBegin;
						}
	
						strArrData.Add(strTemp.Mid(nBegin, nEnd).Trim());
						nBegin = i + 1;
					}
				}
	
				nBegin = 0;
				nEnd = 0;
				int nItem = 0;
				BOOL bStart = FALSE;
	
				nLength = strArrData.GetSize();
				for (int i = 0; i < nLength; i++) {
					strTemp = strArrData.GetAt(i);
	
					if (i == 0) {
						nEnd = strTemp.Find(_T("("));
						strPtn = strTemp.Mid(nBegin, nEnd);
	
						for (int j = 0; j < m_vecAgTgAlarm.size(); j++) {
							if (_ttoi(strPtn) == m_vecAgTgAlarm[j].m_nPtnRGB) {
								bStart = TRUE;
								nItem = j;
								break;
							}
						}
	
						if (bStart) {
							if (!m_vecAgTgAlarm[nItem].m_bUseAlarm) break;
	
							m_strPtn = strTemp;
						}
	
					}
	
					if (!bStart) break;
	
					if (i > 1 && i < nLength - 2) {
						m_nCell++;
						if (_ttoi(strTemp) > m_vecAgTgAlarm[nItem].m_fPtnBase + m_vecAgTgAlarm[nItem].m_fPtnUp ||
							_ttoi(strTemp) < m_vecAgTgAlarm[nItem].m_fPtnBase - m_vecAgTgAlarm[nItem].m_fPtnLo) {
							return AGING_FIND_ALARM;
						}
					}
				}
	
			}
		}
	}
	else
		return AGING_NOT_FILE;
	
	return AGING_NO_ALARM;
}

void CDlgSubManual_AgingSet::DeleteAgingFile()
{
	DeleteFile(m_strAgingFilePath);
}

void CDlgSubManual_AgingSet::OnAgingAlarmUse()
{
	if (AfxMessageBox(_T("AGING 알람 사용 유무를 변경 합니다.\n 계속 하시겠습니까?"), MB_YESNO | MB_ICONQUESTION) != IDYES) {
		m_btnAgingAlarmUse.SetCheck(m_bAgingAlarmUse);
		return;
	}

	m_bAgingAlarmUse = !m_bAgingAlarmUse;
	m_btnAgingAlarmUse.SetCheck(m_bAgingAlarmUse);

	theApp.WriteProfileInt(_T("CONFIG"), _T("AGING_ALARM_USE"), m_bAgingAlarmUse);
}