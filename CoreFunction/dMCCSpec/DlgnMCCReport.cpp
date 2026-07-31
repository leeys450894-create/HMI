// DlgnMCCReport.cpp : implementation file
//

#include "pch.h"
#include "dMCCSpec.h"
#include "DlgnMCCReport.h"
#include <direct.h>
#include <afxmt.h>			//CCriticalSection
#include "..\\MotorFunction\\AxisType.h"
#include "common/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgnMCCReport dialog

CCriticalSection	g_csTime;
CCriticalSection	g_csDir;
CCriticalSection	g_csTLog;
CCriticalSection	g_csILog;

BOOL gf_MakeDir(CString sPath)
{
	g_csDir.Lock();
	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		g_csDir.Unlock();
		return FALSE;
	}
	
	CString sTem;
    int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"),nFind+1)) >= 0)
	{
		sTem = sPath.Left(nFind);
	//	_mkdir(sTem);
		_tmkdir(sTem);
	}
	
	g_csDir.Unlock();
	return TRUE;
}

CString gf_MakeCurTimeString()
{
	g_csTime.Lock();
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%02d%02d_%02d%02d_%02d.%03d"),st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
	g_csTime.Unlock();
	return sTime;
}

CDlgnMCCReport::CDlgnMCCReport(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgnMCCReport::IDD, pParent)
	, m_strAppPath(_T(""))
	, m_nDeleteRowNum(0)
	, m_oldEvtFlag(0)
{
	//{{AFX_DATA_INIT(CDlgnMCCReport)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hbrushBK = ::CreateSolidBrush(RGB(225, 231, 253));


	m_pWorkThread = NULL;

	m_hEvtWorkStart    = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset
	m_hEvtWorkDone     = CreateEvent(NULL, TRUE,  TRUE,  NULL); // manual reset, initially set
	m_hEvtKillThread   = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset
	m_hEvtThreadKilled = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset	// jsyoon 20150224 chnage

	m_strConfigPath.Empty();
	m_strLogPath.Empty();
	m_strLogFile.Empty();
	m_strFilePath.Empty();
	m_strEQName.Format(_T("EQ"));

	m_bInitSendToFtp = FALSE;

	m_hTarget = NULL;
	m_hWndMccLog = NULL;

	m_pMCCMap = NULL;
	m_hMCCMapFile = NULL;

	m_sprdMccILog = NULL;

	m_bStartMCCExe = true;

	CreateMemoryMap();
}


void CDlgnMCCReport::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgnMCCReport)
	DDX_Control(pDX, IDC_LIST_MCC_TLOG, m_list_TLOG);
	DDX_Control(pDX, IDC_LIST_MCC_ILOG, m_list_ILOG);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHECK_T, m_chkUseSend);
//	DDX_Control(pDX, IDC_FPSPREAD_MCC_ILOG, m_stSpreadILog);
}


BEGIN_MESSAGE_MAP(CDlgnMCCReport, CDialog)
	//{{AFX_MSG_MAP(CDlgnMCCReport)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHECK_T, &CDlgnMCCReport::OnBnClickedCheckT)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgnMCCReport message handlers

BOOL CDlgnMCCReport::OnInitDialog()
{
	CDialog::OnInitDialog();

	//JCY 20211221 Farpoint Spread .OCX -> .DLL로 호출
	m_sprdMccILog = new CMySpreadDll(IDC_FPSPREAD_MCC_ILOG, FPS_RESOURCE_ID + 15, this, TRUE);

	//Spread Header 입력 
	TCHAR szBuff[_MAX_PATH];
	memset(szBuff, 0, MAX_PATH);
	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);
	TCHAR* pch = _tcsrchr(szBuff, _T('\\'));
	m_strAppPath = CString(szBuff, pch - szBuff + 1);

	if (m_sprdMccILog->GetSafeHwnd() != NULL)
	{
		m_sprdMccILog->SetOperationMode(SS_OPMODE_READONLY);
		m_sprdMccILog->SetSelColor(RGB(0, 255, 0), RGB(0, 0, 0));
		m_sprdMccILog->SetGrayAreaColor(RGB(255, 255, 255), RGB(0, 0, 0));
		m_sprdMccILog->SetBool(SSB_SCROLLBARMAXALIGN, FALSE);
		m_sprdMccILog->SetBool(SSB_SCROLLBAREXTMODE, FALSE);
		m_sprdMccILog->SetBool(SSB_SCROLLBARSHOWMAX, TRUE);
		m_sprdMccILog->SetBool(SSB_HORZSCROLLBAR, TRUE);
		m_sprdMccILog->SetBool(SSB_VERTSCROLLBAR, FALSE);
		m_sprdMccILog->SetBool(SSB_HSCROLLBARTRACK, FALSE);
		m_sprdMccILog->SetBool(SSB_VSCROLLBARTRACK, FALSE);

		m_sprdMccILog->SetColHeaderRows(1);
		//	SetMccILogHeader();
	}


	// jsyoon 20171019 remove 

	LV_COLUMN lvcolumn;
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = _T("Log");
	lvcolumn.iSubItem = 0;
	lvcolumn.cx = 10000;
	m_list_TLOG.InsertColumn(0, &lvcolumn);
	m_list_ILOG.InsertColumn(0, &lvcolumn);
	//
	//SYSTEMTIME st;
	//::GetLocalTime(&st);
	//CTime ct(st);
	//m_strLogFile.Format("%04d%02d%02d%02d%02d%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
//	//m_strEQPID.Format("%s", m_pSystem->m_strEquipName );
//	//m_strEQPID.Format("%s", m_pSystem->stTitle.m_strEquipName );	// jsyoon 20161226 change
	//m_strSendLogFile.Format("%s", m_strLogFile);	// jsyoon 20170201 add
	//
	//m_iOldTime = st.wHour*60 + st.wMinute;
	//
	//TCHAR szBuff[_MAX_PATH];
	//::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);	
	//char* pch = ::strrchr(szBuff, '\\');
	//m_strAppPath = CString(szBuff, pch - szBuff + 1);
	//SetMccILogHeader();
	//
	//BeginThread_WorkProc();
	//

	m_chkUseSend.ShowWindow(SW_HIDE);

	if (m_bStartMCCExe == true)
	{
		SetTimer(6, 5000, NULL);
		//BOOL bFind = FindTargetWindow(STR_MCC_NAME);
		//if (!bFind)
		//{
		//	LRESULT lRet = FALSE;
		//	//	lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, (LPCSTR)_T(""), NULL, SW_SHOW);
		//	lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, NULL, NULL, SW_SHOW);
		//}

		//SetTimer(5, 60000, NULL);	// Check Live MCCLog.exe	// jsyoon 20180122 업데이트 가능하게 60초 마다 검색하자
	}
	

	m_bInitSendToFtp = TRUE;
	SetTimer(1, 10, NULL);
//	SetTimer(2, 100, NULL);		2017.05.25 khm metrosys.exe에서 기능사용안함 
//								MCCLog.exe의 Queue에 데이터 저장하라는 메시지만 전송함
//	SetTimer(3, 1000, NULL);

	//m_chkUseSend.SetCheck(m_pSystem->m_bMccTransferOn);
//	SetTimer(5, 60000, NULL);	// Check Live MCCLog.exe	// jsyoon 20180122 업데이트 가능하게 60초 마다 검색하자
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CDlgnMCCReport::GetCnfPath(int nEQType, CString sPath)
{
	CString strRet;
	strRet.Format(_T("%s%s%s"), m_strAppPath, _T("CONFIG\\MCC\\"), sPath);

	
	//switch (nEQType)
	//{
	//case EQU_TYPE_ICE:
	//	strRet.Format(_T("%s%s%s"),m_strAppPath, _T("CONFIG\\ICE\\"), sPath);
	//	break;
	//case EQU_TYPE_ICM:
	//	strRet.Format(_T("%s%s%s"),m_strAppPath, _T("CONFIG\\ICM\\"), sPath);
	//	break;
	//case EQU_TYPE_ICP:
	//	strRet.Format(_T("%s%s%s"),m_strAppPath, _T("CONFIG\\ICP\\"), sPath);
	//	break;
	//case EQU_TYPE_OCP:
	//	strRet.Format(_T("%s%s%s"),m_strAppPath, _T("CONFIG\\OCP\\"), sPath);
	//	break;
	//default:
	//	strRet.Format(_T("%s%s%s"),m_strAppPath, _T("CONFIG\\MCC\\"), sPath);
	//	break;
	//}

	return strRet;
}

int CDlgnMCCReport::WriteFile(CFile *pFile, CString &sPath, CString sLog)
{
	if(!sLog.IsEmpty())
		sLog += "\r\n";

    BOOL bNew = FALSE;
    CFileStatus fs;
    if (!CFile::GetStatus(sPath, fs))  //if not exist yet
    {
		gf_MakeDir(sPath);// _mkdir;
        bNew = TRUE;
    }
    else if (pFile->m_hFile != CFile::hFileNull)
    {
		CString s1 = fs.m_szFullName ;
		if ((pFile->GetFilePath() != s1))
		{
			bNew = TRUE;
		}
	}
	
	if (pFile->m_hFile != CFile::hFileNull && bNew)
	{
		pFile->Close();
	}
	
	CFileException* pError = NULL;
	if (pFile->m_hFile == CFile::hFileNull)
	{
		UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
        if (bNew)
			nOpenFlags |= CFile::modeCreate;
		
		if (!pFile->Open(sPath , nOpenFlags, pError))
			return 0;
	}
	if (pFile->m_hFile != CFile::hFileNull)
	{
		if(!sLog.IsEmpty())
		{
			pFile->SeekToEnd();
			pFile->Write(sLog, sLog.GetLength()) ;
			pFile->Flush();
		}
	}
	return 1;
}

int CDlgnMCCReport::TLOG(CString s)
{
	s.FreeExtra();
	if (m_strLogPath.IsEmpty()) return 0;

	g_csTLog.Lock();
	
	CString strPath, strLog;
	m_strTLogFile.Format(_T("T-%s-%s.csv"), m_strEQPID,m_strLogFile);	// "T" LOG T-EQPID-YYYYMMDDhhmmss.csv 
//	strPath.Format("%s\\%s",m_strLogPath,m_strTLogFile);
	strPath.Format(_T("%s\\T\\%s"),m_strLogPath,m_strTLogFile);		// jsyoon 20161226 change
	if( s.IsEmpty())
		strLog.Format(_T(""));		// jsyoon 20161229 add
	else
		strLog.Format(_T("%s"),s);
	//		strLog.Format("%s,%s",gf_MakeCurTimeString(),s);
		
	int nR = WriteFile(&m_fileTLog, strPath, strLog);

	if( !s.IsEmpty())
	{
		if (nR > 0)
		{
			AddString(strLog);
		}
		else
		{
			strLog += _T("-->File Save Failed");
			AddString(strLog);
		}
	}

	g_csTLog.Unlock();
	
	return nR;
}

int CDlgnMCCReport::ILOG(CString s)
{
	s.FreeExtra();
	//if (m_strLogPath.IsEmpty()) return 0;

	g_csILog.Lock();

	int nR = 0;

	if (!s.IsEmpty())
	{
		AddString(s, 1);
	}
	
//	CString strPath, strLog;
//	m_strILogFile.Format(_T("I-%s-%s.csv"), m_strEQPID,m_strLogFile);	// "T" LOG T-EQPID-YYYYMMDDhhmmss.csv 
////	strPath.Format("%s\\%s",m_strLogPath,m_strTLogFile);
//	strPath.Format(_T("%s\\I\\%s"),m_strLogPath,m_strILogFile);		// jsyoon 20161226 change
//	if( s.IsEmpty())
//		strLog.Format(_T(""));		// jsyoon 20161229 add
//	else
//		strLog.Format(_T("%s"),s);
//	
//	int nR = WriteFile(&m_fileILog, strPath, strLog);

	//if( !s.IsEmpty())
	//{
	//	if (nR > 0)
	//	{
	//		AddString(strLog, 1);
	//	}
	//	else
	//	{
	//		strLog += _T("-->File Save Failed");
	//		AddString(strLog, 1);
	//	}
	//}

	g_csILog.Unlock();
	
	return nR;
}

int CDlgnMCCReport::indexLOG()
{
	if (m_strLogPath.IsEmpty()) return 0;
	
	CString strPath = _T(""), strLog = _T("");
	m_strindexFile.Format(_T("index-%s-%s.csv"),m_strEQPID,m_strLogFile);		// "Index" LOG index-EQPID-YYYYMMDDhhmmss.csv 
//	strPath.Format("%s\\%s",m_strLogPath,m_strindexFile);
	strPath.Format(_T("%s\\INDEX\\%s"),m_strLogPath,m_strindexFile);	// jsyoon 20161226 change
	
	int nR = WriteFile(&m_fileindexLog,strPath,strLog);
	
	return nR;
}

void CDlgnMCCReport::AddString(CString &sLog, int iList)
{
	switch (iList)
	{
	case 0:		// TLOG
		{
			if (m_list_TLOG.m_hWnd == NULL) return;
			int i=m_list_TLOG.GetItemCount();
			m_list_TLOG.InsertItem(i, _T(""));
			m_list_TLOG.SetItemText(i, 0, sLog);
			
			CString str = m_list_TLOG.GetItemText(i,0);	
			if (m_list_TLOG.GetScrollLimit(SB_VERT) == m_list_TLOG.GetScrollPos(SB_VERT)+1)
			{
				if (i>0)
					m_list_TLOG.EnsureVisible(i,FALSE);
			}
			
			if (i > 1000)
				m_list_TLOG.DeleteItem(0);
		}
		break;
	case 1:		// ILOG
		{
			if (m_list_ILOG.m_hWnd == NULL) return;
			int i=m_list_ILOG.GetItemCount();
			m_list_ILOG.InsertItem(i, _T(""));
			m_list_ILOG.SetItemText(i, 0, sLog);
			
			CString str = m_list_ILOG.GetItemText(i,0);	
			if (m_list_ILOG.GetScrollLimit(SB_VERT) == m_list_ILOG.GetScrollPos(SB_VERT)+1)
			{
				if (i>0)
					m_list_ILOG.EnsureVisible(i,FALSE);
			}
			
			if (i > 1000)
				m_list_ILOG.DeleteItem(0);
		}
		break;
	}
}

void CDlgnMCCReport::OnTimer(UINT_PTR nIDEvent)
{	
	if(nIDEvent == 1)	// Log File name
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		CTime ct(st);

		int iCurTime = 0;
		iCurTime = st.wHour*60 + st.wMinute;

		// jsyoon 20150224 change
		if( st.wSecond == 0)							// 1m
		{
			if( m_bInitSendToFtp)
			{
				m_iOldTime = iCurTime;
//				SetSendToFtp();
				m_strNewLogFile.Format(_T("%04d%02d%02d%02d%02d%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
				m_bInitSendToFtp = FALSE;

				m_strLogFile.Format(_T("%s"), m_strNewLogFile);
				// m_strLogFile = m_strNewLogFile;
			/*	indexLOG();
				TLOG(_T(""));
				WriteMCCInfo(0);*/

				SetTimer(3, 1000, NULL);
				SetTimer(4, 1000, NULL);

				KillTimer(1);
			}
			else
			{
				//if( abs(iCurTime - m_iOldTime) >= m_pSystem->m_iMCCUploadTime)	// jsyoon 20150205
				//{	
				//	KillTimer(3);
				//	KillTimer(4);
				//	m_strSendLogFile.Format("%s", m_strLogFile);
				//
				//	m_iOldTime = iCurTime;
				//	SetSendToFtp();
				//	m_strNewLogFile.Format("%04d%02d%02d%02d%02d%02d",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,0/*st.wSecond*/);
				//
				//	m_strLogFile.Format("%s", m_strNewLogFile);
				////	m_strLogFile = m_strNewLogFile;
				//	indexLOG();
				//	TLOG(_T(""));
				//	WriteMCCInfo(0);
				//
				//	SetTimer(3, 1000, NULL);
				//	SetTimer(4, 1000, NULL);
				//}
			}
		}
	}

	if(nIDEvent == 2)
	{
// 		int iMaxRow = 0;
// 		if( m_nDeleteRowNum != 0)
// 		{
// 			m_sprdMccILog.DeleteRows(1, m_nDeleteRowNum);					//선택된 Row는 전부 지운다
// 			iMaxRow = m_sprdMccILog.GetMaxRows();
// 			m_sprdMccILog.SetMaxRows(iMaxRow-m_nDeleteRowNum);
// 			m_sprdMccILog.Refresh();
// 			m_nDeleteRowNum = 0;
// 		}
// 
// 		iMaxRow = m_sprdMccILog.GetMaxRows();
// 		UpdateMCCInfo(iMaxRow);

		
		SYSTEMTIME st;
		::GetLocalTime(&st);
		CTime ct(st);

		if( st.wMilliseconds > 500)							// 1m
		{
			KillTimer(2);
			SetTimer(1, 500, NULL);

// 			WriteMCCInfo(0);
// 			SetTimer(4, 1000, NULL);
		}
	}

	if(nIDEvent == 3)
	{
		UpdateMCCTLog();
	}

	if(nIDEvent == 4)
	{
		UpdateMCCILOG();	// jsyoon 20170124 change
	}

	if(nIDEvent == 5)		// jsyoon 20170608 add
	{
		BOOL bFind = FindTargetWindow(STR_MCC_NAME);
		if(!bFind)
		{
			LRESULT lRet    = FALSE;
		//	lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, (LPCSTR)_T(""), NULL, SW_SHOW);
			lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, NULL, NULL, SW_SHOW);
		}
	}

	if (nIDEvent == 6)
	{
		KillTimer(6);
		BOOL bFind = FindTargetWindow(STR_MCC_NAME);
		if (!bFind)
		{
			LRESULT lRet = FALSE;
			//	lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, (LPCSTR)_T(""), NULL, SW_SHOW);
			lRet = (LRESULT)ShellExecute(NULL, _T("open"), m_strFilePath, NULL, NULL, SW_SHOW);
		}

		SetTimer(5, 60000, NULL);	// Check Live MCCLog.exe	// jsyoon 20180122 업데이트 가능하게 60초 마다 검색하자
	}

	CDialog::OnTimer(nIDEvent);
}

// eVent
void CDlgnMCCReport::WriteMCCEvent(int iEvent, BOOL bStart)
{
	CString strLog;
	strLog.Format(_T(""));
	bStart;
	iEvent;
	TLOG(strLog);
}

// information
void CDlgnMCCReport::WriteMCCInfo(int iRow)
{
	m_list_ILOG.DeleteAllItems();

	CString strLog, strValue, strData;
	strLog.Empty();

	//m_sprdMccILog.SetRow(iRow);
	int nMaxCol = 0;
	//nMaxCol = m_sprdMccILog.GetMaxCols();
//	for (int iCol = 1; iCol<MCC_ILOG_COL_END; iCol++)
	for (int iCol = 1; iCol<=nMaxCol; iCol++)
	{
		//m_sprdMccILog.SetCol(iCol);
		//strValue = m_sprdMccILog.GetValue();
		strValue.TrimLeft();
		strValue.TrimRight();
		if( iCol == nMaxCol)
			strData.Format(_T("%s"), strValue);
		else
			strData.Format(_T("%s,"), strValue);

		strLog += strData;
	}

	ILOG(strLog);
}

// Action
void CDlgnMCCReport::WriteMCCAction(CString strTime, int iAction, BOOL bStart, int nStep/* = 0*/, int nFromto/* = 0*/)
{
	// Event Time, Module ID, Log Type, StepID, GlassID, LotID, Eqp PPID, Action Name, Form Pos, to Pos, START or END
	CString strLog;
	if( nStep == 0)
	{
		//strLog.Format("%s,%s,A,%s,%s,%s,%s=%s,%s=%s=%s", 
		//	strTime,
		//	m_pSystem->m_strEquipName, 
		//	m_pSystem->stLotInfo.m_szStepID, 
		//	m_pSystem->stLotInfo.m_szGlassID, 
		//	m_pSystem->stLotInfo.m_szLotID, 
		//	m_pSystem->stLotInfo.m_szRecipeID, 
		//	m_pSystem->stLotInfo.m_szRecipeID, 
		//	GetActiontoStr(iAction), 
		//	GetFromto(nFromto),
		//	(bStart)? "START":"END");
	}
	else
	{
		//strLog.Format("%s,%s,A,%s,%s,%s,%s=%s,%s_%d=%s=%s", 
		//	strTime,
		//	m_pSystem->m_strEquipName, 
		//	m_pSystem->stLotInfo.m_szStepID, 
		//	m_pSystem->stLotInfo.m_szGlassID, 
		//	m_pSystem->stLotInfo.m_szLotID, 
		//	m_pSystem->stLotInfo.m_szRecipeID, 
		//	m_pSystem->stLotInfo.m_szRecipeID, 
		//	GetActiontoStr(iAction),
		//	nStep,
		//	GetFromto(nFromto),
		//	(bStart)? "START":"END");
	}
// 	strLog.Format("%s,%s,A,%s,%s,%s,%s=%s,%s=%s=%s=%s", 
// 		strTime,
// 		m_pSystem->m_strEquipName, 
// 		m_pSystem->stLotInfo.m_szStepID, 
// 		m_pSystem->stLotInfo.m_szGlassID, 
// 		m_pSystem->stLotInfo.m_szLotID, 
// 		m_pSystem->stLotInfo.m_szRecipeID, 
// 		m_pSystem->stLotInfo.m_szRecipeID, 
// 		GetActiontoStr(iAction), 
// 		m_pSystem->m_strEquipName,
// 		m_pSystem->m_strEquipName,
// 		(bStart)? "START":"END");
	
	TLOG(strLog);
}

// Signal
void CDlgnMCCReport::WriteMCCSignal(CString strTime, int iSignal, BOOL bStart, int nFromto/* = 0*/)
{
	// Event Time, Module ID, Log Type, StepID, GlassID, LotID, Eqp PPID, Action Name, Form Pos, to Pos, START or END
	//CString strLog;
	//strLog.Format("%s,%s,S,%s,%s,%s,%s=%s,%s=%s=%s", 
	//	strTime,
	//	m_pSystem->m_strEquipName, 
	//	m_pSystem->stLotInfo.m_szStepID, 
	//	m_pSystem->stLotInfo.m_szGlassID, 
	//	m_pSystem->stLotInfo.m_szLotID, 
	//	m_pSystem->stLotInfo.m_szRecipeID, 
	//	m_pSystem->stLotInfo.m_szRecipeID, 
	//	GetSignaltoStr(iSignal), 
	//	GetFromto(nFromto),
	//	(bStart)? "START":"END");
// 	CString strLog;
// 	strLog.Format("%s,%s,S,%s,%s,%s,%s=%s,%s=%s=%s=%s", 
// 		strTime,
// 		m_pSystem->m_strEquipName, 
// 		m_pSystem->stLotInfo.m_szStepID, 
// 		m_pSystem->stLotInfo.m_szGlassID, 
// 		m_pSystem->stLotInfo.m_szLotID, 
// 		m_pSystem->stLotInfo.m_szRecipeID, 
// 		m_pSystem->stLotInfo.m_szRecipeID, 
// 		GetSignaltoStr(iSignal), 
// 		m_pSystem->m_strEquipName,
// 		m_pSystem->m_strEquipName,
// 		(bStart)? "START":"END");

	//TLOG(strLog);
}

// Error
void CDlgnMCCReport::WriteMCCError(CString strTime,int iError, BOOL bSet)
{
	//CString strLog;
	//strLog.Format("%s,%s,E,%s,%s,%s,%s,%s=%s", 
	//	strTime,
	//	m_pSystem->m_strEquipName, 
	//	m_pSystem->stLotInfo.m_szStepID, 
	//	m_pSystem->stLotInfo.m_szGlassID, 
	//	m_pSystem->stLotInfo.m_szLotID, 
	//	m_pSystem->stLotInfo.m_szRecipeID, 
	//	(bSet)? "SET":"RESET",
 	//	GetErrortoStr(iError));
// 		m_pSystem->m_strEquipName,
// 		m_pSystem->m_strEquipName,
// 		(bSet)? "SET":"RESET");

	//TLOG(strLog);
}

// Warning
void CDlgnMCCReport::WriteMCCWarring(CString strTime,int iError, BOOL bSet)
{
	//CString strLog;
	//strLog.Format("%s,%s,W,%s,%s,%s,%s,%s=%s",
	//	strTime,
	//	m_pSystem->m_strEquipName, 
	//	m_pSystem->stLotInfo.m_szStepID, 
	//	m_pSystem->stLotInfo.m_szGlassID, 
	//	m_pSystem->stLotInfo.m_szLotID, 
	//	m_pSystem->stLotInfo.m_szRecipeID, 
	//	(bSet)? "SET":"RESET",
	//	GetErrortoStr(iError));
// 	//	GetErrortoStr(iError), 
// 	//	m_pSystem->m_strEquipName,
// 	//	m_pSystem->m_strEquipName,
// 	//	(bSet)? "SET":"RESET");
	//
	//TLOG(strLog);
}

CString CDlgnMCCReport:: GetFromto(int iAction)
{
	CString str;
	//str.Empty();
	//
	//switch (iAction)
	//{
	//case 0:
	//	str.Format("%s=%s", m_pSystem->m_strEquipName,m_pSystem->m_strEquipName);
	//	break;
	//case 1:
	//	str.Format("%s=%s", m_pSystem->m_strEquipName2,m_pSystem->m_strEquipName);
	//	break;
	//case 2:
	//	str.Format("%s=%s", m_pSystem->m_strEquipName,m_pSystem->m_strEquipName2);
	//	break;
	//default:
	//	str.Format("%s=%s", m_pSystem->m_strEquipName,m_pSystem->m_strEquipName);
	//	break;
	//}
	//
	return str;
}

CString CDlgnMCCReport::GetActiontoStr(int iAction)
{
	CString str;
	str.Empty();

	CFileFind finder;

	CString strMCCFile;
//	strMCCFile.Format("%s%s%s",m_strAppPath, STR_DIR_CONFIG,  MCC_ACTION_FILE);
//	strMCCFile = GetCnfPath(m_pSystem->stEquip.m_nEQType, MCC_ACTION_FILE);		// jsyoon 20170529 change
	if (finder.FindFile(strMCCFile) == FALSE)
	{
		str.Format(_T("Can not fined '%s'"), strMCCFile);
		AfxMessageBox(str);
		return _T("");
	}

	CString strLine, strValue, strName;
	CStringArray strArray;
	CString strTemp;
	int lenTemp = 0, iCol = 0;

	CFile fileMCC(strMCCFile, CFile::shareDenyNone );
	CArchive ar(&fileMCC, CArchive::load);
	do 
	{
		if (ar.ReadString(strLine))
		{
			int iStart = 0;
			int iEnd   = 0;
			bool bToggle = TRUE;

			strArray.RemoveAll();

			lenTemp = strLine.GetLength();
			for(int i = 0 ; i < lenTemp; i++)
			{		
				if(bToggle)
				{
					iStart = i;
					bToggle = FALSE;
				}

				if(strLine.GetAt(i) == ',')
				{
					bToggle = TRUE;
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd);
					strArray.Add(strTemp);
				}
				else if( i == lenTemp-1 )
				{
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd+1);
					strArray.Add(strTemp);
				}
			}

			if( strArray.GetSize() > 2 )
			{
				strValue = strArray.GetAt(0);
				strValue.TrimRight();

				strName = strArray.GetAt(2);
				strName.TrimRight();
			}

			if( strValue.CompareNoCase(_T("NO")) == 0)
				continue;

			if (iAction == _ttoi(strValue))
			{
				str.Format(_T("%s"), strName);
				break;
			}
		}
		else				// End file
		{
			break;
		}
	} while (TRUE);

	ar.Close();
	fileMCC.Close();	

	return str;
}

CString CDlgnMCCReport::GetSignaltoStr(int iSignal)
{
	CString str;
	str.Empty();

	CFileFind finder;

	CString strMCCFile;
//	strMCCFile.Format("%s%s%s",m_strAppPath, STR_DIR_CONFIG,  MCC_SIGNAL_FILE);
//	strMCCFile = GetCnfPath(m_pSystem->stEquip.m_nEQType, MCC_SIGNAL_FILE);		// jsyoon 20170529 change
	if (finder.FindFile(strMCCFile) == FALSE)
	{
		str.Format(_T("Can not fined '%s'"), strMCCFile);
		AfxMessageBox(str);
		return _T("");
	}

	CString strLine, strValue, strName;
	CStringArray strArray;
	CString strTemp;
	int lenTemp = 0, iCol = 0;

	CFile fileMCC(strMCCFile, CFile::shareDenyNone );
	CArchive ar(&fileMCC, CArchive::load);
	do 
	{
		if (ar.ReadString(strLine))
		{
			int iStart = 0;
			int iEnd   = 0;
			bool bToggle = TRUE;

			strArray.RemoveAll();

			lenTemp = strLine.GetLength();
			for(int i = 0 ; i < lenTemp; i++)
			{		
				if(bToggle)
				{
					iStart = i;
					bToggle = FALSE;
				}

				if(strLine.GetAt(i) == ',')
				{
					bToggle = TRUE;
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd);
					strArray.Add(strTemp);
				}
				else if( i == lenTemp-1 )
				{
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd+1);
					strArray.Add(strTemp);
				}
			}

			if( strArray.GetSize() > 2 )
			{
				strValue = strArray.GetAt(0);
				strValue.TrimRight();

				strName = strArray.GetAt(2);
				strName.TrimRight();
			}

			if( strValue.CompareNoCase(_T("NO")) == 0)
				continue;

			if (iSignal == _ttoi(strValue))
			{
				str.Format(_T("%s"), strName);
				break;
			}
		}
		else				// End file
		{
			break;
		}
	} while (TRUE);

	ar.Close();
	fileMCC.Close();	

	return str;
}

CString CDlgnMCCReport::GetErrortoStr(int iError)
{
	CString str;
	str.Empty();

	CFileFind finder;

	CString strMCCFile;
//	strMCCFile.Format("%s%s%s",m_strAppPath, STR_DIR_CONFIG,  MCC_ERROR_FILE);
//	strMCCFile = GetCnfPath(m_pSystem->stEquip.m_nEQType, MCC_ERROR_FILE);		// jsyoon 20170529 change
	if (finder.FindFile(strMCCFile) == FALSE)
	{
		str.Format(_T("Can not fined '%s'"), strMCCFile);
		AfxMessageBox(str);
		return _T("");
	}

	CString strLine, strValue, strType, strName;
	CStringArray strArray;
	CString strTemp;
	int lenTemp = 0, iCol = 0;

	CFile fileMCC(strMCCFile, CFile::shareDenyNone );
	CArchive ar(&fileMCC, CArchive::load);
	do 
	{
		if (ar.ReadString(strLine))
		{
			int iStart = 0;
			int iEnd   = 0;
			bool bToggle = TRUE;

			strArray.RemoveAll();

			lenTemp = strLine.GetLength();
			for(int i = 0 ; i < lenTemp; i++)
			{		
				if(bToggle)
				{
					iStart = i;
					bToggle = FALSE;
				}

				if(strLine.GetAt(i) == ',')
				{
					bToggle = TRUE;
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd);
					strArray.Add(strTemp);
				}
				else if( i == lenTemp-1 )
				{
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd+1);
					strArray.Add(strTemp);
				}
			}

			if( strArray.GetSize() > 2 )
			{
				strValue = strArray.GetAt(0);
				strValue.TrimRight();

				strType = strArray.GetAt(1);
				strType.TrimRight();

				strName = strArray.GetAt(2);
				strName.TrimRight();
			}

			if( strValue.CompareNoCase(_T("NO")) == 0)
				continue;

			if (iError == _ttoi(strValue))
			{
				str.Format(_T("%s%s=%s"), strType,strValue,strName);
				break;
			}
		}
		else				// End file
		{
			break;
		}
	} while (TRUE);

	ar.Close();
	fileMCC.Close();	

	return str;
}


UINT CDlgnMCCReport::WorkThreadProbeProc(LPVOID pParam)
{
	CDlgnMCCReport* pProc = (CDlgnMCCReport*)pParam;
	
	BOOL bWorkDone = FALSE;
	while (TRUE)
	{
		// m_hEvtWorkStart Event 시그널이 Set 될때 까지 무한 정 기다린다.
		// 실행중인 Job이 끝나면 여기서 대기
		if (WaitForSingleObject(pProc->m_hEvtWorkStart, INFINITE)
			!= WAIT_OBJECT_0)
		{
			SetEvent(pProc->m_hEvtThreadKilled);
			break;
		}
		
		// m_hEvtKillThread Event 시그널이 Set 되면 Loop를 빠져 나간다
		if (WaitForSingleObject(pProc->m_hEvtKillThread, 0)
			== WAIT_OBJECT_0)
		{
			SetEvent(pProc->m_hEvtThreadKilled);
			break; // Terminate this thread by exiting the proc.
		}
		
		//pProc->m_hEvtWorkDone을 리셋 해줘 작업이 안 끝난 상태(not Done)상태로 설정한다
		//리셋, 작업을 시작하기전에 --> 작업중이라는 의미로 됨
		ResetEvent(pProc->m_hEvtWorkDone);
		
		//////////////////////////////////
		bWorkDone = pProc->GoToWork(pParam);	//실제 작업을 진행하는 함수
		//////////////////////////////////

		SetEvent(pProc->m_hEvtWorkDone);
		
		if(pProc->m_nProcessJobNo == WORK_MCC_KILL_JOB)		
		{
			SetEvent(pProc->m_hEvtThreadKilled);		
			break;;
		}
	}
	
	return 0;
}

void CDlgnMCCReport::BeginThread_WorkProc()
{
	m_nProcessJobNo = 0;		// jsyoon 20150224 add

	if (m_pWorkThread == NULL)
	{
		m_pWorkThread =	AfxBeginThread(WorkThreadProbeProc, this);
	}
	
	SetEvent  (m_hEvtWorkDone);
	ResetEvent(m_hEvtKillThread);		//Reset 의미는 쓰레드를 죽이라는 신호가 죽은 상태
	ResetEvent(m_hEvtThreadKilled);		//Reset 의미는 쓰레드가 살아 있음
}

BOOL CDlgnMCCReport::GoToWork(LPVOID pParam)
{
	pParam;

	BOOL bRet = TRUE;
	int nCmd = m_nProcessJobNo;
	
	switch ( nCmd )
	{
	case WORK_MCC_SEND_LOGFILE:
		SendToFtpServer();
		break;
	case WORK_MCC_KILL_JOB:
		break;
	default : 
		break;
	}
	
	return bRet;
}

BOOL CDlgnMCCReport::SendToFtpServer()
{
///	gLog(m_pSystem->hLog, "SendPPIDFileToFTP() Start");
//	
//	char	szConfigFile[MAX_PATH];
//	CString sPath;
//	sPath.Format("%s%s", m_strConfigPath, dfDEFAULT_CONFIG_MCC_FILE_PATH);
///	strcpy(szConfigFile,sPath);
//	_tcscpy_s(szConfigFile, _countof(szConfigFile), sPath);
//	
//	char szIP[256];
//	szIP[0] = 0x00;
//	GetPrivateProfileString("MCC","IP","192.168.100.167", szIP, MAX_PATH,szConfigFile);
//	
//	char szPort[256];
//	szPort[0] = 0x00;
//	GetPrivateProfileString("MCC","PORT","21", szPort, MAX_PATH,szConfigFile);
//	
//	char szID[256];
//	szID[0] = 0x00;
//	GetPrivateProfileString("MCC","ID","administrator", szID, MAX_PATH,szConfigFile);
//	
//	char szPW[256];
//	szPW[0] = 0x00;
//	GetPrivateProfileString("MCC","PASSWORD","1234", szPW, MAX_PATH,szConfigFile);
//
//	CString strTLogFile, strILogFile, strindexFile;
//	strTLogFile.Format("T-%s-%s.csv",		 m_strEQPID,m_strSendLogFile);	// "T" LOG T-EQPID-YYYYMMDDhhmmss.csv
//	strILogFile.Format("I-%s-%s.csv",		 m_strEQPID,m_strSendLogFile);	// "I" LOG T-EQPID-YYYYMMDDhhmmss.csv
//	strindexFile.Format("index-%s-%s.csv",   m_strEQPID,m_strSendLogFile);	// "index" LOG T-EQPID-YYYYMMDDhhmmss.csv
//
/// 	strTLogFile.Format("T-%s-%s.csv",		 m_strEQPID,m_strLogFile);	// "T" LOG T-EQPID-YYYYMMDDhhmmss.csv
/// 	strILogFile.Format("I-%s-%s.csv",		 m_strEQPID,m_strLogFile);	// "I" LOG T-EQPID-YYYYMMDDhhmmss.csv
/// 	strindexFile.Format("index-%s-%s.csv",   m_strEQPID,m_strLogFile);	// "index" LOG T-EQPID-YYYYMMDDhhmmss.csv
//
/// 	m_strLogFile = m_strNewLogFile;
/// 	indexLOG();
/// 	TLOG(_T(""));
/// //	ILOG(_T(""));	// jsyoon 20170201 remove
/// 	WriteMCCInfo(0);
/// 	int iMaxRow = 0;
/// 	iMaxRow = m_sprdMccILog.GetMaxRows();
/// 	for (int iRow=0; iRow <= iMaxRow; iRow++)
/// 	{
/// 		WriteMCCInfo(iRow);
/// 	}
/// 	m_nDeleteRowNum = iMaxRow;
//	
///	if(m_pSystem->m_bMccTransferOn)
///	if( TRUE )
//	{
//		if( m_pFtp == NULL)
//			m_pFtp = new CFTP_Op;
//
//		if(m_pFtp->ConnectToFTP(szIP, szID, szPW, atoi(szPort), TRUE))
//		{
//	//		gLog(m_pSystem->hLog, "SendPPIDFileToFTP() Can not connect FTP");
//
//			if( m_pFtp != NULL)
//			{
//				m_pFtp->CloseFTPConnection();
//				delete m_pFtp;	
//				m_pFtp = NULL;
//			}
//			return FALSE;
//		}
//		
//		if (!m_strLogPath.IsEmpty())
//		{
//			char m_szTemp[6400];
//			m_szTemp[0] = 0x00;
//			char m_szTempFile[1024];
//			m_szTempFile[0] = 0x00;
//			
//			CString strTmep,  strRemotePath;
//			strRemotePath.Format("MCC\\T\\%s", strTLogFile);
///			strcpy(m_szTempFile, strRemotePath);
//			_tcscpy_s(m_szTempFile, _countof(m_szTempFile), strRemotePath);
//
///			strTmep.Format("%s\\%s",m_strLogPath,strTLogFile);
//			strTmep.Format("%s\\T\\%s",m_strLogPath,strTLogFile);		// jsyoon 20161226 change
//			_tcscpy_s(m_szTemp, _countof(m_szTemp), strTmep);
//
//			if( m_pFtp->SendFileToFTP(m_szTemp,m_szTempFile) == 0) 
//			{
//				strRemotePath.Format("MCC\\I\\%s", strILogFile);
//				_tcscpy_s(m_szTempFile, _countof(m_szTempFile), strRemotePath);
//					
///				strTmep.Format("%s\\%s", m_strLogPath, strILogFile);
//				strTmep.Format("%s\\I\\%s", m_strLogPath, strILogFile);		// jsyoon 20161226 change
//				_tcscpy_s(m_szTemp, _countof(m_szTemp), strTmep);
//
//				if( m_pFtp->SendFileToFTP(m_szTemp,m_szTempFile) == 0)
//				{
//					strRemotePath.Format("MCC\\%s", strindexFile);
//					_tcscpy_s(m_szTempFile, _countof(m_szTempFile), strRemotePath);
//
///					strTmep.Format("%s\\%s", m_strLogPath, strindexFile);
//					strTmep.Format("%s\\INDEX\\%s", m_strLogPath, strindexFile);	// jsyoon 20161226 change
//					_tcscpy_s(m_szTemp, _countof(m_szTemp), strTmep);
//					m_pFtp->SendFileToFTP(m_szTemp,m_szTempFile);
//				}
//			}
//		}
//	}
//
//	if( m_pFtp != NULL)
//	{
//		m_pFtp->CloseFTPConnection();
//		delete m_pFtp;	
//		m_pFtp = NULL;
//	}

	return TRUE;
}

void CDlgnMCCReport::SetSendToFtp()
{
	if (WaitForSingleObject(m_hEvtWorkDone, 0)	== WAIT_OBJECT_0)	// jsyoon 20160211 add
	{
		m_nProcessJobNo	= WORK_MCC_SEND_LOGFILE;
		SetEvent(m_hEvtWorkStart);
	}
}

HBRUSH CDlgnMCCReport::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd == this)	return m_hbrushBK;

	return hbr;
}

void CDlgnMCCReport::OnDestroy() 
{
	CDialog::OnDestroy();

	CloseHandle(m_hEvtWorkStart);
	CloseHandle(m_hEvtWorkDone);
	CloseHandle(m_hEvtKillThread);
	CloseHandle(m_hEvtThreadKilled);

	::DeleteObject(m_hbrushBK);

	// 2023.03.07 add
	if (m_sprdMccILog->GetSafeHwnd() != NULL)
	{
		delete m_sprdMccILog;
		m_sprdMccILog = NULL;
	}
}

void CDlgnMCCReport::KillThred()
{
	SetEvent(m_hEvtKillThread);
	m_nProcessJobNo	= WORK_MCC_KILL_JOB;
	SetEvent(m_hEvtWorkStart);

	KillTimer(1);		//2017.05.24 khm
	KillTimer(2);
	KillTimer(3);
	KillTimer(4);
	KillTimer(5);
}

void CDlgnMCCReport::SetLogPath(CString str)
{
	m_strLogPath = str;

	//indexLOG();		2017.05.24 khm 메트로시스에서 파일생성을 하지않기 위해 주석처리
	//TLOG(_T(""));
	//ILOG(_T(""));
}

void CDlgnMCCReport::SetConfigPath(CString str)
{
	m_strConfigPath = str;
}

void CDlgnMCCReport::SetFilePath(CString str)
{
	m_strFilePath = str;
}

void CDlgnMCCReport::SetEQName(CString str)
{
	m_strEQName = str;
}

BOOL CDlgnMCCReport::WaitTerminateWorkerThread()
{
	{	
		if (WaitForSingleObject(m_hEvtThreadKilled, 5000)
			== WAIT_OBJECT_0)
		{
			m_pWorkThread = NULL;
		}
	}
	
	if( m_pWorkThread == NULL )
		TRACE("MCCReport NULL @@@@@@@@@@@\n");
	
	return TRUE;
}


int CDlgnMCCReport::SetMccILogHeader(void)
{
	if (m_sprdMccILog->GetSafeHwnd() == nullptr)
	{
		return 0;
	}

	m_sprdMccILog->SetMaxRows(1);
	m_sprdMccILog->SetColWidth(1, 14.0);	m_sprdMccILog->SetData(1,  0, _T("EventTime"));
	m_sprdMccILog->SetColWidth(2, 6.0);		m_sprdMccILog->SetData(2, 0, _T("LogType"));

	CFileFind finder;
	CString strMCCFile;
	strMCCFile = GetCnfPath(CUR_EQU_TYPE, _T("MCCNAME.csv"));		// jsyoon 20170529 change

	if (finder.FindFile(strMCCFile) == FALSE)
	{
		CString str ;
		str.Format(_T("Can not fined '%s'"), strMCCFile);
		AfxMessageBox(str);
		return 0;
	}
	CString strLine, strFrom, strName, strValue;
	CStringArray strArray;
	CString strTemp;
	int lenTemp = 0, iCol = 0;

//	CFile fileMCC(strMCCFile, CFile::shareDenyNone );
//	CArchive ar(&fileMCC, CArchive::load);

	CStdioFile File(strMCCFile, CFile::modeRead);

	do 
	{
		if (File.ReadString(strLine))
		{
			int iStart = 0;
			int iEnd   = 0;
			bool bToggle = TRUE;

			strArray.RemoveAll();

			lenTemp = strLine.GetLength();
			for(int i = 0 ; i < lenTemp; i++)
			{		
				if(bToggle)
				{
					iStart = i;
					bToggle = FALSE;
				}

				if(strLine.GetAt(i) == ',')
				{
					bToggle = TRUE;
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd);
					strArray.Add(strTemp);
				}
				else if( i == lenTemp-1 )
				{
					iEnd = i;
					iEnd -= iStart;

					strTemp = strLine.Mid(iStart, iEnd+1);
					strArray.Add(strTemp);
				}
			}

			if( strArray.GetSize() > 0 )
			{
				strName = strArray.GetAt(0);
				strName.TrimRight();

				if( strArray.GetSize() > 1 )
				{
					strFrom = strArray.GetAt(1);
					strFrom.TrimRight();
				}
				else
				{
					//strFrom.Format(_T("%s"), _T("OCPU"));
					strFrom.Format(_T("%s"), m_strEQName);
					// 
					//strFrom.Format("%s", m_pSystem->m_strEquipName); // TEST 
				}

				strValue.Format(_T("%s=%s"), strFrom, strName);
				m_sprdMccILog->SetMaxCols	(MCC_ILOG_COL_ITEM + iCol);
				m_sprdMccILog->SetColWidth	(MCC_ILOG_COL_ITEM + iCol, 8.0);
				m_sprdMccILog->SetData		(MCC_ILOG_COL_ITEM + iCol, 0, strValue);	
				iCol++;
			}
		}	
		else				// End file
		{
			break;
		}
	} while (TRUE);

	File.Close();
//	ar.Close();
//	fileMCC.Close();	

	return 0;
}


int CDlgnMCCReport::UpdateMCCInfo(int iRow)
{
// 	m_sprdMccILog.SetMaxRows(iRow+1);
// 
// 	CString  strValue;
// 	m_sprdMccILog.SetRow(iRow+1);
// 
// 	strValue.Format("%s", gf_MakeCurTimeString());
// 	m_sprdMccILog.SetCol(MCC_ILOG_COL_TIME);
// 	m_sprdMccILog.SetValue(strValue);
// 
// 	strValue.Format(_T("I"));
// 	m_sprdMccILog.SetCol(MCC_ILOG_COL_TYPE);
// 	m_sprdMccILog.SetValue(strValue);
// 
// 	int iCol = 0;
// 	int iAxis = 0;
// 	int iUpdata[30] = {AXIS_X,AXIS_Y,AXIS_R,AXIS_LOADER_X, AXIS_LOADER_Y,AXIS_LOADER_Z,AXIS_LP,AXIS_LOADER_R,AXIS_X2,AXIS_Y1,AXIS_Y2,AXIS_Z,AXIS_Z3,AXIS_ZOOM1,AXIS_ZOOM2,AXIS_THETA1,AXIS_THETA2,AXIS_Z2,AXIS_Z5,AXIS_FOCUS1,AXIS_FOCUS2,AXIS_ROTATE1,AXIS_ROTATE2,AXIS_Z1,AXIS_Z4,AXIS_Y3,AXIS_Y4,AXIS_Y5,AXIS_Y6, AXIS_UNIT_Z};
// 	for (int iLoop = 0; iLoop<30; iLoop++)
// 	{
// 		iAxis = iUpdata[iLoop];
// 
// 		if(iAxis == AXIS_X1 || iAxis == AXIS_X3 || iAxis ==  AXIS_LP2 || iAxis ==  AXIS_Z9 || iAxis == AXIS_FILTER1 || iAxis == AXIS_FILTER2 || iAxis ==  AXIS_X4 || iAxis == AXIS_X5 || iAxis ==  AXIS_Z9
// 			|| iAxis == AXIS_Y7 || iAxis == AXIS_Y8 || iAxis == AXIS_R1 || iAxis == AXIS_R2 || iAxis == AXIS_Z6 || iAxis == AXIS_Z7 || iAxis == AXIS_UNIT_Z2)
// 			continue;
// 
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.stMoterInfo[iAxis].dSetAxisSpeed);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM+iCol);
// 		m_sprdMccILog.SetValue(strValue);
// 		iCol++;
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.stMoterInfo[iAxis].dSetAxisPos);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM+iCol);
// 		m_sprdMccILog.SetValue(strValue);
// 		iCol++;
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.stMoterInfo[iAxis].dAxisSpeed);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM+iCol);
// 		m_sprdMccILog.SetValue(strValue);
// 		iCol++;
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.stMoterInfo[iAxis].dAxisPos);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM+iCol);
// 		m_sprdMccILog.SetValue(strValue);
// 		iCol++;
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.stMoterInfo[iAxis].dAxisRMS);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM+iCol);
// 		m_sprdMccILog.SetValue(strValue);
// 		iCol++;
// 	}
// 
// 	int iTem = 0;
// 	for (iTem = 0; iTem<MCC_ILOG_COL_ITEM2_E-MCC_ILOG_COL_ITEM2; iTem++)
// 	{
// 		strValue.Format("%.3f", m_pSystem->m_stMccData.dData[iTem]);
// 		m_sprdMccILog.SetCol(MCC_ILOG_COL_ITEM2+iTem);
// 		m_sprdMccILog.SetValue(strValue);
// 	}

	return 0;
}

int CDlgnMCCReport::UpdateMCCTLog(void)
{
	//MCCICPDATA stMCCData;
	//ZeroMemory(&stMCCData, sizeof(stMCCData));
	//
	//do 
	//{
	//	if(m_queue.DeleteQueue(&stMCCData) == 1)
	//	{
	//		int EvtFlag = 0, iEvn = 0, nStep = 0, nFromto;
	//		BOOL bStart = FALSE;
	//		CString str;
	//		EvtFlag = stMCCData.iEvtFlag;
	//		iEvn = stMCCData.iSpqNum[EvtFlag];
	//		bStart = stMCCData.bFlagStoE[EvtFlag];
	//		nStep = stMCCData.iStepNum[EvtFlag];
	//		nFromto = stMCCData.iFromto[EvtFlag];
	//		str.Format("%s", stMCCData.m_szEvtTime);
	//		TRACE("Queue %d, %d, %d \n", EvtFlag, iEvn, bStart);
	//
	//		switch (EvtFlag)
	//		{
	//		case 0:
	////			WriteMCCAction(str,iEvn, bStart);
	//			WriteMCCAction(str,iEvn, bStart, nStep, nFromto);
	//			break;
	//		case 1:
	//			WriteMCCSignal(str,iEvn, bStart);
	//			break;
	//		case 2:
	//			WriteMCCError(str,iEvn, bStart);
	//			break;
	//		case 3:
	//			WriteMCCWarring(str,iEvn, bStart);
	//			break;
	//		default:
	//			break;
	//		}
	//	}
	//
	//} while (m_queue.IsEmpty() != true);


 //	int EvtFlag = 0;
 //	EvtFlag = m_pMCCMap->m_stMccData.iEvtFlag;
 //	if( EvtFlag != -1)
 //	{
 //		int iEvn = m_pSystem->m_stMccData.iSpqNum[EvtFlag];
 //		BOOL bStart = m_pSystem->m_stMccData.bFlagStoE[EvtFlag];		
 //		switch (EvtFlag)
 //		{
 //		case 0:
 //			WriteMCCAction(iEvn, bStart);
 //			break;
 //		case 1:
 //			WriteMCCSignal(iEvn, bStart);
 //			break;
 //		case 2:
 //			WriteMCCError(iEvn, bStart);
 //			break;
 //		case 3:
 //			WriteMCCWarring(iEvn, bStart);
 //			break;
 //		default:
 //			break;
 //		}
 //
 //		m_pSystem->m_stMccData.iEvtFlag = -1;
 //	}	

	return 0;
}

//// 객체 생성자
//MyQueue::MyQueue()
//{
//	// 맴버 변수를 초기화
//	m_rear = 0;
//	m_front = 0;
//}
//
//// 큐가 Full 인 경우 1 을 반환하는 멤버함수
//// 아닌 경우 0 을 반환
//bool MyQueue::IsFull()
//{
//	if ((m_rear + 1) % MAX_QUEUE_COUNT == m_front) 
//	{
//		return true; // 큐가 Full 인 경우
//	}
//	else 
//	{
//		return false; // Full 이 아닌 경우
//	}
//}
//
//// 큐가 빈 경우 1을 반환하는 함수
//// 큐에 자료가 존재하는 경우 0 을 반환
//bool MyQueue::IsEmpty()
//{
//	if (m_rear == m_front) 
//	{
//		return true; // Empty 인 경우
//	}
//	// 큐에 자료가 존재하는 경우
//	else 
//	{
//		return false;
//	}
//}
//
//// 큐에 parm_temp 자료를 저장하는 멤버함수
//int MyQueue::AddQueue(MCC_EQDATA stData)
//{
//	if (false == IsFull()){
//		// Rear 에 다음 인덱스의 값을 저장
//		// 0 ~ 100 까지 증가한 후  100 에서 다시 0 으로 초기화
//		m_rear = (m_rear + 1) % MAX_QUEUE_COUNT;
//
//		// 큐의 마지막에 데이터를 추가
//		m_queue_data[m_rear] = stData;
//
//		return 1;
//	}
//	// 값을 저장하지 못한 경우
//	return 0;
//}
//
//// 큐에 자료를 삭제하는 멤버함수
//// 인자로 넘어온 parm_temp 의 주소에 처리된 자료를 저장
//int MyQueue::DeleteQueue(PMCC_EQDATA pstData)
//{
//	if (false == IsEmpty()){
//		// Front 값에 다음 인덱스를 저장 
//		// 0 ~ 100 까지 증가한 후  100 에서 다시 0 으로 초기화
//		m_front = (m_front + 1) % MAX_QUEUE_COUNT;
//
//		// 큐의 선두에 있는 자료를 parm_p_value 에 저장
//		*pstData = m_queue_data[m_front];
//
//		memset (&m_queue_data[m_front], 0x00, sizeof(MCC_EQDATA));
//		return 1;
//	}
//	// 큐가 Empty 인 경우 0 을 반환
//	return 0;
//}



int CDlgnMCCReport::AddMccQueue(int iFlag)
{
	MCC_EQDATA_T stMCCData;
	ZeroMemory(&stMCCData, sizeof(stMCCData));
	stMCCData = m_pMCCMap->stMCCDataEQ; // m_pSystem->m_stMccData;
	_tcscpy_s(stMCCData.m_szEvtTime, _countof(stMCCData.m_szEvtTime), gf_MakeCurTimeString());

	if( m_hTarget != NULL)
	{
//		m_hWndMccLog = ::FindWindow(NULL, "MCCLog");
		DWORD	dwResult;
		SendMessageTimeout(m_hTarget, WM_MCC_ADDQUEUE, NULL, iFlag, SMTO_NORMAL, 200, (PDWORD_PTR)&dwResult);
	}
	
	AddQueue(stMCCData);
	return 0;
}


int CDlgnMCCReport::UpdateMCCILOG(void)
{
 	CString strValue, strILog;
 	TCHAR szTmp[10000];
 	TCHAR szCmd[10000];
 	szTmp[0] = 0x00;
 	szCmd[0] = 0x00;
 
 
 	strValue.Format(_T("%s,"), gf_MakeCurTimeString());
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 
 	strValue.Format(_T("I,"));
 	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 		
 	int iCol = 0;
 	int iAxis = 0;
 	int iLoop = 0;
 //	int iUpdata[30] = {AXIS_X,AXIS_Y,AXIS_R,AXIS_LOADER_X, AXIS_LOADER_Y,AXIS_LOADER_Z,AXIS_LP,AXIS_LOADER_R,AXIS_X2,AXIS_Y1,AXIS_Y2,AXIS_Z,AXIS_Z3,AXIS_ZOOM1,AXIS_ZOOM2,AXIS_THETA1,AXIS_THETA2,AXIS_Z2,AXIS_Z5,AXIS_FOCUS1,AXIS_FOCUS2,AXIS_ROTATE1,AXIS_ROTATE2,AXIS_Z1,AXIS_Z4,AXIS_Y3,AXIS_Y4,AXIS_Y5,AXIS_Y6, AXIS_UNIT_Z};
 //	for (iLoop = 0; iLoop<30; iLoop++)
	for (iLoop = 0; iLoop < 90; iLoop++)
 	{
		iAxis = iLoop;	// LYS 2026-07-31 was missing (GMI parity) - loop displayed axis 0 repeatedly
		if (m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].bUse != TRUE)
			continue;

 //		if(iAxis == AXIS_X1 || iAxis ==  AXIS_LP2 || iAxis ==  AXIS_Z9 || iAxis == AXIS_FILTER1 || iAxis == AXIS_FILTER2 || iAxis ==  AXIS_X4 || iAxis == AXIS_X5 || iAxis ==  AXIS_Z9
 //			|| iAxis == AXIS_Y7 || iAxis == AXIS_Y8 || iAxis == AXIS_R1 || iAxis == AXIS_R2 || iAxis == AXIS_Z6 || iAxis == AXIS_Z7 || iAxis == AXIS_UNIT_Z2)
 //			continue;
 
 		strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].dSetAxisSpeed);
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 
 		
 		strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].dSetAxisPos);
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 
 		
 		strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisSpeed);
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 
 		strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisPos);
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 
 		strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisRMS);
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 	}
 
 	for (iLoop = 0; iLoop<MCC_ILOG_COL_ITEM2_E-MCC_ILOG_COL_ITEM2; iLoop++)
 	{
 		if( iLoop + 1 == MCC_ILOG_COL_ITEM2_E-MCC_ILOG_COL_ITEM2)
 			strValue.Format(_T("%.3f"), m_pMCCMap->stMCCDataEQ_ILog.dData[iLoop]);
 		else
 			strValue.Format(_T("%.3f,"), m_pMCCMap->stMCCDataEQ_ILog.dData[iLoop]);
 
 		_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strValue);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
 	}
 
 //	int n = strlen(szCmd);				
	int n = _tcsclen(szCmd);
 	szCmd[n] = 0;			
 
 	strILog.Format(_T("%s"), szCmd);
 	ILOG(strILog);

	return 0;
}


void CDlgnMCCReport::OnBnClickedCheckT()
{
	UpdateData(FALSE);

	//m_pSystem->m_bMccTransferOn = (BOOL)m_chkUseSend.GetCheck();
}

BOOL CDlgnMCCReport::FindTargetWindow(CString sWndName)
{
	m_hTarget = ::FindWindow((LPCTSTR)NULL, (LPCTSTR)sWndName);
	if(!m_hTarget) return FALSE;
	return TRUE;
}

bool CDlgnMCCReport::CreateMemoryMap()
{
	if (m_hMCCMapFile == NULL && m_pMCCMap == NULL)
	{	
		m_hMCCMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof( VTM_MCC ), VTM_SHARE_MEM_STRING_MCC);

		if(!m_hMCCMapFile)
		{
			AfxMessageBox(_T("CreateFileMapping Failed (VTM_SHARE_MEM_STRING_MCC)"));
			return false;
		}

		m_pMCCMap = (VTM_MCC *) MapViewOfFile(m_hMCCMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof( VTM_MCC ));
		if( m_pMCCMap == NULL)
		{
			AfxMessageBox(_T("MapViewOfFile Failed (VTM_SHARE_MEM_STRING_MCC)"));
			return false;
		}

		Clear();
	}

	return true;
}


void CDlgnMCCReport::Clear()
{
	m_pMCCMap->m_front = 0;
	m_pMCCMap->m_rear = 0;
}

// 큐가 Full 인 경우 1 을 반환하는 멤버함수
// 아닌 경우 0 을 반환
bool CDlgnMCCReport::IsFull()
{
	if ((m_pMCCMap->m_rear + 1) % MAX_QUEUE_COUNT == m_pMCCMap->m_front)
	{
		return true; // 큐가 Full 인 경우
	}
	else
	{
		return false; // Full 이 아닌 경우
	}
}

// 큐가 빈 경우 1을 반환하는 함수
// 큐에 자료가 존재하는 경우 0 을 반환
bool CDlgnMCCReport::IsEmpty()
{
	if (m_pMCCMap->m_rear == m_pMCCMap->m_front)
	{
		return true; // Empty 인 경우
	}
	// 큐에 자료가 존재하는 경우
	else
	{
		return false;
	}
}

// 큐에 parm_temp 자료를 저장하는 멤버함수
int CDlgnMCCReport::AddQueue(MCC_EQDATA_T stData)
{
	int nRear = 0;
	if (false == IsFull()) {
		// Rear 에 다음 인덱스의 값을 저장
		// 0 ~ 100 까지 증가한 후  100 에서 다시 0 으로 초기화
		m_pMCCMap->m_rear = (m_pMCCMap->m_rear + 1) % MAX_QUEUE_COUNT;

		nRear = m_pMCCMap->m_rear;

		// 큐의 마지막에 데이터를 추가
		m_pMCCMap->m_queue_data[nRear] = stData;

		return 1;
	}
	// 값을 저장하지 못한 경우
	return 0;
}

// 큐에 자료를 삭제하는 멤버함수
// 인자로 넘어온 parm_temp 의 주소에 처리된 자료를 저장
int CDlgnMCCReport::DeleteQueue(PMCC_EQDATA_T pstData)
{
	int nfront = 0;
	if (false == IsEmpty()) {
		// Front 값에 다음 인덱스를 저장 
		// 0 ~ 100 까지 증가한 후  100 에서 다시 0 으로 초기화
		m_pMCCMap->m_front = (m_pMCCMap->m_front + 1) % MAX_QUEUE_COUNT;
		nfront = m_pMCCMap->m_front;

		// 큐의 선두에 있는 자료를 parm_p_value 에 저장
		*pstData = m_pMCCMap->m_queue_data[nfront];

		memset(&m_pMCCMap->m_queue_data[nfront], 0x00, sizeof(MCC_EQDATA_T));	// LYS 2026-07-31 was sizeof(PMCC_EQDATA_T)=pointer size
		return 1;
	}
	// 큐가 Empty 인 경우 0 을 반환
	return 0;
}