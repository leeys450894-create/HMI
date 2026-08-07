#pragma once

#define EVENT_COMM_ERROR			0
#define EVENT_COMM_MSG				1
#define EVENT_COMM_NAK				2
#define EVENT_COMM_DISCONNECT		3
#define EVENT_COMM_TIMEOUT			4

#define EVENT_COOM_PG_COLOR_CMD		5
#define EVENT_COOM_PG_MAIN_CMD		6

#define EVENT_COMM_FILTER_IN		10
#define EVENT_COMM_FILTER_OUT		11

#define	COMM_RS_FINISH				0x00
#define	COMM_RS_RUN					0x01
#define	COMM_RS_STOP				0x02
#define	COMM_RS_ERROR				0x04
#define COMM_RS_TIMEOUT 			0x05

//#define PLC_F_ADDRESS_READ				_T("500000FF03FF000018009004010001")
//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 0018(길이) 0090(타이머) 0401(읽기) / 0001(비트단위) or 0000(워드단위)
#define PLC_CMD_HEAD				_T("500000FF03FF00")
#define PLC_CMD_SIZE				24
#define PLC_CMD_TIMER				_T("0090")

#define PLC_CMD_READ				_T("0401")
#define PLC_CMD_WRITE				_T("1401")
#define PLC_CMD_BIT					_T("0001")
#define PLC_CMD_WORD				_T("0000")

#define PLC_RECEIVE_HEAD			_T("D00000FF03FF")
#define PLC_RECEIVE_HEAD_SIZE		22
////

struct S_TcpIpInfo {
	BOOL	m_bUse;
	BOOL	m_bOutOfService;
	CString m_strName;			//통신 이름
	int		m_nHead;			//동일 측정 CCD 분기
	CString m_strIP;
	int		m_nPortNum;
	CString m_strResultDir;		//SDC 결과 파일 저장되는 경로
	CString m_strRecipeFile;	//SDC 레시피 파일에 레시피명 작성

	S_TcpIpInfo () {
		Clear ();
	}

	void Clear () {
		m_bUse			= FALSE;
		m_bOutOfService = FALSE;
		m_strName		= _T("");
		m_nHead			= -1;
		m_strIP			= _T("");
		m_nPortNum		= 0;
		m_strResultDir	= _T("");
		m_strRecipeFile = _T("");
	}
};

struct S_SerialInfo {
	BOOL	m_bUse;
	BOOL	m_bOutOfService;
	CString m_strName;			//통신 이름
	int		m_nHead;			//동일 측정 CCD 분기
	int		m_nMaxCH;
	CString m_strComPort;
	int		m_nRepDelay;

	S_SerialInfo () {
		Clear ();
	}

	void Clear () {
		m_bUse			= FALSE;
		m_bOutOfService = FALSE;
		m_strName		= _T("");
		m_nHead			= -1;
		m_nMaxCH		= 0;
		m_strComPort	= _T("");
		m_nRepDelay	    = 0;
	}
};