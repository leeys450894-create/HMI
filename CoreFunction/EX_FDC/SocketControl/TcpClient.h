#pragma once

#define MAX_RECEIVE_SIZE            64000                // 데이터를 수신할 최대 사이즈 

typedef enum eSocketType
{
	SOCKET_TYPE_CLIENT		= 0,
	SOCKET_TYPE_SERVER,
	SOCKET_TYPE_NONE,
};

typedef enum eSocketMsg
{
	SOCKET_MSG_ACCEPT		= (WM_USER + 4111),
	SOCKET_MSG_CLOSE,
	SOCKET_MSG_RECEIVE,
	SOCKET_MSG_CONNECT,
};

//////////////////////////////////////////////////////////////////////////
// CCSocket
class CCSocket : public CAsyncSocket
{
public:
	CCSocket(CWnd *pParent);
	virtual ~CCSocket(void);

	BOOL		CreateSocket();
	BOOL		Connect(LPCTSTR lpszHostAddress, UINT nHostPort);
	void		Close();

	int			SendData(BYTE *pbySend, UINT nSize);
	BOOL		IsConnected();
	void		SetConnected(BOOL bConnected);
	void		SetCreated(BOOL bCreated);

public:
	void		OnReceive(int nErrCode);
	void		OnClose(int nErrorCode);
	void		OnConnect(int nErrorCode);
	UINT		GetSizeReceiveData();
	void		SetSizeReceiveData(UINT iSize);
	BYTE		m_byteReceiveData[MAX_RECEIVE_SIZE];               // 데이터를 수신하였을 경우에 저장하는 버퍼 

private:
	CWnd*		m_pParent;
	BOOL		m_bConnected;
	BOOL		m_bCreated;
	UINT		m_nSizeReceiveData;
	CRITICAL_SECTION	m_csCSock;
};


//////////////////////////////////////////////////////////////////////////
// CTcpClient
class CTcpClient 
{
public:
	CTcpClient(CWnd *pParent);
	virtual ~CTcpClient(void);

	BOOL		Initialize();
	void		Finalize();

	BOOL		ConnectToServer(CString lpszIpAddr, UINT nPort);
	BOOL		Connect(LPCTSTR lpszIpAddr, UINT nPort);
	void		Disconnect();
	BOOL		IsConnected();
	int			SendData(BYTE *pSendData, UINT nDataSize);
	int			GetReceiveData(BYTE* pData, UINT nBufSize);                         // 수신데이터를 가져가는 함수 
	CString		GetLocalIP();

protected:
	void		StartThread();
	static UINT ThreadProc(LPVOID pParam);
	void		EndThread();



private:
	CCSocket*	m_pClient;
	CWnd*		m_pParent;

	CString		m_lpszIpAddr;
	UINT		m_nPort;

	CWinThread*	m_pThread;
	BOOL		m_bFlagThreadStop;
public:
	UINT GetReceiveDataSize(void);
	CString GetServerIPAddress();                                           // 서버의 IP어드레스를 얻는 함수 
	CString GetClientIPAddress();
};
