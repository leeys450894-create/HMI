#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"
//#include "..\\EXTRN\FPSpreadSheet\spreadsheet.h"



class CDlgSubManual_AutoContact : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_AutoContact)
	enum { IDD = IDD_DLG_SUB_MANUAL_AUTO_CONTACT };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_AutoContact(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_AutoContact();

public:
	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer
	double			m_fAutoCntReady;
	double			m_fAutoCntContactPos;
	double			m_fAutoCntODPos;
	CString			m_strAutoCntRecipeID;
	CString			m_strAutoCnttProbeID;
	BOOL			m_bAutoCntSucess;

	BOOL			m_bAutoCntUse;
	BOOL			m_bAutoCntModel;
	BOOL			m_bAutoCntProbeID;
	BOOL			m_bAutoCntAlways;

	CString			m_strIniPath;
	CString			m_strAutoLogPath;

	CMySpreadDll*	m_spreadAutoContact;
	HFONT			m_hfont;
protected:
	CMFCButton		m_btnStop;
	CMFCButton		m_btnAutoContactUse;
	CMFCButton		m_btnAutoModelChangeUse;
	CMFCButton		m_btnAutoProbeChangeUse;
	CMFCButton		m_btnAutoProbeAlways;

public:
	void GetAutoContactData();
	void UpdateStatus(BOOL bEnable);
	BOOL IsOfflineStatus();
	void OnAutoContactSave();
	void OnAutoContactProcStrart();
	void OnPgRecipeDownload();
	void SaveAutoCntPos(double fReady, double fPreContact, double fOD);

	void SetAutoCntRcpID(CString strID);
	void SetAutoCntProbeID(CString strID);
	void SetAutoCntSucess(BOOL bSucess);
	void SetAutoCntLog();//20200323 add by Hee

	BOOL GetAutoProcUse() { return m_bAutoCntUse; }
	BOOL GetModelChangeUse() { return m_bAutoCntModel; }
	BOOL GetProbeChangeUse() { return m_bAutoCntProbeID; }
	BOOL GetProbeAlwaysUse() { return m_bAutoCntAlways; }

	double GetReadyPos() { return m_fAutoCntReady; }
	double GetPreContactPos() { return m_fAutoCntContactPos; }
	double GetODPos() { return m_fAutoCntODPos; }

	CString GetRcpID() { return m_strAutoCntRecipeID; }
	CString GetProbeID() { return m_strAutoCnttProbeID; }
	BOOL	GetSucessState() { return m_bAutoCntSucess; }

	void AutoCntPosReset();

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnStop();
	afx_msg void OnRcpOpen();

	afx_msg void OnAutoContactUse();
	afx_msg void OnModelChangeUse();
	afx_msg void OnProbeChangeUse();
	afx_msg void OnAlwaysUse();


	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
