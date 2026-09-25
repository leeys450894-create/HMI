#pragma once

#include "common\Label.h"
// CDlgSubLogFile 대화 상자입니다.
class CLogFileAccessor;
class CDlgSubLogFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubLogFile)

private:
	CFont		m_logFont;
	CLabel		m_LabelInfo;
	CListBox	m_listLog;
	CString		m_strTitle;
	CLogFileAccessor *m_pLogFile;

public:
	void LinkParam(CLogFileAccessor *log);

public:
	CDlgSubLogFile(CString strTitle, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubLogFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_LOG_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnGuiLog(WPARAM wp, LPARAM lp);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	CLogFileAccessor *GetLogFileAccessorPtr() { return m_pLogFile; }
};
