#pragma once

#include "EthernetManagerTCPDefine.h"

class CClientSocket
{
    CWinThread			*m_pThread;
	bool				m_bThreadLive;
    HANDLE				m_hStartEvent;
    HANDLE				m_hCloseEvent;
    bool				m_bIsCreated;
    bool				m_bConnected;

	// 소켓에 정보를 기입하기 위한 파라미터
    SOCKADDR_IN			m_sin;
    WSAEVENT			m_hMesgSocketEvent;
	SOCKET				m_client_socket;
	char				*m_pszSendData;		// 데이터를 송신/수신 하는 버퍼
	char				*m_pszRecvData;
    TRECEIVEDDATA		*m_pRecvPacket;

public:
    bool	CreateSocket(char *szServerName, USHORT usPortNo = 9000 );

	bool	SendMesgData(LPVOID lpData, int nSize);
	bool	IsCreated()	const;
	bool	IsThreadLive() const;
	bool	GetConnected() const;
    bool    CloseSocket();

private:
    bool	FillAddress(USHORT usPortNo, char *szServerName);
    bool	SocketCleanUp();
    bool	Connect();
    bool	RecvData();
    void	NotifyReceive(eXNET_COMM_STATE eCommState, LPVOID pData = NULL, size_t nSize = 0);

	static UINT	ThreadRun( LPVOID pVoid );
	void ThreadFunc();


    //************************************************************************************************
    // 리시브 함수를 위한 콜백함수 설정 함수들
    //************************************************************************************************
private:
    LPVOID					m_CallbackPtr;		// 콜백 설정 클래스 포인터
    CallbackClassFunction2	m_CallbackFunc;		// 콜백 설정 클래스 함수 포인터
public:
	void SetCallbackFunction( LPVOID callbackPtr, CallbackClassFunction2 callbackFunc)	// 콜백 설정
	{
		m_CallbackPtr  = callbackPtr;
		m_CallbackFunc = callbackFunc;
	}

public:
    CClientSocket();
    ~CClientSocket();
};
