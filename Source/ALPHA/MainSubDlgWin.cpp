// MainSubDlgWin.cpp : implementation file
//

#include "pch.h"
#include "ALPHA.h"
#include "MainSubDlgWin.h"
#include "afxdialogex.h"
#include "common\WinMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainSubDlgWin dialog

IMPLEMENT_DYNAMIC(CMainSubDlgWin, CDialogEx)

CMainSubDlgWin::CMainSubDlgWin(UINT idTemplate, CWnd *pParent)
	: CDialogEx(idTemplate, pParent)
	, m_idTemplate(idTemplate)
	, m_pParent(pParent)
{

}

CMainSubDlgWin::~CMainSubDlgWin()
{
}

BEGIN_MESSAGE_MAP(CMainSubDlgWin, CDialogEx)
	ON_REGISTERED_MESSAGE(WM_WINMGR, OnWinMgr)
END_MESSAGE_MAP()


// CMainSubDlgWin message handlers
BOOL CMainSubDlgWin::Create(UINT id, CWnd *pParent, BOOL bShow)
{
	m_pParent = pParent;

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

void CMainSubDlgWin::UpdateOperateStatus(CString str)
{

}

void CMainSubDlgWin::ShowReportAlarm()
{

}

void CMainSubDlgWin::UpdateSeqItemElapsedTime()
{
}

void CMainSubDlgWin::UpdateMeasureElapsedTime(bool bStart)
{
}

void CMainSubDlgWin::UpdateCycleInspectionStart()
{
}

void CMainSubDlgWin::UpdateCycleInspectionResult()
{
}

void CMainSubDlgWin::UpdateCycleInspectionComplete()
{
}

void CMainSubDlgWin::UpdateChangeInitialize()
{
}

void CMainSubDlgWin::UpdateRecipeInformation()
{
}

void CMainSubDlgWin::UpdateInformation()
{
}

void CMainSubDlgWin::UpdateDataLampTime(int iLamp, bool bOn)
{

}

void CMainSubDlgWin::UpdateWriteLampTime(int iLamp, bool bOn)
{

}

void CMainSubDlgWin::UpdateMeasColor()
{

}


void CMainSubDlgWin::RefreshListData()
{

}


BOOL CMainSubDlgWin::PreTranslateMessage(MSG *pMsg)
{
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

LRESULT CMainSubDlgWin::OnWinMgr(WPARAM wp, LPARAM lp)
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


void CMainSubDlgWin::UpdateCIMState(int iState)
{

}

void CMainSubDlgWin::UpdateControlMode(int iState)
{

}

void CMainSubDlgWin::UpdateAgingMode(bool bStart, int nTime)
{

}

void CMainSubDlgWin::UpdateSACCMode(bool bStart)
{

}

void CMainSubDlgWin::UpdateCIMAlarm(int iAlarm)
{

}