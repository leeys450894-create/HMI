#pragma once
#include <bitset>

typedef	void	(*QUALITY_PG_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

enum QualityPGItem {
	QPG_WAIT_CLEAR			= -1,
	QPG_WAIT_TURN_ON		= 0,
	QPG_WAIT_TURN_OFF,
	QPG_WAIT_NEXT,
	QPG_WAIT_VOLTAGE,

	QPG_WAIT_MAX,
};

class	AFX_EXT_CLASS	C_QualityPGComm : public C_SerialComm
{
	public: 
		C_QualityPGComm () {};
		C_QualityPGComm (QUALITY_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_QualityPGComm ();

	protected:
		CCriticalSection			m_section;
		int							m_nIndex;
		S_SerialInfo				m_sInfo;
		QUALITY_PG_EVENT_HANDLER	m_pHandler;
		BOOL						m_bWaitItem[QPG_WAIT_MAX];
		int							m_iPattern; // PG On Pattern

	protected:
		void	WriteMsg (CString strCmd);

	public:
		void	SetWaitItem (int nItem, BOOL bSet, int iPattern = 0);
		BOOL	GetWaitItem (int nItem);
		int		GetPtrnItem ();

		void	ReadIniInfo    ();
		void	ReConnect	   ();
		//void	WriteIniInfo (S_SerialInfo *sInfo);
		//BOOL	SetPG_On_Ack   ();
		//BOOL	SetPG_Off_Ack  ();
		//BOOL	SetPG_Next_Ack ();
		BOOL	SetPG_Ack_Msg  ();

		BOOL	GetCommOutOfService	() { return m_sInfo.m_bOutOfService; };
		BOOL	IsConnected			() { return m_bConnected; };
		CString GetComPort			() { return m_sInfo.m_strComPort; };

	protected:
		virtual void ReceiveCode (TCHAR* pBuf);
};