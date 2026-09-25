// DlgSubLogFile.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubLogFile.h"
#include "afxdialogex.h"

#include "LogFunction\LogDefine.h"
#include "LogFunction\LogAccessor\file_access\FileAccessor.h"	// LYS 2026-09-11 화면 로그 파일 저장

// CDlgSubLogFile 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubLogFile, CDialogEx)

CDlgSubLogFile::CDlgSubLogFile(CString strTitle, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_SUB_LOG_FILE, pParent)
	, m_strTitle(strTitle)
	, m_pLogFile(nullptr)
	, m_pSaveAccessor(nullptr)
{
	//----------------------------------------------------------
	// FONT 만들기
	//----------------------------------------------------------
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 13;
	_tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Arial"));
	VERIFY(m_logFont.CreateFontIndirect(&lf));



}

CDlgSubLogFile::~CDlgSubLogFile()
{
	m_logFont.DeleteObject();

	// LYS 2026-09-11 저장 파일 닫기
	if (m_pSaveAccessor != nullptr)
	{
		m_pSaveAccessor->CloseStream();
		delete m_pSaveAccessor;
		m_pSaveAccessor = nullptr;
	}
}

void CDlgSubLogFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOG_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}


BEGIN_MESSAGE_MAP(CDlgSubLogFile, CDialogEx)
	ON_MESSAGE(WM_GUILOG, &CDlgSubLogFile::OnGuiLog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CDlgSubLogFile 메시지 처리기입니다.
LRESULT CDlgSubLogFile::OnGuiLog(WPARAM wp, LPARAM lp)
{
	const CString strLog = (LPCTSTR)wp;

	// 100 개 이상은 담아두지 말자. - 속도가 느려진데..
	if (100 <= m_listLog.GetCount())
	{
		m_listLog.DeleteString(0);
	}

	m_listLog.AddString(strLog);
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);

	// LYS 2026-09-11 화면 리스트는 100줄만 남기고 지우므로, 표시한 줄을 그대로 파일에도 남긴다.
	__SaveToFile(strLog);

	return 0;
}

// LYS 2026-09-11 이 창에 표시되는 줄을 파일로도 남긴다.
//   strFilePath 는 "...\\LogFile\\DLOG.log" 처럼 주고, CLogFileAccessor 와 같은 규칙으로
//   ".log" 를 "_yyyymmdd.log" 로 바꿔 연다. 날짜가 바뀌면 __SaveToFile 에서 다시 연다.
void CDlgSubLogFile::EnableFileSave(CString strFilePath)
{
	if (strFilePath.IsEmpty() == TRUE)
		return;

	strFilePath.Replace(_T("\\\\"), _T("\\"));
	m_strSaveBase = strFilePath;
	m_strSaveDate = _T("");

	if (m_pSaveAccessor == nullptr)
		m_pSaveAccessor = new CFileAccessor;
}

// LYS 2026-09-11 저장 대상 창이 아니면(EnableFileSave 미호출) 아무것도 하지 않는다.
void CDlgSubLogFile::__SaveToFile(const CString &strLine)
{
	if (m_pSaveAccessor == nullptr || m_strSaveBase.IsEmpty() == TRUE)
		return;

	SYSTEMTIME st;
	::GetLocalTime(&st);

	CString strDate;
	strDate.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);

	if (m_strSaveDate != strDate)
	{
		// 최초 1회 + 날짜 변경 시 재오픈 (CLogFileAccessor::Initialize 와 같은 방식)
		m_pSaveAccessor->CloseStream();

		CString strTemp = m_strSaveBase;
		strTemp.Replace(_T(".log"), _T("_") + strDate + _T(".log"));

		if (m_pSaveAccessor->Open_Stream(strTemp) == false)
			return;

		m_strSaveDate = strDate;
	}

	m_pSaveAccessor->WriteStream(strLine);
}

void CDlgSubLogFile::LinkParam(CLogFileAccessor *log)
{
	if (log != nullptr)
	{
		log->SetLogMsgWindow(GetSafeHwnd());
		m_pLogFile = log;
	}
}

void CDlgSubLogFile::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialogEx::OnShowWindow(bShow, nStatus);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (bShow)
	{
		m_LabelInfo.SetText(m_strTitle);
	}
}


BOOL CDlgSubLogFile::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		// To prevent to close a dialog.
		switch (pMsg->wParam)
		{
		case VK_ESCAPE:
		case VK_RETURN:
		case VK_SPACE:
			return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL CDlgSubLogFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_listLog.SetFont(&m_logFont);

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
