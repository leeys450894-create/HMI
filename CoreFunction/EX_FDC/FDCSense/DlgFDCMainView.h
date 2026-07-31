#pragma once

#include "FPSpreadsheet/spreadsheet.h"
//#include "..\\Include\API_MXComponent.h"
#include "..\\Include\API_SocketControl.h"

// CDlgFDCMainView 대화 상자입니다.

class CDlgFDCMainView : public CDialog
{
	DECLARE_DYNAMIC(CDlgFDCMainView)

public:
	CDlgFDCMainView(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgFDCMainView();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_FDC_MAINUI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	void ConnectSock();
	void GetReceiveData();
	void DisConnect();
	void SetMainPoint(void* pvoid);
	void CreateSocket(SOCKET_OPEN stSockOpen);
	void GetConfig();
//	LRESULT	OnSocketMsgReceive(WPARAM wParam, LPARAM lParam);
	SOCKET_OPEN		m_stSockOpen;
	UINT			m_nSocketIndex;

	int				m_nPlcPort;
	CString			m_szIPAddress;
	CFile			m_fileCommLog;
	BOOL			m_bOutOfService;



	void	EventLog(CString &s);
	BOOL	gf_MakeDir(CString sPath);
	CString gf_MakeCurTimeString();
	CString gf_MakeCurTimeString2();
	int		WriteFile(CFile *pFile, CString &sPath, CString sLog);
	CString GetLogFileName();
	void	AnalysisReceiveData(UINT nSize, BYTE *pByte);

	CString m_strAppPath;
	void	SetAppPath(CString str) { m_strAppPath = str; };

	afx_msg void OnBnClickedBtnConnetClient();
//	afx_msg void OnStnClickedStaticLivecheck();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	CListBox m_lbCimLog;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnMsgAccept(WPARAM wParam, LPARAM lParam);          // 소켓 연결(서버모드일경우 onAccept수신시)
	afx_msg LRESULT OnMsgClose(WPARAM wParam, LPARAM lParam);	        // 소켓 해제(서버, 클라이언트 모드일 경우 onClose수신시)
	afx_msg LRESULT OnMsgReceive(WPARAM wParam, LPARAM lParam);	        // 데이터 수신시(서버, 클라이언트 모드일 경우 onReceive수신시)
};
