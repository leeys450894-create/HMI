#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"



#include <vector>
using namespace std;

//#define MAX_AGING_STICK	10
#define MAX_AGING_ITEM	6

#define AGING_NO_ALARM	 0
#define AGING_NOT_FILE	 1
#define AGING_FIND_ALARM 2

class CAuroraMgr;	// LYS 2026-08-05 Phase 1

class CDlgSubManual_AgingSet : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_AgingSet)
	enum { IDD = IDD_DLG_SUB_MANUAL_AGING_SET };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_AgingSet(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_AgingSet();

public:
	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer
	vector <S_AgingTargetAlarm>	m_vecAgTgAlarm;
	CString						m_strPtn;
	int							m_nCell;
	BOOL						m_bAgingAlarmUse;

	CMySpreadDll*				m_spreadAgAlarmList;
	CAuroraMgr*					m_pAuroraMgr;	// LYS 2026-08-05 Phase 1
	HFONT						m_hfont;
protected:
	//CMFCButton		m_chkStop;
	CString			m_strAgingRcpPath;
	CString			m_strOldRcp;
	CString			m_strAgingFilePath;
	CMFCButton		m_btnAgingAlarmUse;

public:
	void UpdateStatus(BOOL bEnable);
	void UpdateListNum();
	BOOL IsOfflineStatus();
	BOOL LoadAgingInfo(CString strRcp);
	void SaveAgingInfo(CString strRcp);
	void AgingItemClearAll();
	void AgingItemAdd();
	BOOL CmAgingFileFindTimeOut(CString strFile, int nTimeOut);
	BOOL CmAgingFileFind(CString strFile);
	int FindAgingAlarm();
	CString GetPtn() { return m_strPtn; };
	int		GetCell() { return m_nCell; };
	void DeleteAgingFile();
	BOOL	GetAgingAlarmUse() { return m_bAgingAlarmUse; };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//afx_msg void OnStop ();
	afx_msg void OnRcpOpen();
	afx_msg void OnItemClearAll();
	afx_msg void OnItemAdd();
	afx_msg void OnItemDel();
	afx_msg void OnItemSave();
	afx_msg void OnLoadAgingInfo();
	afx_msg void OnAgingProcStrart();
	afx_msg void OnAgingAlarmUse();

	DECLARE_MESSAGE_MAP()
};
