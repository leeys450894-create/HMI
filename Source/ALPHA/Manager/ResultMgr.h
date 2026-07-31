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

