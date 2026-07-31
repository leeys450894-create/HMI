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
	void		SetSys_LampTimeOver(int index, bool bOn);
	bool		GetSys_LampTimeOver(int index);
	void		SetSys_TemperOver(int index, bool bOn);
	bool		GetSys_TemperOver(int index);
	void		SetSys_ProbeChange(int nType);
	int			GetSys_ProbeChange();

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

