#include "pch.h"
#include "CMotionProbeToMcOffMove.h"
#include "MaintControlMgr.h"
#include "MotorControlMgr.h"
#include "IOMapControlMgr.h"
#include "IOMapAddressDefine.h"
#include "CommTcpIpFactoryMgr.h"
#include "CommTcpIpDefine.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// MotorControlMgr.cpp 의 s_dProbeZContactPos(IsProbeZContact) 와 같은 값 - 이보다 아래(+)면 컨택 중으로 본다.
static const double s_dMcOffProbeZContactPos = 0.1;

CMotionProbeToMcOffMove::CMotionProbeToMcOffMove(CString strLogPath, CString strConfigPath, CMaintControlMgr* pMaintControlMgr, CCommTcpIpFactoryMgr* pCommTcpIpFactoryMgr)
	: m_pLogFileAccessor(new CLogFileAccessor)
	, m_pMaintControlMgr(pMaintControlMgr)
	, m_pCommTcpIpFactoryMgr(pCommTcpIpFactoryMgr)
	, m_bUse(true)
	, m_dRetractPos(0.0)
	, m_bPgOff(true)
	, m_nLoopMs(20)
	, m_bMCOff(false)
	, m_bEnable(false)
	, m_bFirst(true)
	, m_pThread(nullptr)
	, m_bThreadLive(false)
	, m_hStartEvent(NULL)
	, m_hCloseEvent(NULL)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\McOffMove\\");
	CString strLogFile;
	strLogFile.Format(_T("%s%s_Z.log"), strFolderPath, _T("Probe_Move"));
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// Config.ini [PROBE_MCOFF]
	CString strIni;
	strIni.Format(_T("%s\\Config.ini"), strConfigPath);
	m_bUse   = (GetPrivateProfileInt(_T("PROBE_MCOFF"), _T("USE"), 1, strIni) != 0);
	m_bPgOff = (GetPrivateProfileInt(_T("PROBE_MCOFF"), _T("PG_OFF"), 1, strIni) != 0);
	m_nLoopMs = GetPrivateProfileInt(_T("PROBE_MCOFF"), _T("LOOP_MS"), 20, strIni);
	if (m_nLoopMs < 1) m_nLoopMs = 1;
	if (m_nLoopMs > 1000) m_nLoopMs = 1000;
	TCHAR szPos[64] = { 0 };
	GetPrivateProfileString(_T("PROBE_MCOFF"), _T("RETRACT_POS"), _T("0.0"), szPos, _countof(szPos), strIni);
	m_dRetractPos = _ttof(szPos);

	// 감시 축: 사용 중인 Probe Z (OCP 는 Z5 만. Z1 은 Z5 와 ACS HW34 를 공유하므로 bUse 로 걸러야 중복 지령이 없다)
	const int nZ[5] = { AXIS_PROBE_Z1, AXIS_PROBE_Z2, AXIS_PROBE_Z3, AXIS_PROBE_Z4, AXIS_PROBE_Z5 };
	CMotorControlMgr* pMotor = (m_pMaintControlMgr != nullptr) ? m_pMaintControlMgr->GetMotorControlMgrPtr() : nullptr;
	for (int i = 0; i < 5; i++)
	{
		if (pMotor == nullptr || pMotor->GetAxisProperty(nZ[i]) == nullptr) continue;
		if (pMotor->GetAxisProperty(nZ[i])->bUse != TRUE) continue;
		m_vecAxisZ.push_back(nZ[i]);
	}

	CString strLog;
	strLog.Format(_T("ProbeMcOffMove init - USE %d, RETRACT_POS %.3f, PG_OFF %d, LOOP_MS %d, Z axes %d"),
		m_bUse ? 1 : 0, m_dRetractPos, m_bPgOff ? 1 : 0, m_nLoopMs, (int)m_vecAxisZ.size());
	Log(SYST, strLog);

	//----------------------------------------------------------------------
	// Thread 생성
	//----------------------------------------------------------------------
	m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	// manual set, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_pThread = AfxBeginThread(ThreadRun, this);			// 이벤트 핸들을 만든 뒤 스레드를 만든다(원본은 순서가 반대)
}

CMotionProbeToMcOffMove::~CMotionProbeToMcOffMove()
{
	//----------------------------------------------------------------------
	// Thread 종료
	//----------------------------------------------------------------------
	if (m_hStartEvent) ResetEvent(m_hStartEvent);
	if (m_hCloseEvent) SetEvent(m_hCloseEvent);
	if (m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
	if (m_hStartEvent) CloseHandle(m_hStartEvent);
	if (m_hCloseEvent) CloseHandle(m_hCloseEvent);

	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

void CMotionProbeToMcOffMove::Log(enumLogLevel nLevel, const CString& str)
{
	if (m_pLogFileAccessor != nullptr)
		m_pLogFileAccessor->KMLOG(nLevel, str);
	CLogWrapper::GetInstance()->KMLOG(nLevel, str);	// 메인 OCP 로그에도 남긴다
}

UINT CMotionProbeToMcOffMove::ThreadRun(LPVOID p1)
{
	CMotionProbeToMcOffMove* pThis = (CMotionProbeToMcOffMove*)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			pThis->ThreadRun();
			Sleep(pThis->m_nLoopMs);
		}

		// Close Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
		{
			break;
		}

		Sleep(1);
	}

	pThis->m_bThreadLive = false;

	return 0;
}

void CMotionProbeToMcOffMove::SetStart()
{
	if (m_hStartEvent) SetEvent(m_hStartEvent);
}

void CMotionProbeToMcOffMove::SetStop()
{
	if (m_hStartEvent) ResetEvent(m_hStartEvent);
}

UINT CMotionProbeToMcOffMove::ThreadRun()
{
	if (m_bUse == false || m_pMaintControlMgr == nullptr) return 0;
	CMotorControlMgr* pMotor = m_pMaintControlMgr->GetMotorControlMgrPtr();
	CIOMapControlMgr* pIO = m_pMaintControlMgr->GetIOMapControlMgrPtr();
	if (pMotor == nullptr || pIO == nullptr) return 0;
	if (pMotor->IsOutOfService() == true) return 0;
	if (m_vecAxisZ.empty()) return 0;

	// 1) MC 101 (EQ MOTION) - PLC M 캐시. -1(통신 실패)은 판정하지 않는다.
	bool bMCOff = m_bMCOff;
	if (pIO->IsConnected() == true)
	{
		int nRet = pIO->GetParam_DIOMapMemoryBitImport(PLC_BIT_MC_CONTACT_01);
		if (nRet != -1)
			bMCOff = (nRet == 0);
	}

	// 2) 감시 Z 축 앰프 Enable (ACS 에 직접 질의). -1(핸들 없음)은 판정하지 않는다.
	bool bEnable = true;
	for (size_t i = 0; i < m_vecAxisZ.size(); i++)
	{
		long lEn = pMotor->GetAmpEnable(m_vecAxisZ[i]);
		if (lEn == 0)
			bEnable = false;
	}

	if (m_bFirst == true)
	{
		m_bMCOff = bMCOff;
		m_bEnable = bEnable;
		m_bFirst = false;
		CString strLog;
		strLog.Format(_T("ProbeMcOffMove monitor start - MC101 %s, AMP %s"), bMCOff ? _T("OFF") : _T("ON"), bEnable ? _T("ON") : _T("OFF"));
		Log(SYST, strLog);
		return 0;
	}

	// 3) 에지 검출 - 그 한 번만 후퇴한다.
	LPCTSTR pszTrigger = nullptr;
	if (bMCOff == true && m_bMCOff == false)
		pszTrigger = _T("MC101 OFF");
	else if (bEnable == false && m_bEnable == true)
		pszTrigger = _T("AMP DISABLE");

	if (pszTrigger != nullptr)
		DoRetract(pszTrigger);

	m_bMCOff = bMCOff;
	m_bEnable = bEnable;

	return 0;
}

void CMotionProbeToMcOffMove::DoRetract(LPCTSTR pszTrigger)
{
	CMotorControlMgr* pMotor = m_pMaintControlMgr->GetMotorControlMgrPtr();
	const ULONGLONG ullT0 = GetTickCount64();
	CString strLog;

	for (size_t i = 0; i < m_vecAxisZ.size(); i++)
	{
		const int iAxis = m_vecAxisZ[i];
		const double dCur = pMotor->GetOrgPos(iAxis);
		LPCTSTR pszName = (pMotor->GetAxisMapProperty(iAxis) != nullptr) ? (LPCTSTR)pMotor->GetAxisMapProperty(iAxis)->m_strAxisName : _T("?");

		if (dCur > s_dMcOffProbeZContactPos)
		{
			// 인터락·SW 리밋·완료 대기 없이 ACS 에 바로 보낸다. 앰프가 이미 꺼졌으면 ACS 가 거부한다(ret 0).
			long lRet = pMotor->MoveToPos(iAxis, m_dRetractPos, SPEED_HIGH);
			strLog.Format(_T("***** [%s] PROBE Z RETRACT - %s(nAXIS:%d) cur %.3f -> %.3f, MoveToPos ret %ld (%s), amp %ld, +%llu ms *****"),
				pszTrigger, pszName, iAxis, dCur, m_dRetractPos, lRet, (lRet != 0) ? _T("sent") : _T("REJECTED"),
				pMotor->GetAmpEnable(iAxis), GetTickCount64() - ullT0);
			Log(WARN, strLog);
		}
		else
		{
			strLog.Format(_T("[%s] %s(nAXIS:%d) already up (cur %.3f <= %.3f), no retract"), pszTrigger, pszName, iAxis, dCur, s_dMcOffProbeZContactPos);
			Log(WARN, strLog);
		}
	}

	// PG OFF (원본과 동일 - 후퇴 지령 뒤에 보낸다. Phase 0 결정 4: enumTCPIpFactoryt_PGEvent_8)
	if (m_bPgOff == true && m_pCommTcpIpFactoryMgr != nullptr)
	{
		long lPg = m_pCommTcpIpFactoryMgr->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);
		strLog.Format(_T("[%s] PG Turn Off sent (PGEvent_8) ret %ld, +%llu ms"), pszTrigger, lPg, GetTickCount64() - ullT0);
		Log(WARN, strLog);
	}
}
