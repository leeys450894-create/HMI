#pragma once

typedef	void	(*QUALITY_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

#define QUALITY_BUFFER_MAX		2	//SDC 화질 두개 버퍼로 진행

#define QUALITY_FILTER_IN		0
#define QUALITY_FILTER_OUT		1

struct S_QualityWait {
	BOOL	m_bWaitStatus;
	BOOL	m_bRunStatus;
	CString m_strInnerID;
	CString m_strCellID;

	int		m_nSheetRow; //Data 출력 위치

	S_QualityWait () {
		Clear ();
	}

	void Clear () {
		m_bWaitStatus = FALSE;
		m_bRunStatus  = FALSE;
		m_strInnerID  = _T("");
		m_strCellID   = _T("");
		m_nSheetRow   = -1;
	}
};

enum QualityItem {
	QUALITY_WAIT_CLEAR				= -1,
	QUALITY_WAIT_RECIPE_CHANGE_OK	= 0,
	QUALITY_WAIT_GRAB_START_OK,
	QUALITY_WAIT_GRAB_END_OK,
	QUALITY_WAIT_CLASS_END_OK,
	QUALITY_WAIT_MAX,
};

class	AFX_EXT_CLASS	C_QualityComm : public C_ClientSocket
{
	public:
		C_QualityComm (QUALITY_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_QualityComm ();

	public:
		int m_nRunning;
		int	m_nIndex;	//Socket 통신 순서
		QUALITY_EVENT_HANDLER	m_pHandler;

		BOOL			m_bWaitItem[QUALITY_WAIT_MAX];
		S_TcpIpInfo		m_sInfo;

		S_QualityWait	sQualityWait[QUALITY_BUFFER_MAX];

		int				m_nMoveFilter;

	public:
		virtual	void	ReceiveCode (TCHAR* pBuf);
		virtual void	OnClose (int nErrorCode);
		virtual void	OnConnect (int nErrorCode);

	public:
		void			ConnectCtrl (BOOL bConnect);
		void			ReadIniInfo ();
		void			WriteIniInfo (S_TcpIpInfo *sInfo);
		S_TcpIpInfo		GetIniInfo () { return m_sInfo; };

		void	SetWaitItem (int nItem, BOOL bSet);
		BOOL	GetWaitItem (int nItem);
		void	TimeOutAlarm (CString strCmd, int nTime);
		void	NakAlarm (CString strCmd);
		void	SendAlarm (CString strCmd, int nTime);

		BOOL	WaitSTOP ();

		//명령
		BOOL	SetFilterMoveMsg (BOOL bMoveOK);
		BOOL	SetInspectStart (CString strInnerID, int nSheetRow, CString strCellID);
		BOOL	WaitInspectEnd ();

		BOOL	ActChangeRecipe (CString strRcpName);
		BOOL	ActInspectStart (CString strInnerID, int nSheetRow, CString strCellID);

		//응답 처리
		void	ReceiveEventOK (int nEvent, TCHAR* pBuf);
		void	ReceiveEventNG ();
		void	ReceiveClassEnd (CString strRcv);
		void	ReceiveFilter (int nItem);

		void	WriteComm (LPCTSTR lpszMsg);
		BOOL	Delay (DWORD dwTime = 100);
		BOOL	RunModalLoop (int nWaitItem = -1, DWORD dwTime = 20000);
		BOOL	RunModalFinish (BOOL bFinishSet = TRUE);

		//화질 전용
		BOOL	QualityBufferEmpty ();
		BOOL	QualityBufferEmptyAll ();
		void	QualityWaitSet (CString strInnerID, int nSheetRow, CString strCellID);
		void	QualityWaitRun (CString strInnerID);
		void	QualityWaitClear ();

		BOOL	GetCommUse () { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
};

