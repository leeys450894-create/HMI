#pragma once

class	AFX_EXT_CLASS	C_EnsisPGComm : public C_PGComm
{
public:
	C_EnsisPGComm (PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
	virtual ~C_EnsisPGComm ();

private:
	int				m_nErrorCode;
	int				m_nAgingTime;
	int				m_nStick;
	BOOL			m_bRunning;
	int				m_nPGStatus[PG_MAX_STICK][PG_MAX_CELL];

private:
	virtual	void	ReceiveCode	  (TCHAR* pBuf)					override;
	virtual void	OnClose		  (int nErrorCode)				override;
	virtual void	OnConnect	  (int nErrorCode)				override;
	virtual void	ReadIniInfo	  ()							override;
	virtual void	SetWaitItem	  (int nItem, BOOL bSet)		override;
	virtual BOOL	GetWaitItem   (int nItem)					override;
	virtual void	TimeOutAlarm  (CString strCmd, int nTime)	override;
	virtual void	NakAlarm	  (CString strCmd)				override;
	virtual void	SendAlarm	  (CString strCmd, int nTime)	override;
	virtual void	ParsingPGData (int nItem, CString strData)	override;
	virtual void	SetPGStick	  (int nStick)					override { m_nStick = nStick; };

public:
	virtual void			ConnectCtrl	  (BOOL bConnect)					override;
	virtual void			WriteIniInfo  (S_TcpIpInfo* sInfo)				override;
	virtual S_TcpIpInfo		GetIniInfo	  ()								override { return m_sInfo; };
	virtual void			SetEmptyStick (int nStick = -1)					override;
	virtual int				GetCellState  (int nStick = -1, int nCell = 0)	override;
	virtual BOOL			IsRunning	  ()								override { return m_bRunning == PG_RUN; };
	virtual BOOL			WaitSTOP	  ()								override;

	//명령
	virtual BOOL	SetTestStop ()											override;
	virtual BOOL	SetTurnOff ()											override;
	virtual BOOL	ActChangeProgram (int nSelProgram)						override;
	virtual BOOL	ActSelectCell (int nArea, int nCell)					override;
	virtual BOOL	ActChangeModel (CString strModelName, int nSelPG = 1)	override;
	virtual BOOL	ActChangeRecipe (CString strRcpName)					override;
	virtual BOOL	ActRunStatus ()											override;
	virtual BOOL	ActAging (int nStick, BOOL bRtestMaskUse = TRUE)		override;
	virtual BOOL	ActTurnOn (int nSelCell = 0)							override;
	virtual BOOL	ActTurnOff (int nSelCell = 0)							override;
	virtual BOOL	ActPattrenNext (int nSelCell = 0)						override;
	virtual BOOL	ActPattrenPrev (int nSelCell = 0)						override;
	virtual BOOL	ActOST (int nStick, int nSelCell = 0)					override;
	virtual BOOL	ActPTP (int nStick, int nSelCell = 0)					override;
	virtual BOOL	ActRTest (int nStick, int nSelCell = 0)					override;

	virtual BOOL	GetCommUse ()											override { return m_sInfo.m_bUse; };
	virtual BOOL	GetCommOutOfService ()									override { return m_sInfo.m_bOutOfService; };

private:
	//응답 처리
	virtual void	ReceiveEventOK (int nEvent, TCHAR* pBuf)				override;
	virtual void	ReceiveEventNG ()										override;

	virtual void	WriteComm (LPCTSTR lpszMsg)								override;
	virtual BOOL	RunModalLoop (int nWaitItem = -1, DWORD dwTime = 20000)	override;
	virtual BOOL	RunModalFinish (BOOL bFinishSet = TRUE)					override;
};

