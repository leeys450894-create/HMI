#pragma once

#include <map>
#include <list>

struct S_Log {
	CString m_msg;
	int		m_nthWnd;
	CString m_time;
	int		m_nItem;
};

#define END_EVENT_LOG		10

using namespace std;

class C_LogMgr
{
	public:
		C_LogMgr ();
		virtual ~C_LogMgr ();

	protected:
		//int					m_nListCount[END_EVENT_LOG];
		//map<int, CString>	m_mapItem;
		CString				m_path[3];
		//list<S_Log>			m_sLogList;
		CCriticalSection	m_csWaitLogData;

	//private:
	//	CEvent				*m_pWriteEvent;

	public:
		void	AddEventLog (CString msg, int nItem, BOOL bReply = FALSE, BOOL bAckNak = FALSE);
		void	SetPath (CString path, int nItem = 0, BOOL bTestLog = FALSE) { m_path[nItem] = path;};

	protected:
		CString	GetItem (int nthWnd);
		void	AdjusrHorzScroll (CListBox& wndListBox);
		void	WriteEventLog (int nthWnd, CString msg, int nItem);
		//void	WriteEventLogError (int nthWnd, CString msg);
		//BOOL	CreateFolder (CString &filePath);
		void	PrintLog (S_Log &log);
	//public:
	//	void	WriteEventLog (S_Log *pLog);
};

extern C_LogMgr	*pLogMgr;
