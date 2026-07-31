#pragma once

#include "common\Label.h"
#include "..\CoreFunction\common\XStringHelper.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
// CDlgSubConfigLoginFile 대화 상자입니다.
class CLogInControlMgr;
class CDlgSubConfigLoginFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubConfigLoginFile)

private:
	CLogInControlMgr* m_pLoginMgr;

	CXStingHelper	m_strHelper;
	CLabel		m_LabelInfo;
	CGridCtrl   m_gridList;
	CFont			m_font;
	CString			m_strId;
	CString			m_strPW;
	CComboBox		m_cmbLevel;
	long			m_nCurSel;
		
public:
	CDlgSubConfigLoginFile(CLogInControlMgr* pLoginMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubConfigLoginFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CONFIG_USER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	void __InitListControl();
	void __InitialListData();
	void __RefreshListData();
	void InitLoginLevel();
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnGridItemActivateList(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonSave();
};
