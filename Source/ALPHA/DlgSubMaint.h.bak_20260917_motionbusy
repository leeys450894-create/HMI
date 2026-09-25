#pragma once

#include "MainSubDlgWin.h"
#include "WindowEventHandler.h"

// CDlgSubMaint 대화 상자입니다.
class CDlgSubMaint_MotionFile;
class CDlgSubMaint_IOFile;
class CDlgSubMaint_ETCFile;
class CDlgSubMaint : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubMaint)

private:
	CAuroraMgr					*m_pAuroraMgr;

	CGridCtrl   m_gridListMotionTest;
	CGridCtrl   m_gridListIOOutp;
	CGridCtrl   m_gridListLightCtrl;
	CLabel      m_LabelInfo[3];

	CDlgSubMaint_MotionFile* m_pDlgSub_Motion;
	CDlgSubMaint_IOFile* m_pDlgSub_IO;
	CDlgSubMaint_ETCFile* m_pDlgSub_Etc;


	CWinThread	*m_pThread;
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
	CDlgSubMaint(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubMaint();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MAINT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	//void __InitListControl();
	//void __InitialListData();
	//void __RefreshListData();
	//void __InitListControl2();
	//void __InitialListData2();
	//void __RefreshListData2();
	//void __InitListControl3();
	//void __InitialListData3();
	//void __RefreshListData3();
	void RefreshListData();

	afx_msg void OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemClickList2(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemClickList3(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemActivateList(NMHDR* pNotifyStruct, LRESULT* result);
	afx_msg void OnGridItemChangedList(NMHDR * pNotifyStruct, LRESULT * result);
	afx_msg void OnGridItemActivateList3(NMHDR* pNotifyStruct, LRESULT* result);
	void UpdateInformation();
	int m_iCurSel;
	int m_iCurSelLight;
	afx_msg void OnBnClickedButtonMaintMotion();
	afx_msg void OnBnClickedButtonMaintIo();
	afx_msg void OnBnClickedButtonMaintSerial();
	void RefreshFileView(BOOL bShow);
	int m_iCurFileView;
	afx_msg void OnBnClickedButtonMaintEtc2();
};
