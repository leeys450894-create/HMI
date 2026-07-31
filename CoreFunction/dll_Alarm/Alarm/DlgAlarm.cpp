// DlgAlarm.cpp : implementation file
//

#include "pch.h"
#include "Alarm.h"
#include "DlgAlarm.h"
#include "DlgAlarmModule.h"
#include <afxmt.h>			//CCriticalSection

//#include "..\\include\\resource_lang.h" //lhj 20181225 lang

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CDlgAlarm dialog
CCriticalSection	g_csAlarm;

CDlgAlarm::CDlgAlarm(CWnd* pParent /*=NULL*/)
: CDialog(CDlgAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarm)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
//	m_hBrush = ::CreateSolidBrush(RGB(115, 141, 172));
	m_hBrush = ::CreateSolidBrush(RGB(225, 231, 253));		// jsyoon 20150225 change

	m_pDlgAlarmModule = nullptr;
}

void CDlgAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarm)
	DDX_Control(pDX, IDC_STATIC_ALARM_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_LIST_ALARM, m_listAlarm);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgAlarm, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarm)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CLICK, IDC_LIST_ALARM, OnClickListAlarm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarm message handlers

void CDlgAlarm::OnDestroy() 
{
	CDialog::OnDestroy();

	if (m_pDlgAlarmModule != nullptr)
	{
		m_pDlgAlarmModule->DestroyWindow();
		delete m_pDlgAlarmModule;
		m_pDlgAlarmModule = nullptr;
	}

	// 2023.03.07 add
	if (m_sprdResult != nullptr)
	{
		delete m_sprdResult;
		m_sprdResult = nullptr;
	}
	
	::DeleteObject(m_hBrush);
}

HBRUSH CDlgAlarm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if(pWnd == this)
		return m_hBrush;

	return hbr;
}

BOOL CDlgAlarm::OnInitDialog()
{
	CDialog::OnInitDialog();

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 255));
	m_LabelInfo.SetFontBold(TRUE);

	InitControls();

	// 알람모듈창 생성
	CreateAlarmModule(this->GetSafeHwnd());
	ShowAlarmModule(FALSE);

	m_sprdResult = new CMySpreadDll(IDC_FPSPREAD_RESULT_DATA, FPS_RESOURCE_ID + 1111, this, TRUE);

	if (m_sprdResult->GetSafeHwnd() != NULL)
	{
		//Spread Sheet Naming 
		m_sprdResult->SetSheetCount(1);

		m_sprdResult->InitDefault();

		SS_CELLTYPE cellType;
		m_sprdResult->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
		m_sprdResult->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

		m_sprdResult->MakeFont(&m_hfont);
		m_sprdResult->SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

		m_sprdResult->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_sprdResult->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_sprdResult->SetColHeaderRows(1);

		m_sprdResult->ShowWindow(SW_HIDE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarm::CreateAlarmModule(HWND hParent)
{
	if (!hParent) return;

	//---------------------------------------------------------------------
	// 알람모듈창 생성
	//---------------------------------------------------------------------
	m_pDlgAlarmModule = new CDlgAlarmModule;
	m_pDlgAlarmModule->Create(IDD_DLG_ALARM_MODULE, FromHandle(hParent));
	CRect rect;
	GetDlgItem(IDC_STATIC_ALARM_MODULE)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_pDlgAlarmModule->MoveWindow(rect);
	
	/*if (::IsWindow(m_dlgAlarmModule.GetSafeHwnd()))
	{
		m_dlgAlarmModule.DestroyWindow();
	}
	
	OleInitialize(NULL);
	
	m_dlgAlarmModule.Create(IDD_DLG_ALARM_MODULE,this);
	m_dlgAlarmModule.SetParent(FromHandle(hParent));
	
	CRect rect, rectP;
	int iPos = 0, a = 0, b = 0;
	GetDlgItem(IDC_STATIC_ALARM_TITLE)->GetWindowRect(&rectP);
	ScreenToClient(&rectP);

	m_dlgAlarmModule.GetWindowRect(&rect);
	ScreenToClient(&rect);

	a = rectP.left + (rectP.Width()/2);
	b = rect.left + (rect.Width()/2);

	if( a > b )
		iPos = a - b;
	else
		iPos = b - a;

	m_dlgAlarmModule.MoveWindow(iPos,20, rect.Width(), rect.Height());*/

//	iPos = (rectP.Width()-rectP.left)/2 - (rect.Width()/2);
//	iPos = (rectP.Width()/2) - (rect.Width()/2);	// jsyoon 20170331 change, Center Calc
//	m_dlgAlarmModule.MoveWindow(iPos, 20, rect.Width(), rect.Height());
}

void CDlgAlarm::ShowAlarmModule(BOOL bShow)
{
	if (bShow == TRUE)
	{
		m_pDlgAlarmModule->ShowWindow(SW_SHOW);
	}
	else
	{
		m_pDlgAlarmModule->ShowWindow(SW_HIDE);
	}
}

void CDlgAlarm::InitControls()
{
	LV_COLUMN lvcolumn;
	TCHAR* szText[4] = {_T("Time"), _T("Alarm Id"), _T("Module"), _T("Alarm Text")};
	int nWidth[4] = {200, 100, 100, 500};

	for(int i=0; i<4; i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = szText[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = nWidth[i];
		m_listAlarm.InsertColumn(i,&lvcolumn);
	}

	m_listAlarm.SetExtendedStyle(LVS_EX_FULLROWSELECT);	
}

void CDlgAlarm::InsertList(ALARM_EVENT_DATA *pAlarm)
{
	int nIndex = m_listAlarm.GetItemCount();
	CString str;

	str.Format(_T("%s"), pAlarm->happendTime);
	str.TrimRight();
	m_listAlarm.InsertItem(nIndex, str);

	str.Format(_T("%d"), pAlarm->id);
	str.TrimRight();
	m_listAlarm.SetItemText(nIndex, 1, str);

	str.Format(_T("%s"), pAlarm->module);
	str.TrimRight();
	m_listAlarm.SetItemText(nIndex, 2, str);

	str.Format(_T("%s"), pAlarm->text);
	str.TrimRight();
	m_listAlarm.SetItemText(nIndex, 3, str);

	m_listAlarm.SetItemData(nIndex, (DWORD_PTR)pAlarm);

	// 추가된 아이템 선택.
	m_listAlarm.SetItemState( -1, 0, LVIS_SELECTED|LVIS_FOCUSED );
	m_listAlarm.SetItemState(nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	m_listAlarm.SetFocus();

	// 알람모듈창 출력.
	m_pDlgAlarmModule->ShowAlarm(pAlarm);
}

void CDlgAlarm::DeleteList(UINT id)
{
	int nCount = m_listAlarm.GetItemCount();
	unsigned int nId = 0;
	int nIndex = -1;

	for(int i=0; i<nCount; i++)
	{
		nId = _ttoi(m_listAlarm.GetItemText(i, 1));
		if(nId == id)
		{
			nIndex = i;
			break;
		}
	}

	if (nIndex == -1)
		return;

	m_listAlarm.DeleteItem(nIndex);

	// 알람이 있으면
	nCount = m_listAlarm.GetItemCount();
	if(nCount > 0)
	{
		// 마지막 알람 선택.
		m_listAlarm.SetItemState( -1, 0, LVIS_SELECTED|LVIS_FOCUSED );
		m_listAlarm.SetItemState(nCount-1, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		m_listAlarm.SetFocus();

		// 알람모듈창 출력.
		ALARM_EVENT_DATA *pAlarm = (ALARM_EVENT_DATA*)(m_listAlarm.GetItemData(nCount-1));

		if( pAlarm != nullptr)
			m_pDlgAlarmModule->ShowAlarm(pAlarm);
	}
	// 알람이 없다면, 알람모듈창 숨김
	else
	{
		ShowAlarmModule(FALSE);
	}
}

BOOL CDlgAlarm::FindList(UINT id)
{
	g_csAlarm.Lock();

	BOOL bRet = FALSE;
	int nCount = m_listAlarm.GetItemCount();
	unsigned int nId;

	for(int i=0; i<nCount; i++)
	{
		nId = _ttoi(m_listAlarm.GetItemText(i, 1));
		if(nId == id)
		{
			bRet = TRUE;
			break;
		}
	}

	g_csAlarm.Unlock();

	return bRet;
}

void CDlgAlarm::OnClickListAlarm(NMHDR* pNMHDR, LRESULT* pResult) 
{
 	CListCtrl* p=(CListCtrl*)FromHandle(pNMHDR->hwndFrom);
 	POSITION pos = p->GetFirstSelectedItemPosition();
	int nItem = p->GetNextSelectedItem(pos);

	if(nItem < 0 || nItem > m_listAlarm.GetItemCount())
	{
		return;
	}

	// 알람모듈창 출력.
	ALARM_EVENT_DATA *pAlarm = (ALARM_EVENT_DATA*)(m_listAlarm.GetItemData(nItem));

	if( pAlarm != NULL)
		m_pDlgAlarmModule->ShowAlarm(pAlarm);

	*pResult = 0;
}

int CDlgAlarm::GetListCount()
{
	if (m_listAlarm)
		return m_listAlarm.GetItemCount();
	else
		return 0;
}


//CString CDlgAlarm::GetText(int iRow, int col)
//{
//	m_sprd.SetRow(iRow);
//	m_sprd.SetCol(col);
//	return m_sprd.GetText();
//}
//
//
//BOOL CDlgAlarm::LoadSpred(CString strFileName, bool bNew)
//{
//	CString filename;
////	if(strFileName.IsEmpty())
//	if( bNew  == true)
//	{
//		CString filter   = _T("Alarm Files (*.xls)|*.xls|");
////		CString filter   = "Configuration Files (*.xls)|*.xls|All Files(*.*)|*.*||";  
//
//		//CFileDialog dlg(TRUE);
//		//filename.Format(_T("C:\\%s"), _T("*.xls"));
//		filename.Format(_T("%s"), strFileName);
//		
//		CFileDialog dlg(TRUE, _T(""), filename, OFN_FILEMUSTEXIST, filter);
//		//dlg.m_ofn.lpstrTitle = "Select Excel File";
//		if (dlg.DoModal() == IDOK)
//		{
//			filename = dlg.GetPathName();
//		}
//		else
//		{
//			return FALSE;
//			//filename.Empty();
//		}
//	}
//	else
//	{
//		filename = strFileName;
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	VARIANT varr, varg;
//	short	iRes, iCount, iHandle;
//	SAFEARRAYBOUND rgsabound[1];
//	long	lG		= 0;
//	BOOL	bRes	= FALSE;
//
//	CString strLog = _T("C:\\Errorlog.txt");
//
//	VariantClear(&varr);
//	VariantInit(&varr);
//	VariantClear(&varg);
//	VariantInit(&varg);
//	varr.vt = VT_ARRAY | VT_BSTR;
//	varg.vt = VT_BSTR;
//	rgsabound[0].lLbound = 0;
//	rgsabound[0].cElements = 4;
//	varr.parray = SafeArrayCreate(VT_BSTR, 1, rgsabound);
//
//	//m_sprd.ClearRange(1, df_StartRow, df_MaxCol, df_MaxRow, FALSE);	//기존의 데이터를 클리어 시켜주고 다시 그려준다
//	m_sprd.ClearRange(-1, -1, 0, 0, TRUE);	//전체 데이터 클리어 -jhkang
//	iRes = m_sprd.IsExcelFile(filename);
//	if ( iRes == 1)
//	{	
//		bRes = m_sprd.GetExcelSheetList(filename, varr, &iCount, strLog, &iHandle, TRUE);
//		if ( bRes)
//		{
//			SafeArrayGetElement(varr.parray, &lG, &varg.bstrVal);
//			bRes = m_sprd.ImportExcelSheet(iHandle, varg);
//
//			if( bRes)
//			{
////				MessageBox("Import complete.");
//				return TRUE;
//			}
//			else		MessageBox(_T("Import did not succeed."));
//		}
//		else
//			MessageBox(_T("Cannot return information for Excel file."));
//	}
//	else if( iRes == 0)
//	{
//		MessageBox(_T("The file is not an Excel file in BIFF8 format, and Spread cannot open the file"));
//	}
//	else if( iRes == 2)
//	{
//		MessageBox(_T("The file is locked or the type cannot be determined, and Spread cannot open the file.Examples of when this value might be returned are when the file is open in Excel or if the file is in use by another application"));
//	}
//	else
//	{
//		MessageBox(_T("File is not an Excel file or is locked and cannot be imported."));
//	}
//		
//
//	//////////////////////////////////////////////////////////////////////////
//	return FALSE;
//}



BOOL CDlgAlarm::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	switch(pMsg->message)
	{
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

CString CDlgAlarm::MessageCode2Str(int nID, CString strPath)
{
	CString sRtn = _T("Abnormal Error");

	CFileFind finder;
//	CString sAlarmFile = strPath + "\\MessageList.csv";
	CString sAlarmFile;
	sAlarmFile.Format(_T("%s\\%s"), strPath, _T("Config\\CoreFunction\\AlarmConfig\\AlarmMessage.csv"));
	sAlarmFile.Replace(_T("\\"), _T("/"));
	if (finder.FindFile(sAlarmFile) == FALSE)
	{
		CString str = _T("Can not Find MessageList Defined file : ");
		str += sAlarmFile;
		return str;
	}

	CStdioFile AlarmFile(sAlarmFile, CStdioFile::shareDenyNone );
//	CArchive ar(&AlarmFile, CArchive::load);

	int iStart = 0, iEnd = 0;
	int	nAlarmID = 0;
	CString strLine;
	CString strAlarmID, strAltx;

	AlarmFile.ReadString(strLine);		// Header

	while(TRUE)
	{
		if (AlarmFile.ReadString(strLine))
		{
			strLine.TrimLeft();
			iStart = strLine.Find(_T("\""));
			if (iStart == 0)
				iStart = 1;
			else
				iStart = 0;

			iEnd = strLine.Find(_T(","));	//ALARM ID

			strAlarmID = strLine.Mid(iStart, iEnd);
			nAlarmID = _tstoi(strAlarmID);

//			// UNICODE
//#ifdef _UNICODE
//			strAlarmID = strLine.Mid(1, iEnd);
//#else
//			strAlarmID = strLine.Mid(0, iEnd);
//#endif
//			nAlarmID = _ttoi(strAlarmID);

			if(nID == nAlarmID)
			{
				strLine.Delete(0,iEnd+1);
				iEnd	= strLine.Find(_T(","));	//TYPE

				strLine.Delete(0,iEnd+1);
				iEnd	= strLine.Find(_T(","));	//USE

				strAlarmID = strLine.Mid(0, iEnd);
				if( strAlarmID.CollateNoCase(_T("K")) == 0)		// 한글
				{
					strLine.Delete(0,iEnd+1);
					iEnd	= strLine.Find(_T(","));
					strAltx = strLine.Mid(0, iEnd);
				}
				else									// 영어
				{
					strLine.Delete(0,iEnd+1);

					iEnd	= strLine.Find(_T(","));
					strLine.Delete(0,iEnd+1);
					strAltx = strLine;
				}

		//		ar.Close();
				AlarmFile.Close();
				return strAltx;
			}
		}
		else
		{
			break;
		}
	}
//	ar.Close();
	AlarmFile.Close();

	return _T("Unknown Alarm");
}

//lhj 20181225 lang Report Alarm
int CDlgAlarm::UpdateLanguage(HINSTANCE hResource_Lang_Dll)
{
	TCHAR strText[MAX_PATH]={0,};
	TCHAR strText100[MAX_PATH]={0,};
	TCHAR strText101[MAX_PATH]={0,};
	TCHAR strText102[MAX_PATH]={0,};
	TCHAR strText103[MAX_PATH]={0,};
	TCHAR strText104[MAX_PATH]={0,};
	TCHAR strText105[MAX_PATH]={0,};


	if(hResource_Lang_Dll == NULL)
		return 0;

// 	LoadString(hResource_Lang_Dll,IDS_STRING316_Time, strText100,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING104_ALARM, strText101,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING142_TEXT, strText105,MAX_PATH);

	CString strTime;
	strTime.Format(_T("%s"),strText100);
	_tcscpy_s(strText102, _countof(strText102), strTime);
//	sprintf(strText102,strTime);

	CString strAlarmId;
	strAlarmId.Format(_T("%s Id"),strText101);
	_tcscpy_s(strText103, _countof(strText103), strAlarmId);
//	sprintf(strText103,strAlarmId);

	CString strAlarmText;
	strAlarmText.Format(_T("%s %s"),strText101,strText105);
	_tcscpy_s(strText104, _countof(strText104), strAlarmText);
//	sprintf(strText104,strAlarmText);


	TCHAR* szText[4] = {strText102, strText103, _T("Module"), strText104};
	int nWidth[4] = {200, 100, 100, 500};
	for(int i=0;i<4;i++)
	{
//		TCHAR name[40];
		LVCOLUMN col;

		col.mask=LVCF_TEXT;
		col.pszText=szText[i];
		col.cchTextMax=40;

		// Set column name
		m_listAlarm.SetColumn(i, &col);
	}

	return 0;
}

BOOL CDlgAlarm::LoadSpred(CString strFileName, bool bNew)
{
	CString filename;
	//	if(strFileName.IsEmpty())
	if (bNew == true) 
	{
		CString filter = _T("Alarm Files (*.xls)|*.xls|");
		//		CString filter   = "Configuration Files (*.xls)|*.xls|All Files(*.*)|*.*||";  

				//CFileDialog dlg(TRUE);
				//filename.Format(_T("C:\\%s"), _T("*.xls"));
		filename.Format(_T("%s"), strFileName);

		CFileDialog dlg(TRUE, _T(""), filename, OFN_FILEMUSTEXIST, filter);
		//dlg.m_ofn.lpstrTitle = "Select Excel File";
		if (dlg.DoModal() == IDOK)
		{
			filename = dlg.GetPathName();
		}
		else
		{
			return FALSE;
			//filename.Empty();
		}
	}
	else
	{
		filename = strFileName;
	}

	CString strLog = _T("C:\\Errorlog.txt");

	if (m_sprdResult != nullptr)
	{
		m_sprdResult->ClearRange(-1, -1, 0, 0);

		if (m_sprdResult->IsExcelFile(filename) == 1)
		{
			m_sprdResult->ImportExcelBook(filename, strLog);

			return TRUE;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////
	//VARIANT varr, varg;
	//short	iRes, iCount, iHandle;
	//SAFEARRAYBOUND rgsabound[1];
	//long	lG = 0;
	//BOOL	bRes = FALSE;

	//CString strLog = _T("C:\\Errorlog.txt");

	//VariantClear(&varr);
	//VariantInit(&varr);
	//VariantClear(&varg);
	//VariantInit(&varg);
	//varr.vt = VT_ARRAY | VT_BSTR;
	//varg.vt = VT_BSTR;
	//rgsabound[0].lLbound = 0;
	//rgsabound[0].cElements = 4;
	//varr.parray = SafeArrayCreate(VT_BSTR, 1, rgsabound);

	////m_sprd.ClearRange(1, df_StartRow, df_MaxCol, df_MaxRow, FALSE);	//기존의 데이터를 클리어 시켜주고 다시 그려준다
	//m_sprd.ClearRange(-1, -1, 0, 0, TRUE);	//전체 데이터 클리어 -jhkang
	//iRes = m_sprd.IsExcelFile(filename);
	//if (iRes == 1)
	//{
	//	bRes = m_sprd.GetExcelSheetList(filename, varr, &iCount, strLog, &iHandle, TRUE);
	//	if (bRes)
	//	{
	//		SafeArrayGetElement(varr.parray, &lG, &varg.bstrVal);
	//		bRes = m_sprd.ImportExcelSheet(iHandle, varg);

	//		if (bRes)
	//		{
	//			//				MessageBox("Import complete.");
	//			return TRUE;
	//		}
	//		else		MessageBox(_T("Import did not succeed."));
	//	}
	//	else
	//		MessageBox(_T("Cannot return information for Excel file."));
	//}
	//else if (iRes == 0)
	//{
	//	MessageBox(_T("The file is not an Excel file in BIFF8 format, and Spread cannot open the file"));
	//}
	//else if (iRes == 2)
	//{
	//	MessageBox(_T("The file is locked or the type cannot be determined, and Spread cannot open the file.Examples of when this value might be returned are when the file is open in Excel or if the file is in use by another application"));
	//}
	//else
	//{
	//	MessageBox(_T("File is not an Excel file or is locked and cannot be imported."));
	//}


	//////////////////////////////////////////////////////////////////////////
	return FALSE;
}

CString CDlgAlarm::GetText(int iRow, int col)
{
	if (m_sprdResult != nullptr)
	{
		return m_sprdResult->GetDataString(col, iRow);
	}
	
	return _T("");
}