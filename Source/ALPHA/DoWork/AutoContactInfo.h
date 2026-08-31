// AutoContactInfo.h
// LYS 2026-08-10 AutoContact Phase 2: 프로브 오토 컨택 파라미터/상태/플래그의 비UI 저장소.
// 영속 계약 = Config.ini [PROBE_AUTO_CONTACT] (OCP와 동일 키, 다이얼로그가 기록).
// 시퀀스는 다이얼로그를 건드리면 안 된다(OCP JOB_Proc는 직접 읽었으나 HMI는 금지).
// 싱글톤: 다이얼로그 UI + Recipe 스텝 + PreMeas Contact 스텝이 메모리 사본 하나를
// 공유하고, 디스크상의 단일 진실은 INI 파일이다.
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

	// LYS 2026-08-10 Phase 4: 시퀀스 Contact 스텝과 다이얼로그 START 워커가 공유하는
	// 컨택 루틴(OCP DoWorkAutoContact / DataMove 이식).
	// 반환: VTM_RV_SUCCESSED / PROBE_AUTO_CNT_* 알람 id / 모션 에러 / VTM_RV_STOP_BY_USER.
	// fnStopCheck()가 true면 중단(시퀀스 일시정지/정지 훅). 다이얼로그 STOP은 RequestStop() 사용.
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
