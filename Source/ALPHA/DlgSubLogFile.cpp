// DlgSubLogFile.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubLogFile.h"
#include "afxdialogex.h"

#include "LogFunction\LogDefine.h"

// CDlgSubLogFile 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubLogFile, CDialogEx)

CDlgSubLogFile::CDlgSubLogFile(CString strTitle, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_SUB_LOG_FILE, pParent)
	, m_strTitle(strTitle)
	, m_pLogFile(nullptr)
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

	return 0;
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
