#ifndef __ETHERNET_DEFINE_H__
#define __ETHERNET_DEFINE_H__

#include <vector>
#include <MMSystem.h>

#define MAX_SOCKET						1

#define WSA_ASYNC						(WM_USER + 4443)

#define DEFAULT_TIMEDELAY				3000
#define RESOLUTION						100000

#define DEFAULT_CLIENT_AUTOID			99

#define XNET_STR_SERVER_READY			_T("$XNET_SERVER_READY")
#define XNET_STR_SERVER_NOT_READY		_T("$XNET_SERVER_NOT_READY")
#define XNET_STR_SERVER_ACCEPT			_T("$XNET_SERVER_ACCEPT")
#define XNET_STR_NOTCONNECT				_T("$XNET_NOTCONNECT")
#define XNET_STR_NOTALLCONNECT			_T("$XNET_NOTALLCONNECT")
#define XNET_STR_CONNECTED				_T("$XNET_CONNECTED")
#define XNET_STR_CONNECT_FAIL			_T("$XNET_CONNECT_FAIL")
#define XNET_STR_SENDTIMEOUT			_T("$XNET_SENDTIMEOUT")
#define XNET_STR_RECEIVEDATA			_T("$XNET_RECEIVEDATA")
#define XNET_STR_SENDDATA				_T("$XNET_SENDDATA")

// 보통 Network Interface Card 의 Send 버퍼 크기는 8Kb 라고 함.
// 근데 ;;; 4MB 해야 spectrum 데이터를 모두 받네..
#define PACKET_BODY_SIZE				1024 * 1024 * 4

#ifndef ENUM2STR
#define ENUM2STR(p)						#p
#endif

enum enumCOMM
{
    COMM_UNKNOWN = -1,
    COMM_CONNECT,
    COMM_DISCONNECT,
    COMM_INIT_REQ,
    COMM_INIT_RES,
    COMM_GRAB_START_REQ,
    COMM_GRAB_START_RES,
    COMM_GRAB_END_REQ,
    COMM_GRAB_END_RES,
    COMM_GRAB_STOP_REQ,
    COMM_GRAB_STOP_RES,
	COMM_INSP_RESULT_REQ,
	COMM_INSP_RESULT_RES,
    COMM_USER_DEFINE_REQ,
    COMM_USER_DEFINE_RES,
    COMM_TYPE_COUNT,
};

enum eRUNSTATUS
{
    TCPDONE = 0,
    TCPSERVER,
    TCPCLIENT
};
enum eXNET_COMM_STATE
{
    XNET_SERVER_READY = 0,
    XNET_SERVER_NOT_READY,
    XNET_SERVER_ACCEPT,
    XNET_CONNECTED,
    XNET_NOTCONNECT,
    XNET_NOTALLCONNECT,
    XNET_CONNECT_FAIL,
    XNET_SENDDATA, XNET_SENDDATA_ERROR,
    XNET_RECEIVEDATA, XNET_RECEIVEDATA_ERROR,
    XNET_TIMEOUT
};

// 사용자 프로그램에 통지할 메시지 구조체
typedef struct _tagReceivedData
{
    int					iClientID;
    eXNET_COMM_STATE 	eCommState;
	size_t				sizeBody;
	char				szBody[PACKET_BODY_SIZE];
    SOCKADDR_IN         cliaddr;
   // char                szClientIP[32];
   // int                 iClientPort;

    inline void	Clear()	{ memset(this, 0x0, sizeof(TRECEIVEDDATA)); }
} TRECEIVEDDATA, *LPRECEIVEDDATA;


typedef void (*CallbackClassFunction2)(LPVOID, LPVOID);

#endif // __ETHERNET_DEFINE_H__
