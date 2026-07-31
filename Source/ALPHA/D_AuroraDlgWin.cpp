// D_AuroraDlgWin.cpp : implementation file
//

#include "pch.h"
#include "ALPHA.h"
#include "D_AuroraDlgWin.h"
#include "afxdialogex.h"
#include "common\Winmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_AuroraDlgWin dialog

IMPLEMENT_DYNAMIC(CD_AuroraDlgWin, CDialogEx)

CD_AuroraDlgWin::CD_AuroraDlgWin(UINT idTemplate, CWnd *pParent)
	: CDialogEx(idTemplate, pParent)
	, m_idTemplate(idTemplate)
	, m_pParent(pParent)
{

}

CD_AuroraDlgWin::~CD_AuroraDlgWin()
{
}

BEGIN_MESSAGE_MAP(CD_AuroraDlgWin, CDialogEx)
	ON_REGISTERED_MESSAGE(WM_WINMGR, OnWinMgr)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CD_AuroraDlgWin message handlers
BOOL CD_AuroraDlgWin::Create(UINT id, BOOL bShow)
{
	BOOL ret = CDialogEx::Create(m_idTemplate, m_pParent);

	if (ret)
	{
		::SetWindowLong(GetSafeHwnd(), GWL_ID, id);

		CRect rcWnd;
		GetClientRect(rcWnd);

		m_szMinimum = rcWnd.Size();

		if (bShow)
		{
			ShowWindow(SW_SHOW);
		}
	}

	return ret;
}

void CD_AuroraDlgWin::ControlButtonEnable(BOOL bEnable)
{

}

void CD_AuroraDlgWin::PostNcDestroy(void)
{
	DestroyWindow();

	delete this;

	CDialogEx::PostNcDestroy();
}

BOOL CD_AuroraDlgWin::PreTranslateMessage(MSG *pMsg)
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
			case VK_LEFT:
			case VK_UP:
			case VK_RIGHT:
			case VK_DOWN:
                return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CD_AuroraDlgWin::OnWinMgr(WPARAM wp, LPARAM lp)
{
	ASSERT(lp);

	NMWINMGR &nmw = *(NMWINMGR *)lp;

	if (nmw.code == NMWINMGR::GET_SIZEINFO)
	{
		nmw.sizeinfo.szMin = m_szMinimum;

		return TRUE;
	}

	return 0;
}

void CD_AuroraDlgWin::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	ShowWindow(SW_HIDE);
	EnableWindow(FALSE);
}
