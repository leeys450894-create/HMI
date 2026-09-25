#pragma once
// LYS 2026-09-23 MC OFF 시 Probe Z 긴급 후퇴 - IT_8.6G_HBS(dMainForm) CMotionProbeToMcOffMove 이식.
//   MC 101(EQ MOTION, PLC_BIT_MC_CONTACT_01 = M4080) 이 ON->OFF 로 바뀌거나 감시 Z 축의 앰프 Enable 이 떨어지는
//   그 순간(에지) 한 번, 컨택 중(> 0.1)인 Probe Z 를 MoveToPos(인터락·완료대기 없는 ACS 직행 지령)로 후퇴 위치에 보낸다.
//   목표 도달은 보장하지 않는다 - 드라이브 전원이 끊기기 전까지 '최대한' 빠지는 것이 목적이다(Phase 0 결정 2).
//   원본과 다른 점(Phase 0 결정): 감시 축은 OCP 에서 bUse 인 Probe Z(=Z5) 만, 앰프는 그 Z 축 자신을 본다,
//   MC OFF 1초 뒤 전 축 Kill 은 하지 않는다(중알람 PLC_MC_OFF_ALARM_101 은 기존 AlarmMonitorToMcOFFState 가 낸다),
//   원본에서 주석이던 로그를 살려 Event\McOffMove\Probe_Move_Z.log + 메인 OCP 로그(WARN)에 남긴다.
//   설정: Config.ini [PROBE_MCOFF] USE / RETRACT_POS / PG_OFF / LOOP_MS.
//   한계: MC 비트는 PLC M 스캔(약 220~250ms) 캐시라 검출까지 0.25~0.5s 가 걸리고, 앰프 Enable 이 떨어진 뒤의 지령은
//   ACS 가 거부한다. 실제로 빠지려면 MC 101 이 끊기기까지의 시간이 검출+이동(약 0.3s)보다 길어야 한다 - 로그의 경과 ms 로 확인.
#include <vector>

class CMaintControlMgr;
class CCommTcpIpFactoryMgr;
class CLogFileAccessor;

class CMotionProbeToMcOffMove
{
	CLogFileAccessor*		m_pLogFileAccessor;
	CMaintControlMgr*		m_pMaintControlMgr;
	CCommTcpIpFactoryMgr*	m_pCommTcpIpFactoryMgr;

	std::vector<int>	m_vecAxisZ;		// 감시·후퇴 대상 Probe Z (bUse 인 축만)

	bool		m_bUse;				// Config.ini [PROBE_MCOFF] USE
	double		m_dRetractPos;		// Config.ini [PROBE_MCOFF] RETRACT_POS (mm)
	bool		m_bPgOff;			// Config.ini [PROBE_MCOFF] PG_OFF - 후퇴 뒤 PG Turn Off(enumTCPIpFactoryt_PGEvent_8) 전송
	int			m_nLoopMs;			// Config.ini [PROBE_MCOFF] LOOP_MS

	bool		m_bMCOff;			// 이전 상태
	bool		m_bEnable;			// 이전 상태
	bool		m_bFirst;			// 첫 판정은 상태만 저장한다(기동 직후 에지 오검출 방지)

private:
	CWinThread*	m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();
	void DoRetract(LPCTSTR pszTrigger);
	void Log(enumLogLevel nLevel, const CString& str);

public:
	void SetStart();
	void SetStop();

public:
	CMotionProbeToMcOffMove(CString strLogPath, CString strConfigPath, CMaintControlMgr* pMaintControlMgr, CCommTcpIpFactoryMgr* pCommTcpIpFactoryMgr);
	~CMotionProbeToMcOffMove();
};
