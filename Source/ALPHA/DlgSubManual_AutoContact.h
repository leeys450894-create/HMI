#pragma once
#include "afxwin.h"
#include "DlgSubManual_STEP.h"
//#include "..\\EXTRN\FPSpreadSheet\spreadsheet.h"



class CAuroraMgr;	// LYS 2026-08-10 Phase 1

class CDlgSubManual_AutoContact : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_AutoContact)
	enum { IDD = IDD_DLG_SUB_MANUAL_AUTO_CONTACT };

private:
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_AutoContact(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_AutoContact();

public:
	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer
	CAuroraMgr*		m_pAuroraMgr;	// LYS 2026-08-10 Phase 1
	double			m_fAutoContactParam[8];	// LYS 2026-08-10 = AUTO_CONTACT_ITEM_MAX (local store until Phase 2 helper)
	BOOL			m_bAutoCntRunning;		// LYS 2026-08-10 Phase 4
	CComboBox		m_comboAcStick;			// LYS 2026-08-11 manual-only: stick selector
	int				m_nAcStickCnt;			// LYS 2026-08-11 combo rebuild cache
	volatile int	m_nAcSelStick;			// LYS 2026-08-11 -1=ALL, else 0-based stick
	volatile int	m_nAcProgState;			// LYS 2026-08-11 0 idle/1 move/2 search/3 done/4 fail/5 stop
	volatile BOOL	m_bAcSpsMarked = FALSE;	// LYS 2026-10-13 Hybrid Phase 7 D7-16 (a) START set SPS_CYCLETEST (hybrid runtime recipe only) - the worker guard restores SPS_IDLE on every exit
	CStatic			m_stAcSeg;				// LYS 2026-10-13 Hybrid Phase 7 D7-17 runtime segment label (created hidden, shown for hybrid recipes only)
	BOOL			m_bAcSegShown = FALSE;	// LYS 2026-10-13 Hybrid Phase 7 D7-17 label visible (1-segment recipe: stays FALSE, the label is never touched)
	bool			CheckHybridAcStart();	// LYS 2026-10-13 Hybrid Phase 7 (14)-I START gate (hybrid only): idle re-check, stick range, R-A / R-C, SPS mark
	void			UpdateAcSegLabel();		// LYS 2026-10-13 Hybrid Phase 7 D7-17 timer: segment label (reads the DLL only while idle)
	volatile int	m_nAcProgCur;
	volatile int	m_nAcProgTotal;
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
	afx_msg void OnTimer(UINT_PTR nIDEvent);	// LYS 2026-08-10 Phase 1
	static UINT AutoContactSearchThread(LPVOID pParam);	// LYS 2026-08-10 Phase 4
	static UINT PgDownloadThread(LPVOID pParam);		// LYS 2026-08-10 Phase 4
	void UpdateStickList();								// LYS 2026-08-11 manual-only


	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
};
