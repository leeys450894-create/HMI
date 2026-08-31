#pragma once

typedef	void	(*PG_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

#define PGSTX	0x02
#define PGETX	0x03

#define PGACK	0
#define PGNAK	1

#define PG_MAX_STICK	50
#define PG_MAX_CELL		50

#define PG_IDLE		0
#define PG_RUN		1

//Probe Test
#define PG_OST		0
#define PG_PTP		1
#define PG_RTEST	2
#define PG_AGING	3

//Act Program
#define PG_PROGRAM_INOUT	0
#define PG_PROGRAM_WAD		1
#define PG_PROGRAM_QUALITY	2

//Act Cell
#define PG_AREA_LEFT	0
#define PG_AREA_RIGHT	1

//Commnad
#define S1F1	_T("0101")
#define S1F5	_T("0105")
#define S2F1	_T("0201")
#define S2F9	_T("0209")
#define S5F1	_T("0501")
#define S6F1	_T("0601")
#define S8F1	_T("0801")
#define S10F1	_T("1001")
#define S10F3	_T("1003")

//Receive
#define S1F2	_T("0102")	
#define S1F6	_T("0106")
#define	S3F2	_T("0302")
#define	S3F3	_T("0303")
#define	S3F6	_T("0306")
#define	S3F7	_T("0307")
#define	S3F12	_T("0312")
#define	S3F14	_T("0314")
#define	S3F16	_T("0316")
#define	S3F18	_T("0318")
#define	S3F20	_T("0320")
#define	S5F2	_T("0502")
#define S2F2	_T("0202")
#define S2F3	_T("0203")
#define S2F10	_T("0210")
#define S2F12	_T("0212")
#define S6F2	_T("0602")
#define S6F3	_T("0603")
#define S8F2	_T("0802")
#define S8F3	_T("0803")
#define S10F2	_T("1002")
#define S10F4	_T("1004")

//Receive Item
#define TURN_ON		_T("01")
#define TURN_OFF	_T("02")
#define PTRN_NEXT	_T("03")
#define PTRN_PREV	_T("04")
#define OST			_T("05")
#define PTP			_T("06")
#define RTEST		_T("07")

//Cell Status
namespace PG {
	enum {
		CELL_IDLE = 0,

		CELL_PTP_NOR,		// Normal
		CELL_PTP_SHORT,		// Short
		CELL_PTP_OPEN,		// Open

		CELL_OST_NOR,		// Normal
		CELL_OST_SHORT,		// Short
		CELL_OST_OPEN,		// Open

		CELL_RTEST_PASS,	// R_TEST PASS
		CELL_RTEST_OVER,	// Over
		CELL_RTEST_UNDER,	// Under

		CELL_AGING_NG,
		CELL_AGING_OK,
	};
}

enum PGItem {
	PG_WAIT_CLEAR = -1,
	PG_WAIT_PROGRAM_SET_OK = 0,
	PG_WAIT_SELECT_CELL_OK,
	PG_WAIT_MODEL_CHANGE_CMD_OK,
	PG_WAIT_MODEL_CHANGE_END_OK,
	PG_WAIT_RECIPE_CHANGE_OK,
	PG_WAIT_STATUS_RCV_OK,
	PG_WAIT_AGING_TEST_CMD_OK,
	PG_WAIT_AGING_TEST_END_OK,
	PG_WAIT_CELL_CMD_OK,
	PG_WAIT_TURN_ON_END_OK,
	PG_WAIT_TURN_OFF_END_OK,
	PG_WAIT_PTRN_NEXT_END_OK,
	PG_WAIT_PTRN_PREV_END_OK,
	PG_WAIT_OST_END_OK,
	PG_WAIT_PTP_END_OK,
	PG_WAIT_RTEST_END_OK,
	PG_WAIT_MAX,
};

class	AFX_EXT_CLASS	C_PGComm : public C_ClientSocket
{
public:
	C_PGComm () {};
	virtual ~C_PGComm () {};

public:
	int m_nRunning;
	int	m_nIndex;	//Socket 통신 순서
	BOOL				m_bWaitItem[PG_WAIT_MAX];
	PG_EVENT_HANDLER	m_pHandler;
	S_TcpIpInfo			m_sInfo;

private:
	virtual	void		ReceiveCode		(TCHAR* pBuf) = 0;
	virtual void		OnClose			(int nErrorCode) = 0;
	virtual void		OnConnect		(int nErrorCode) = 0;
	virtual void		ReadIniInfo		() = 0;
	virtual void		SetWaitItem		(int nItem, BOOL bSet) = 0;
	virtual BOOL		GetWaitItem		(int nItem) = 0;
	virtual void		TimeOutAlarm	(CString strCmd, int nTime) = 0;
	virtual void		NakAlarm		(CString strCmd) = 0;
	virtual void		SendAlarm		(CString strCmd, int nTime) = 0;
	virtual void		ParsingPGData	(int nItem, CString strData) = 0;
	virtual void		SetPGStick		(int nStick) = 0;

public:
	virtual void		ConnectCtrl		(BOOL bConnect) = 0;
	virtual void		WriteIniInfo	(S_TcpIpInfo* sInfo) = 0;
	virtual S_TcpIpInfo	GetIniInfo		() = 0;
	virtual void		SetEmptyStick	(int nStick = -1) = 0;
	virtual int			GetCellState	(int nStick = -1, int nCell = 0) = 0;
	virtual BOOL		IsRunning		() = 0;
	virtual BOOL		WaitSTOP		() = 0;

	//명령
	virtual BOOL	SetTestStop			() {return FALSE;};
	virtual BOOL	SetTurnOff			() {return FALSE;};
	virtual BOOL	ActChangeProgram	(int nSelProgram) {return FALSE;};
	virtual BOOL	ActSelectCell		(int nArea, int nCell) {return FALSE;};
	virtual BOOL	ActChangeModel		(CString strModelName, int nSelPG = 1) {return FALSE;};
	virtual BOOL	ActChangeRecipe		(CString strRcpName) { return FALSE; };
	virtual BOOL	ActRunStatus		() { return FALSE; };
	virtual BOOL	ActAging			(int nStick, BOOL bRtestMaskUse = TRUE) { return FALSE; };
	virtual BOOL	ActTurnOn			(int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActTurnOff			(int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActPattrenNext		(int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActPattrenPrev		(int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActOST				(int nStick, int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActPTP				(int nStick, int nSelCell = 0) { return FALSE; };
	virtual BOOL	ActRTest			(int nStick, int nSelCell = 0) { return FALSE; };

	virtual BOOL	GetCommUse			() = 0;
	virtual BOOL	GetCommOutOfService () = 0;

private:
	//응답 처리
	virtual void	ReceiveEventOK (int nEvent, TCHAR* pBuf) = 0;
	virtual void	ReceiveEventNG () = 0;

	virtual void	WriteComm (LPCTSTR lpszMsg) = 0;
	virtual BOOL	RunModalLoop (int nWaitItem = -1, DWORD dwTime = TIMEOUT_20) = 0;
	virtual BOOL	RunModalFinish (BOOL bFinishSet = TRUE) = 0;
};
