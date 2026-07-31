#pragma once

#include "..\..\Include\API_RECIPE.h"

class CRecipeMgr
{

public:
	void    Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath);
	CWnd* GetReportmWnd();

	VTM_RECIPE* m_pSharedRecipe;

	//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
private:
	static CRecipeMgr* m_pInstance;
public:
	static CRecipeMgr* GetInstance();
	static void		   DelInstance();

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
private:
	void Finalize();
	CRecipeMgr(void);
	virtual ~CRecipeMgr(void);


public:
};

