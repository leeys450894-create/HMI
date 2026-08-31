#pragma once
#include <bitset>

typedef	void	(*SEND_PG_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

class	AFX_EXT_CLASS	C_SendElpPGComm : public C_SyncSerialComm
{
	public: 
		C_SendElpPGComm () {};
		C_SendElpPGComm (SEND_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_SendElpPGComm ();

	protected:
		CCriticalSection			m_section;
		int							m_nIndex;
		S_SerialInfo				m_sInfo;
		SEND_PG_EVENT_HANDLER		m_pHandler;

	protected:
		void	WriteMsg (CString strCmd);
		void	WriteMsg2(CString strCmd);

	public:
		void	ReadIniInfo ();
		void	ReConnect   ();
		//void	WriteIniInfo (S_SerialInfo *sInfo);
		BOOL	ActPGOn		();
		BOOL	ActPGOff	();
		BOOL	ActPGNext	();
		BOOL	ActPGSet	(int i);

		BOOL	GetCommOutOfService	() { return m_sInfo.m_bOutOfService; };
		BOOL	IsConnected			() { return m_bConnected; };
		CString GetComPort			() { return m_sInfo.m_strComPort; };
};
