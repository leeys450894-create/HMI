#pragma once

#include "..\AuroraSubMgr.h"
#include "ShareMemFunction\ShareMemFunction.h"
#include <afxmt.h>

class CIniHelper;
class CShareMemFunction;
class CShareMemoryMgr : public CAuroraSubMgr
{
    CIniHelper			*m_pIniHelper;
	bool				m_bUse;				// 장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	bool				m_bOutOfService;	// 모듈 사용 여부 0: USE
	CString				m_strControlType;
    CString				m_strAppPath;
	CShareMemFunction	*m_pFunction;
	CCriticalSection	m_cs_set;
	CCriticalSection	m_cs_get;
	
public:
    VTM_SYSTEM *GetSharedMemory();
    HWND		GetMetrosysViewWnd();
    void		SetMetrosysViewWnd(HWND hWnd);
    void		SetSys_AlarmStatus(int nState);
    int			GetSys_AlarmStatus();
    void		SetSys_ProcessStatus(int nState);
    int			GetSys_ProcessStatus();
    void		SetSys_UserPermit(char *szPermit);
    CString		GetSys_UserPermit();
    void		SetSys_User(char *szUser);
    CString		GetSys_User();
    int			GetSys_UserNo();
    void		SetSys_UserNo(int nNoUser);
    void		SetSys_MachineMode(int nMode);
    int			GetSys_MachineMode();
    void		SetSys_MetrosysVersion(char *szMetrosysVersion);
    CString		GetSys_MetrosysVersion();
    CString     GetSys_AppPath();
    void		SetSys_MachineType(int nType);
    int			GetSys_MachineType();
    void		SetSys_Validation(bool bOn);
    bool		GetSys_Validation();
    void		SetSys_GlassJudge(bool bOn);
    bool		GetSys_GlassJudge();
    void		SetSys_AutoAgingMode(bool bOn);
    bool		GetSys_AutoAgingMode();
    void		SetSys_NewAgingMode(bool bOn);	// LYS 2026-09-28 EFA
    bool		GetSys_NewAgingMode();
	void		SetSys_LampTimeOver(int index, bool bOn);
	bool		GetSys_LampTimeOver(int index);
	void		SetSys_TemperOver(int index, bool bOn);
	bool		GetSys_TemperOver(int index);
	void		SetSys_ProbeChange(int nType);
	int			GetSys_ProbeChange();

	// LYS 2026-09-25 설비 내 프로브 ID(PLC D2110~D2150 = P1~P5) 접근자. iProbeUnit 은 1..5 (MaintControlMgr.h _PROBE_TYPE_SET, 5 = MEAS/P5).
	//   m_szPROBEID[5] / stProbeInfo[5] 는 공유메모리 계약이라 배열을 줄이지 않고, 인덱스 계산(unit-1)과 범위 검사를 여기로 모은다.
	//   OCP 에서는 ProbeACC 스캐너가 P1~P4 슬롯을 비워 두므로 P5 외 요청은 자연히 빈 문자열이 된다.
	enum { EQ_PROBE_UNIT_MIN = 1, EQ_PROBE_UNIT_MAX = 5, EQ_PROBE_UNIT_MEAS = 5 };
	CString		GetEqProbeID(int iProbeUnit);					// Trim 된 ID, 범위 밖/미설정이면 ""
	bool		SetEqProbeID(int iProbeUnit, LPCTSTR pszID);
	CString		GetEqProbeMountTime(int iProbeUnit);
	UINT		GetEqProbeUseCnt(int iProbeUnit);
	CString		GetEqProbeAlignRecipeName(int iProbeUnit);	// PROBE_ALIGNn[_앞2자] / PROBE_MEAS[_앞2자] (구 5벌 switch 와 동일 규칙)

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
	CShareMemoryMgr(CString strAppPath, CString strConfigPath);
	virtual ~CShareMemoryMgr();
};

