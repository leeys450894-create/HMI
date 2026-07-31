#pragma once

#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"

// CDlgSubConfigModuleFile 대화 상자입니다.
class CAuroraMgr;
class CDlgSubConfigModuleFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubConfigModuleFile)

private:
	CAuroraMgr* m_pAuroraMgr;
	CLabel		m_LabelInfo;
	CGridCtrl   m_gridList;

public:
	CDlgSubConfigModuleFile(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubConfigModuleFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CONFIG_MODULE_FILE };
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
	void __InitListControl2();
	void __InitialListData2();
	void __RefreshListData2();
	afx_msg void OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnBnClickedButtonConnectLed();
	afx_msg void OnBnClickedButtonDisconnectLed();
};
