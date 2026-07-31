//API_SocketControl.cpp

#include "stdafx.h"
#include "SocketControl.h"

#include "SocketCtrlManager.h"


//=====================================================================
// Export되는 모든 함수 코드에 AFX_MANAGE_STATE 매크로 반드시 기입할 것
// 쓰레드간에 보호를 받을 수 있다.
// Export되지 않는 함순즌 AFX_MANAGE_STATE를 쓸 필요 없다
// Export되는 함수는 반드시 모듈 정의 함수 이름 추가해야 .def 파일 참조
// 
// Created by Jay Hong : 2009.04.28
//=====================================================================

#define dfMAX_INDEX  30		// 최대 Socket 30개까지 설정	// jsyoon 20160406 change 24 to 30

//=====================================================================
// g_SocketCtrlManager가 Global로 선언 되어 있기 때문에 배열 형태로 생성
// 상위단에서는 nIdex를 이용하여 g_SocketCtrlManager[nIndex] 사용가능
// 
// Modified by Jay Hong : 2009.05.01 (근로자의 날에도 출근해서 일한다!!!!)
//=====================================================================
CSocketCtrlManager g_SocketCtrlManager[dfMAX_INDEX];

BOOL WINAPI api_SocketCtrl_Initialize(UINT nIdex, CWnd *pParentWnd, int nServer, UINT nPort, TCHAR* szIPAddress /*CString szIPAddress = "127.0.0.1"*/)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(nIdex >= dfMAX_INDEX || nIdex<0)
	{
		AfxMessageBox("(Socket Manager) Out of range... ");
		return FALSE;
	}

	CString strIP;
	strIP.Format("%s", szIPAddress);

	if(!g_SocketCtrlManager[nIdex].m_bCreated)
	{
		g_SocketCtrlManager[nIdex].Create(pParentWnd);

		return g_SocketCtrlManager[nIdex].Initialize(pParentWnd, nServer, nPort, strIP);
	}
	else
	{
		return g_SocketCtrlManager[nIdex].Initialize(pParentWnd, nServer, nPort, strIP);
	}

	AfxMessageBox("(Socket Manager) Create Fail... ");

	return FALSE;
}

BOOL	WINAPI	api_SocketCtrl_Close(UINT nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].Close();
	}

	return FALSE;
}

void WINAPI api_SocketCtrl_GetLocalIP(UINT nIdex, TCHAR* szIPAddress)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strIp;

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		strIp.Format("%s", g_SocketCtrlManager[nIdex].m_pNetWallTalk->GetLocalIP());
	}
	else
	{
		strIp.Format("%s", "127.0.0.1");
	}

	int iLength = 0;
	iLength = strIp.GetLength();
	memcpy(szIPAddress, strIp, sizeof(char)*iLength);

//	return "127.0.0.1";
}

void WINAPI api_SocketCtrl_GetServerIPAddress(UINT nIdex, TCHAR* szIPAddress)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strIp;

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		if( g_SocketCtrlManager[nIdex].m_pNetWallTalk != NULL )
			strIp.Format("%s", g_SocketCtrlManager[nIdex].m_pNetWallTalk->GetServerIPAddress());
		else
			strIp.Format("%s", g_SocketCtrlManager[nIdex].m_pClient->GetServerIPAddress());
	}
	else
	{
		strIp.Format("%s", "127.0.0.1");
	}

	int iLength = 0;
	iLength = strIp.GetLength();
	memcpy(szIPAddress, strIp, sizeof(char)*iLength);

//	return "127.0.0.1";
}

void WINAPI api_SocketCtrl_GetClientIPAddress(UINT nIdex, TCHAR* szIPAddress)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strIp;

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		if( g_SocketCtrlManager[nIdex].m_pNetWallTalk != NULL )
			strIp.Format("%s", g_SocketCtrlManager[nIdex].m_pNetWallTalk->GetClientIPAddress());
		else
			strIp.Format("%s", g_SocketCtrlManager[nIdex].m_pClient->GetClientIPAddress());


	}
	else
	{
		strIp.Format("%s", "127.0.0.1");
	}
	
	int iLength = 0;
	iLength = strIp.GetLength();
	memcpy(szIPAddress, strIp, sizeof(char)*iLength);

//	return "127.0.0.1";
}

int WINAPI api_SocketCtrl_SendData(UINT nIdex, BYTE *pSendData, UINT nDataSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].SendData(pSendData, nDataSize);
	}

	return -1;
}

int	WINAPI	api_SocketCtrl_GetReceiveData(UINT nIdex, BYTE *pData, UINT nBufSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].GetReceiveData(pData, nBufSize);
	}

	return 0;
}

UINT WINAPI  api_SocketCtrl_GetClientPort(UINT nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].m_pNetWallTalk->GetClientPort();
	}
	
	return 0;
}

UINT WINAPI	api_SocketCtrl_GetReceiveDataSize(UINT nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].GetReceiveDataSize();
	}

	return 0;
}

UINT WINAPI	api_SocketCtrl_GetServerPort(UINT nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].m_pNetWallTalk->GetServerPort();
	}

	return 0;
}


BOOL WINAPI	api_SocketCtrl_IsConnected(UINT nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{

		return g_SocketCtrlManager[nIdex].IsConnected();
	}

	return FALSE;
}


BOOL WINAPI	api_SocketCtrl_SetModeChange(UINT nIdex, BOOL bIsServer)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].m_pNetWallTalk->SetModeChange(bIsServer);
	}

	return FALSE;
}


BOOL WINAPI	api_SocketCtrl_SetServerIPAddress(UINT nIdex, CString sIPAddress)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].m_pNetWallTalk->SetServerIPAddress(sIPAddress);
	}

	return FALSE;
}

BOOL WINAPI	api_SocketCtrl_SetServerPort(UINT nIdex, UINT nPort)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_SocketCtrlManager[nIdex].m_bCreated)
	{
		return g_SocketCtrlManager[nIdex].m_pNetWallTalk->SetServerPort(nPort);
	}

	return FALSE;
}

