#pragma once
#include "API_SACC.h"

// CShareManager
#define RITEM_BUFFER_MAX        10   // 10개의 버퍼를 사용.

class CShareManager : public CWnd
{
	DECLARE_DYNAMIC(CShareManager)

public:
	CShareManager();
	virtual ~CShareManager();

protected:
	DECLARE_MESSAGE_MAP()

	CWinThread*			m_pRMonitorThread;
	HANDLE				m_hEventStop;
	HANDLE				m_hEventResume;
	bool				m_bStop;
	bool				m_bInit;
	TCHAR				sErrMsg[1024];
	RITEMDATA			*pBuffer[RITEM_BUFFER_MAX];
	int					*nBuffer;
	
	RITEMDATA			m_RData;
	static UINT			CBResultMonitorProc(LPVOID pParam);
	void				CBResultMonitorBody();
	void				SetResultMonitoring(BOOL bSet);

	int		  InitShareMemory(void);
	int		  GetNBuffer();                        // 현재 몇개의 버퍼가 사용되었는지를 return 한다.
	TCHAR   * GetErrMsg();                         // 발생한 에러중 마지막 것을 받아온다.
	bool      ClearAll();                          // 버퍼를 비운다.
	bool	  Upload  (PRITEMDATA pArea);
	bool	  Download(PRITEMDATA pArea, bool bJustMonitoring=false, int iIdx=0); 

	int SetSearchMode(int iMode);

	HWND	m_hParentWnd;
	HANDLE	hMem_pBuffer; 
	HANDLE	hMem_nBuffer; 
	HANDLE	hMem_nMoniter;

public:
	PDITEMDATA			pMoniter;

public:
	int InitializeReport(HWND hParentWnd);
	int FinalizeReport(void);
	int UploadData(PRITEMDATA result);
	int SetHostMode(int iMode);
	int DownloadData(PDITEMDATA data);
	int Updata(PDITEMDATA data);
	void CloseShareMemory(void);
};


