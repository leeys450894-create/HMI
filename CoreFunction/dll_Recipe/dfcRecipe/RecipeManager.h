#pragma once
#include <afxwin.h>
#include "resource.h"

class C_RecipeMgrDlg;
class CRecipeManager :
    public CWnd
{
    C_RecipeMgrDlg* m_pRcpMgrDlg;

protected:
	DECLARE_MESSAGE_MAP()

public:
	CRecipeManager();
	virtual ~CRecipeManager();

	void Initialize(HWND hParent, CRect r);
	void Finalize();
	void CreateDlg(HWND hParent, CRect r);
	CWnd* GetRecipeMgrWnd();
};

