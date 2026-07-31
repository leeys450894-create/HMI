#pragma once

#include "EthernetManagerTCPDefine.h"

class CEthernetManagerTCP;
class CCommEthFunction
{
	CCriticalSection	m_cs;
    CEthernetManagerTCP	*m_pEthernet;
    CWinThread			*m_pThread;
    bool				m_bLive;
    HANDLE				m_hCloseEvent;
    HANDLE				m_hWaitsEvent;
	bool				m_bConnected;
	bool				m_bForcedDisconnect;
	bool				m_bNetDelete;


private:
	bool NetStart();
    void NetStop();
	void NetDelete();
    static void xNetCallbackRecieveData(LPVOID, LPVOID);
    static UINT	ThreadCheckConnect(LPVOID pParam);

private:
	CString	m_strName;
	bool	m_bRunMode;
	CString	m_strIP;
    UINT	m_nPort;
public:
	void	SetName(CString strName);
	CString	GetName() const				{ return m_strName;			}
	void	SetNetInfo(bool bRunMode, CString strIP, long nPort);
	bool	GetConnectMode() const		{ return m_bRunMode;		}
	CString GetIPNumber() const			{ return m_strIP;			}
	long	GetPortNumber() const		{ return m_nPort;			}
	bool	Initialize(bool bDefault = true);
    void	Finalize();
	void	CloseConnect();
	bool	IsConnected() const;
    bool	IsLive();
	void	SetConnect(bool bStart);
	bool	IsOutOfService() const;
	bool	SendCommand(CString strMsg, bool bUseTail = false, bool bUseLog = true);

	bool	GetForcedDisconnect();
	void	SetForcedDisconnect(bool bFlag);


//************************************************************************************************
// 메인으로 이벤트를 전달하기 위한 콜백함수
//************************************************************************************************
protected:
	LPVOID					m_CallbackPtr;			// 콜백 설정 클래스 포인터
	CallbackClassFunction2	m_CallbackFunc;			// 콜백 설정 클래스 함수 포인터
public:
	void SetCallbackFunction(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc)
	{
		m_CallbackPtr = callbackPtr;
		m_CallbackFunc = callbackFunc;
	}

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
protected:
    virtual CString GetStringCMDID(int iCmdID);
	virtual bool	SendPacket(CString strMsg, bool bUseTail = false, bool bUseLog = false);
	virtual bool	SendPacket(void *pData, int iSize);
	virtual std::vector<CString> GetCommand(CString &strData);
	virtual std::vector<CString> GetCommand_SDC(CString& strData);
	virtual std::vector<CString> GetExeHextoStr(CString& strData);
	virtual std::vector<CString> GetExeHextoStr2(CString& strData);
	virtual std::vector<CString> GetExeHextoStrForElpAging(CString& strData);
    virtual long	ParseMsg(CString strMsg);
	virtual CString ExeHextoStr(CString strRcv);
    virtual void	xNetCallbackRecieveData(LPVOID);

public:
    CCommEthFunction();
    virtual ~CCommEthFunction();
};