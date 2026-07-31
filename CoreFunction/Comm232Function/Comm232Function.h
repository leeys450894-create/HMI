#pragma once

#include "CommSerialDefine.h"

class C232MQueue
{
    BYTE buff[QUEUE_BUFF_SIZE];		//큐 버퍼 사이즈는 4096 으로 만듬
    int m_iHead, m_iTail;			//큐 버퍼에 데이터를 넣고 빼 올때 사용할 변수

public:
    C232MQueue()
    {
        Clear();
    }

	void Clear()
    {
        m_iHead = m_iTail = 0;
        memset(buff, 0, QUEUE_BUFF_SIZE);
    }
	int GetSize()
    {
        return (m_iHead - m_iTail + QUEUE_BUFF_SIZE) % QUEUE_BUFF_SIZE;
    }
    BOOL PutByte(BYTE b)
    {
        if(GetSize() == (QUEUE_BUFF_SIZE - 1)) 
			return FALSE;

        buff[m_iHead++] = b;
        m_iHead %= QUEUE_BUFF_SIZE;

        return TRUE;
    }
    BOOL GetByte(BYTE *pb)
    {
        if(GetSize() == 0)
			return FALSE;

        *pb = buff[m_iTail++];
        m_iTail %= QUEUE_BUFF_SIZE;

        return TRUE;
    }
	BYTE GetBuff(long idx)
	{
		if( (0 <= idx) && (idx < QUEUE_BUFF_SIZE) )
		{
			return buff[idx];
		}
		
		return 0x00;
	}
};

class CComm232Config;
class CComm232Function
{
	CWinThread			*m_pThread;
	HANDLE				m_hCloseEvent;
    bool				m_bLive;
protected:
	CComm232Config		*m_pConfig;
	long				m_nIndex;

public:
    static UINT ThreadWorkingLoop(LPVOID pParam);
	virtual void ThreadWorkingLoop();    


	//------------------------------------------------------------------
    // 통신을 관리하는 Attribute
    //------------------------------------------------------------------
protected:
    OVERLAPPED		m_osRead;										// 포트 파일 IO를 위한 Overlapped structure
    OVERLAPPED		m_osWrite;										// 포트 파일 IO를 위한 Overlapped structure
    C232MQueue		m_QueueRead;									// 실제 데이터 : 큐 버퍼

    HANDLE			m_hComm;										// 통신 포트 파일 핸들
	bool			m_bConnected;									// 포트가 열렸는지 유무를 나타냄.
    bool			m_bDataRecieveOK;								// 데이터가 도착했는지를 확인하는 변수
	CString			m_strRecieveData;

protected:
    bool OpenPort(DWORD wPortID);									// Serial Port를 연다. 인자로 포트명의 String과 속도, 포트 번호를 준다.
	bool OpenPort();												// Serial Port를 연다. 인자로 포트명의 String과 속도, 포트 번호를 준다.
    void ClosePort();												// Serial Port를 닫는다.
	virtual DWORD WriteComm(CString strBuf);						// Serial Port에 데이터를 쓴다.
    DWORD ReadComm(BYTE *pBuff, DWORD dwToRead = COMM_BUFF_SIZE);	// Serial Port를 읽어 본다.
	CLogFileAccessor			*m_pLogFileAccessor;

public:
    bool	Open(long nIndex);
    bool	Close();
	bool	Is_Connected() const								{ return m_bConnected;		}
    bool	RecieveData(CString &strMsg);							// RingBuffer에 있는 데이터를 읽는 함수. 메시지를 받은 CommWnd에서 사용.
    bool	SendCommand(CString command);
	void	SetLogFileAccessor(CLogFileAccessor			*pLogFileAccessor);



	//************************************************************************************************
    // 메인으로 이벤트를 전달하기 위한 콜백함수
    //************************************************************************************************
protected:
    LPVOID					m_CallbackPtr;			// 콜백 설정 클래스 포인터
    CallbackClassFunction2	m_CallbackFunc;			// 콜백 설정 클래스 함수 포인터
public:
	void SetCallbackFunction(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc)
	{
		m_CallbackPtr	= callbackPtr;
		m_CallbackFunc	= callbackFunc;
	}

public:
	bool Initialize();
	void Finalize();
	bool IsLive();
	void SetConfig(CComm232Config *pConfig);

public :
    CComm232Function();
    virtual ~CComm232Function();
};
