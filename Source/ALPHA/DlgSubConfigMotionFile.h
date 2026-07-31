#pragma once

#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCombo.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"

// CDlgSubConfigMotionFile 대화 상자입니다.
class CMotorControlMgr;
class CDlgSubConfigMotionFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubConfigMotionFile)

private:
	CMotorControlMgr	    *m_pMotorControlMgr;
	int						m_iCurSel;

public:
	CDlgSubConfigMotionFile(CMotorControlMgr *pMotorControlMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubConfigMotionFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_CONFIG_MOTION_FILE };
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
	CLabel		m_LabelInfo;
	CGridCtrl   m_gridList;
	afx_msg void OnBnClickedButtonLoad();
	afx_msg void OnBnClickedButtonSave();
	void OnGridItemActivateList(NMHDR * pNotifyStruct, LRESULT * result);
	void OnGridItemChangedList(NMHDR * pNotifyStruct, LRESULT * result);
	void GetCurAxisInfo(int nGurAxis);
	void LoadConfig();
	void SaveConfig();
};
