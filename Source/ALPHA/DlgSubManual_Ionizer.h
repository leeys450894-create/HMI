#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"

class CDlgSubManual_Ionizer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_Ionizer)
	enum { IDD = IDD_DLG_SUB_MANUAL_IONIZER };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_Ionizer(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_Ionizer();

public:
	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer

protected:
	CMFCButton		m_chkIonUse;
	CMFCButton		m_chkIonAutoOff;
	CMFCButton		m_chkIonAlarmUse;
	CMFCButton		m_chkIonLoadOn;
	CMFCButton		m_chkIonUnloadOn;
	CMFCButton		m_chkStop;
	CComboBox		m_cmbIonizerCnt;
	CEdit			m_edtProcCount;

public:
	void UpdateStatus(BOOL bEnable);

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnIonizer(UINT nID);
	afx_msg void OnUpdateDelay();
	afx_msg void OnAutoProcess();
	afx_msg void OnStop();
	afx_msg void OnIonizerUse();
	afx_msg void OnIonizerAutoOff();
	afx_msg void OnIonizerAlarmUse();
	afx_msg void OnIonizerLoadOn();
	afx_msg void OnIonizerUnloadOn();
	afx_msg void OnSelectItem();

	DECLARE_MESSAGE_MAP()
};
