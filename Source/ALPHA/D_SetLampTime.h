#pragma once

#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\GridCtrl_src\GridCell.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
// CD_SetLampTime 대화 상자입니다.
class CRegistry;
class CAuroraMgr;
class CD_SetLampTime : public CDialogEx
{
	DECLARE_DYNAMIC(CD_SetLampTime)

private:
	CAuroraMgr			       *m_pAuroraMgr;
	CRegistry*					m_pRegistry;
	CGridCtrl					m_gridList;
	CLabel						m_LabelInfo;

public:
	CD_SetLampTime(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_SetLampTime();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_LAMP_TIME};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonReset();
	afx_msg void OnBnClickedButtonApply();
	afx_msg void OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result);

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void __InitListControl();
	void __InitialListData();
	void __RefreshListData();
	afx_msg void OnDestroy();
};
