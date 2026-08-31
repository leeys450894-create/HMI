// AutoContactInfo.h
// LYS 2026-08-10 AutoContact Phase 2: non-UI store for probe auto-contact
// params / state / flags. Persist contract = Config.ini [PROBE_AUTO_CONTACT]
// (same keys as OCP CDlgNaviManualAutoContact; written by CDlgSubManual_AutoContact).
// The sequence must NOT touch the dialog (OCP JOB_Proc read the dialog directly
// - HMI forbids). Singleton: dialog UI + Recipe step + PreMeas Contact step
// share one in-memory copy; INI file is the single source of truth on disk.
#pragma once

#include "common\CommonFunc.h"		// AUTO_CT_*, AUTO_CONTACT_ITEM_MAX, RTEST_MAX
#include <functional>				// LYS 2026-08-10 Phase 4: stop-check callback

class CMaintControlMgr;				// LYS 2026-08-10 Phase 4
class CCommTcpIpFactoryMgr;			// LYS 2026-08-10 Phase 4

class CAutoContactInfo
{
public:
	static CAutoContactInfo* GetInstance();
	static void DelInstance();

	// Config.ini [PROBE_AUTO_CONTACT] -> all members (params + flags + state)
	void	LoadAll();
	// params (8) -> INI (dialog SAVE path)
	void	SaveParams();

	// search params (AUTO_CT_* index)
	double	GetParam(int iIdx) const;
	void	SetParam(int iIdx, double fVal);

	// use flags (persisted on set)
	bool	GetUse() const				{ return m_bUse; }
	bool	GetModelChangeUse() const	{ return m_bModelChange; }
	bool	GetProbeChangeUse() const	{ return m_bProbeChange; }
	bool	GetAlwaysUse() const		{ return m_bAlways; }
	void	SetUse(bool bUse);
	void	SetModelChangeUse(bool bUse);
	void	SetProbeChangeUse(bool bUse);
	void	SetAlwaysUse(bool bUse);

	// search result state (persisted on set)
	double	GetReadyPos() const			{ return m_fReady; }
	double	GetPreContactPos() const	{ return m_fPreContact; }
	double	GetODPos() const			{ return m_fOD; }
	CString	GetRcpID() const			{ return m_strRcpID; }
	CString	GetProbeID() const			{ return m_strProbeID; }
	bool	GetSuccess() const			{ return m_bSuccess; }
	void	SavePos(double fReady, double fPreContact, double fOD);
	void	SetRcpID(CString strID);
	void	SetProbeID(CString strID);
	void	SetSuccess(bool bSuccess);
	void	Reset();					// pos 0 + Success FALSE (OCP AutoCntPosReset)
	void	SaveLog();					// [ProbeID] Ready/ContactPos/OD (OCP SetAutoCntLog)

	// OCP JOB_Proc measure-start reset judgment (Always / ModelChange / ProbeChange).
	// Call after LoadAll() with current recipe id + probe card id (PLC scanner shm).
	void	CheckAndReset(CString strRecipeID, CString strProbeCardID);

	// R-Test per-cell status (Phase 3: filled from PG R-Test reply; 21=OK, 28=port fail)
	void	ClearCellStatus();
	void	SetCellStatus(int iCell, int nStatus);
	int		GetCellStatus(int iCell) const;

	// LYS 2026-08-10 Phase 4: contact routines shared by the sequence Contact
	// step and the dialog START worker (OCP DoWorkAutoContact / DataMove port).
	// Return: VTM_RV_SUCCESSED / PROBE_AUTO_CNT_* alarm id / motion error /
	// VTM_RV_STOP_BY_USER. fnStopCheck() returning true aborts (sequence
	// pause/stop hook); the dialog STOP button uses RequestStop() instead.
	int		DoSearch(CMaintControlMgr* pMaint, CCommTcpIpFactoryMgr* pComm,
					 int nCellCount, std::function<bool()> fnStopCheck = nullptr);
	int		DoDataMove(CMaintControlMgr* pMaint);
	void	RequestStop()			{ m_bStopReq = true; }
	void	ClearStopRequest()		{ m_bStopReq = false; }
	bool	IsStopRequested() const	{ return m_bStopReq; }

private:
	CAutoContactInfo();
	~CAutoContactInfo();
	static CAutoContactInfo* m_pInstance;

	CString	GetIniPath() const;			// {ConfigFolderPath}\Config.ini
	CString	GetLogPath() const;			// {ConfigFolderPath}\AutoContactLog.ini
	void	WriteIniDouble(LPCTSTR szKey, double fVal);
	void	WriteIniInt(LPCTSTR szKey, int nVal);
	void	WriteIniStr(LPCTSTR szKey, LPCTSTR szVal);

	double	m_fParam[AUTO_CONTACT_ITEM_MAX];
	bool	m_bUse;
	bool	m_bModelChange;
	bool	m_bProbeChange;
	bool	m_bAlways;
	double	m_fReady;
	double	m_fPreContact;
	double	m_fOD;
	CString	m_strRcpID;
	CString	m_strProbeID;
	bool	m_bSuccess;
	int		m_nCellStatus[RTEST_MAX];
	volatile bool	m_bStopReq;		// LYS 2026-08-10 Phase 4
};
