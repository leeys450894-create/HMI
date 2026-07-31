#pragma once

//------------------------------------------------------------------------------
// [ Serial-Comm Read를 위한 Notify Message ]
//------------------------------------------------------------------------------
#define COMM_BUFF_SIZE		2048			// 포트의 In-Q로부터 한번에 읽어올 수 있는 Read-Buffer의 크기 지정
#define QUEUE_BUFF_SIZE		4192
#define	ASCII_LF			0x0A			// 포트에서 사용하는 - 통신제어문자 LINE FEED
#define	ASCII_CR			0x0D			// 포트에서 사용하는 - 통신제어문자 CARRAGE RETURN 
#define ASCII_NEWLINE		0x15			// LINE FEED + CARRAGE RETURN
#define	ASCII_STX			0x02			// 시작
#define	ASCII_ETX			0x03			// 종료

#define EFU_INFO_MODE1		0x8A
#define EFU_INFO_MODE2		0x87
#define EFU_RPM_MODE1		0x89
#define EFU_RPM_MODE2		0x84
#define EFU_RPM_FLAG_OK		0xB9
#define EFU_LV32_ID			0x81
#define EFU_DPU_ID			0x9F

//------------------------------------------------------------------------------
// [ TcpIP-Comm Read를 위한 Notify Message ]
//------------------------------------------------------------------------------
enum enumCOMM232
{
    enumCOMM232_UNKNOWN = -1,
    enumCOMM232_CONNECT,
    enumCOMM232_DISCONNECT,
    enumCOMM232_INIT_REQ,
    enumCOMM232_INIT_RES,
    enumCOMM232_USER_DEFINE_REQ,
    enumCOMM232_USER_DEFINE_RES,
	enumCOMM232_TYPE_COUNT,
};

enum enumLED_LAMP_CONTROL
{
	enumLED_LAMP_CONTROL_TURN__OFF = 0,
	enumLED_LAMP_CONTROL_TURN___ON,
};

enum enumRES_CHECK_TYPE
{
	enumRES_CHECK_TYPE_INTENSITY = 0,
	enumRES_CHECK_TYPE_TURN___ON,
	enumRES_CHECK_TYPE_TURN__OFF,
	enumRES_CHECK_TYPE_SHUTTER__ON,
	enumRES_CHECK_TYPE_SHUTTER_OFF,
	enumRES_CHECK_TYPE_COUNT,
	enumRES_CHECK_TYPE_CUR_INTENSITY,
};

#ifndef ENUM2STR
#define ENUM2STR(p)		#p
#endif

typedef void (*CallbackClassFunction2)(LPVOID, LPVOID);			// 연결 관련
