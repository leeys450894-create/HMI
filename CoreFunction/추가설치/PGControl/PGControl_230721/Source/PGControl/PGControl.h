#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "PGControlDlg.h"

struct	S_RunState
{
	int				m_nRun;
	LPVOID			m_pData;
	int				m_nDelay[M_DELAY];
	DWORD			m_dwData[10];
	double			m_fData[10];
	int				m_nData[10];
	CString			m_strData1;
	CString			m_strData2;

	S_RunState () {
		memset (m_nDelay, 0, sizeof (int) * M_DELAY);
		Empty ();
	}

	void	Empty() {
		m_nRun		= 0;
		m_pData		= 0;
		memset (m_dwData, 0, sizeof (DWORD)  * 10);
		memset (m_fData,  0, sizeof (double) * 10);
		memset (m_nData,  0, sizeof (int)    * 10);
		m_strData1  = _T("");
		m_strData2  = _T("");
	}
};

class CPGControlApp : public CWinApp
{
	public:
		CPGControlApp();
		inline CPGControlDlg* GetMainPtr() { return reinterpret_cast <CPGControlDlg*> (__super::GetMainWnd()); }

	public:
		S_RunState			m_runState;
		CEvent*				m_pEventDelay;

		C_QualityPGComm		*m_pQualityPG232Comm;
		C_SendElpPGComm		*m_pPG232Comm;

		C_ColorPGServerComm *m_pColorPGServerComm;
		C_MainPGClientComm  *m_pMainPGClientComm;

	public:
		void	EnableControls(BOOL bEnable = TRUE);

	protected:
		BOOL	CheckOverlapExcute();

	public:
		virtual BOOL InitInstance();
		virtual int	 ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CPGControlApp theApp;
extern C_LogMgr		 theLogMgr;
