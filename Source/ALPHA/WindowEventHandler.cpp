//#include "stdafx.h"
#include "pch.h"
#include "WindowEventHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CWindowEventHandler *CWindowEventHandler::m_pInstance = nullptr;
CWindowEventHandler *CWindowEventHandler::GetInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new CWindowEventHandler);
}
void CWindowEventHandler::DelInstance()
{
    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

CWindowEventHandler::CWindowEventHandler(void)
{
	for(int i = 0; i < enumWndIDCount; ++i)
	{
		m_Spec_MsgHandleWnd[i] = nullptr;
	}

	for(int i = 0; i < enumSeqItemIDCount; ++i)
	{
		m_Spec_MsgHandleSeqItem[i] = nullptr;
	}
}

CWindowEventHandler::~CWindowEventHandler(void)
{
}

void CWindowEventHandler::SetMessageHandle(enumWndID id, HWND hWnd)
{
	m_Spec_MsgHandleWnd[id] = hWnd;
}
LRESULT CWindowEventHandler::SendMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam)
{
#ifdef WIN64
	UINT64 dwResult = 0;
	return ::SendMessageTimeout(m_Spec_MsgHandleWnd[id], idMessage, wParam, lParam, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
#else
	ULONG dwResult = 0;
	return ::SendMessageTimeout(m_Spec_MsgHandleWnd[id], idMessage, wParam, lParam, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
#endif
}
BOOL CWindowEventHandler::PostMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam)
{
	return PostMessage(m_Spec_MsgHandleWnd[id], idMessage, wParam, lParam);
}


void CWindowEventHandler::SetMessageHandle(enumSeqItemID id, HWND hWnd)
{
	m_Spec_MsgHandleSeqItem[id] = hWnd;
}
LRESULT CWindowEventHandler::SendMessageToUI(enumSeqItemID id, UINT idMessage, WPARAM wParam, LPARAM lParam)
{
#ifdef WIN64
	UINT64 dwResult = 0;
	return ::SendMessageTimeout(m_Spec_MsgHandleSeqItem[id], idMessage, wParam, lParam, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
#else
	ULONG dwResult = 0;
	return ::SendMessageTimeout(m_Spec_MsgHandleSeqItem[id], idMessage, wParam, lParam, SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, &dwResult);
#endif

}
BOOL CWindowEventHandler::PostMessageToUI(enumSeqItemID id, UINT idMessage, WPARAM wParam, LPARAM lParam)
{
	return PostMessage(m_Spec_MsgHandleSeqItem[id], idMessage, wParam, lParam);
}

CWnd* CWindowEventHandler::GetHandleWnd(enumWndID id)
{
	HWND hParent = m_Spec_MsgHandleWnd[id];
	CWnd * p_temp_wnd = CWnd::FromHandle(hParent);

	return p_temp_wnd;
}
