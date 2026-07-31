#pragma once
#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"

struct stMotorInfo
{
	int axis;
	double dPosition;
	bool bEnable;
	bool bLimitP;			// +
	bool bLimitN;			// -
	bool bHomeDone;
	double	dSetAxisSpeed;	// Set Speed
	double	dAxisSpeed;		// Current Speed
	double  dSetAxisPos;	// Set position
	double	dAxisPos;		// Current position
	double	dAxisRMS;		// 부하률
};

// CD_PopUp_Motion_Move 대화 상자
class CMaintControlMgr;
class CD_PopUp_Motion_Move : public CDialogEx
{
	DECLARE_DYNAMIC(CD_PopUp_Motion_Move)

	CMaintControlMgr* m_pMaintControlMgr;

	int						m_iCurSel;
	int			m_iGridSel;

	CLabel		m_LabelInfo[4];
	CGridCtrl   m_gridList[4];

	stMotorInfo m_stMoterInfo[90];

	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStart1Event;
	HANDLE		m_hStart2Event;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hMultiEvent[3];
	UINT static ThreadRun(LPVOID pParam);
	UINT		ThreadFunc1();
	UINT		ThreadFunc2();
	UINT		ThreadFuncDone(LPARAM lp);

public:
	CD_PopUp_Motion_Move(CMaintControlMgr* pMaintControlMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_PopUp_Motion_Move();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PUPUP_MOTION_MOVE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnGridItemClickList1(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemClickList2(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemClickList3(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemClickList4(NMHDR* pNotifyStruct, LRESULT* result);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void __InitListControl();
	void __InitialListData();
	void __RefreshListData();
	void __RefreshListData1();
	void __InitListControl2();
	void __InitialListData2();
	void __RefreshListData2();
	void __InitListControl3();
	void __InitialListData3();
	void __RefreshListData3();
	void __InitListControl4();
	void __InitialListData4();
	void __RefreshListData4();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};
