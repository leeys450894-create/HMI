#pragma once
#include "..\Include\API_Result.h"

class CResultMgr
{
public:
	void    Initialize(HWND hParentWnd, CRect rtROI);
	CWnd* GetReportmWnd();
	void	Updata_Data(PRSRITEMDATA result);
	bool	SaveResult(bool bSaveFile, bool bRemote, CString strStart);
	bool	SaveResultDone();
	void	SetConfigPath(CString strPath);
	void	ClearData();
	// LYS 2026-10-06 Hybrid Phase 5 journal replay (stage 5-0 dResult exports)
	void	SetFastUpload(bool bOn);						// true = skip the 1 s display wait (CResumeJournal::Replay only, reset on every exit)
	void	GetUploadState(int* pQueue, long* pDone);	// rows waiting in the dResult ring / finished uploads since the DLL loaded (monotonic)



	//----------------------------------------------------------------------------------------------------
	// Singleton
	//----------------------------------------------------------------------------------------------------
private:
	static CResultMgr* m_pInstance;
public:
	static CResultMgr* GetInstance();
	static void		DelInstance();

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
private:
	void Finalize();
	CResultMgr(void);
	virtual ~CResultMgr(void);
};

