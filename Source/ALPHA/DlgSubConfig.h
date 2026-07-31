#pragma once

#include "MainSubDlgWin.h"
#include "WindowEventHandler.h"

// CDlgSubConfig 대화 상자입니다.
class CDlgSubConfigFile;
class CDlgSubConfigMotionFile;
class CDlgSubConfigModuleFile;
class CDlgSubConfigLoginFile;
class CPositionConfigMgr;
class CMaintControlMgr;
class CSystemConfigMgr;
class CDlgSubConfig : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubConfig)

private:
	CAuroraMgr*					m_pAuroraMgr;
	CDlgSubConfigFile*			m_pSubConfigFile;
	CDlgSubConfigMotionFile*	m_pSubConfigMotionFile;
	CDlgSubConfigModuleFile*	m_pSubConfigModuleFile;
	CDlgSubConfigLoginFile*		m_pSubConfigLogInFile;
	CPositionConfigMgr*			m_pPositionConfigMgr;
	CMaintControlMgr*			m_pMaintControlMgr;
	CSystemConfigMgr*			m_pSystemConfigMgr;
	CGridCtrl					m_gridList;
	CGridCtrl					m_gridHeadOffset;
	CLabel						m_LabelInfo[2];

public:
	CDlgSubConfig(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubConfig();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CONFIG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	int m_iCurFileView;
	void RefreshFileView(BOOL bShow);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonConfig();
	afx_msg void OnBnClickedButtonConfigMotion();
	afx_msg void OnBnClickedButtonConfigModule();

	void __InitListControl_LoadPos();
	void __InitialListData_LoadPos();
	void __RefreshListData_LoadPos();
	void __InitListControl_HeadOffset();
	void __InitialListData_HeadOffset();
	void __RefreshListData_HeadOffset();
	void RefreshListData();
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonSave();
	afx_msg void OnBnClickedButtonConfigUser();
	afx_msg void OnBnClickedButtonLampTime();
	afx_msg void OnBnClickedButtonLoadMotion();
	afx_msg void OnBnClickedButtonSaveMotion();
	afx_msg void OnBnClickedButtonSetColor();
};
