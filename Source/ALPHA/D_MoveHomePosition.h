#pragma once

#include "common\Label.h"

#define POPUP_OPERATE_HOME_POSITION_TIMER_ID1			9001
#define POPUP_OPERATE_HOME_POSITION_TIMER_ID2			9002

// CD_MoveHomePosition 대화 상자입니다.

class CSystemConfigMgr;
class CIOMapControlMgr;
class CD_MoveHomePosition : public CDialogEx
{
	DECLARE_DYNAMIC(CD_MoveHomePosition)

private:
	CLabel	m_LabelInfo;
	CSystemConfigMgr		*m_pSystemConfigMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;

public:
	CD_MoveHomePosition(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_MoveHomePosition();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_MOVE_HOME_POSITION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonHomePositionDone();
};
