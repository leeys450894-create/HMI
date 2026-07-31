#pragma once
#include "..\Include\API_SocketControl.h"

// CDlgPLCDRead 대화 상자입니다.

class CDlgPLCDRead : public CDialog
{
	DECLARE_DYNAMIC(CDlgPLCDRead)

public:
	CDlgPLCDRead(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgPLCDRead();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_DREAD_PLC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	BOOL m_bReceiveOut;
	void CreateSocket(SOCKET_OPEN stSockOpen);
	void CloseSocket();

	int				m_nMode;
	int				m_nStationNum;
	BOOL			m_bOutOfService;
	SOCKET_OPEN		m_stSockOpen;
	UINT			m_nSocketIndex;
	CString			m_szIPAddress;
	int				m_nPlcPort;
	CFile			m_fileCommLog;
	CString			m_strAppPath;

	double	m_dPLCReadData[300];
	int  Cmd_Send(CString strSend);
	void EventLog(CString &s);
	CString			m_strReceiveLog;
	void GetConfig();
	CString gf_MakeCurTimeString();
	CString gf_MakeCurTimeString2();
	CString GetLogFileName();
	int WriteFile(CFile *pFile, CString &sPath, CString sLog);
	BOOL GetReceiveData(CString *reply, UINT nRcvSize, UINT nTimeout);

	long DoWorkStartGetPLCMemoryBit(CString strDeviceText,int iDeviceNum);	
	BOOL DoWorkStartGetPLCMemoryBitAll();
	BOOL DoWorkStartGetPLCMemoryBitAll2();
	void SetMainPoint(void* pvoid);
	afx_msg void OnBnClickedBtnConnetPlcDRead();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CListBox m_lbPlcLog;
	void SetAppPath(CString str) { m_strAppPath = str; };
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	BOOL GetPLCMemoryBitAll();
};
