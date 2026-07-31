#if !defined(AFX_SOCKETCTRLMANAGER_H__63368804_CD1B_4678_BB5E_E27041FE0566__INCLUDED_)
#define AFX_SOCKETCTRLMANAGER_H__63368804_CD1B_4678_BB5E_E27041FE0566__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SocketCtrlManager.h : header file
//

#include "TcpClient.h"
#include "NetWallTalk.h"

/////////////////////////////////////////////////////////////////////////////
// CSocketCtrlManager window

class CSocketCtrlManager : public CWnd
{
// Construction
public:
	CSocketCtrlManager();

// Attributes
public:

	CWnd*	pParent;
	CWnd*	m_wndParent; 
	BOOL	m_bCreated;	
	CTcpClient*	m_pClient;
	CNetWallTalk* m_pNetWallTalk;
	
// Operations
public:
	BOOL Create(CWnd *pParentWnd);
	BOOL Initialize(CWnd *pParentWnd,UINT nServer, UINT nPort, CString szIPAddress);
	BOOL Close();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSocketCtrlManager)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSocketCtrlManager();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSocketCtrlManager)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// 	int			StartThread(void);
// 	static UINT ThreadProc(LPVOID pParam);
	void		EndThread(void);

private:
	CWinThread* m_pThread;
	bool m_bFlagThreadStop;
public:
//	int ConnectToServer(void);
//	CString m_szIPAddress;
//	int m_nSocketPort;
	BOOL IsConnected(void);
	int SendData(BYTE *pSendData, UINT nDataSize);
	int GetReceiveData(BYTE* pData, UINT nBufSize);  
	int m_nSocketType;
	UINT GetReceiveDataSize(void);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOCKETCTRLMANAGER_H__63368804_CD1B_4678_BB5E_E27041FE0566__INCLUDED_)
