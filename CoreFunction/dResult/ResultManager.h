#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "API_Result.h"
#include "CDlgResult.h"
#include "..\common\CommonFunc.h"

class CResultManager : public CWnd
{
// Operations
private:
	CDlgResult* m_dlgResult;

protected:
	DECLARE_MESSAGE_MAP()

public:
	CResultManager();
	virtual ~CResultManager();

	void SetConfigPath(CString str);
	void InitializeResult(HWND hParent, CRect r);
	void FinalizeReport();
	void CreateResultDlg(HWND hParent, CRect r);
	bool SaveResult(bool bSaveFile, bool bRemote, CString strStart);
	bool SaveResultDone();
	int UploadData(PRSRITEMDATA result);
	void SetSearchMode(int iMode);
	void DataClear();
	CWnd* GetReportResultWnd();
};

