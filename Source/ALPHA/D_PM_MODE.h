#pragma once
#include "common\Label.h"

// CD_PM_MODE 대화 상자입니다.

class CMaintControlMgr;
class CLogInControlMgr;
class CD_PopUp_Motion_Move;
class CD_PM_MODE : public CDialogEx
{
	DECLARE_DYNAMIC(CD_PM_MODE)

private:
	CMaintControlMgr	* m_pMaintControlMgr;
	CLogInControlMgr	*m_pLogInControlMgr;
	CStatic				m_stPModeImage;
	HBITMAP				m_hbitmapImage;

	CD_PopUp_Motion_Move* m_pMotionMove;

	CLabel		m_statusInfo[2];

public:
	CD_PM_MODE(CMaintControlMgr *pMaintControlMgr, CLogInControlMgr *pLogInControlMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_PM_MODE();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_PM_MODE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonDoorLock();
	afx_msg void OnBnClickedButtonDoorUnlock();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	int m_nNGCount;
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedButtonBuzzerOff();
	afx_msg void OnBnClickedButtonMutingOn();
};
