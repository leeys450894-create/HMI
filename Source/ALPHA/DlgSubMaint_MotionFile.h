#pragma once

#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"

#define DF_PROBE_USE_MAX 5

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

// CDlgSubMaint_MotionFile 대화 상자
class CAuroraMgr;
class CDlgSubMaint_MotionFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubMaint_MotionFile)

private:
	CAuroraMgr* m_pAuroraMgr;
	int						m_iCurSel;
	int			m_iGridSel;

	CLabel		m_LabelInfo[4];
	CGridCtrl   m_gridList[4];

	stMotorInfo m_stMoterInfo[90];

	CMFCButton m_chkProbe[DF_PROBE_USE_MAX];

	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStart1Event;
	HANDLE		m_hStart2Event;
	HANDLE		m_hStart3Event;
	HANDLE		m_hStart4Event;
	HANDLE		m_hStart5Event;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hMultiEvent[5];
	UINT static ThreadRun(LPVOID pParam);
	UINT		ThreadFunc1();
	UINT		ThreadFunc2();
	UINT		ThreadFunc3();
	UINT		ThreadFunc4();
	UINT		ThreadFuncDone(LPARAM lp);

public:
	CDlgSubMaint_MotionFile(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubMaint_MotionFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MAINT_MOTION };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg void OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result);
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
	afx_msg void OnBnClickedButtonMaintMotionInit();
	afx_msg void OnBnClickedButtonMaintMotionStop();

	void SetProbeUseColor(int nItem);
};
