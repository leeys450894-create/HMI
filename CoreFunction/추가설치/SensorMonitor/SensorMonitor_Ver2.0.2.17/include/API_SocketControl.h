//API_SocketControl.h

/*===========================================================================================
Function : Socket Control용 DLL
Using	 : 아직 미 확정
Date : 2009-04-27
Added by:Jay Hong
===========================================================================================*/
#define NET_MSG_ACCEPT                  WM_USER + 4111         // 서버모드에서 클라이언트가 접속했을 경우 
#define NET_MSG_CLOSE                   WM_USER + 4112         // 소켓을 종료할때
#define NET_MSG_RECEIVE                 WM_USER + 4113         // 데이터를 수신하였을 경우 

#if !defined(STRUCT_SOCKET_OPEN_DEFINE)
#define STRUCT_SOCKET_OPEN_DEFINE

typedef struct stSOCKET_OPEN{
	int			nSockMode;	
	char		szIPAddr[16];
	int			nPort;
}SOCKET_OPEN;

const enum SOCKET_MODE
{
	SOCKET_MODE_CLIENT		= 0,
	SOCKET_MODE_SERVER
};

#endif // !defined(STRUCT_SOCKET_OPEN_DEFINE)


BOOL	WINAPI	api_SocketCtrl_Initialize(UINT nIdex,CWnd *pParentWnd, int nServer, UINT nPort, TCHAR* szIPAddress);
BOOL	WINAPI	api_SocketCtrl_Close(UINT nIdex);
void	WINAPI	api_SocketCtrl_GetLocalIP(UINT nIdex, TCHAR* szIPAddress);
void	WINAPI	api_SocketCtrl_GetServerIPAddress(UINT nIdex, TCHAR* szIPAddress);
void	WINAPI	api_SocketCtrl_GetClientIPAddress(UINT nIdex, TCHAR* szIPAddress);
int		WINAPI	api_SocketCtrl_SendData(UINT nIdex,BYTE *pSendData, UINT nDataSize);
int		WINAPI	api_SocketCtrl_GetReceiveData(UINT nIdex, BYTE *pData, UINT nBufSize);

UINT	WINAPI  api_SocketCtrl_GetClientPort(UINT nIdex);
UINT	WINAPI	api_SocketCtrl_GetReceiveDataSize(UINT nIdex);
UINT	WINAPI	api_SocketCtrl_GetServerPort(UINT nIdex);
BOOL	WINAPI	api_SocketCtrl_IsConnected(UINT nIdex);
BOOL	WINAPI	api_SocketCtrl_SetModeChange(UINT nIdex, BOOL bIsServer);
BOOL	WINAPI	api_SocketCtrl_SetServerIPAddress(UINT nIdex, CString sIPAddress);
BOOL	WINAPI	api_SocketCtrl_SetServerPort(UINT nIdex, UINT nPort);
