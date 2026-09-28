#include "pch.h"
#include "DoWorkGlassMeasureStep.h"
#include "..\Manager\SystemConfigMgr.h"	// LYS 2026-06-26 OCP option
#include "CommTcpIpObjectPG.h"			// LYS 2026-06-30 add for Aging Test
#include "AgingAlarmCheck.h"			// LYS 2026-08-05 AgingSet Phase 4

#include "DoWorkGlassSequenceImpl.h"
#include "..\Manager\WadMultiMeasMgr.h"	// LYS 2026-07-23 WAD Multi Point (Phase 4)
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"

#include "MeasureSequenceItemEngine.h"
#include "..\Recipe\RecipeEditMgr.h"
//#include "RecipeData.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\PositionConfigMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\Manager\LightControlMgr.h"
#include "..\FunctionModule\FunctionModuleCentering.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"
#include "..\Manager\SeqTactTimeMgr.h"
#include "..\Manager\AlignCameraMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQState.h"

#include "common\TimeChecker.h"
#include "common/CommonFunc.h"

#define SAFE_DELETEA(p) if(p) { delete[] p; p = NULL; }

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void CDoWorkGlassMeasureStep::SetMeasureStick(int nMeasureStick)
{
	//if (m_pStInnerID != nullptr)
	//{
	//	for (int k = 0; k < m_nMeasureArea; k++)
	//		delete m_pStInnerID[k];

	//	delete[] m_pStInnerID;
	//	m_pStInnerID = NULL;
	//}

	//if (nMeasureArea > 0)
	//{
	//	m_pStInnerID = new TCHAR * [nMeasureArea];
	//	for (int i = 0; i < nMeasureArea; i++)
	//	{
	//		m_pStInnerID[i] = new TCHAR[32];
	//		memset(m_pStInnerID[i], 0x00, sizeof(m_pStInnerID[i]));			// INNER ID 저장 공간
	//	}
	//}

	m_nMeasureStick = nMeasureStick;
}

int CDoWorkGlassMeasureStep::GetMeasureStick()
{
	return m_nMeasureStick;
}

void CDoWorkGlassMeasureStep::SetMeasureArea(int nMeasureArea)
{
	//if (m_pStInnerID != nullptr)
	//{
	//	for (int k = 0; k < m_nMeasureArea; k++)
	//		delete m_pStInnerID[k];

	//	delete[] m_pStInnerID;
	//	m_pStInnerID = NULL;
	//}

	//if (nMeasureArea > 0)
	//{
	//	m_pStInnerID = new TCHAR * [nMeasureArea];
	//	for (int i = 0; i < nMeasureArea; i++)
	//	{
	//		m_pStInnerID[i] = new TCHAR[32];
	//		memset(m_pStInnerID[i], 0x00, sizeof(m_pStInnerID[i]));			// INNER ID 저장 공간
	//	}
	//}
	
	m_nMeasureArea = nMeasureArea;
}

int CDoWorkGlassMeasureStep::GetMeasureArea()
{
	return m_nMeasureArea;
}

void CDoWorkGlassMeasureStep::SetMeasureLoop(bool bLoop)
{
	m_bMeasureLoop = bLoop;
}

bool CDoWorkGlassMeasureStep::GetMeasureLoop()
{
	return m_bMeasureLoop;
}

void CDoWorkGlassMeasureStep::ResetGlassMeasAreaInfo()
{
	for (int i = 0; i < df_MAX_PROBE; i++)
	{
		stGlassMeasAreaInfo[i].bUse = false;
		stGlassMeasAreaInfo[i].nCurrentArea = -1;
		stGlassMeasAreaInfo[i].nCurrentHead = -1;
	}
}

// jsyoon 2022.09.29 add Cell ID
int CDoWorkGlassMeasureStep::MakeCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID, int iRotate, int iStick, int nMaxStick)
{
	int nMeasStick = 0, nCellNum = 0;
	
	CString strCellId;
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));

	for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
	{
		if (pbMeasProbe[iProbe] == true)
		{
			nCellNum = pnCellNum[iProbe]; 

			if (iRotate == 1) // YY, BT
			{
				strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
				nMeasStick = iStick;
			}
			else // DG
			{
				strCellId.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, nCellNum + 1);
				nMeasStick = nMaxStick - iStick - 1;
			}

			_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
		}
	}

	memcpy(ppcCellID, cCellID, sizeof(cCellID));

	return nMeasStick;
}

void CDoWorkGlassMeasureStep::GenMeasID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID)
{
	// Generate CELL ID
	GenCellID(pnCellNum, pbMeasProbe, ppcCellID);

	// Generate InnerID
	GenInnerID(pnCellNum, pbMeasProbe);
}


void CDoWorkGlassMeasureStep::GenCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID)
{
	CString strMeasCell;
	strMeasCell.Empty();
	int iTop = 0, nMaxTop = 5;

	//2022.08.26 JCY Modify
	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	
	// jsyoon 2022.09.21 add
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
	{
		nMaxTop = pRecipeData->m_vecStick.size();
	}
	else
	{
		nMaxTop = pRecipeData->m_vecStick[0].m_vecCell.size();
	}
	//

	TCHAR cCellID[df_MAX_PROBE][32];
	memcpy(cCellID, ppcCellID, sizeof(cCellID));
	
	// Generate CELL ID
	CString strTmp, strGlassID, strCellID, strMcrID, strInnerID;

	for (size_t nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	{
		strMcrID.Empty();

		if (pbMeasProbe[nProbe])
		{
			// GlassID + CellID
			strTmp.Format(_T("%s "), cCellID[nProbe]); // stMeasGroup.stProbe[nProbe].m_CellID);
			strMeasCell += strTmp;

			// Cell ID
			strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

			// Glass Part A or B
			CString strPart;
			strPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));

			if (strPart.CompareNoCase(_T("B")) == 0)	// AB
			{
				CString strBuf;
				strBuf.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + _ttoi(strTmp.Mid(1, 2)));
				strTmp.Format(_T("%s"), strBuf);
			}
			else										// AA
			{
				;
			}
			//////////////////////////////////////////////////////////////////////////

			strCellID.Format(_T("%s%s"), strGlassID.Left(strGlassID.GetLength() - 1), strTmp);  // Glass + Cell
			strMcrID.Format(_T("%s"), gf_MakeMcrID(strCellID));								    // Glass + Cell + CheckSum
		}

		TRACE(_T("GenCellID:%s %d,%d:%s\n"), strMeasCell, pnCellNum[nProbe] + 1, nProbe + 1, strMcrID);
		_tcscpy_s(m_pMeasSeqItemEngine->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[nProbe], 32, strMcrID);		// CELL ID 설정
	}
}

void CDoWorkGlassMeasureStep::GenInnerID(int* pnCellNum, bool* pbMeasProbe)
{
	CString strInnerID;
	for (size_t nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	{
		strInnerID.Empty();

		if (pbMeasProbe[nProbe]) 
		{
			if (m_pMeasSeqItemEngine->m_pStInnerID != NULL && pnCellNum[nProbe] >= 0)
			{
				strInnerID.Format(_T("%s"), m_pMeasSeqItemEngine->m_pStInnerID[pnCellNum[nProbe]]);
				if (strInnerID.GetLength() < 5)
				{
					strInnerID.Format(_T("%d%s"), nProbe + 1, gf_GetCurTimeString(3));
					// LYS 2026-08-12 HHMMSS id는 같은 초에 충돌하므로 유일하게 만든다
					// (길이는 그대로라 장비 패킷 필드에는 영향 없음)
					strInnerID = m_pMeasSeqItemEngine->MakeUniqueInnerID(strInnerID);

					_tcscpy_s(m_pMeasSeqItemEngine->m_pStInnerID[pnCellNum[nProbe]], 32, strInnerID);
				}
			}
			else
			{
				strInnerID.Format(_T("%d%s"), nProbe + 1, gf_GetCurTimeString(3));
				strInnerID = m_pMeasSeqItemEngine->MakeUniqueInnerID(strInnerID);	// LYS 2026-08-12
			}
		}
		
		TRACE(_T("GenInnerID:%d,%d:%s\n"), pnCellNum[nProbe] + 1, nProbe + 1, strInnerID);
		_tcscpy_s(m_pMeasSeqItemEngine->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[nProbe], 32, strInnerID);
	}
}

//=========================================================================
// LYS 2026-08-12 AgingCycle R2: 에이징 반복마다 새 InnerID를 강제한다.
// GenInnerID가 셀 인덱스당 id 하나를 캐시해, 같은 셀을 N회 측정하면 같은 id ->
// 같은 장비 raw 파일명 -> TossResultFile의 CopyFile(..., FALSE)가 이전 회차를 덮어썼다.
// OCP엔 이 캐시가 없다(JOB_Proc.cpp:14116은 매번 새 id 생성).
// 앞 바이트를 0으로 만들어 GenInnerID가 길이 < 5로 보고 재생성하게 하는 방식.
//=========================================================================
void CDoWorkGlassMeasureStep::ClearInnerIDForAging(int* pnCellNum)
{
	if (m_pMeasSeqItemEngine == NULL) return;
	if (m_pMeasSeqItemEngine->IsAgingCycleActive() == false) return;
	if (m_pMeasSeqItemEngine->m_pStInnerID == NULL) return;

	for (int nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	{
		if (pnCellNum[nProbe] < 0) continue;
		if (pnCellNum[nProbe] >= m_pMeasSeqItemEngine->GetMeasureArea()) continue;

		memset(m_pMeasSeqItemEngine->m_pStInnerID[pnCellNum[nProbe]], 0x00,
			sizeof(m_pMeasSeqItemEngine->m_pStInnerID[pnCellNum[nProbe]]));
	}
}

//=========================================================================
// LYS 2026-08-18 AgingCycle: 셀 단위 측정 필터.
// 종전엔 stick 단위(IsStickUse)로 적용해, 일부 셀만 고른 맵도 stick 전체를 측정했다.
// OCP는 셀마다 마스크를 평가하므로 동일하게 맞춘다.
// 의도적 fail-open: 사이클 없음 / 맵 없음 / 맵 행이 짧음은 모두 false(측정)라 기존 경로 불변.
//=========================================================================
// LYS 2026-09-14 2Head 셀 짝짓기 - 순번 방식 (선언부 DoWorkGlassMeasureStep.h 주석 참조)
//   구 방식은 X<50 인 셀마다 A01 부터 "헤드 간격보다 멀리 떨어진 첫 셀"을 찾아 묶었다.
//   (1) 50 기준에 걸린 +측 셀(2026-09-12 레시피 J01 X=41.8)이 -측으로 분류돼 J01-T01 이 1번 그룹이 되었고
//   (2) 헤드 간격 값에 따라 짝이 한 칸씩 밀리며(L01-A01 / O01-A01 시작) 뒤쪽 셀이 단독 측정으로 남았다.
//   이제 헤드 간격은 짝 선택에 쓰지 않고, 순번으로 묶인 쌍의 간섭 검사에만 쓴다.
//=========================================================================
// LYS 2026-09-18 단독 셀 담당 헤드 선택 (부호 -> 간격). 선언부 주석 참조.
//=========================================================================
void GetHeadOffsetY(CPositionConfigMgr* pPosCfg, int nMode, double* pdPlus, double* pdMinus)
{
	if (pdPlus  != nullptr) *pdPlus  = 0.0;
	if (pdMinus != nullptr) *pdMinus = 0.0;
	if (pPosCfg == nullptr) return;

	double dX = 0.0, dY = 0.0, dZ = 0.0;
	pPosCfg->GetHeadOffset(0, nMode, &dX, &dY, &dZ);	if (pdPlus  != nullptr) *pdPlus  = dY;
	pPosCfg->GetHeadOffset(1, nMode, &dX, &dY, &dZ);	if (pdMinus != nullptr) *pdMinus = dY;
}

int PickSingleCellHead(CPositionConfigMgr* pPosCfg, CMaintControlMgr* pMaint, double dCellX,
                       double dOffPlus, double dOffMinus, bool bFrontGantry, LPCTSTR pszTag)
{
	// 구 규칙(되돌아갈 기본값)
	const int nFallback = (dCellX > 0) ? 1 : 0;
	if (pPosCfg == nullptr || pMaint == nullptr) return nFallback;
	if (pszTag == nullptr) pszTag = _T("");

	const int nAxPlus  = bFrontGantry ? AXIS_Y3 : AXIS_Y1;
	const int nAxMinus = bFrontGantry ? AXIS_Y4 : AXIS_Y2;

	double dSafetyY1 = 0.0, dSafetyY2 = 0.0;
	pPosCfg->GetSafetyPosition(nullptr, nullptr, nullptr, &dSafetyY1, &dSafetyY2, nullptr, nullptr);
	double dSafetyY3 = dSafetyY1, dSafetyY4 = dSafetyY2;
	pPosCfg->GetFrontHeadSafetyPosition(&dSafetyY3, &dSafetyY4);

	const double dSafPlus  = bFrontGantry ? dSafetyY3 : dSafetyY1;
	const double dSafMinus = bFrontGantry ? dSafetyY4 : dSafetyY2;

	// Head2 : +측 축이 셀을 맡고 -측 축이 Safety 로 빠진다
	double dGap2 = -1.0;
	const double dAct2 = dCellX + dOffPlus;
	if (dAct2 <= pMaint->GetMaxSWLimit(nAxPlus) && dAct2 >= pMaint->GetMinSWLimit(nAxPlus))
		dGap2 = fabs(dAct2 - dSafMinus);

	// Head1 : -측 축이 셀을 맡는다
	double dGap1 = -1.0;
	const double dAct1 = dCellX + dOffMinus;
	if (dAct1 <= pMaint->GetMaxSWLimit(nAxMinus) && dAct1 >= pMaint->GetMinSWLimit(nAxMinus))
		dGap1 = fabs(dAct1 - dSafPlus);

	if (dGap2 < 0.0 && dGap1 < 0.0)
	{
		CString strLog;
		strLog.Format(_T("[%s] PickHead cellX=%.3f - both heads out of SW limit (Head2 %.3f / Head1 %.3f), keep sign rule"),
			pszTag, dCellX, dAct2, dAct1);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		return nFallback;
	}

	const int nPick = (dGap2 >= dGap1) ? 1 : 0;

	// 구 규칙과 달라질 때만 남긴다(셀마다 찍으면 글래스당 수십 줄이 된다).
	if (nPick != nFallback)
	{
		CString strLog;
		strLog.Format(_T("[%s] PickHead cellX=%.3f : %s -> %s (gap Head2 %.1f / Head1 %.1f)"),
			pszTag, dCellX, nFallback ? _T("Head2") : _T("Head1"), nPick ? _T("Head2") : _T("Head1"), dGap2, dGap1);
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}

	return nPick;
}

int MakeHeadPairByOrder(const vector<S_CellInfo>& vecCell, const bool* pbUse, int nArea, double dHeadGap, int* pnPair, LPCTSTR pszTag)
{
	if (pnPair == nullptr || nArea <= 0) return 0;
	if (pszTag == nullptr) pszTag = _T("");

	for (int iCell = 0; iCell < nArea; iCell++)
		pnPair[iCell] = -1;


	vector<int> vecMinus, vecPlus;
	for (int iCell = 0; iCell < nArea && iCell < (int)vecCell.size(); iCell++)
	{
		if (pbUse != nullptr && pbUse[iCell] != true)
			continue;

		if (vecCell[iCell].m_fCellX < 0)
			vecMinus.push_back(iCell);
		else
			vecPlus.push_back(iCell);
	}

	int nMinus = (int)vecMinus.size();
	int nPlus = (int)vecPlus.size();
	int nCandidate = (nMinus < nPlus) ? nMinus : nPlus;
	int nPair = 0;
	int nGapNG = 0;	// LYS 2026-09-14
	CString strFirstNG;
	CString strLog;

	for (int k = 0; k < nCandidate; k++)
	{
		const S_CellInfo& cellMinus = vecCell[vecMinus[k]];
		const S_CellInfo& cellPlus = vecCell[vecPlus[k]];
		double dDist = fabs(cellMinus.m_fCellX - cellPlus.m_fCellX);

		if (dDist > dHeadGap)
		{
			pnPair[vecMinus[k]] = vecPlus[k];
			nPair++;
		}
		else
		{
			// LYS 2026-09-14 쌍마다 WARN 을 찍으면 단변(스틱 폭 626.98 < 헤드 간격 720, 전 쌍 미달이 정상)에서 글래스당 백여 줄이 된다.
			//   첫 건만 기억해 두었다가 아래 요약 한 줄(SYST)에 붙인다.
			//strLog.Format(_T("[%s] HeadPair NG %s(X:%.3f) - %s(X:%.3f) dist %.3f <= HeadGap %.3f, measure single"),
			//	pszTag, cellMinus.m_chCellID, cellMinus.m_fCellX, cellPlus.m_chCellID, cellPlus.m_fCellX, dDist, dHeadGap);
			//CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
			if (nGapNG == 0)
				strFirstNG.Format(_T(" firstNG %s(X:%.3f)-%s(X:%.3f) dist %.3f"), cellMinus.m_chCellID, cellMinus.m_fCellX, cellPlus.m_chCellID, cellPlus.m_fCellX, dDist);
			nGapNG++;
		}
	}

	strLog.Format(_T("[%s] HeadPair by order: minus=%d plus=%d pair=%d gapNG=%d single=%d HeadGap=%.3f%s"),
		pszTag, nMinus, nPlus, nPair, nGapNG, nMinus + nPlus - nPair * 2, dHeadGap, (LPCTSTR)strFirstNG);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

	return nPair;
}

// LYS 2026-09-14 쉬는 헤드 표시 (선언부 DoWorkGlassMeasureStep.h 주석 참조).
//   가짜 목표는 dLeft/dRight(그리고 dLeft2/dRight2)에 m_fCellX 를 그대로 대입해 만든 값이라 같은 식으로 계산하면 정확히 일치한다.
//   nMeasArea==1 분기처럼 +-gap/2 로 덮어쓴 축은 값이 달라 건드리지 않는다.
int MarkIdleHeadSafety(stMotorPosition* pPos, int nAxis, const S_MeasPos& measPos, double dShiftX)
{
	if (pPos == nullptr) return 0;

	int nMark = 0;
	for (int iSlot = 0; iSlot < 2; iSlot++)
	{
		if (measPos.m_sCellInfo[iSlot].m_nArea >= 0)
			continue;

		double dDummy = measPos.m_sCellInfo[iSlot].m_fCellX + dShiftX;
		for (int k = 0; k < nAxis; k++)
		{
			int iAxis = pPos[k].axis;
			if (iAxis != AXIS_Y1 && iAxis != AXIS_Y2 && iAxis != AXIS_Y3 && iAxis != AXIS_Y4)
				continue;
			if (fabs(pPos[k].dPosition - dDummy) > 1.0e-6)
				continue;

			pPos[k].dPosition = df_HEAD_Y_IDLE_SAFETY;
			nMark++;
		}
	}
	return nMark;
}

bool CDoWorkGlassMeasureStep::IsAgingCellSkip(int iKind, int iCell)
{
	if (m_pMeasSeqItemEngine == NULL) return false;
	if (m_pMeasSeqItemEngine->IsAgingCycleActive() == false) return false;

	CAgingCycleInfo* pInfo = m_pMeasSeqItemEngine->GetAgingCycleInfo();
	if (pInfo == NULL) return false;
	if (pInfo->GetStickCount() <= 0) return false;

	int iStick = GetMeasureStick();
	if (iCell < 0 || iCell >= pInfo->GetCellCount(iStick)) return false;

	return (pInfo->IsCellUse(iStick, iCell, iKind) == false);
}

void CDoWorkGlassMeasureStep::LogAgingRow(int iKind, const TCHAR* szTag, int* pnAreaNum)
{
	if (m_pMeasSeqItemEngine == NULL) return;
	if (m_pMeasSeqItemEngine->IsAgingCycleActive() == false) return;

	CString strLog;
	strLog.Format(_T("AgingCycle row: %s item(%d) repeat(%d) stick(%d) row(%d,%d)"),
		szTag, m_pMeasSeqItemEngine->GetAgingCurItem() + 1,
		m_pMeasSeqItemEngine->GetAgingRepeat(iKind), GetMeasureStick(),
		pnAreaNum[0], pnAreaNum[1]);
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
}

bool CDoWorkGlassMeasureStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

 //   if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
  //  	return true;

	//m_pShareItem->SetFail(false);
 //   m_pShareItem->SetComplete(false);
	//m_pShareItem->SetStopUser(false);

//	SetMeasureArea(0);
	SetMeasureLoop(true);


	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int iStick = 0, nArea = 0;
	iStick = GetMeasureStick();
	nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();

	m_pMeasSeqItemEngine->SetMeasureArea(nArea);

	m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, true); // jsyoon 2022.05.26 add

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_None End."));
	
	return true;
}

bool CDoWorkGlassMeasureStep_CheckMeasureComplete::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_CheckMeasureComplete Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
		return true;

	int nRecipeMeasAreaCount = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasAreaCount();

	//결과 대기 후 이동
	if (0 != GetMeasureArea())
	{

	}

	if (GetMeasureArea() == nRecipeMeasAreaCount)
	{
		SetMeasureLoop(false);
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_CheckMeasureComplete SetMeasureLoop False."));
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_CheckMeasureComplete End."));

	return true;
}

bool CDoWorkGlassMeasureStep_SetMeasureArea::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_SetMeasureArea Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
		return true;

	m_nMeasureArea++;

	ResetGlassMeasAreaInfo();

	if (m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeOptionParam.nUseMeasHead == 0)
	{
		//Multi Heaed
		for (int i = 0; i < m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItemCount(); i++)
		{
			if (m_nMeasureArea == m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasArea(i)->nMultiHeadMeasArea)
			{
				int nMeasHead = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasArea(i)->iMeasHeadID_linked;

				stGlassMeasAreaInfo[nMeasHead].bUse = true;
				stGlassMeasAreaInfo[nMeasHead].nCurrentArea = m_nMeasureArea;
				stGlassMeasAreaInfo[nMeasHead].stGlassMeasInfo = *m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasArea(m_nMeasureArea);
			}
		}
	}
	else if (m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeOptionParam.nUseMeasHead == 1)
	{
		//Head 1
		stGlassMeasAreaInfo[0].bUse = true;
		stGlassMeasAreaInfo[0].nCurrentArea = m_nMeasureArea;
		stGlassMeasAreaInfo[0].stGlassMeasInfo = *m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasArea(m_nMeasureArea);
	}
	else if (m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeOptionParam.nUseMeasHead == 2)
	{
		//Head 2
		stGlassMeasAreaInfo[1].bUse = true;
		stGlassMeasAreaInfo[1].nCurrentArea = m_nMeasureArea;
		stGlassMeasAreaInfo[1].stGlassMeasInfo = *m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeMeasArea(m_nMeasureArea);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_SetMeasureArea End."));

	return true;
}

bool CDoWorkGlassMeasureStep_MoveMeasureArea::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_MoveMeasureArea Start."));

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	int iStick = 0, nArea = 0;
	iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_PROC, true, 0);

	long nRet = df_MOTION_RV_SUCCESSED;

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

	S_RecipeMap* pRecipeMap;
	pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();

	double dOffsetX = 0., dOffsetR = 0.;
	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	dOffsetX = pRecipeInfo->m_sMotionOffsetPos.m_fStageY;
	dOffsetR = pRecipeInfo->m_sMotionOffsetPos.m_fStageRotate;	// jsyoon 2022.08.26 add

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK, true, 0);

	// Interlock
	bool bRet = false;
	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP1, true, 0);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();
		if (bRet == true)
		{
			nRet = PLC_ARM_SENSOR_DETECT;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP1, false, 0);
	}
	
	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP2, true, 0);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();
		if (bRet != true)
		{
			nRet = PLC_LOADER_SHUTTER_UP_ERROR;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP2, false, 0);
	}

	// jsyoon 2022.05.12 add
	bool bUseMeas = false;
	if (pRecipeInfo != nullptr)
	{
		if (pRecipeInfo->m_sInspectItem.m_bUseSpec_3 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseQuality_4 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseColor_5 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseAngle_6 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseTSP_7 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseLCR_8 == TRUE)
			bUseMeas = true;
	}

	// jsyoon 2022.06.13 add
	bool bUserStop = false;
	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bUserStop = true;
		nRet = VTM_RV_STOP_BY_USER;
	}
	///

	if (bUserStop == false && bUseMeas == true)
	{
		int iLampIntensity[2];
		ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));

		if (pRecipeInfo != nullptr)
		{
			iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
			iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		}

		// Set CCD and Light
		int iCh[4] = { 3, 2, 1, 0 };
		for (size_t i = 0; i < 2; i++)
		{
			m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
		}

		bool bMoveStage = true;
		int nCell = 0;
		double dStageY = 0, dStageR = 0, dCurY = 0., dCurR = 0.;
		if (nRet == VTM_RV_SUCCESSED)
		{
			if (iStick < pRecipeData->m_vecStick.size())
			{
				nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size();

				if (iStick > 0)
				{
					dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[iStick].m_fStageY  + dOffsetX;
					if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
						dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;		// LYS 2026-06-29
				}
				else
				{
					dStageY = pRecipeMap->m_fCellDistance.y * iStick + dOffsetX;
					if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
						dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
				}
				
			//	dStageR = pRecipeData->m_vecStick[iStick].m_fStageR;
				dStageR = pRecipeData->m_vecStick[iStick].m_fStageR + dOffsetR;	// jsyoon 2022.08.26 change

				dCurY = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X);
				dCurR = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_LOADER_R);

				if (fabs(dStageY - dCurY) < 0.0001 && fabs(dStageR - dCurR) < 0.0001)
				{
					bMoveStage = false;
				}
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP3, true, 0);

			if (bMoveStage == true)
			{
				// Probe UnContact
				if (nRet == df_MOTION_RV_SUCCESSED)
				{
					bool bMove = false;
					double dPosProbe = 0;
					// LYS 2026-06-26 OCP mode: skip P1-P4 Z check, only P5
					if (!(m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()))
					{
						for (int i = AXIS_PROBE_Z1; i < AXIS_PROBE_Z5; i++)
						{
							dPosProbe = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(i);
							if (dPosProbe > 0.1)
							{
								bMove = true;
								break;
							}
						}
					}

					double dfMove = 0.;
					dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
					if (bMove == true)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);
					}
					else
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_PROBE_Z5, dfMove);
					}

					/*stMotorPosition stPositionZ[5];
					ZeroMemory(&stPositionZ, sizeof(stPositionZ));
					stPositionZ[0].axis = AXIS_PROBE_Z1;
					stPositionZ[1].axis = AXIS_PROBE_Z2;
					stPositionZ[2].axis = AXIS_PROBE_Z3;
					stPositionZ[3].axis = AXIS_PROBE_Z4;
					stPositionZ[4].axis = AXIS_PROBE_Z5;
					
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionZ, 5, SPEED_HIGH, true);*/
				}

				if (nRet == VTM_RV_SUCCESSED)
				{
//					Sleep(2000);
					//for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
					//{
					//	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
					//	if (bRet != true)
					//	{
					//		nRet = MOTION_INTERLOCK_6;
					//		break;
					//	}
					//}
				}
			}
			else
			{
				bool bMove = false;
				// LYS 2026-06-26 OCP: skip P1-P4 Z check
				if (!(m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()))
				for (int i = AXIS_PROBE_Z1; i < AXIS_PROBE_Z5; i++)
				{
					if (m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(i) > 0.1)
					{
						bMove = true;
						break;
					}
				}

				if (bMove == true)
				{
					double dfMove = 0.;
					dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);	// 2023.08.31 change
				}
			}
			
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK_STEP3, false, 0);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_CHK, false, 0);

		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE, true, 0);

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				double dZPos = 0.;
				dZPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_Z2);
				if (dZPos > 0)
				{
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z2, 0, false, SPEED_HIGH);
				}
			}

			// Probe 이동
			double dProbeShift = 0., dProbePos = 0.;
			dProbeShift = pRecipeInfo->m_sMotionOffsetPos.m_fProbeShift;
			dProbePos = pRecipeData->m_vecStick[0].m_fStageY + dProbeShift + dOffsetX;
			// LYS 2026-06-26 OCP: skip Probe X move (no X axis)
			if (!(m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()))
			if (fabs(m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - dProbePos) > 0.1)
			{
				// Probe X
				stMotorPosition stPositionProbeX[5];
				ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));
				m_pShareItem->GetMaintControlMgrPtr()->GetCmdPos_ProbeX(stPositionProbeX, df_PROBE_MEAS, dProbePos, 190, FALSE);

				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
			}

			// Review Move
			double dLeft = 0., dRight = 0.;

			double dLeftReview = 0., dRightReview = 0.;
			S_RecipeInfo* pRecipeInfo;
			pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
			dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft;
			dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;

			if (pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX > pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX)
			{
				dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
				dRight = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
			}
			else
			{
				dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
				dRight = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
			}

			// jsyoon 2022.05.12 add
			dLeft += dLeftReview;
			dRight += dRightReview;

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));
				stPosition[0].axis = AXIS_Y1;   nAxis++;
				stPosition[0].dPosition = dLeft;
				stPosition[1].axis = AXIS_Y2;   nAxis++;
				stPosition[1].dPosition = dRight;

				// 2023.03.09 이동 동작 제거
				/*
				stPosition[2].axis = AXIS_Y3;   nAxis++;
				stPosition[2].dPosition = dLeft;
				stPosition[3].axis = AXIS_Y4;   nAxis++;
				stPosition[3].dPosition = dRight;
				// LYS 2026-06-26 OCP: X3 primary axis for ANGLE
				if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
					stPosition[4].axis = AXIS_X3;
				else
					stPosition[4].axis = AXIS_X2;
				nAxis++;
				stPosition[4].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				stPosition[5].axis = AXIS_Z1;   nAxis++;
				stPosition[5].dPosition = pRecipeData->m_sHeadZ.m_fQualityZ;
				stPosition[6].axis = AXIS_Z2;   nAxis++;
				stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
				*/
				stPosition[2].axis = AXIS_X2;   nAxis++;
				//stPosition[2].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				stPosition[2].dPosition = dOffsetX;		// LYS 2026-06-29
				// LYS 2026-09-07 Z1(BACK Z, OCP 미사용) -> Sub Z RIGHT/LEFT(Z3/Z4), 레시피 화상 Sub Z
				{
					S_RecipeInfo* pRcpSubZ = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
					stPosition[3].axis = AXIS_Z3;   nAxis++;
					stPosition[3].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzRight : 0.0;
					stPosition[4].axis = AXIS_Z4;   nAxis++;
					stPosition[4].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzLeft : 0.0;
				}
				//stPosition[4].axis = AXIS_Z2;   nAxis++;
				//stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosReview(iStick, stPosition, nAxis, SPEED_HIGH);
			}

			if (bMoveStage == true)
			{
				// LYS 2026-06-26 OCP: check P5 only
				int nProbeStart = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()) ? df_PROBE_MEAS : df_PROBE_AGING1;
				for (int i = nProbeStart; i <= df_PROBE_MEAS; i++)
				{
					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
					if (bRet != true)
					{
						nRet = MOTION_INTERLOCK_6;
						break;
					}
				}
			}

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE_STEP1, true, 0);
			if (bMoveStage == true)
			{
				//// Probe Y
				if (nRet == df_MOTION_RV_SUCCESSED)
				{
					//	dProbeY = pImpl->GetCurPos(AXIS_PROBE_Y5, 0, 0);
					double dProbeY = 0.;	// jsyoon 2022.08.16 change 

					stMotorPosition stPositionProbeY[5];
					ZeroMemory(&stPositionProbeY, sizeof(stPositionProbeY));

					stPositionProbeY[0].axis = AXIS_PROBE_Y1;
					stPositionProbeY[0].dPosition = dProbeY;
					stPositionProbeY[1].axis = AXIS_PROBE_Y2;
					stPositionProbeY[1].dPosition = dProbeY;
					stPositionProbeY[2].axis = AXIS_PROBE_Y3;
					stPositionProbeY[2].dPosition = dProbeY;
					stPositionProbeY[3].axis = AXIS_PROBE_Y4;
					stPositionProbeY[3].dPosition = dProbeY;
					stPositionProbeY[4].axis = AXIS_PROBE_Y5;
					stPositionProbeY[4].dPosition = dProbeY;

					// LYS 2026-06-26 OCP: skip Probe Y move (not used)
						if (!(m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()))
						{
							nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeY, 5, SPEED_HIGH, true);
						}
				}

				if (nRet == df_MOTION_RV_SUCCESSED)
				{
					dCurR = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_LOADER_R);
					if (fabs(dStageR - dCurR) > 0.0001)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_LOADER_R, dStageR, true, SPEED_HIGH);
					}
				}
			}
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE_STEP1, false, 0);

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE_STEP2, true, 0);
			if (bMoveStage == true)
			{
				if (nRet == df_MOTION_RV_SUCCESSED)
				{
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH);
				}
			}
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE_STEP2, true, 0);


			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_MOVE, false, 0);
		}
	}
	else
	{
		Sleep(500);	// 2023.05.10 change 1000 to 500
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_MoveMeasureArea End."));

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_End, NULL);


	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	if (nRet == df_MOTION_RV_SUCCESSED)
	{
		return true;
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
		return false;
	}
		



//	stRecipeUTGParam    *pstRecipeUTG = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeUTG;
//
//	double dAlignToMeasGap = 0;
//	m_pShareItem->GetPositionConfigMgrPtr()->GetAlignToMeasGap(&dAlignToMeasGap);
//
//	long nRet = df_MOTION_RV_SUCCESSED;
//	int nMeasHead = 0, nMeasMode = 0;
//	nMeasHead = pstRecipeUTG->nUseMeasHead;
//	nMeasMode = pstRecipeUTG->nMeasMode;
//
//	int nUseAxis = 0, iMeasHead = -1;
//	double dPosZ = 0;
//	stMotorPosition stPosition[5];
//	ZeroMemory(&stPosition, sizeof(stPosition));
//	stMotorPosition stPosition2[5];
//	ZeroMemory(&stPosition2, sizeof(stPosition2));
//
//	float nFValuePos = 0, nFValueStartPos = 0, nTriggerCount = 0;
//	nFValuePos		= pstRecipeUTG->stTriggerParam.dMeseRange;
//	nFValueStartPos = pstRecipeUTG->stTriggerParam.dStartPos + (nFValuePos / 2);
//
//	if(nMeasMode == 1 || nMeasMode == 3)
//		pstRecipeUTG->stCoordMeasPos[0].dx = dAlignToMeasGap - (pstRecipeUTG->dCellHWCenter + pstRecipeUTG->stTriggerParam.dMeasureReadyPos + (nFValuePos / 2));
//	else
//		pstRecipeUTG->stCoordMeasPos[0].dx = dAlignToMeasGap + pstRecipeUTG->dCellHWCenter - (pstRecipeUTG->stTriggerParam.dMeasureReadyPos + (nFValuePos / 2));
//
//	if (pstRecipeUTG->stTriggerParam.dTriggerPitch > 0)
//	{
//		nTriggerCount = (nFValuePos / (pstRecipeUTG->stTriggerParam.dTriggerPitch / 1000));
//	}
//	else
//	{
//		nTriggerCount = 200;
//	}
//
//	pstRecipeUTG->stTriggerParam.iTriggerCount = (int)nTriggerCount;
//
//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_MoveMeasureArea Moving Pos."));
//	if (nRet == df_MOTION_RV_SUCCESSED)
//	{
//		switch (nMeasHead)
//		{
//		case 0:
//			iMeasHead = -1;
//		break;
//		case 1:
//			iMeasHead = 0;
//			break;
//		case 2:
//			iMeasHead = 1;
//			break;
//		default:
//			break;
//		}
//
//	//	CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->MovePosition(iMeasHead, dPosZ);
//
//		switch (nMeasHead)
//		{
//		case 0:
//		{
//			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, pstRecipeUTG->stCoordMeasPos[0].dx, false, SPEED_HIGH, -1, -1);
//			/*if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nRet = m_pShareItem->GetMotorControlMgrPtr()->DoWork_MotionDone(AXIS_X);
//			}*/
//
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nUseAxis = 2;
//				stPosition[0].axis = AXIS_Y1;
//				stPosition[0].dPosition = pstRecipeUTG->stCoordMeasPos[0].dy;
//				stPosition[1].axis = AXIS_Z2;
//				stPosition[1].dPosition = pstRecipeUTG->stCoordMeasPos[1].dz + 0.03;
//				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPosition, nUseAxis, SPEED_HIGH);
//			}
//
//			// Z
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				Sleep(500);
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z1, pstRecipeUTG->stCoordMeasPos[0].dz - 0.03, false, SPEED_HIGH, -1, -1);
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Y2, pstRecipeUTG->stCoordMeasPos[1].dy, false, SPEED_HIGH, -1, -1);
//			}
//		}
//			break;
//		case 1:
//		{
//			//nUseAxis = 2;
//			//stPosition[0].axis = AXIS_X;
//			//stPosition[0].dPosition = pstRecipeUTG->stCoordMeasPos[0].dx;
//			//stPosition[1].axis = AXIS_Y1;
//			//stPosition[1].dPosition = pstRecipeUTG->stCoordMeasPos[0].dy;
//
//			//nRet &= m_pShareItem->GetMotorControlMgrPtr()->MoveCmdPosMulti(stPosition, nUseAxis, SPEED_HIGH);
//			//if (nRet == df_MOTION_RV_SUCCESSED)
//			//{
//			//	nRet = m_pShareItem->GetMotorControlMgrPtr()->DoWork_MotionDone(AXIS_ALL);
//			//}
//
//			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, pstRecipeUTG->stCoordMeasPos[0].dx, false, SPEED_HIGH, -1, -1);
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_X);
//			}
//
//			// Z
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Y1, pstRecipeUTG->stCoordMeasPos[0].dy, false, SPEED_HIGH, -1, -1);
//				Sleep(500);
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z1, pstRecipeUTG->stCoordMeasPos[0].dz - 0.03, false, SPEED_HIGH, -1, -1);
//			}
//		}
//			break;
//		case 2:
//		{
//			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, pstRecipeUTG->stCoordMeasPos[0].dx, false, SPEED_HIGH, -1, -1);
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_X);
//			}
//
//			if (nRet == df_MOTION_RV_SUCCESSED)
//			{
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z2, pstRecipeUTG->stCoordMeasPos[1].dz + 0.03, false, SPEED_HIGH, -1, -1);
//				Sleep(500);
//				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Y2, pstRecipeUTG->stCoordMeasPos[1].dy, false, SPEED_HIGH, -1, -1);
//			}
//		}
//			break;
//		default:
//			break;
//		}
//	}
//
//	// Trigger Set
//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_MoveMeasureArea Trigger Set."));
//
////	double dAlignToMeasGap = 0;
////	m_pShareItem->GetPositionConfigMgrPtr()->GetAlignToMeasGap(&dAlignToMeasGap);
//
//	double dHomeVal = 0;
//	dHomeVal = m_pShareItem->GetMaintControlMgrPtr()->GetHomeVel(AXIS_X);
//
//	WORD nwValueOperating = 1, nwValueDirection = 0, nwValueTriggerMode = 0;
//	float nFValueEncoder = 0, nFValueEndPos = 0, nFValueTriggerPitch = 0;
//	CString strVal;
//	float nFValueOffset[2];
//
//	nFValueEncoder = pstRecipeUTG->stTriggerParam.dEncoderResulution;
//	nFValuePos = pstRecipeUTG->stTriggerParam.dMeseRange;
//
//	if (nMeasMode == 1 || nMeasMode == 3)
//	{
//		nFValueStartPos = dAlignToMeasGap - (pstRecipeUTG->dCellHWCenter + (nFValuePos / 2)) - dHomeVal;
//	}
//	else
//	{
//		nFValueStartPos = dAlignToMeasGap + pstRecipeUTG->dCellHWCenter - (nFValuePos / 2) - dHomeVal;
//	}
//
//	nFValueEndPos = nFValueStartPos + nFValuePos;
//
//	nFValueOffset[0] = 0;
//	nFValueOffset[1] = pstRecipeUTG->stTriggerParam.dTriggerOffset;
//
//	nFValueTriggerPitch = pstRecipeUTG->stTriggerParam.dTriggerPitch;
//
//	// Set Trigger Count
//	float nFValueTriggerCount = 0;
//
//	if (nFValueStartPos > nFValueEndPos)
//	{
//		nwValueDirection = 0;
//		nFValueTriggerCount = (nFValueStartPos - nFValueEndPos) / (nFValueTriggerPitch / 1000) + 0.5;
//	}
//	else
//	{
//		nwValueDirection = 1;
//		nFValueTriggerCount = (nFValueEndPos - nFValueStartPos) / (nFValueTriggerPitch / 1000) + 0.5;
//	}
//
//	if (nRet == df_MOTION_RV_SUCCESSED)
//	{
//		nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_ALL);
//	}
//
//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_End, NULL);
//
//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_MoveMeasureArea End."));
//
//	if (nRet != df_MOTION_RV_SUCCESSED)
//	{
//		//error
//		m_pShareItem->SetFail(true);
//		return false;
//	}
//
	return true;
}

bool CDoWorkGlassMeasureStep_PtrnSearch::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_PATTERN_SEARCH_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_PtrnSearch Start."));

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	int iStick = GetMeasureStick();

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_SEARCH_PTRN, true, 0);

	long nRet = df_MOTION_RV_SUCCESSED;
	bool bRet = false;
	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	// jsyoon 2022.05.12 add
	bool bUseMeas = false;
	if (pRecipeInfo != nullptr)
	{
		if (pRecipeInfo->m_sInspectItem.m_bUseSpec_3 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseQuality_4 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseColor_5 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseAngle_6 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseTSP_7 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseLCR_8 == TRUE)
			bUseMeas = true;
	}

	if (bUseMeas == true)
	{
		bUseMeas = false;

		if (pRecipeInfo != nullptr)
		{
			bUseMeas = pRecipeInfo->m_sOptionItem.m_bUseReviewSnap;
		}

		int iLampIntensity[2];
		int iExposureTime[2];
		int iCCDGain[2];

		ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
		ZeroMemory(&iExposureTime, sizeof(iExposureTime));
		ZeroMemory(&iCCDGain, sizeof(iCCDGain));

		if (pRecipeInfo != nullptr)
		{
			//iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontRight; 
			//iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
			iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
			iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		}

		S_Align* pAlignInfo = nullptr;
		pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;
		if (pAlignInfo != nullptr)
		{
			iExposureTime[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nExposureTime;
			iExposureTime[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nExposureTime;

			iCCDGain[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nGain;
			iCCDGain[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nGain;
		}

		// Set CCD and Light
		int iCh[4] = { 3, 2, 1, 0 };

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_START, NULL);

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

		for (int i = 0; i < 2; i++)
		{
			m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraExposureTime_ms(iExposureTime[i]);
			m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraGain(iCCDGain[i]);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

		Sleep(500);

		if (bUseMeas == true)
		{
			nRet = df_MOTION_RV_SUCCESSED;
			float Score = 1.0;
			float MaxSorce = -1;
			int iPtrn[2] = { -1, -1 };
			CPtrnInfo ptrnInfo[df_MAX_PTRNINFO];
			for (size_t i = 0; i < df_MAX_PTRNINFO; i++)
			{
				ptrnInfo[i].ClearInfo();
			}

//#ifdef _USE_TEST_ALIGN
//			ptrnInfo[4].m_strName.Format(_T("Test_Mark.png"));
//#endif // _USE_TEST_ALIGN

			CPtrnInfo* pPtrnInfo = NULL;
			double dPixelx[2] = { 0, }, dPixely[2] = { 0, };

			CMaintControlMgr* pImpl = nullptr;
			pImpl = m_pShareItem->GetMaintControlMgrPtr();

			bool bAlignDone = false;

			// 이미지 패턴 매칭
			int k = 4;
			for (int j = 0; j < 2; j++)	// 2 Head
			{
				MaxSorce = -1;

				if (j == 0)
				{
					ptrnInfo[0].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nMatchingScore;
					ptrnInfo[0].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontLeft[k]);
				}
				else
				{
					ptrnInfo[0].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nMatchingScore;
					ptrnInfo[0].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontRight[k]);
				}

				// 이미지 패턴 매칭
				pPtrnInfo = &ptrnInfo[0];
				nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_MEAS);
				if (nRet == VTM_RV_SUCCESSED)
				{
					if (Score > MaxSorce)
					{
						iPtrn[j] = k;
						dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
						dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
					}
				}
			}

			bAlignDone = true;

			for (int j = 0; j < 2; j++)	// 2 Head
			{
				if (iPtrn[j] < 0)
				{
					bAlignDone = false;
					break;
				}
			}

			if (bAlignDone == true)
			{
				bRet = true;
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
//			Sleep(1000);

			for (int j = 0; j < 2; j++)	// 2 Head
			{
				m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(2);
			}

			bRet = true;
		}
	}
	else
	{
		bRet = true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_PtrnSearch End."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_SEARCH_PTRN, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_PATTERN_SEARCH_End, NULL);

	if (bRet != true)
	{
		m_pShareItem->SetFail(true, nRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	return bRet;
}

bool CDoWorkGlassMeasureStep_Autofocus::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_AF_Start, NULL);
	
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Autofocus Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{
		Sleep(1000);
		return true;
	}

	stRecipeUTGParam    stRecipeUTG = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeUTG;

	long nRet = df_MOTION_RV_SUCCESSED;
	//int nMeasHead = 0, iMeasHead = -1;

	//nMeasHead = stRecipeUTG.nUseMeasHead;

	//switch (nMeasHead)
	//{
	//case 0:
	//	iMeasHead = -1;
	//	break;
	//case 1:
	//	iMeasHead = 0;
	//	break;
	//case 2:
	//	iMeasHead = 1;
	//	break;
	//default:
	//	break;
	//}

	bool bRet = true;
	//Head Info 설정 필요함
	/*int nAFCh = 0;
	for (size_t i = 0; i < 2; i++)
	{
		if (stRecipeUTG.stCoordMeasPos[i].bAF)
		{
			nAFCh = stRecipeUTG.stCoordMeasPos[i].nAFCh;
			CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->SetAFParameterOther(i, nAFCh);
			CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->StartAFOther(i, false);
		}
	}

	for (size_t i = 0; i < 2; i++)
	{
		if (stRecipeUTG.stCoordMeasPos[i].bAF)
		{
			if (CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->CheckAFDone(i) != 1)
			{
				bRet = false;
				break;
			}
		}
	}*/

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_AF_End, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Autofocus End."));

	return bRet;
}

bool CDoWorkGlassMeasureStep_PatternCentering::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_PatternCentering Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
		return true;

	//for (int i = 0; i < 2; i++)
	//{
	//	if (stGlassMeasAreaInfo[i].bUse)
	//	{
	//		CSequenceManager::GetInstance()->GetFunctionModuleCentering()->SetHead(i);
	//		
	//		CSequenceManager::GetInstance()->GetFunctionModuleCentering()->SetAlignType(1);
	//		CSequenceManager::GetInstance()->GetFunctionModuleCentering()->Start();
	//	}
	//}

	//for (int i = 0; i < 2; i++)
	//{
	//	if (stGlassMeasAreaInfo[i].bUse)
	//	{
	//		bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleCentering()->Is_Complete();
	//		if (!bRet)
	//		{
	//			return false;
	//		}
	//	}
	//}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_PatternCentering End."));

	return true;
}

bool CDoWorkGlassMeasureStep_Measure::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{
		Sleep(1000);
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure End. TEST"));

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);
		return true;
	}

	
	//stRecipeUTGParam    *pstRecipeUTG = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeUTG;

	//long nRet = df_MOTION_RV_SUCCESSED;
	//bool bRet = false;
	//int nMeasHead = 0;
	//nMeasHead = pstRecipeUTG->nUseMeasHead;

	//// Meas Start
	//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure INSP_START Set."));

	//int nMeasMode = 0;
	//nMeasMode = pstRecipeUTG->nMeasMode;
	//bool bSideOut = true;

	//if (pstRecipeUTG->stCoordMeasPos[1].nAFCh != 0)
	//	bSideOut = false;

	//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(nMeasHead, nMeasMode, bSideOut);

	//int iMeasHead = 0;
	//switch (nMeasHead)
	//{
	//case 0:
	//	iMeasHead = -1;
	//	break;
	//case 1:
	//	iMeasHead = 0;
	//	break;
	//case 2:
	//	iMeasHead = 1;
	//	break;
	//default:
	//	break;
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	//if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == true)
	//	//{
	//	//	switch (nMeasHead)
	//	//	{
	//	//	case 0:
	//	//	{
	//	//		for (size_t iCol = 0; iCol < 2; iCol++)
	//	//		{
	//	//			m_pShareItem->GetTriggerControlMgr()->SetWriteDataBIT(iCol, 1);		// START
	//	//		}
	//	//	}
	//	//	break;
	//	//	case 1:
	//	//	{
	//	//		m_pShareItem->GetTriggerControlMgr()->SetWriteDataBIT(0, 1);		// START
	//	//	}
	//	//	break;
	//	//	case 2:
	//	//	{
	//	//		m_pShareItem->GetTriggerControlMgr()->SetWriteDataBIT(1, 1);		// START
	//	//	}
	//	//	break;
	//	//	default:
	//	//		break;
	//	//	}
	//	//}

	//	// 모션 이동

	//	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == true)
	//	{
	//		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure Moving."));

	//		//double dVel, dAcc, dJerk, dMoving = 0;
	//		double dVel, dAcc, dJerk, dReady = 0, dMoving = 0, dMeseRange = 0;
	//		dVel = pstRecipeUTG->stTriggerParam.dSpeedVel;
	//		dAcc = pstRecipeUTG->stTriggerParam.dSpeedAcc;
	//		dJerk = dAcc * 10;
	//		dReady = pstRecipeUTG->stTriggerParam.dMeasureReadyPos;
	//		dMeseRange = pstRecipeUTG->stTriggerParam.dMeseRange;

	//		// jsyoon 20211019 change
	//		//dMoving = pstRecipeUTG->stTriggerParam.dMeasureReadyPos + pstRecipeUTG->stTriggerParam.dMeseRange + 2.0;  // (pstRecipeUTG->stTriggerParam.dMeasureReadyPos / 2);
	//		dMoving = dReady + dMeseRange + dReady;

	//		m_pShareItem->GetMaintControlMgrPtr()->SetSpeedDirect(AXIS_X, dVel, dAcc, dJerk, 2);

	//		CTimeChecker	timecheck_Move;
	//		//timecheck_Move.Reset();
	//		float fElapsed_ms;
	//		double dStartPos = 0, dCurPos = 0;
	//		dStartPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X);

	//		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X, dMoving, false, -1);

	//		bool bStart = false;
	//		int nLoop = 0;
	//		do
	//		{
	//			dCurPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X);

	//			if (dCurPos > dReady + dStartPos && bStart != true)
	//			{
	//				timecheck_Move.Reset();
	//				bStart = true;
	//			}
	//			else if (dCurPos > dReady + dMeseRange + dStartPos)
	//			{
	//				fElapsed_ms = timecheck_Move.Elapsed_ms();
	//				break;
	//			}
	//			else
	//			{
	//				;
	//			}

	//			Sleep(10);

	//			nLoop++;

	//			if (nLoop > 1000)	// 10sec
	//			{
	//				fElapsed_ms = timecheck_Move.Elapsed_ms();
	//				break;
	//			}

	//		} while (true);

	//		// Meas End
	//		if (nRet == df_MOTION_RV_SUCCESSED)
	//		{
	//			nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_X);
	//		}

	//	//	const float fElapsed_ms = timecheck_Move.Elapsed_ms();

	//		if (nRet == df_MOTION_RV_SUCCESSED)
	//		{
	//			CString strLog;
	//			double dRealVel = 0;
	//		//	dRealVel = dMoving * 1000 / fElapsed_ms;
	//		//	strLog.Format(_T(">>> Measure Moving. SET SPPED(%.1f), TIME(%.3f)ms, MOVING(%.3f), SPEED(%.3f)"), dVel, fElapsed_ms, dMoving, dRealVel);

	//			dRealVel = dMeseRange * 1000 / fElapsed_ms;

	//			strLog.Format(_T(">>> Measure Moving. SPEED(%.3f), SET SPPED(%.1f), TIME(%.3f)ms, MOVING(%.3f)"), dRealVel, dVel, fElapsed_ms, dMoving);

	//			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
	//			CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
	//		}

	//		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure Moving End."));
	//	
	//		//CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->AFStop(iMeasHead);

	//		//for (size_t iCol = 0; iCol < 2; iCol++)
	//		//{
	//		//	m_pShareItem->GetTriggerControlMgr()->SetWriteDataBIT(iCol, 0);		// STOP
	//		//}
	//	}
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	// Meas Start
	//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure INSP_END Set."));

	//	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(nMeasHead);
	//}
	//
	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	bRet = true;
	//}
	//else
	//{
	//	m_pShareItem->SetFail(true);
	//}
	//
	//CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure End."));

	return true;
}

bool CDoWorkGlassMeasureStep_Safety::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_SAFETY_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Safety Start."));

	long nRet = df_MOTION_RV_SUCCESSED;

	if(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk == TRUE)
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmd_SafetyAll();
	else
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmd_SafetyAll(true);


	//for (size_t i = 0; i < MAIN_CCD_MAX_NUM; i++)
	//{
	//	m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(i, 0);
	//}

	//long nRet = df_MOTION_RV_SUCCESSED;

	//// Probe Move
	//if (nRet == df_MOTION_RV_SUCCESSED) 
	//{
	//	stMotorPosition stPositionProbe[5];
	//	stPositionProbe[0].axis = AXIS_PROBE_Z1;
	//	stPositionProbe[0].dPosition = 0;
	//	stPositionProbe[1].axis = AXIS_PROBE_Z2;
	//	stPositionProbe[1].dPosition = 0;  
	//	stPositionProbe[2].axis = AXIS_PROBE_Z3;
	//	stPositionProbe[2].dPosition = 0;
	//	stPositionProbe[3].axis = AXIS_PROBE_Z4;
	//	stPositionProbe[3].dPosition = 0;
	//	stPositionProbe[4].axis = AXIS_PROBE_Z5;
	//	stPositionProbe[4].dPosition = 0;

	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbe, 5, SPEED_HIGH, true);
	//	Sleep(1000);
	//}

	//// Auto Change Move
	//double dACXPos[3];
	//ZeroMemory(&dACXPos, sizeof(dACXPos));
	//m_pShareItem->GetPositionConfigMgrPtr()->GetACPosition(&dACXPos[0], &dACXPos[1], &dACXPos[2]);

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true, SPEED_HIGH, 0, 0);

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dACXPos[1], true, SPEED_HIGH, 0, 0);

	//// Probe X
	//double dSafetyProbe[5];
	//ZeroMemory(dSafetyProbe, sizeof(dSafetyProbe));

	//m_pShareItem->GetPositionConfigMgrPtr()->GetProbeSafetyPosition(&dSafetyProbe[0], &dSafetyProbe[1], &dSafetyProbe[2], &dSafetyProbe[3], &dSafetyProbe[4]);
	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	stMotorPosition stPositionProbeX[5];
	//	stPositionProbeX[0].axis = AXIS_PROBE_X1;
	//	stPositionProbeX[0].dPosition = dSafetyProbe[0];
	//	stPositionProbeX[1].axis = AXIS_PROBE_X3;
	//	stPositionProbeX[1].dPosition = dSafetyProbe[1];
	//	stPositionProbeX[2].axis = AXIS_PROBE_X5;
	//	stPositionProbeX[2].dPosition = dSafetyProbe[2];
	//	stPositionProbeX[3].axis = AXIS_PROBE_X7;
	//	stPositionProbeX[3].dPosition = dSafetyProbe[3];
	//	stPositionProbeX[4].axis = AXIS_PROBE_X9;
	//	stPositionProbeX[4].dPosition = dSafetyProbe[4];

	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, 1);
	//}

	//// All Axis
	//double dSafetyStageX = 0.0;
	//double dSafetyHeadX1 = 0.0;
	//double dSafetyHeadX2 = 0.0;
	//double dSafetyHeadY1 = 0.0;
	//double dSafetyHeadY2 = 0.0;
	//double dSafetyZ = 0.0;
	//m_pShareItem->GetPositionConfigMgrPtr()->GetSafetyPosition(&dSafetyStageX, &dSafetyHeadX1, &dSafetyHeadX2, &dSafetyHeadY1, &dSafetyHeadY2, &dSafetyZ);

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	stMotorPosition stPositionZ[2];
	//	stPositionZ[0].axis = AXIS_Z1;
	//	stPositionZ[0].dPosition = dSafetyZ;
	//	stPositionZ[1].axis = AXIS_Z2;
	//	stPositionZ[1].dPosition = dSafetyZ;

	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionZ, 2, SPEED_HIGH, true);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	stMotorPosition stPositionCross[4];
	//	stPositionCross[0].axis = AXIS_Y1;
	//	stPositionCross[0].dPosition = dSafetyHeadY1;
	//	stPositionCross[1].axis = AXIS_Y2;
	//	stPositionCross[1].dPosition = dSafetyHeadY2;
	//	stPositionCross[0].axis = AXIS_Y3;
	//	stPositionCross[0].dPosition = dSafetyHeadY1;
	//	stPositionCross[1].axis = AXIS_Y4;
	//	stPositionCross[1].dPosition = dSafetyHeadY2;

	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionCross, 4, SPEED_HIGH);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	stMotorPosition stPositionX[2];
	//	stPositionX[0].axis = AXIS_X2;
	//	stPositionX[0].dPosition = dSafetyHeadX1;
	//	stPositionX[1].axis = AXIS_X3;
	//	stPositionX[1].dPosition = dSafetyHeadX2;

	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionX, 2, SPEED_HIGH);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dSafetyStageX, true, SPEED_HIGH, 0, 0);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_ALL);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_LOADER_R, 0, false, SPEED_HIGH, 0, 0);
	//}

	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_LOADER_R);
	//}

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_SAFETY_End, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("CDoWorkGlassMeasureStep_Safety End."));

	if (nRet == df_MOTION_RV_SUCCESSED)
	{
		return true;
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
		return false;
	}
}

bool CDoWorkGlassMeasureStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
	ResetEvent(m_pShareItem->GetStartEventHandle());

	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료

		m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, false); // jsyoon 2022.05.26 add
	}

	

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS, true);
	
    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("CDoWorkGlassMeasureStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_START, NULL);

	return false;
}

bool CDoWorkGlassMeasureStep_Probe_Contact::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Probe_Contact Start."));

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT, true, 0);

	int nRet = VTM_RV_SUCCESSED;
	nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_X);

	int iLampIntensity[MAIN_CCD_MAX_NUM];

	ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));

	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	// jsyoon 2022.05.12 add
	bool bUseMeas = false;
	if (pRecipeInfo != nullptr)
	{
		if (pRecipeInfo->m_sInspectItem.m_bUseSpec_3 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseQuality_4 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseColor_5 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseAngle_6 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseTSP_7 == TRUE)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseLCR_8 == TRUE)
			bUseMeas = true;
	}

	if (bUseMeas == true)
	{
		//if (pRecipeInfo != nullptr)
		//{
		//	iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		//	iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
		//}

		// Set CCD and Light
		int iCh[4] = { 3, 2, 1, 0 };

		//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

		//for (size_t i = 0; i < MAIN_CCD_MAX_NUM; i++)
		//{
		//	m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
		//}

		//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

		// Probe Contact1
		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			double dfMove = 0., dEMove = 0., dOD = 0., dCurZ = 0.;
			dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
			dEMove = pRecipeInfo->m_sContactPos.m_fEndContactPos;
			dOD = pRecipeInfo->m_sContactPos.m_fAddOD;

			// LYS 2026-09-09 Stick별 PROBE Z CONTACT OFFSET(레시피 STAGE MOVE 5행, +=하강)을 End Contact에만 가산. Pre/UnContact 높이는 공통
			double dZOffset = m_pShareItem->GetRecipeEditMgrPtr()->GetStickProbeZOffset(iStick);
			dEMove += dZOffset;
			{
				CString strLog;
				strLog.Format(_T("Probe Contact stick=%d offset=%.4f -> Z5 target=%.4f (End %.4f + OD %.4f)"), iStick + 1, dZOffset, dEMove + dOD, dEMove - dZOffset, dOD);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
			}

			dCurZ = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_Z5);

			if (fabs(dCurZ - (dEMove + dOD)) > 0.001)
			{
				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z5, dfMove, dEMove, dOD);
			}

			//double dMove = 0;
			//dMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;

			//nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_PROBE_Z5, dMove, SPEED_HIGH, true);

			//// AutoContactData 나중에 추가

			//if (nRet == df_MOTION_RV_SUCCESSED)
			//{
			//	dMove = pRecipeInfo->m_sContactPos.m_fEndContactPos + pRecipeInfo->m_sContactPos.m_fAddOD;
			//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_PROBE_Z5, dMove, SPEED_LOW, true);
			//}
		}
		// 
		//Sleep(500);

		// Review Snap
		for (int j = 0; j < 2; j++)	// 2 Head
		{
			m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(3);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

		//20220516
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_STOP, NULL);
		//m_pMainAlignCCD
		//

		// Set CCD and Light
		for (size_t i = 0; i < MAIN_CCD_MAX_NUM; i++)
		{
			m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0); // SPO
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT, false, 0);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Probe_Contact End."));

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N_CONTACT_PROC, true, 0);
	}

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		return false;
	}

	bool bRet = false;
	if (nRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
	}

	

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_WAD::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 3);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_WAD, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_WAD Start."));

	//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_5);	// Turn On All		// LYS 2026-09-15 Manual TEST
	//
	//Sleep(1000);
	//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_6);	// Turn On All		// LYS 2026-09-15 Manual TEST

	bool bRet = true;
	bool bUseMeas = true;
	bool bUseMTP = false;
	bool bUseRtestNgPass = false;
	bool bUseSampling = false;
	int nUseCELLID[df_MAX_PROBE];
	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
	bool bMeasProbe[df_MAX_PROBE];
	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME;

	double dOffsetX = 0.;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseSpec_3;
		// LYS 2026-08-06 AgingCycle E3: per-item select (WAD = Property flag + stick cell map)
		if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
		{
			const S_AgingTest* pAgingItem = m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetItem(m_pMeasSeqItemEngine->GetAgingCurItem());
			bUseMeas = (pAgingItem != NULL && pAgingItem->m_bUseProperty)
				&& (m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetStickCount() == 0
					|| m_pMeasSeqItemEngine->GetAgingCycleInfo()->IsStickUse(GetMeasureStick(), enumAgingMeasKind_WAD));

			if (bUseMeas && m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
			{
				// LYS 2026-08-06 AgingCycle E4: progress marker (col 4 = WAD)
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = m_pMeasSeqItemEngine->GetAgingCurItem() + 1;
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 4;
			}
		}

		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[3];

		bUseRtestNgPass = pReciepInfo->m_sOptionItem.m_bUseRtestNgPass;	// jsyoon 2022.06.21

		bUseSampling = pReciepInfo->m_sOptionItem.m_bUseSampling;
//		bUseMTP = pReciepInfo->m_sOptionItem.;


		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;
	}

	// Check Meas
	if (bUseMeas == true)	
	{
		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nMeasArea = 0;
		double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
	//	m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, nullptr, nullptr, &dHeadGap);	// 2023.03.08 change

		dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		if (dHeadGapRcp > dHeadGap)
		{
			dHeadGap = dHeadGapRcp;
		}

		// LYS 2026-09-21 짝 판정 간격(헤드 오프셋 반영)을 함수 범위에 둔다 - 아래 그룹 생성 재검사도 이 값을 쓴다.
		//   축 목표 = 셀 X + HEAD_OFFSET 이라 두 헤드 목표 간격 = 셀 거리 + (HEAD1_WAD_Y - HEAD2_WAD_Y).
		//   인터락(INTERLOCKHEADGAPB / 레시피 BackHeadGap 중 큰 값)은 축 목표 간격을 보므로 셀 거리 기준은 dHeadGap - (o1 - o2).
		double dPairGapWad = dHeadGap;
		{
			double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
			m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 0, &dOffX, &dOffY1, &dOffZ);	// HEAD1_WAD_Y (Y1)
			m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 0, &dOffX, &dOffY2, &dOffZ);	// HEAD2_WAD_Y (Y2)
			dPairGapWad = dHeadGap - (dOffY1 - dOffY2);
		}

		double dGlassConactPos = 0;
//		dGlassConactPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - 20;

		S_RecipeData* pRecipeData;
		pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		int iStick = 0, nArea = 0, nMaxStick = 0;
		iStick = GetMeasureStick();
		nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
		nMaxStick = pRecipeData->m_vecStick.size();

		//dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;
		dGlassConactPos = pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;		// LYS 2026-06-29

		// Generate 2Head Group
		bool* pbSet = new bool[nArea];
		for (int iGroup = 0; iGroup < nArea; iGroup++)
			pbSet[iGroup] = false;

		vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
		vecMeasGroup.clear();

		S_CellInfo cellFirst, cellSecond;
		S_MeasPos measPos;
		cellFirst.Clear();
		cellSecond.Clear();

		// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (MakeHeadPairByOrder 주석 참조).
		//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 필터는 아래 루프와 같은 조건을 미리 적용한다.
		int* pnPair = new int[nArea];
		{
			bool* pbUse = new bool[nArea];
			for (int iCell = 0; iCell < nArea; iCell++)
			{
				pbUse[iCell] = true;
				if (IsAgingCellSkip(enumAgingMeasKind_WAD, iCell))
					pbUse[iCell] = false;
				if (bUseSampling == true && pRecipeData->m_vecStick[iStick].m_vecCell[iCell].m_nSelectCell[M_SPEC] != 1)
					pbUse[iCell] = false;
			}
			// LYS 2026-09-14 짝 조건에 헤드 오프셋 반영. 축 목표 = 셀 X + 오프셋이므로 두 헤드 목표 간격 = 셀 거리 + (HEAD1 - HEAD2 오프셋).
			//   인터락(dHeadGap)은 축 목표 간격을 보므로 셀 거리 조건은 dHeadGap - (o1 - o2). WAD(+168/-168)는 384, VISION(0)은 그대로,
			//   ANGLE(-272/+272)은 1264 가 되어 사실상 단독 측정으로 간다(2026-09-14 센서 배치 기준).
			double dPairGap = dHeadGap;
			{
				double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 0, &dOffX, &dOffY1, &dOffZ);
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 0, &dOffX, &dOffY2, &dOffZ);
				dPairGap = dHeadGap - (dOffY1 - dOffY2);
			}
			MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, pbUse, nArea, dPairGap, pnPair, _T("WAD"));
			delete[] pbUse;
		}

		if (nArea > 1)
		{
			for (int iGroup = 0; iGroup < nArea; iGroup++)
			{
				if (pbSet[iGroup] != true)
				{
					cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

					// LYS 2026-08-18 AgingCycle: AgingRcp [MASECELL] 기반 셀 단위 필터.
					// pbSet 표시로 아래 잔여 단일헤드 패스에서도 제외된다.
					if (IsAgingCellSkip(enumAgingMeasKind_WAD, iGroup))
					{
						pbSet[iGroup] = true;
						continue;
					}

					if (bUseSampling == true)	// jsyoon 2022.07.19 add
					{
						//	if (cellFirst.m_nSelectCell[M_SPEC] != true)
						if (cellFirst.m_nSelectCell[M_SPEC] != 1)
						{
							pbSet[iGroup] = true;
							continue;
						}
					}

					cellSecond.Clear();
					for (int iSearch = 0; iSearch < nArea; iSearch++)
					{
						if (pbSet[iSearch] != true)
						{
							cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

							// LYS 2026-08-18 AgingCycle: per-cell filter from AgingRcp [MASECELL]
							if (IsAgingCellSkip(enumAgingMeasKind_WAD, iSearch))
							{
								pbSet[iSearch] = true;
								continue;
							}

							if (bUseSampling == true)	// jsyoon 2022.07.19 add
							{
								//	if (cellSecond.m_nSelectCell[M_SPEC] != true)
								if (cellSecond.m_nSelectCell[M_SPEC] != 1)
								{
									pbSet[iSearch] = true;
									continue;
								}
							}

							// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
							//if (cellFirst.m_fCellX < 50)
							if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
							{
								// LYS 2026-09-21 재검사도 헤드 오프셋 반영 간격으로 본다. 구 조건은 셀 거리를 오프셋 없이 dHeadGap 과 비교해,
								//   MakeHeadPairByOrder 가 만든 짝(A01<->K01 셀 거리 729.77, 헤드 간격 1065.77)을 765.6 미달로 전부 깨뜨렸다
								//   -> WAD 20셀이 모두 단독 측정(2026-09-21 18:01, 로그 pair=10 인데 idle head Y2 -> SAFETY 반복).
								//if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
								if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dPairGapWad)
								{
									pbSet[iGroup] = true;
									pbSet[iSearch] = true;
									measPos.Clear();
									//measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
									//measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
									measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
									measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
									measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fSpecZ;
									measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

									if (cellFirst.m_nArea >= nArea)
									{
										cellFirst.m_nArea -= (nArea * iStick);
									}

									if (cellSecond.m_nArea >= nArea)
									{
										cellSecond.m_nArea -= (nArea * iStick);
									}

									measPos.m_sCellInfo[0] = cellFirst;
									measPos.m_sCellInfo[1] = cellSecond;


									vecMeasGroup.push_back(measPos);

									break;
								}
							}
						}
					}
				}
			}
		}

		// 선택하고 남은것
		for (int iGroup = 0; iGroup < nArea; iGroup++) 
		{
			if (pbSet[iGroup] != true) 
			{
				pbSet[iGroup] = true;

				cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

				// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
				// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0. VISION 오프셋이 0 이 되어 X=0 셀을 Y3/Y1 이 맡아도 되고,
				//   ANGLE(Y3 = X-272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3 로 보내야 쉬는 Y4 Safety(-860)와 근접센서 간격(580)이 나온다.
				//if (cellFirst.m_fCellX > 50 )  // Head 2
				// LYS 2026-09-18 담당 헤드를 셀 X 부호 대신 '갈 수 있고 쉬는 헤드와 간격이 큰 쪽'으로 고른다.
				//                구 줄은 바로 아래에 남겨 두었다(원복 시 이 블록 제거 후 주석 해제).
				//if (cellFirst.m_fCellX > 0)  // Head 2
				double dOffHeadP = 0.0, dOffHeadM = 0.0;
				GetHeadOffsetY(m_pShareItem->GetPositionConfigMgrPtr(), 0, &dOffHeadP, &dOffHeadM);
				if (PickSingleCellHead(m_pShareItem->GetPositionConfigMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(),
					cellFirst.m_fCellX, dOffHeadP, dOffHeadM, false, _T("WAD")) != 0)  // Head 2
				{
					measPos.Clear();
					//measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					//measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
					measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fSpecZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;


					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[1] = cellFirst;
					cellFirst.m_fCellX -= dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[0] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
				else  // Head 1
				{
					measPos.Clear();
					//measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					//measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
					measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fSpecZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[0] = cellFirst;
					cellFirst.m_fCellX += dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[1] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
			}
		}

		delete[] pbSet;
		delete[] pnPair;	// LYS 2026-09-14

		long nRet = df_MOTION_RV_SUCCESSED;
		nMeasArea = (int)vecMeasGroup.size();
		bool bMoveFirstDone = false;

		if (nMeasArea > 0)
		{
			int nMeasStick = 0;	// jsyoon 2022.09.29 add

			// TP 특성 설정
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_1);

			// MTP Start
			if (bUseMTP == true) { m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, enumTCPIpFactoryt_Event_WAD_3);	}

			//2023.05.22 자동 시퀀스 특성 1Cell 3점
			BOOL bOneCell = FALSE;
			int nOneCellCnt = 1;
			// LYS 2026-08-18 bOneCell(1그룹 x 5포인트 WAD 고정 스윕)은 폐기 - WAD Multi Point가 대체.
			// 분기는 남기고 1회만 돌게 해 상수 하나로 되돌릴 수 있게 했다(완전 제거는 후속 정리).
			// 주의: nMeasArea는 2Head GROUP 수이지 셀 수가 아니다.
			if (nMeasArea == 1) {
				bOneCell = TRUE;
				nOneCellCnt = 1;
			}

			// LYS 2026-07-23 WAD 다점 측정(Phase 4) - 레시피가 정의한 점들을 사용한다.
			// Multi 레시피가 활성이면 하드코딩된 1셀 5점 루프를 대체한다(D-1).
			bool bMultiWad = false;
			CWadMultiMeasMgr* pMultiMgr = m_pShareItem->GetWadMultiMeasMgrPtr();
			if (pMultiMgr != nullptr && pReciepInfo != nullptr && pReciepInfo->m_sInspectItem.m_bUseSpec_Multi == TRUE)	// LYS 2026-07-23 recipe option gate
			{
				pMultiMgr->ResetRunState();
				pMultiMgr->LoadMeasData(m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID());
				if (pMultiMgr->IsMultiMode() && pMultiMgr->GetUseSize() > 0)
				{
					bMultiWad = true;
					bOneCell = FALSE;
					nOneCellCnt = 1;
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("WAD Multi Point mode active"));
				}
			}
			//

			for (int k = 0; k < nOneCellCnt; k++)
			{
				for (int i = 0; i < nMeasArea; i++)
				{
					if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
					{
						bRet = false;
						break;
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_WAD, true, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Move"), i);

					nRet = VTM_RV_SUCCESSED;
				
					nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;

					bMeasProbe[0] = true;
					bMeasProbe[1] = true;

					if (nUseCELLID[0] < 0) bMeasProbe[0] = false; 
					if (nUseCELLID[1] < 0) bMeasProbe[1] = false;

					// Set PG Num
					int nCellNum = 0;
					int iMeasMode = 0;
					if (nRet == VTM_RV_SUCCESSED)
					{
						/*!
						* 2023.03.09
						* 위치 이동 nCellStatus 위치는 iRotate 에 따라 표시 하는 위치가 달라져서 먼저 nMeasStick 을 구하고 상태를 읽어야 함
						*/
						// jsyoon 2022.09.29 change
						int iRotate = 0;
						if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
						{
							iRotate = 1;
						}

						ZeroMemory(cCellID, sizeof(cCellID));
						nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);

						// jsyoon 2022.06.21 add
						int iCellStatus = 0;
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] != true)	// 2023.03.09 add
								continue;

							nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
							iCellStatus = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick];

							// PROBE CHECK
													// PROBE CHECK
							/*!
							* 2023.03.09
							* 측정 동작 조건 수정
							*/
							if (iCellStatus == df_CELLSTATE_PTP_SHORT ||		// PTP
								iCellStatus == df_CELLSTATE_PTP_OPEN ||		// PTP
								iCellStatus == df_CELLSTATE_OST_SHORT ||		// OST
								iCellStatus == df_CELLSTATE_OST_OPEN ||		// OST
								iCellStatus == df_CELLSTATE_AGING_NG  			// AGING
								)
							{
								bMeasProbe[iProbe] = false;
							}

							//if (iCellStatus == df_CELLSTATE_ILDE ||			// 
							//	iCellStatus == df_CELLSTATE_PTP_NOR ||		// PTP
							//	iCellStatus == df_CELLSTATE_OST_NOR ||		// OST
							//	iCellStatus == df_CELLSTATE_PASS ||			// R-TEST
							//	iCellStatus == df_CELLSTATE_AGING_OK)		// AGING
							//{
							//	bMeasProbe[iProbe] = true;
							//}
							//else
							//{
							//	bMeasProbe[iProbe] = false;
							//}

							if (iCellStatus == df_CELLSTATE_OVER || iCellStatus == df_CELLSTATE_UNDER)  // R-TEST 
							{
								if (bUseRtestNgPass == true)
								{
									bMeasProbe[iProbe] = true;
								}
								else
								{
									bMeasProbe[iProbe] = false;
								}
							}
						}

						if (bMeasProbe[0] == true && bMeasProbe[1] != true)
						{
							iMeasMode = 1;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
						{
							iMeasMode = 2;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] != true)
						{
							iMeasMode = -1;
						}
						else
						{
							iMeasMode = 0;
						}
					}

					// Move
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_WAD_MOVE, true, 0);

					if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
					{
						//2023.02.23 InnerID 초기화 (1 HEAD 만, 3점 측정시)
						if (bOneCell) memset(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]], 0x00, sizeof(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]]));

						ClearInnerIDForAging(nUseCELLID);	// LYS 2026-08-12 AgingCycle R2
						GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

						stMotorPosition stPosition[10];
						ZeroMemory(&stPosition, sizeof(stPosition));
						int nAxis = 0;

						double dLeft = 0., dRight = 0.;
						double dLeft2 = 0., dRight2 = 0.;	// 2023.03.08 add
						bool bDirLtoR = true;

						if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
						{
							dLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
							dRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
						}
						else
						{
							dLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
							dRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

							bDirLtoR = false;
						}

						if (nMeasArea == 1)
						{
							/*!
							* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
							*/
							double dHeadGapRcp = 700, dMovePos = 0;
							if (pReciepInfo != nullptr)
							{
								dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
							}

							dMovePos = dHeadGapRcp / 2;

							if (bDirLtoR == true)	//  - -> +
							{
								dLeft2 = dMovePos * -1;
								dRight2 = dMovePos;
							}
							else  //  + -> -
							{
								dLeft2 = dMovePos;
								dRight2 = dMovePos * -1;
							}
						}
						else
						{
							dLeft2 = dLeft;
							dRight2 = dRight;
						}

						//2023.05.23 특성 3점 //2023.06.09 add 특성 5점 (k == 3, k == 4)
						double fCellRatio = 0.0;
						double fSafetyH1 = 0.0, fSafetyH2 = 0.0;
						m_pShareItem->GetPositionConfigMgrPtr()->GetSafetyPosition(nullptr, nullptr, nullptr, &fSafetyH1, &fSafetyH2, nullptr, nullptr);

						if (bOneCell) {
							S_RecipeMap* pRecipeMap;
							pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();

							fCellRatio = (pRecipeMap->m_fCellSize.x / 2.0) / 4.0;

							if		(k == 0) dRight = dRight - (fCellRatio * 0.0);
							else if (k == 1) dRight = dRight - (fCellRatio * 1.0);
							else if (k == 2) dRight = dRight - (fCellRatio * 2.0);
							else if (k == 3) {
								dLeft = dRight + (fCellRatio * 1.0);
								dRight = fSafetyH2;
							}
							else if (k == 4) {
								dLeft = dRight + (fCellRatio * 2.0);
								dRight = fSafetyH2;
							}

						}


						if (bMoveFirstDone == true)
						{
							stPosition[0].axis = AXIS_Y1;   nAxis++;
							stPosition[0].dPosition = dLeft;
							stPosition[1].axis = AXIS_Y2;   nAxis++;
							stPosition[1].dPosition = dRight;
						}
						else
						{
							stPosition[0].axis = AXIS_Y1;   nAxis++;
							stPosition[0].dPosition = dLeft;
							stPosition[1].axis = AXIS_Y2;   nAxis++;
							stPosition[1].dPosition = dRight;
							stPosition[2].axis = AXIS_Y3;   nAxis++;
							stPosition[2].dPosition = dLeft2;
							stPosition[3].axis = AXIS_Y4;   nAxis++;
							stPosition[3].dPosition = dRight2;
							// LYS 2026-09-14 OCP: 첫 이동의 FRONT 헤드 Y3/Y4 는 셀이 아니라 [SAFETY] 로 보낸다 (df_HEAD_Y_IDLE_SAFETY -> MoveCmdPos* 의 ApplyIdleHeadSafetyY 가 치환).
							//   WAD 첫 이동에서 반대편(FRONT) 헤드가 셀을 따라갈 이유가 없다(FRONT 갠트리는 SafetyHeadX2 에 있다).
							//if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
							//{
							//	stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
							//	stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
							//}
							// LYS 2026-09-21 위 블록을 다시 살린다 - WAD 첫 이동의 FRONT 헤드 Y3/Y4 는 셀을 따라가지 않고 [SAFETY] 로 간다.
							//   주석 상태에서는 Y3/Y4 가 셀 좌표(dLeft2/dRight2)로 나가 A01(690.2)에서 Y3 상한 697 까지 6.8mm 뿐이었고,
							//   WAD 가 짝 측정(A01+K01)이 된 뒤로는 셀이나 Glass Align 보정이 조금만 커져도 3901 이 난다.
							//   마커는 MoveCmdPosWAD 의 ApplyIdleHeadSafetyY 가 SafetyHeadY3/Y4 원시좌표로 바꾼다. 둘째 이동부터는 Y3/Y4 를 안 보낸다.
							if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
							{
								stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
								stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
							}
							// LYS 2026-06-26 OCP: X3 primary axis for ANGLE
							// LYS 2026-07-24 WAD gantry axis = X2 (user-confirmed; OCP AXIS_X was ignored by MoveCmdPosWAD)
							stPosition[4].axis = AXIS_X2;	nAxis++;
							stPosition[4].dPosition = vecMeasGroup[i].dHeadBackG + dOffsetX;
							// LYS 2026-09-07 Z1(BACK Z, OCP 미사용) 항목 제거 - Sub Z(Z3/Z4)로 대체
							stPosition[5].axis = AXIS_Z2;   nAxis++;
							stPosition[5].dPosition = -1000;		// LYS 2026-09-10 Safety로 변경 하자

							// LYS 2026-08-28 Sub Z(Z3/Z4) - WAD(특성) 측정
							stPosition[6].axis = AXIS_Z3;   nAxis++;
							stPosition[6].dPosition = (pReciepInfo != nullptr) ? pReciepInfo->m_sMotionSubZ.m_fWadSubzRight : 0.0;
							stPosition[7].axis = AXIS_Z4;   nAxis++;
							stPosition[7].dPosition = (pReciepInfo != nullptr) ? pReciepInfo->m_sMotionSubZ.m_fWadSubzLeft : 0.0;
						}

						// jsyoon 2022.05.27 add
						pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
						double dXPos = 0.;
						dXPos = vecMeasGroup[i].dHeadBackG - m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X) - pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

						m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasPos(enumTCPIpFactorytID_WAD, dXPos, dLeft, dRight);
						/*!
						* 이동시 Cross 축만 모션 완료 확인을 나중에 한다
						*/
						MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosWAD(stPosition, nAxis, SPEED_HIGH, false);
						if (nRet == VTM_RV_SUCCESSED)
						{
							bMoveFirstDone = true;
						}	

						// PG 번호 변경
						if (nRet == VTM_RV_SUCCESSED)
						{
							for (int iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
							{
								if (bMeasProbe[iProbe] == true)
								{
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
								}
								else
								{
									if (iProbe == 0) {
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
									}
									else
									{
										/*!
										* 2023.03.09 change 
										* Cell 하나 측정 시 다른 PG 번호를 임의로 설정
										* m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
										*/ 
										if(nArea == 1)
											m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 11);
										else
											m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
									}
									
								}
							}
						}

						// Wait Done
						if (nRet == VTM_RV_SUCCESSED)
						{
							int axis = 0;
							for (int n = 0; n < 2; n++)
							{
								axis = stPosition[n].axis;
								nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
								if (nRet != df_MOTION_RV_SUCCESSED)
									break;
							}
						}
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_WAD_MOVE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);

					if (nRet != VTM_RV_SUCCESSED) {
						bRet = false;
						m_pShareItem->SetFail(true, nRet);
						break;
					}

					// 특성 검사
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_WAD_SAMPLE, true, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), i);

					if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true)
							{
								nCellNum = nUseCELLID[iProbe];  // nCellNum = nMeasArea * iProbe + i;
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_WAD_START;
							}
						}

						//Area 정보, 측정 진행 유무
						int nAreaNum[DF_MAX_HEAD] = { 0, };
						/*nAreaNum[0] = cellFirst.m_nArea + (nArea * iStick);
						nAreaNum[1] = cellSecond.m_nArea + (nArea * iStick);*/

						BOOL bOneCellWadRight = FALSE;

						// LYS 2026-08-12 AgingCycle R1: 에이징 반복마다 결과 행 블록 하나를 예약
						// (OCP nAreaNum + nMaxArea * nPropertyRepeat, JOB_Proc.cpp:13934).
						// 사이클 비활성이면 Repeat=0이라 표준 행은 불변.
						int nAgingRep = 0;
						if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
							nAgingRep = m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_WAD);
						int nAgingRow = nArea * nMaxStick * nAgingRep;

						if (bOneCell) {
							// LYS 2026-08-18 nOneCellCnt는 1 고정(bOneCell 폐기)이라 k는 항상 0이고
							// 에이징 오프셋은 아래 일반 분기와 같은 블록 단위다.
							// (이전 주석의 "nMeasArea == 1이면 셀 1개"는 오기 - 2Head GROUP 수다.)
							nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nMaxStick * k) + nAgingRow;
							nAreaNum[1] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nMaxStick * k) + nAgingRow;
							//nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;

							if (k >= 3) {
								iMeasMode = 2;
								bOneCellWadRight = TRUE;
							}
						}
						else {
							nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + nAgingRow;
							nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1 + nAgingRow;
						}
						m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);
						LogAgingRow(enumAgingMeasKind_WAD, _T("WAD"), nAreaNum);	// LYS 2026-08-12 AgingCycle R1
						//

						//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, iMeasMode);
						// LYS 2026-07-23 WAD Multi run state: base point = 1
						if (bMultiWad && pMultiMgr != nullptr)
						{
							for (int p = 0; p < df_MAX_PROBE; p++)
							{
								bool bAct = bMeasProbe[p] && pMultiMgr->IsMultiCell(iStick, nUseCELLID[p]);
								pMultiMgr->SetRunActive(p, bAct);
								pMultiMgr->SetRunCurrent(p, 1);
								pMultiMgr->SetRunPointIdx(p, 0);	// LYS 2026-08-18 base point = index 0

								// LYS 2026-08-18 MultiPoint 추적. OOS에서는 TossResultFile까지 가지 않으므로
								// 값이 정해지는 여기서 로그를 남긴다.
								if (bAct)
								{
									CString strMPLog;
									// LYS 2026-08-18 리스트 0행은 base 자리표시자라 n/N 표기에서 제외한다
									// (아래 "WAD Multi Point" 표시 로그와 동일 규칙).
									strMPLog.Format(_T("WAD MultiPoint: probe(%d) point(base) stick(%d) cell(%d) row(%d)"),
										p, GetMeasureStick(), nUseCELLID[p], nAreaNum[p]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMPLog);
								}
							}
						}

						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, iMeasMode, 0, true, bOneCellWadRight);
						/*if (nRet == VTM_RV_SUCCESSED)
						{
							nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_WAD, iMeasMode, nTimeOut);
						}*/

						if (nRet == VTM_RV_SUCCESSED)
						{
							nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_WAD, iMeasMode, nTimeOut, k + 1);
							//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_WAD, iMeasMode, nTimeOut);
						}

						if (nRet == VTM_RV_SUCCESSED)
						{
							for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
							{
								if (bMeasProbe[iProbe] == true)
								{
									nCellNum = nUseCELLID[iProbe];  //nCellNum = nMeasArea * iProbe + i;
									m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_WAD_END;
								}
							}
						}
					}

					// LYS 2026-07-23 WAD Multi Point loop (Phase 4): move + measure points 1..N-1 after base point
					if (bMultiWad && pMultiMgr != nullptr && nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
					{
						bool bHeadMulti[df_MAX_PROBE];
						for (int p = 0; p < df_MAX_PROBE; p++)
							bHeadMulti[p] = bMeasProbe[p] && pMultiMgr->IsMultiCell(iStick, nUseCELLID[p]);

						if (bHeadMulti[0] || bHeadMulti[1])
						{
							// base head Y positions (same left/right rule as base move block)
							double dMLeft = 0., dMRight = 0.;
							if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
							{
								dMLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
								dMRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
							}
							else
							{
								dMLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
								dMRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
							}

							int nPtTotal = pMultiMgr->GetPointCount();
							int nRunCnt = 1;
							S_WadMultiPoint ptMulti;
							CString strMLog;

							for (int ip = 1; ip < nPtTotal; ip++)
							{
								if (pMultiMgr->GetPoint(ip, ptMulti) != true)
									continue;
								if (ptMulti.bEnable != true)
									continue;

								if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
								{
									bRet = false;
									break;
								}

								nRunCnt++;
								for (int p = 0; p < df_MAX_PROBE; p++)
								{
									pMultiMgr->SetRunCurrent(p, nRunCnt);
									pMultiMgr->SetRunPointIdx(p, ip);	// LYS 2026-08-18 actual loop index
								}

								// new InnerID per point (separate result file per point)
								GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

								if (bHeadMulti[0] == true && bHeadMulti[1] == true)
								{
									strMLog.Format(_T("WAD Multi Point %d/%d (X=%.4f, Y=%.4f) %s, %s"), nRunCnt - 1, pMultiMgr->GetUseSize() -1, ptMulti.dPosX, ptMulti.dPosY, cCellID[0], cCellID[1]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMLog);
								}
								else if (bHeadMulti[0] == true && bHeadMulti[1] == false)
								{
									strMLog.Format(_T("WAD Multi Point %d/%d (X=%.4f, Y=%.4f) %s"), nRunCnt - 1, pMultiMgr->GetUseSize() - 1, ptMulti.dPosX, ptMulti.dPosY, cCellID[0]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMLog);
								}
								else if (bHeadMulti[0] == false && bHeadMulti[1] == true)
								{
									strMLog.Format(_T("WAD Multi Point %d/%d (X=%.4f, Y=%.4f) %s"), nRunCnt - 1, pMultiMgr->GetUseSize() - 1, ptMulti.dPosX, ptMulti.dPosY, cCellID[1]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMLog);
								}
								else
								{
									strMLog.Format(_T("WAD Single Point %d/%d (X=%.4f, Y=%.4f) %s, %s"), nRunCnt - 1, pMultiMgr->GetUseSize() - 1, ptMulti.dPosX, ptMulti.dPosY, cCellID[0], cCellID[1]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMLog);
								}



								// move: head Y += point X, gantry += -point Y (OCP sign rule)
								stMotorPosition stPosM[4];
								ZeroMemory(&stPosM, sizeof(stPosM));
								int nAxisM = 0;

								stPosM[nAxisM].axis = AXIS_Y1;
								stPosM[nAxisM].dPosition = dMLeft + ptMulti.dPosX;
								nAxisM++;
								stPosM[nAxisM].axis = AXIS_Y2;
								stPosM[nAxisM].dPosition = dMRight + ptMulti.dPosX;
								nAxisM++;
								// LYS 2026-07-24 WAD Multi gantry axis = X2 (user-confirmed axis logic)
								stPosM[nAxisM].axis = AXIS_X2;
								stPosM[nAxisM].dPosition = vecMeasGroup[i].dHeadBackG + dOffsetX - ptMulti.dPosY;
								nAxisM++;

								MarkIdleHeadSafety(stPosM, nAxisM, vecMeasGroup[i], ptMulti.dPosX);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
								nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosWAD(stPosM, nAxisM, SPEED_HIGH, false);

								if (nRet == VTM_RV_SUCCESSED)
								{
									for (int n2 = 0; n2 < nAxisM; n2++)
									{
										nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(stPosM[n2].axis);
										if (nRet != df_MOTION_RV_SUCCESSED)
											break;
									}
								}

								if (nRet != VTM_RV_SUCCESSED)
									break;

								// PG number set (same as base point)
								for (int p = 0; p < df_MAX_PROBE; p++)
								{
									if (bMeasProbe[p] == true)
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + p, nUseCELLID[p] + 1);
								}

								// result row block per point
								int nAreaNumM[DF_MAX_HEAD] = { 0, };
								if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
								{
									// LYS 2026-08-12 AgingCycle R1: 멀티포인트 블록은 에이징 반복 블록 뒤에 두어
									// 서로 충돌하지 않게 한다(OCP JOB_Proc.cpp:14530 레이아웃).
									// 아래 표준 경로는 의도적으로 그대로 둔다.
									int nBlock = m_pMeasSeqItemEngine->GetAgingKindMax(enumAgingMeasKind_WAD)
										+ m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_WAD) * (nPtTotal - 1)
										+ (ip - 1);
									nAreaNumM[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nArea * nMaxStick * nBlock);
									nAreaNumM[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1 + (nArea * nMaxStick * nBlock);
								}
								else
								{
								nAreaNumM[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nArea * nMaxStick * ip);
								nAreaNumM[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1 + (nArea * nMaxStick * ip);
								}
								m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNumM, nTimeOut);
								LogAgingRow(enumAgingMeasKind_WAD, _T("WAD-MP"), nAreaNumM);	// LYS 2026-08-12 AgingCycle R1

								// LYS 2026-08-18 MultiPoint trace (see the base-point log above)
								for (int pl = 0; pl < df_MAX_PROBE; pl++)
								{
									if (bMeasProbe[pl] != true) continue;
									CString strMPLog;
									strMPLog.Format(_T("WAD MultiPoint: probe(%d) point(%d/%d) idx(%d) stick(%d) cell(%d) row(%d)"),
										pl, nRunCnt - 1, pMultiMgr->GetUseSize() - 1, ip, GetMeasureStick(), nUseCELLID[pl], nAreaNumM[pl]);
									m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMPLog);
								}

								nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, iMeasMode, 0, true, false);

								if (nRet == VTM_RV_SUCCESSED)
									nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_WAD, iMeasMode, nTimeOut, 1);

								if (nRet != VTM_RV_SUCCESSED)
									break;
							}

							if (nRet != VTM_RV_SUCCESSED)
							{
								bRet = false;
								m_pShareItem->SetFail(true, nRet);
							}
						}

						pMultiMgr->ResetRunState();
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_WAD_SAMPLE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_WAD, false, 0);
				}
			}

			// MTP End
			if (bUseMTP == true) { m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, enumTCPIpFactoryt_Event_WAD_4); }
		}
	}

	// LYS 2026-08-12 AgingCycle R1: advance the WAD repeat once per step run (not per cell)
	if (bUseMeas == true && m_pMeasSeqItemEngine != NULL)
		m_pMeasSeqItemEngine->AgingRepeatNext(enumAgingMeasKind_WAD);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_WAD End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_WAD, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, 3);

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_VISION::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 4);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_VISION, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_VISION Start."));

	bool bRet = true;
	bool bUseMeas = true;
	bool bUseRtestNgPass = false;
	bool bUseSampling = false;

	int nUseCELLID[df_MAX_PROBE];
	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
	bool bMeasProbe[df_MAX_PROBE];
	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME;
	// Check Meas
	double dOffsetX = 0.;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	bool bUseVisionColor = m_pShareItem->GetSystemConfigMgrPtr()->IsUseInspVisionColor();

	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseQuality_4;
		// LYS 2026-08-06 AgingCycle E3: per-item select (VISION = Quality flag + stick cell map)
		if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
		{
			const S_AgingTest* pAgingItem = m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetItem(m_pMeasSeqItemEngine->GetAgingCurItem());
			bUseMeas = (pAgingItem != NULL && pAgingItem->m_bUseQuality)
				&& (m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetStickCount() == 0
					|| m_pMeasSeqItemEngine->GetAgingCycleInfo()->IsStickUse(GetMeasureStick(), enumAgingMeasKind_VISION));

			if (bUseMeas && m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
			{
				// LYS 2026-08-06 AgingCycle E4: progress marker (col 5 = VISION)
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = m_pMeasSeqItemEngine->GetAgingCurItem() + 1;
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 5;
			}
		}
		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[4];

		bUseRtestNgPass = pReciepInfo->m_sOptionItem.m_bUseRtestNgPass;	// jsyoon 2022.06.21

		bUseSampling = pReciepInfo->m_sOptionItem.m_bUseSampling; // jsyoon 2022.07.19 add

		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;	// 2023.07.06 add
	}

	int iMeasMode = 0;

	//if (bUseMeas == true)
	if (bUseMeas == true && !bUseVisionColor)
	{
		// 좌표 생성
		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nMeasArea = 0;
		double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);

		dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		if (dHeadGapRcp > dHeadGap)
		{
			dHeadGap = dHeadGapRcp;
		}

	//	m_pShareItem->GetPositionConfigMgrPtr()->GetHeadGap(&dHeadGap);


		double dGlassConactPos = 0;
	//	dGlassConactPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - 20;
		

		S_RecipeData* pRecipeData;
		pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		int iStick = 0, nArea, nMaxStick = 0;
		iStick = GetMeasureStick();
		nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
		nMaxStick = pRecipeData->m_vecStick.size();

		//dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;
		dGlassConactPos = pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

		// Generate 2Head Group
		bool* pbSet = new bool[nArea];
		for (int iGroup = 0; iGroup < nArea; iGroup++)
			pbSet[iGroup] = false;

		vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
		vecMeasGroup.clear();

		S_CellInfo cellFirst, cellSecond;
		S_MeasPos measPos;
		cellFirst.Clear();
		cellSecond.Clear();

		// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (MakeHeadPairByOrder 주석 참조).
		//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 필터는 아래 루프와 같은 조건을 미리 적용한다.
		int* pnPair = new int[nArea];
		{
			bool* pbUse = new bool[nArea];
			for (int iCell = 0; iCell < nArea; iCell++)
			{
				pbUse[iCell] = true;
				if (IsAgingCellSkip(enumAgingMeasKind_VISION, iCell))
					pbUse[iCell] = false;
				if (bUseSampling == true && pRecipeData->m_vecStick[iStick].m_vecCell[iCell].m_nSelectCell[M_QUALITY] != 1)
					pbUse[iCell] = false;
			}
			// LYS 2026-09-14 짝 조건에 헤드 오프셋 반영. 축 목표 = 셀 X + 오프셋이므로 두 헤드 목표 간격 = 셀 거리 + (HEAD1 - HEAD2 오프셋).
			//   인터락(dHeadGap)은 축 목표 간격을 보므로 셀 거리 조건은 dHeadGap - (o1 - o2). WAD(+168/-168)는 384, VISION(0)은 그대로,
			//   ANGLE(-272/+272)은 1264 가 되어 사실상 단독 측정으로 간다(2026-09-14 센서 배치 기준).
			double dPairGap = dHeadGap;
			{
				double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 2, &dOffX, &dOffY1, &dOffZ);
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 2, &dOffX, &dOffY2, &dOffZ);
				dPairGap = dHeadGap - (dOffY1 - dOffY2);
			}
			MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, pbUse, nArea, dPairGap, pnPair, _T("VISION"));
			delete[] pbUse;
		}

		if (nArea > 1)
		{
			for (int iGroup = 0; iGroup < nArea; iGroup++)
			{
				if (pbSet[iGroup] != true)
				{
					cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

					// LYS 2026-08-18 AgingCycle: AgingRcp [MASECELL] 기반 셀 단위 필터.
					// pbSet 표시로 아래 잔여 단일헤드 패스에서도 제외된다.
					if (IsAgingCellSkip(enumAgingMeasKind_VISION, iGroup))
					{
						pbSet[iGroup] = true;
						continue;
					}

					if (bUseSampling == true)	// jsyoon 2022.07.19 add
					{
						//	if (cellFirst.m_nSelectCell[M_QUALITY] != true)
						if (cellFirst.m_nSelectCell[M_QUALITY] != 1)
						{
							pbSet[iGroup] = true;
							continue;
						}
					}

					cellSecond.Clear();
					for (int iSearch = 0; iSearch < nArea; iSearch++)
					{
						if (pbSet[iSearch] != true)
						{
							cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

							// LYS 2026-08-18 AgingCycle: per-cell filter from AgingRcp [MASECELL]
							if (IsAgingCellSkip(enumAgingMeasKind_VISION, iSearch))
							{
								pbSet[iSearch] = true;
								continue;
							}

							if (bUseSampling == true)	// jsyoon 2022.07.19 add
							{
								//	if (cellSecond.m_nSelectCell[M_QUALITY] != true)
								if (cellSecond.m_nSelectCell[M_QUALITY] != 1)
								{
									pbSet[iSearch] = true;
									continue;
								}
							}

							// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
							//if (cellFirst.m_fCellX < 50)
							if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
							{
								if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
								{
									pbSet[iGroup] = true;
									pbSet[iSearch] = true;
									measPos.Clear();
									//measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
									//measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;

									// LYS 2026-06-30
									measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
									measPos.dHeadBackG = measPos.dHeadBackG - dGantryGap;
									measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fReviewZ;
									measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

									if (cellFirst.m_nArea >= nArea)
									{
										cellFirst.m_nArea -= (nArea * iStick);
									}

									if (cellSecond.m_nArea >= nArea)
									{
										cellSecond.m_nArea -= (nArea * iStick);
									}

									measPos.m_sCellInfo[0] = cellFirst;
									measPos.m_sCellInfo[1] = cellSecond;
									vecMeasGroup.push_back(measPos);

									break;
								}
							}
						}
					}
				}
			}
		}
		

		// 선택하고 남은것
		for (int iGroup = 0; iGroup < nArea; iGroup++)
		{
			if (pbSet[iGroup] != true)
			{
				pbSet[iGroup] = true;

				cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

				// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
				// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0. VISION 오프셋이 0 이 되어 X=0 셀을 Y3/Y1 이 맡아도 되고,
				//   ANGLE(Y3 = X-272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3 로 보내야 쉬는 Y4 Safety(-860)와 근접센서 간격(580)이 나온다.
				//if (cellFirst.m_fCellX > 50)
				// LYS 2026-09-18 담당 헤드를 셀 X 부호 대신 '갈 수 있고 쉬는 헤드와 간격이 큰 쪽'으로 고른다.
				//                구 줄은 바로 아래에 남겨 두었다(원복 시 이 블록 제거 후 주석 해제).
				//if (cellFirst.m_fCellX > 0)  // Head 2
				double dOffHeadP = 0.0, dOffHeadM = 0.0;
				GetHeadOffsetY(m_pShareItem->GetPositionConfigMgrPtr(), 2, &dOffHeadP, &dOffHeadM);
				if (PickSingleCellHead(m_pShareItem->GetPositionConfigMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(),
					cellFirst.m_fCellX, dOffHeadP, dOffHeadM, true, _T("VISION")) != 0)  // Head 2
				{
					measPos.Clear();
					//measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					//measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;

					// LYS 2026-06-30
					measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadBackG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[1] = cellFirst;
					cellFirst.m_fCellX -= dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[0] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
				else
				{
					measPos.Clear();
					//measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					//measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;

					// LYS 2026-06-30
					measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadBackG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[0] = cellFirst;
					cellFirst.m_fCellX += dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[1] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
			}
		}

		delete[] pbSet;
		delete[] pnPair;	// LYS 2026-09-14

		double dGlassSize = 0; // 925 / 2;
		long nRet = df_MOTION_RV_SUCCESSED;
		nMeasArea = vecMeasGroup.size();
		bool bMoveFirst = false;
		stMotorPosition stPosition[20];

		//// 위치 이동 하여 검사 초기화 및 해제 (Cycle 시작전, Auto Proc 시작전, 검사 종료 후)
		//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_RESET);		  //화질 자동검사 시작시 초기화
		//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_WAIT_USE, TRUE); //화질 자동결과 설정

		if (nMeasArea > 0)
		{
			int nMeasStick = 0;	// jsyoon 2022.09.29 add

			// TP 설정
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_2);
			
			for (int i = 0; i < nMeasArea; i++)
			{
				if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
				{
					bRet = false;
					break;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Move"), i);

				nRet = VTM_RV_SUCCESSED;

				nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
				nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;

				bMeasProbe[0] = true;
				bMeasProbe[1] = true;

				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;


				int nCellNum = 0;
				// Set PG Num
				if (nRet == VTM_RV_SUCCESSED)
				{
					/*!
					* 2023.03.09
					* 위치 이동 nCellStatus 위치는 iRotate 에 따라 표시 하는 위치가 달라져서 먼저 nMeasStick 을 구하고 상태를 읽어야 함
					*/
					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					ZeroMemory(cCellID, sizeof(cCellID));

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						int nUseHead[df_MAX_PROBE];
						ZeroMemory(nUseHead, sizeof(nUseHead));

						bool bUseProbe[df_MAX_PROBE];
						ZeroMemory(bUseProbe, sizeof(bUseProbe));

						nUseHead[0] = nUseCELLID[0];
						nUseHead[1] = nUseCELLID[0];
						bUseProbe[0] = true;
						bUseProbe[1] = true;

						nMeasStick = MakeCellID(nUseHead, bUseProbe, *cCellID, iRotate, iStick, nMaxStick);
					}
					else
					{
						nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);
					}
					

					// jsyoon 2022.06.21 add
					int iCellStatus = 0;
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] != true)	// 2023.03.09 add
							continue;

						nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
						iCellStatus = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick];

						// PROBE CHECK
						/*!
						* 2023.03.09
						* 측정 동작 조건 수정
						*/
						if (iCellStatus == df_CELLSTATE_PTP_SHORT ||		// PTP
							iCellStatus == df_CELLSTATE_PTP_OPEN  ||		// PTP
							iCellStatus == df_CELLSTATE_OST_SHORT ||		// OST
							iCellStatus == df_CELLSTATE_OST_OPEN  ||		// OST
							iCellStatus == df_CELLSTATE_AGING_NG  			// AGING
							)
						{
							bMeasProbe[iProbe] = false;
						}

						//if (iCellStatus == df_CELLSTATE_ILDE ||			// 
						//	iCellStatus == df_CELLSTATE_PTP_NOR ||		// PTP
						//	iCellStatus == df_CELLSTATE_OST_NOR ||		// OST
						//	iCellStatus == df_CELLSTATE_PASS ||			// R-TEST
						//	iCellStatus == df_CELLSTATE_AGING_OK 		// AGING
						//	)		
						//{
						//	bMeasProbe[iProbe] = true;
						//}
						//else
						//{
						//	bMeasProbe[iProbe] = false;
						//}

						if (iCellStatus == df_CELLSTATE_OVER || iCellStatus == df_CELLSTATE_UNDER)  // R-TEST 
						{
							if (bUseRtestNgPass == true)
							{
								bMeasProbe[iProbe] = true;
							}
							else
							{
								bMeasProbe[iProbe] = false;
							}
						}
					}

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						iMeasMode = 0;	// 동시 측정명령 전송 2PC 2Cam
					}
					else
					{
						if (bMeasProbe[0] == true && bMeasProbe[1] != true)
						{
							iMeasMode = 1;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
						{
							iMeasMode = 2;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] != true)
						{
							iMeasMode = -1;
						}
						else
						{
							iMeasMode = 0;
						}
					}

					

					//// jsyoon 2022.09.29 change
					//int iRotate = 0;
					//if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//{
					//	iRotate = 1;
					//}

					//ZeroMemory(cCellID, sizeof(cCellID));
					//nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);
					//CString strCellId;
					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	if (bMeasProbe[iProbe] == true)
					//	{
					//		nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
					////		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][iStick] = df_CELLSTATE_VISION_START;

					//		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		}
					//		else
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + iStick, nArea - nCellNum);
					//		}

					//		// strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		//strCellId.Format(_T("%c%02d"), 65, nCellNum + 1);
					//		//strCellId.Format(_T("%c%02d"), 65 + nCellNum, nMaxStick - iStick);
					//		/*strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + iProbe + 1);*/
					//		_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//	}
					//}
				}

				// Move
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, true, 0);

				ZeroMemory(&stPosition, sizeof(stPosition));

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					if (nMeasArea == 1)	// 2023.05.09 add
					{
						int nUseHead[df_MAX_PROBE];
						ZeroMemory(nUseHead, sizeof(nUseHead));
						
						bool bUseProbe[df_MAX_PROBE];
						ZeroMemory(bUseProbe, sizeof(bUseProbe));

						nUseHead[0] = nUseCELLID[0];
						nUseHead[1] = nUseCELLID[0];
						bUseProbe[0] = true;
						bUseProbe[1] = true;

						ClearInnerIDForAging(nUseHead);	// LYS 2026-08-12 AgingCycle R2
						GenMeasID(nUseHead, bUseProbe, *cCellID);
					}
					else
					{
						ClearInnerIDForAging(nUseCELLID);	// LYS 2026-08-12 AgingCycle R2
						GenMeasID(nUseCELLID, bMeasProbe, *cCellID);
					}
					
	
					int nAxis = 0;
					double dLeft = 0., dRight = 0.;
					double dLeft2 = 0., dRight2 = 0.;
					bool bDirLtoR = true;

					if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
					}
					else
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

						bDirLtoR = false;
					}

					if (nMeasArea == 1)
					{
						/*!
						* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
						*/
						double dHeadGapRcpF = 700.0,  dHeadGapRcpB = 700.0, dMovePosF = 0.0, dMovePosB = 0.0;
						if (pReciepInfo != nullptr)
						{
							dHeadGapRcpF = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
							dHeadGapRcpB = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
						}

						dMovePosF = dHeadGapRcpF / 2;
						dMovePosB = dHeadGapRcpB / 2;

						if (bDirLtoR == true)	//  - -> +
						{
							dLeft2 = dMovePosF * -1;
							dRight2 = dMovePosF;

							dLeft  = dMovePosB * -1;
							dRight = dMovePosB;
						}
						else  //  + -> -
						{
							dLeft2 = dMovePosF;
							dRight2 = dMovePosF * -1;

							dLeft  = dMovePosB;
							dRight = dMovePosB * -1;
						}
					}
					else
					{
						dLeft2 = dLeft;
						dRight2 = dRight;
					}

					if (bMoveFirst == true)
					{
						stPosition[0].axis = AXIS_Y3;   nAxis++;
						stPosition[0].dPosition = dLeft2;
						stPosition[1].axis = AXIS_Y4;   nAxis++;
						stPosition[1].dPosition = dRight2;

						stPosition[2].axis = AXIS_FILTER1;   nAxis++;
						stPosition[2].dPosition = 0;
						stPosition[3].axis = AXIS_FILTER2;   nAxis++;
						stPosition[3].dPosition = 0;
					}
					else
					{
						stPosition[0].axis = AXIS_Y3;   nAxis++;
						stPosition[0].dPosition = dLeft2;
						stPosition[1].axis = AXIS_Y4;   nAxis++;
						stPosition[1].dPosition = dRight2;

						stPosition[2].axis = AXIS_Y1;   nAxis++;
						stPosition[2].dPosition = dLeft;
						stPosition[3].axis = AXIS_Y2;   nAxis++;
						stPosition[3].dPosition = dRight;
						// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (df_HEAD_Y_IDLE_SAFETY -> MoveCmdPos* 의 ApplyIdleHeadSafetyY 가 치환).
						//   FRONT 측정 첫 이동에서 BACK 헤드가 셀 좌표 + ANGLE 오프셋(MoveCmdPosVision/Angle)으로 나가 ?272 면 Y1-Y2 간격이 인터락(720) 미달로 거부된다. BACK 갠트리는 이때 SafetyHeadX1 에 있다.
						if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
						{
							stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
							stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
						}
				
						stPosition[4].axis = AXIS_X3;   nAxis++;
						//stPosition[4].dPosition = vecMeasGroup[i].dHeadBackG + dOffsetX;
						stPosition[4].dPosition = vecMeasGroup[i].dHeadFrontG + dOffsetX;		// LYS 2026-06-30

						stPosition[5].axis = AXIS_Z1;   nAxis++;
						stPosition[5].dPosition = vecMeasGroup[i].dHeadFrontZ;
						stPosition[6].axis = AXIS_Z2;   nAxis++;
						stPosition[6].dPosition = vecMeasGroup[i].dHeadFrontZ;

						stPosition[7].axis = AXIS_ZOOM1;   nAxis++;
						stPosition[7].dPosition = 0;
						stPosition[8].axis = AXIS_THETA1;   nAxis++;
						stPosition[8].dPosition = 0;
						stPosition[9].axis = AXIS_ZOOM2;   nAxis++;
						stPosition[9].dPosition = 0;
						stPosition[10].axis = AXIS_THETA2;   nAxis++;
						stPosition[10].dPosition = 0;

						stPosition[11].axis = AXIS_FILTER1;   nAxis++;
						stPosition[11].dPosition = 0;
						stPosition[12].axis = AXIS_FILTER2;   nAxis++;
						stPosition[12].dPosition = 0;

						// LYS 2026-08-28 Sub Z(Z5/Z6)
						// LYS 2026-09-18 ANGLE 이 전용 값(m_fAngleSubz*)으로 갈라져 여기는 VISION 전용이 됐다.
						stPosition[13].axis = AXIS_Z5;   nAxis++;
						stPosition[13].dPosition = (pReciepInfo != nullptr) ? pReciepInfo->m_sMotionSubZ.m_fVisionSubzRight : 0.0;
						stPosition[14].axis = AXIS_Z6;   nAxis++;
						stPosition[14].dPosition = (pReciepInfo != nullptr) ? pReciepInfo->m_sMotionSubZ.m_fVisionSubzLeft : 0.0;
					}

					/*!
					* 이동시 Cross 축만 모션 완료 확인을 나중에 한다
					*/
					MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosVision(stPosition, nAxis, SPEED_HIGH, false);

					if (nRet == VTM_RV_SUCCESSED)
					{
						bMoveFirst = true;
					}

					// PG 번호 변경
					if (nRet == VTM_RV_SUCCESSED)
					{
						for (int iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true) {
								m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
							}
							else
							{
								if (iProbe == 0) {
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
								}
								else
								{
									/*!
									* 2023.03.09 change
									* Cell 하나 측정 시 다른 PG 번호를 임의로 설정
									* m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
									*/
									if (nArea == 1)
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 11);
									else
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
								}
							}
						}
					}

					if (i == 0)
					{
						// Wait Done
						if (nRet == VTM_RV_SUCCESSED)
						{
							int axis = 0;
							for (int n = 0; n < 2; n++)
							{
								axis = stPosition[n].axis;
								nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
								if (nRet != df_MOTION_RV_SUCCESSED)
									break;
							}
						}

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, false, 0);

						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);
					}
				}

				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					//20220426 add Buffer Check
					if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(NOT_LAST_ITEM, nTimeOut)) {
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityAlarm(); //알람 리턴 방안 확인 필요
						bRet = false;
					}
				}

				// 계산 완료 후 모션 이동 완료 확인
				if (i > 0)
				{
					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i - 1);

					// Wait Done
					if (nRet == VTM_RV_SUCCESSED)
					{
						int axis = 0;
						for (int n = 0; n < 2; n++)
						{
							axis = stPosition[n].axis;
							nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
							if (nRet != df_MOTION_RV_SUCCESSED)
								break;
						}
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);
				}


				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				// 검사
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_VISION_SAMPLE, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), i);

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] == true)
						{
							nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_VISION_START;
						}
					}

					// Change Recipe Done
					if (iStick == 0 && i == 0)
					{
						DWORD dwTimeOut = DEFINE_RECIPE_WAIT_TIME2; // 60; // jsyoon 2022.09.26 change Recipe Wait Time

						if (nTimeOut > DEFINE_RECIPE_WAIT_TIME2)
						{
							dwTimeOut = nTimeOut;	// 2022.12.27 add
						}
						
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_VISION, dwTimeOut);
					}

					if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true) Sleep(500);
					//Area 정보, 측정 진행 유무
					int nAreaNum[DF_MAX_HEAD] = { -1, -1};

					// LYS 2026-08-12 AgingCycle R1: 에이징 반복마다 결과 행 블록 하나
					// (OCP nAreaNum + nMaxArea * nQualityRepeat, JOB_Proc.cpp:17133).
					int nAgingRow = 0;
					if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
						nAgingRow = nArea * nMaxStick * m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_VISION);
					//nAreaNum[0] = cellFirst.m_nArea + (nArea * iStick);
					//nAreaNum[1] = cellSecond.m_nArea + (nArea * iStick);

					if (nMeasArea == 1)
					{
						//nAreaNum[0] = -1;
						//nAreaNum[1] = nUseCELLID[0] + (nArea * nMeasStick) + 1;  // Head 2 is Master
						nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + nAgingRow;  // Head 1 is Master
						nAreaNum[1] = -1;
						bMeasProbe[0] = true;
						bMeasProbe[1] = true;
					}
					else
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if(nUseCELLID[iProbe] >= 0) nAreaNum[iProbe] = nUseCELLID[iProbe] + (nArea * nMeasStick) + 1 + nAgingRow;
						}
						
					/*	nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
						nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;*/
					}
					
					m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);
					LogAgingRow(enumAgingMeasKind_VISION, _T("VISION"), nAreaNum);	// LYS 2026-08-12 AgingCycle R1
					//
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_GRAB);// DoWorkInspectStart(enumTCPIpFactorytID_VISION, iMeasMode);
					}
					
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_GRAB_WAIT);
						//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_VISION, iMeasMode, nTimeOut);
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true)
							{
								nCellNum = nUseCELLID[iProbe]; // nCellNum = nMeasArea * iProbe + (i - 1);
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_VISION_END;
							}
						}
					}
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_VISION_SAMPLE, false, 0);

				if (i + 1 == nMeasArea)
				{
					if (iMeasMode != -1)
					{
						// jsyoon 2022.08.18 add
					//모든 검사 완료 후 대기, 전수 검사 아닐시 처리 방안 확인 필요
						if (iStick + 1 == m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr()->GetStickCount()) {
							if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(LAST_ITEM, nTimeOut)) {
								/*nRet = */m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityAlarm();
								bRet = false;
							}
						}
					}

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);
				}

				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				////완료 대기 위치 이동
				//if (nRet == VTM_RV_SUCCESSED)
				//{
				//	if (i + 1 == nMeasArea)
				//	{
				////		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_VISION, iMeasMode, nTimeOut);
				//		m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);
				//	}
				//}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION, false, 0);
				if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true) Sleep(500);
			}
		}
	}

	////모든 검사 완료 후 대기, 전수 검사 아닐시 처리 방안 확인 필요
	//if (iStick == m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr()->GetStickCount()) {
	//	if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(LAST_ITEM, nTimeOut)) {
	//		/*nRet = */m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityAlarm();
	//		bRet = false;
	//	}
	//}

	// LYS 2026-08-12 AgingCycle R1: advance the VISION repeat once per step run
	if (bUseMeas == true && m_pMeasSeqItemEngine != NULL)
		m_pMeasSeqItemEngine->AgingRepeatNext(enumAgingMeasKind_VISION);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_VISION End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_VISION, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, 4);

	//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);	// Turn On All		// LYS 2026-09-15 Manual TEST

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_COLOR::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 5);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_COLOR, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_COLOR Start."));

	bool bRet = true;
	bool bUseMeas = true;
	bool bUseRtestNgPass = false;
	bool bUseSampling = false;

	int nUseCELLID[df_MAX_PROBE];
	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
	bool bMeasProbe[df_MAX_PROBE];
	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME;

	// Check Meas
	double dOffsetX = 0.;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseColor_5;

		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[5];

		bUseRtestNgPass = pReciepInfo->m_sOptionItem.m_bUseRtestNgPass;	// jsyoon 2022.06.21

		bUseSampling = pReciepInfo->m_sOptionItem.m_bUseSampling;

		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;	// 2023.07.06 add
	}
	bool bUseVisionColor = m_pShareItem->GetSystemConfigMgrPtr()->IsUseInspVisionColor();

	//if (bUseMeas == true)
	if (bUseMeas == true && !bUseVisionColor)
	{
		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nMeasArea = 0;
		double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);

		dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		if (dHeadGapRcp > dHeadGap)
		{
			dHeadGap = dHeadGapRcp;
		}

//		m_pShareItem->GetPositionConfigMgrPtr()->GetHeadGap(&dHeadGap);

		double dGlassConactPos = 0;
	//	dGlassConactPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - 20;

		S_RecipeData* pRecipeData;
		pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		int iStick = 0, nArea, nMaxStick = 0;
		iStick = GetMeasureStick();
		nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
		nMaxStick = pRecipeData->m_vecStick.size();

		dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

		// Generate 2Head Group
		bool* pbSet = new bool[nArea];
		for (int iGroup = 0; iGroup < nArea; iGroup++)
			pbSet[iGroup] = false;

		vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
		vecMeasGroup.clear();

		S_CellInfo cellFirst, cellSecond;
		S_MeasPos measPos;
		cellFirst.Clear();
		cellSecond.Clear();

		// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (MakeHeadPairByOrder 주석 참조).
		//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 필터는 아래 루프와 같은 조건을 미리 적용한다.
		int* pnPair = new int[nArea];
		{
			bool* pbUse = new bool[nArea];
			for (int iCell = 0; iCell < nArea; iCell++)
			{
				pbUse[iCell] = true;
				if (bUseSampling == true && pRecipeData->m_vecStick[iStick].m_vecCell[iCell].m_nSelectCell[M_COLOR] != 1)
					pbUse[iCell] = false;
			}
			// LYS 2026-09-14 짝 조건에 헤드 오프셋 반영. 축 목표 = 셀 X + 오프셋이므로 두 헤드 목표 간격 = 셀 거리 + (HEAD1 - HEAD2 오프셋).
			//   인터락(dHeadGap)은 축 목표 간격을 보므로 셀 거리 조건은 dHeadGap - (o1 - o2). WAD(+168/-168)는 384, VISION(0)은 그대로,
			//   ANGLE(-272/+272)은 1264 가 되어 사실상 단독 측정으로 간다(2026-09-14 센서 배치 기준).
			double dPairGap = dHeadGap;
			{
				double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 3, &dOffX, &dOffY1, &dOffZ);
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 3, &dOffX, &dOffY2, &dOffZ);
				dPairGap = dHeadGap - (dOffY1 - dOffY2);
			}
			MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, pbUse, nArea, dPairGap, pnPair, _T("COLOR"));
			delete[] pbUse;
		}

		if (nArea > 1)
		{
			for (int iGroup = 0; iGroup < nArea; iGroup++)
			{
				if (pbSet[iGroup] != true)
				{
					cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

					if (bUseSampling == true)	// jsyoon 2022.07.19 add
					{
						//	if (cellFirst.m_nSelectCell[M_COLOR] != true)
						if (cellFirst.m_nSelectCell[M_COLOR] != 1)
						{
							pbSet[iGroup] = true;
							continue;
						}
					}

					cellSecond.Clear();
					for (int iSearch = 0; iSearch < nArea; iSearch++)
					{
						if (pbSet[iSearch] != true)
						{
							cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

							if (bUseSampling == true)	// jsyoon 2022.07.19 add
							{
								//	if (cellSecond.m_nSelectCell[M_COLOR] != true)
								if (cellSecond.m_nSelectCell[M_COLOR] != 1)
								{
									pbSet[iSearch] = true;
									continue;
								}
							}

							// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
							//if (cellFirst.m_fCellX < 50)
							if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
							{
								if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
								{
									pbSet[iGroup] = true;
									pbSet[iSearch] = true;
									measPos.Clear();
									measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
									measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
									measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fReviewZ;
									measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fColorZ;

									if (cellFirst.m_nArea >= nArea)
									{
										cellFirst.m_nArea -= (nArea * iStick);
									}

									if (cellSecond.m_nArea >= nArea)
									{
										cellSecond.m_nArea -= (nArea * iStick);
									}

									measPos.m_sCellInfo[0] = cellFirst;
									measPos.m_sCellInfo[1] = cellSecond;
									vecMeasGroup.push_back(measPos);

									break;
								}
							}
						}
					}
				}
			}
		}

		// 선택하고 남은것
		for (int iGroup = 0; iGroup < nArea; iGroup++)
		{
			if (pbSet[iGroup] != true)
			{
				pbSet[iGroup] = true;

				cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

				// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
				// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0. VISION 오프셋이 0 이 되어 X=0 셀을 Y3/Y1 이 맡아도 되고,
				//   ANGLE(Y3 = X-272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3 로 보내야 쉬는 Y4 Safety(-860)와 근접센서 간격(580)이 나온다.
				//if (cellFirst.m_fCellX > 50)
				// LYS 2026-09-18 담당 헤드를 셀 X 부호 대신 '갈 수 있고 쉬는 헤드와 간격이 큰 쪽'으로 고른다.
				//                구 줄은 바로 아래에 남겨 두었다(원복 시 이 블록 제거 후 주석 해제).
				//if (cellFirst.m_fCellX > 0)  // Head 2
				double dOffHeadP = 0.0, dOffHeadM = 0.0;
				GetHeadOffsetY(m_pShareItem->GetPositionConfigMgrPtr(), 3, &dOffHeadP, &dOffHeadM);
				if (PickSingleCellHead(m_pShareItem->GetPositionConfigMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(),
					cellFirst.m_fCellX, dOffHeadP, dOffHeadM, true, _T("COLOR")) != 0)  // Head 2
				{
					measPos.Clear();
					measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fColorZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[1] = cellFirst;
					cellFirst.m_fCellX -= dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[0] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
				else
				{
					measPos.Clear();
					measPos.dHeadBackG  = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ  = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fColorZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[0] = cellFirst;
					cellFirst.m_fCellX += dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[1] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
			}
		}

		delete[] pbSet;
		delete[] pnPair;	// LYS 2026-09-14

		double dGlassSize = 0; // 925 / 2;
		long nRet = df_MOTION_RV_SUCCESSED;
		nMeasArea = vecMeasGroup.size();
		bool bMoveFirst = false;

		if (nMeasArea > 0)
		{
			int nMeasStick = 0; // jsyoon 2022.09.29 add

			// TP 설정
	//		m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_3);
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_3);	 // jsyoon 2022.05.17 change

			//2023.05.22 자동 시퀀스 면계측
			BOOL bOneCell = FALSE;
			int nOneCellCnt = 1;
			if (nMeasArea == 1) {
				bOneCell = TRUE;
				nOneCellCnt = 2;
			}
			//

			for (int k = 0; k < nOneCellCnt; k++) 
			{
				if (k == 1) m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_4); //OneCell Head2는 시야각으로 설정

				for (int i = 0; i < nMeasArea; i++)
				{
					if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
					{
						bRet = false;
						break;
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_COLOR, true, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Move"), i);

					nRet = VTM_RV_SUCCESSED;

					if (bOneCell && k == 1) {
						nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;
						nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					}
					else {
						nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
						nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;
					}

					bMeasProbe[0] = true;
					bMeasProbe[1] = true;

					if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
					if (nUseCELLID[1] < 0) bMeasProbe[1] = false;

					// Set PG Num
					int nCellNum = 0;
					int iMeasMode = 0;
					if (nRet == VTM_RV_SUCCESSED)
					{
						/*!
						* 2023.03.09
						* 위치 이동 nCellStatus 위치는 iRotate 에 따라 표시 하는 위치가 달라져서 먼저 nMeasStick 을 구하고 상태를 읽어야 함
						*/
						// jsyoon 2022.09.29 change
						int iRotate = 0;
						if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
						{
							iRotate = 1;
						}

						ZeroMemory(cCellID, sizeof(cCellID));
						nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);

						// jsyoon 2022.06.21 add
						int iCellStatus = 0;
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] != true)	// 2023.03.09 add
								continue;

							nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
							iCellStatus = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick];

							// PROBE CHECK
													// PROBE CHECK
							/*!
							* 2023.03.09
							* 측정 동작 조건 수정
							*/
							if (iCellStatus == df_CELLSTATE_PTP_SHORT ||		// PTP
								iCellStatus == df_CELLSTATE_PTP_OPEN ||		// PTP
								iCellStatus == df_CELLSTATE_OST_SHORT ||		// OST
								iCellStatus == df_CELLSTATE_OST_OPEN ||		// OST
								iCellStatus == df_CELLSTATE_AGING_NG  			// AGING
								)
							{
								bMeasProbe[iProbe] = false;
							}

							//if (iCellStatus == df_CELLSTATE_ILDE ||			// 
							//	iCellStatus == df_CELLSTATE_PTP_NOR ||		// PTP
							//	iCellStatus == df_CELLSTATE_OST_NOR ||		// OST
							//	iCellStatus == df_CELLSTATE_PASS ||			// R-TEST
							//	iCellStatus == df_CELLSTATE_AGING_OK)		// AGING
							//{
							//	bMeasProbe[iProbe] = true;
							//}
							//else
							//{
							//	bMeasProbe[iProbe] = false;
							//}

							if (iCellStatus == df_CELLSTATE_OVER || iCellStatus == df_CELLSTATE_UNDER)  // R-TEST 
							{
								if (bUseRtestNgPass == true)
								{
									bMeasProbe[iProbe] = true;
								}
								else
								{
									bMeasProbe[iProbe] = false;
								}
							}
						}

						// 검사
						if (bMeasProbe[0] == true && bMeasProbe[1] != true)
						{
							iMeasMode = 1;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
						{
							iMeasMode = 2;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] != true)
						{
							iMeasMode = -1;
						}
						else
						{
							iMeasMode = 0;
						}
					}

					// Move
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_COLOR_MOVE, true, 0);

					if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
					{
						GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

						stMotorPosition stPosition[10];
						ZeroMemory(&stPosition, sizeof(stPosition));
						int nAxis = 0;

						double dLeft = 0., dRight = 0.;
						double dLeft2 = 0., dRight2 = 0.;
						bool bDirLtoR = true;

						if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
						{
							dLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
							dRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
						}
						else
						{
							dLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
							dRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

							bDirLtoR = false;
						}

						if (bOneCell)
						{
							/*!
							* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
							*/
							double dHeadGapRcpF = 700.0, dHeadGapRcpB = 700.0, dMovePosF = 0.0, dMovePosB = 0.0;
							if (pReciepInfo != nullptr)
							{
								dHeadGapRcpF = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
								dHeadGapRcpB = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
							}

							dMovePosF = dHeadGapRcpF / 2;
							dMovePosB = dHeadGapRcpB / 2;

							if (bDirLtoR == true)	//  - -> +
							{
								dLeft2 = dMovePosF * -1;
								dRight2 = dMovePosF;

								dLeft = dMovePosB * -1;
								dRight = dMovePosB;
							}
							else  //  + -> -
							{
								dLeft2 = dMovePosF;
								dRight2 = dMovePosF * -1;

								dLeft = dMovePosB;
								dRight = dMovePosB * -1;
							}
						}
						else
						{
							dLeft2 = dLeft;
							dRight2 = dRight;
						}

						double fSafetyH1 = 0.0, fSafetyH2 = 0.0;
						m_pShareItem->GetPositionConfigMgrPtr()->GetSafetyPosition(nullptr, nullptr, nullptr, &fSafetyH1, &fSafetyH2, nullptr, nullptr);
						m_pShareItem->GetPositionConfigMgrPtr()->GetFrontHeadSafetyPosition(&fSafetyH1, &fSafetyH2);	// LYS 2026-09-14 아래 dLeft2/dRight2 는 FRONT 헤드(Y3/Y4)라 FRONT Safety 키를 쓴다
						if (bOneCell) {
							if (k == 0)	dLeft2  = fSafetyH1;
							else		dRight2 = fSafetyH2;
						}

						if (bMoveFirst == true)
						{
							stPosition[0].axis = AXIS_Y3;   nAxis++;
							stPosition[0].dPosition = dLeft2;// dLeft;
							stPosition[1].axis = AXIS_Y4;   nAxis++;
							stPosition[1].dPosition = dRight2;// dRight;
						}
						else
						{
							stPosition[0].axis = AXIS_Y3;   nAxis++;
							stPosition[0].dPosition = dLeft2;// dLeft;
							stPosition[1].axis = AXIS_Y4;   nAxis++;
							stPosition[1].dPosition = dRight2;// dRight;

							stPosition[2].axis = AXIS_Y1;   nAxis++;
							stPosition[2].dPosition = dLeft;
							stPosition[3].axis = AXIS_Y2;   nAxis++;
							stPosition[3].dPosition = dRight;
							// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (df_HEAD_Y_IDLE_SAFETY -> MoveCmdPos* 의 ApplyIdleHeadSafetyY 가 치환).
							//   FRONT 측정 첫 이동에서 BACK 헤드가 셀 좌표 + ANGLE 오프셋(MoveCmdPosVision/Angle)으로 나가 ?272 면 Y1-Y2 간격이 인터락(720) 미달로 거부된다. BACK 갠트리는 이때 SafetyHeadX1 에 있다.
							if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
							{
								stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
								stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
							}
						
							stPosition[4].axis = AXIS_X3;   nAxis++;
							stPosition[4].dPosition = vecMeasGroup[i].dHeadBackG + dOffsetX;	// 2023.07.05 add

							stPosition[5].axis = AXIS_Z1;   nAxis++;
							stPosition[5].dPosition = vecMeasGroup[i].dHeadFrontZ;
							stPosition[6].axis = AXIS_Z2;   nAxis++;
							stPosition[6].dPosition = vecMeasGroup[i].dHeadBackZ;
						}

						/*!
						* 이동시 Cross 축만 모션 완료 확인을 나중에 한다
						*/
						MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosColor(stPosition, nAxis, SPEED_HIGH, false);

						if (nRet == VTM_RV_SUCCESSED)
						{
							bMoveFirst = true;
						}

						if (nRet == VTM_RV_SUCCESSED)
						{
							// PG 번호 변경
							for (int iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
							{
								if (bMeasProbe[iProbe] == true) {
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
								}
								else
								{
									if (iProbe == 0) {
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
									}
									else
									{
										/*!
										* 2023.03.09 change
										* Cell 하나 측정 시 다른 PG 번호를 임의로 설정
										* m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
										*/
										if (nArea == 1)
											m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 11);
										else
											m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
									}
								}

								/*if (nUseCELLID[iProbe] >= 0)
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);*/
							}
						}

						// Wait Done
						if (nRet == VTM_RV_SUCCESSED)
						{
							int axis = 0;
							for (int n = 0; n < 2; n++)
							{
								axis = stPosition[n].axis;
								nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
								if (nRet != df_MOTION_RV_SUCCESSED)
									break;
							}
						}
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_COLOR_MOVE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);

					if (nRet != VTM_RV_SUCCESSED) {
						bRet = false;
						m_pShareItem->SetFail(true, nRet);
						break;
					}

					// 검사
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_COLOR_SAMPLE, true, 0);
					
					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), i);

					if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true)
							{
								nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_COLOR_START;
							}
						}

						//Area 정보, 측정 진행 유무
						int nAreaNum[DF_MAX_HEAD] = { 0, };
						//nAreaNum[0] = cellFirst.m_nArea + (nArea * iStick);
						//nAreaNum[1] = cellSecond.m_nArea + (nArea * iStick);
						nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
						nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
						m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);
						//

						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_COLOR, iMeasMode);
						if (nRet == VTM_RV_SUCCESSED)
						{
							nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
						}

						if (nRet == VTM_RV_SUCCESSED)
						{
							nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
						}

						if (nRet == VTM_RV_SUCCESSED)
						{
							for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
							{
								if (bMeasProbe[iProbe] == true)
								{
									nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
									m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_COLOR_END;
								}
							}
						}

						if (nRet != VTM_RV_SUCCESSED)
							m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);	// jsyoon 2022.06.17 add
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_COLOR_SAMPLE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_COLOR, false, 0);
				}
			}
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_COLOR End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_COLOR, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_ANGLE::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 6);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_ANGLE, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_ANGLE Start."));


	bool bRet = true;
	bool bUseMeas = true;
	bool bUseMeasLCR = true;
	bool bUseRtestNgPass = false;
	bool bUseSampling = false;
	int nUseCELLID[df_MAX_PROBE];
	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
	bool bMeasProbe[df_MAX_PROBE];
	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME, nTimeOutLCR = DEFINE_INSPECITON_WAIT_TIME;

	// Check Meas
	double dOffsetX = 0.;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas    = (bool)pReciepInfo->m_sInspectItem.m_bUseAngle_6;
		bUseMeasLCR = (bool)pReciepInfo->m_sInspectItem.m_bUseLCR_8;
		// LYS 2026-08-06 AgingCycle E3: per-item select (ANGLE flag + stick cell map)
		if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
		{
			const S_AgingTest* pAgingItem = m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetItem(m_pMeasSeqItemEngine->GetAgingCurItem());
			bUseMeas = (pAgingItem != NULL && pAgingItem->m_bUseAngle)
				&& (m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetStickCount() == 0
					|| m_pMeasSeqItemEngine->GetAgingCycleInfo()->IsStickUse(GetMeasureStick(), enumAgingMeasKind_ANGLE));

			if (bUseMeas && m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
			{
				// LYS 2026-08-06 AgingCycle E4: progress marker (col 6 = ANGLE)
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = m_pMeasSeqItemEngine->GetAgingCurItem() + 1;
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 6;
			}
		}
		if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive()) bUseMeasLCR = false;	// LYS 2026-08-06 AgingCycle E3

		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[6];
		nTimeOutLCR = pReciepInfo->m_sInspectItem.m_nTimeOut[8];

		bUseRtestNgPass = pReciepInfo->m_sOptionItem.m_bUseRtestNgPass;	// jsyoon 2022.06.21

		bUseSampling = pReciepInfo->m_sOptionItem.m_bUseSampling;

		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;	// 2023.07.06 add
	}

	if (bUseMeas == true)
	{
		// 좌표 생성

		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nMeasArea = 0;

		double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
	//	m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, nullptr, nullptr, &dHeadGap);	// 2023.03.08 change

		dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		if (dHeadGapRcp > dHeadGap)
		{
			dHeadGap = dHeadGapRcp;
		}

		double dGlassConactPos = 0;
//		dGlassConactPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - 20;

		S_RecipeData* pRecipeData;
		pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		int iStick = 0, nArea, nMaxStick = 0;
		iStick = GetMeasureStick();
		nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
		nMaxStick = pRecipeData->m_vecStick.size();

		//dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;
		dGlassConactPos = pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;		// LYS 2026-06-29

		// Generate 2Head Group
		bool* pbSet = new bool[nArea];
		for (int iGroup = 0; iGroup < nArea; iGroup++)
			pbSet[iGroup] = false;

		vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
		vecMeasGroup.clear();

		S_CellInfo cellFirst, cellSecond;
		S_MeasPos measPos;
		cellFirst.Clear();
		cellSecond.Clear();

		// LYS 2026-08-26 특성 Z축 인터락
		double dBackZ = 0;
		//m_pShareItem->GetPositionConfigMgrPtr()->GetSafetyPosition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &dBackZ);

		// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (MakeHeadPairByOrder 주석 참조).
		//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 필터는 아래 루프와 같은 조건을 미리 적용한다.
		int* pnPair = new int[nArea];
		{
			bool* pbUse = new bool[nArea];
			for (int iCell = 0; iCell < nArea; iCell++)
			{
				pbUse[iCell] = true;
				if (IsAgingCellSkip(enumAgingMeasKind_ANGLE, iCell))
					pbUse[iCell] = false;
				if (bUseSampling == true && pRecipeData->m_vecStick[iStick].m_vecCell[iCell].m_nSelectCell[M_ANGLE] != 1)
					pbUse[iCell] = false;
			}
			// LYS 2026-09-14 짝 조건에 헤드 오프셋 반영. 축 목표 = 셀 X + 오프셋이므로 두 헤드 목표 간격 = 셀 거리 + (HEAD1 - HEAD2 오프셋).
			//   인터락(dHeadGap)은 축 목표 간격을 보므로 셀 거리 조건은 dHeadGap - (o1 - o2). WAD(+168/-168)는 384, VISION(0)은 그대로,
			//   ANGLE(-272/+272)은 1264 가 되어 사실상 단독 측정으로 간다(2026-09-14 센서 배치 기준).
			double dPairGap = dHeadGap;
			{
				double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 1, &dOffX, &dOffY1, &dOffZ);
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 1, &dOffX, &dOffY2, &dOffZ);
				dPairGap = dHeadGap - (dOffY1 - dOffY2);
			}
			MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, pbUse, nArea, dPairGap, pnPair, _T("ANGLE"));
			delete[] pbUse;
		}

		if (nArea > 1)
		{
			for (int iGroup = 0; iGroup < nArea; iGroup++)
			{
				if (pbSet[iGroup] != true)
				{
					cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

					// LYS 2026-08-18 AgingCycle: AgingRcp [MASECELL] 기반 셀 단위 필터.
					// pbSet 표시로 아래 잔여 단일헤드 패스에서도 제외된다.
					if (IsAgingCellSkip(enumAgingMeasKind_ANGLE, iGroup))
					{
						pbSet[iGroup] = true;
						continue;
					}

					if (bUseSampling == true)	// jsyoon 2022.07.19 add
					{
						//	if (cellFirst.m_nSelectCell[M_ANGLE] != true)
						if (cellFirst.m_nSelectCell[M_ANGLE] != 1)
						{
							pbSet[iGroup] = true;
							continue;
						}
					}

					cellSecond.Clear();
					for (int iSearch = 0; iSearch < nArea; iSearch++)
					{
						if (pbSet[iSearch] != true)
						{
							cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

							// LYS 2026-08-18 AgingCycle: per-cell filter from AgingRcp [MASECELL]
							if (IsAgingCellSkip(enumAgingMeasKind_ANGLE, iSearch))
							{
								pbSet[iSearch] = true;
								continue;
							}

							if (bUseSampling == true)	// jsyoon 2022.07.19 add
							{
								//	if (cellSecond.m_nSelectCell[M_ANGLE] != true)
								if (cellSecond.m_nSelectCell[M_ANGLE] != 1)
								{
									pbSet[iSearch] = true;
									continue;
								}
							}

							// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
							//if (cellFirst.m_fCellX < 50)
							if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
							{
								if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
								{
									pbSet[iGroup] = true;
									pbSet[iSearch] = true;
									measPos.Clear();
									measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
									measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
									//measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
									//measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;
									measPos.dHeadBackZ = dBackZ;
									measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fAngleZ;

									if (cellFirst.m_nArea >= nArea)
									{
										cellFirst.m_nArea -= (nArea * iStick);
									}

									if (cellSecond.m_nArea >= nArea)
									{
										cellSecond.m_nArea -= (nArea * iStick);
									}

									measPos.m_sCellInfo[0] = cellFirst;
									measPos.m_sCellInfo[1] = cellSecond;
									vecMeasGroup.push_back(measPos);

									break;
								}
							}
						}
					}
				}
			}
		}

		// 선택하고 남은것
		for (int iGroup = 0; iGroup < nArea; iGroup++)
		{
			if (pbSet[iGroup] != true)
			{
				pbSet[iGroup] = true;

				cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

				// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
				// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0. VISION 오프셋이 0 이 되어 X=0 셀을 Y3/Y1 이 맡아도 되고,
				//   ANGLE(Y3 = X-272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3 로 보내야 쉬는 Y4 Safety(-860)와 근접센서 간격(580)이 나온다.
				//if (cellFirst.m_fCellX > 50)
				// LYS 2026-09-18 담당 헤드를 셀 X 부호 대신 '갈 수 있고 쉬는 헤드와 간격이 큰 쪽'으로 고른다.
				//                구 줄은 바로 아래에 남겨 두었다(원복 시 이 블록 제거 후 주석 해제).
				//if (cellFirst.m_fCellX > 0)  // Head 2
				double dOffHeadP = 0.0, dOffHeadM = 0.0;
				GetHeadOffsetY(m_pShareItem->GetPositionConfigMgrPtr(), 1, &dOffHeadP, &dOffHeadM);
				if (PickSingleCellHead(m_pShareItem->GetPositionConfigMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(),
					cellFirst.m_fCellX, dOffHeadP, dOffHeadM, true, _T("ANGLE")) != 0)  // Head 2
				{
					measPos.Clear();
					measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
					//measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					//measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;
					measPos.dHeadBackZ = dBackZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fAngleZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[1] = cellFirst;
					cellFirst.m_fCellX -= dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[0] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
				else
				{
					measPos.Clear();
					measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
					//measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					//measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;
					measPos.dHeadBackZ = dBackZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fAngleZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[0] = cellFirst;
					cellFirst.m_fCellX += dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[1] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
			}
		}

		delete[] pbSet;
		delete[] pnPair;	// LYS 2026-09-14

		double dGlassSize = 0;//925 / 2;
		long nRet = df_MOTION_RV_SUCCESSED;
		nMeasArea = vecMeasGroup.size();
		bool bMoveFirstDone = false;
		if (nMeasArea > 0)
		{
			int nMeasStick = 0;	// jsyoon 2022.09.29 add

			// TP 설정
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_4);
			
			for (int i = 0; i < nMeasArea; i++)
			{
				if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
				{
					bRet = false;
					break;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_ANGLE, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Move"), i);

				nRet = VTM_RV_SUCCESSED;

				nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
				nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;

				bMeasProbe[0] = true;
				bMeasProbe[1] = true;

				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;

				// Set PG Num
				int nCellNum = 0;
				int iMeasMode = 0;
				if (nRet == VTM_RV_SUCCESSED)
				{
					/*!
					* 2023.03.09
					* 위치 이동 nCellStatus 위치는 iRotate 에 따라 표시 하는 위치가 달라져서 먼저 nMeasStick 을 구하고 상태를 읽어야 함
					*/
					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					ZeroMemory(cCellID, sizeof(cCellID));
					nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);

					// jsyoon 2022.06.21 add
					int iCellStatus = 0;
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] != true)	// 2023.03.09 add
							continue;

						nCellNum = nUseCELLID[iProbe]; // nCellNum = nMeasArea* iProbe + i;
						iCellStatus = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick];

						// PROBE CHECK
												// PROBE CHECK
						/*!
						* 2023.03.09
						* 측정 동작 조건 수정
						*/
						if (iCellStatus == df_CELLSTATE_PTP_SHORT ||		// PTP
							iCellStatus == df_CELLSTATE_PTP_OPEN ||		// PTP
							iCellStatus == df_CELLSTATE_OST_SHORT ||		// OST
							iCellStatus == df_CELLSTATE_OST_OPEN ||		// OST
							iCellStatus == df_CELLSTATE_AGING_NG  			// AGING
							)
						{
							bMeasProbe[iProbe] = false;
						}

						//if (iCellStatus == df_CELLSTATE_ILDE ||			// 
						//	iCellStatus == df_CELLSTATE_PTP_NOR ||		// PTP
						//	iCellStatus == df_CELLSTATE_OST_NOR ||		// OST
						//	iCellStatus == df_CELLSTATE_PASS ||			// R-TEST
						//	iCellStatus == df_CELLSTATE_AGING_OK)		// AGING
						//{
						//	bMeasProbe[iProbe] = true;
						//}
						//else
						//{
						//	bMeasProbe[iProbe] = false;
						//}

						if (iCellStatus == df_CELLSTATE_OVER || iCellStatus == df_CELLSTATE_UNDER)  // R-TEST 
						{
							if (bUseRtestNgPass == true)
							{
								bMeasProbe[iProbe] = true;
							}
							else
							{
								bMeasProbe[iProbe] = false;
							}
						}
					}

					// 검사
					if (bMeasProbe[0] == true && bMeasProbe[1] != true)
					{
						iMeasMode = 1;
					}
					else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
					{
						iMeasMode = 2;
					}
					else if (bMeasProbe[0] != true && bMeasProbe[1] != true)
					{
						iMeasMode = -1;
					}
					else
					{
						iMeasMode = 0;
					}

					//// jsyoon 2022.09.29 change
					//int iRotate = 0;
					//if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//{
					//	iRotate = 1;
					//}

					//ZeroMemory(cCellID, sizeof(cCellID));
					//nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);
					//CString strCellId;
					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	if (bMeasProbe[iProbe] == true)
					//	{
					//		nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
					////		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][iStick] = df_CELLSTATE_ANGLE_START;

					//		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		}
					//		else
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + iStick, nArea - nCellNum);
					//		}

					//		//strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		//	strCellId.Format(_T("%c%02d"), 65, nCellNum + 1);
					//		//	strCellId.Format(_T("%c%02d"), 65 + nCellNum, nMaxStick - iStick);
					//			/*strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + iProbe + 1);*/
					//		_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//	}
					//}
				}

				// Move
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_ANGLE_MOVE, true, 0);

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					ClearInnerIDForAging(nUseCELLID);	// LYS 2026-08-12 AgingCycle R2
					GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

					stMotorPosition stPosition[22];	// LYS 2026-08-19 7 head + 10 angle module axes
					ZeroMemory(&stPosition, sizeof(stPosition));
					int nAxis = 0;

					double dLeft = 0., dRight = 0.;
					double dLeft2 = 0., dRight2 = 0.;	// 2023.03.08 add
					bool bDirLtoR = true;

					if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
					}
					else
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

						bDirLtoR = false;
					}

					if (nMeasArea == 1)
					{
						/*!
						* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
						*/
						double dHeadGapRcp = 700, dMovePos = 0;
						if (pReciepInfo != nullptr)
						{
							dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
						}

						dMovePos = dHeadGapRcp / 2;

						if (bDirLtoR == true)	//  - -> +
						{
							dLeft2 = dMovePos * -1;
							dRight2 = dMovePos;
						}
						else  //  + -> -
						{
							dLeft2 = dMovePos;
							dRight2 = dMovePos * -1;
						}
					}
					else
					{
						dLeft2 = dLeft;
						dRight2 = dRight;
					}

					if (bMoveFirstDone == true)
					{
						//stPosition[0].axis = AXIS_Y1;   nAxis++;
						//stPosition[0].dPosition = dLeft;
						//stPosition[1].axis = AXIS_Y2;   nAxis++;
						//stPosition[1].dPosition = dRight;

						stPosition[0].axis = AXIS_Y3;   nAxis++;
						stPosition[0].dPosition = dLeft2;
						stPosition[1].axis = AXIS_Y4;   nAxis++;
						stPosition[1].dPosition = dRight2;
					}
					else
					{
						//stPosition[0].axis = AXIS_Y1;   nAxis++;
						//stPosition[0].dPosition = dLeft;
						//stPosition[1].axis = AXIS_Y2;   nAxis++;
						//stPosition[1].dPosition = dRight;
						stPosition[2].axis = AXIS_Y3;   nAxis++;
						stPosition[2].dPosition = dLeft2;
						stPosition[3].axis = AXIS_Y4;   nAxis++;
						stPosition[3].dPosition = dRight2;


						// LYS 2026-06-26 OCP: X3 primary axis for ANGLE
						if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
							stPosition[4].axis = AXIS_X3;
						else
							stPosition[4].axis = AXIS_X2;
						nAxis++;
						stPosition[4].dPosition = vecMeasGroup[i].dHeadFrontG + dOffsetX; // 2023.07.06 add
						//stPosition[5].axis = AXIS_Z1;   nAxis++;
						//stPosition[5].dPosition = vecMeasGroup[i].dHeadFrontZ;
						//stPosition[6].axis = AXIS_Z2;   nAxis++;
						//stPosition[6].dPosition = vecMeasGroup[i].dHeadBackZ;
						stPosition[5].axis = AXIS_Z1;   nAxis++;
						stPosition[5].dPosition = vecMeasGroup[i].dHeadBackZ;
						stPosition[6].axis = AXIS_Z2;   nAxis++;
						stPosition[6].dPosition = vecMeasGroup[i].dHeadFrontZ;

						// LYS 2026-08-19 Angle 모듈 광학계는 레시피를 따른다. 레시피가 모듈당 값 1세트만
						// 가지므로 셀마다 달라질 수 없어, 첫 전체 이동에서 적용하면 ANGLE 스텝 전체를
						// 커버하고 tact 손해도 없다(MoveCmdPosAngle이 헤드 이동 전에 지령하고 끝에서 대기).
						if (pReciepInfo != nullptr)
						{
							stPosition[7].axis = AXIS_ANGLE_SF1;	nAxis++;
							stPosition[7].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleScheimpflug1;
							stPosition[8].axis = AXIS_ANGLE_ROTATE1;	nAxis++;
							stPosition[8].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleRotate1;
							stPosition[9].axis = AXIS_ANGLE_ZOOM1;	nAxis++;
							stPosition[9].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleZoom1;
							stPosition[10].axis = AXIS_ANGLE_TILT1;	nAxis++;
							stPosition[10].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleTilt1;
							stPosition[11].axis = AXIS_ANGLE_AZM1;	nAxis++;
							stPosition[11].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleAzimuth1;
							stPosition[12].axis = AXIS_ANGLE_SF2;	nAxis++;
							stPosition[12].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleScheimpflug2;
							stPosition[13].axis = AXIS_ANGLE_ROTATE2;	nAxis++;
							stPosition[13].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleRotate2;
							stPosition[14].axis = AXIS_ANGLE_ZOOM2;	nAxis++;
							stPosition[14].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleZoom2;
							stPosition[15].axis = AXIS_ANGLE_TILT2;	nAxis++;
							stPosition[15].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleTilt2;
							stPosition[16].axis = AXIS_ANGLE_AZM2;	nAxis++;
							stPosition[16].dPosition = pReciepInfo->m_sMotionAngle.m_fAngleAzimuth2;

							//LYS 2026-08-28 Sub Z(Z5/Z6) - VISION/ANGLE(화질/시야각) 측정 공용
							// LYS 2026-09-18 ANGLE 은 VISION 과 Sub Z 축(Z5/Z6)은 같지만 목표 높이를 따로 쓴다.
							//                광학계가 달라 초점 거리가 같지 않다. 레시피 SUB Z 그리드의 ANGLE RIGHT/LEFT.
							//                구파일(V1009 이하)에는 값이 없어 0 으로 로드된다 - 레시피에 값을 넣고 저장할 것.
							stPosition[17].axis = AXIS_Z5;	nAxis++;
							//stPosition[17].dPosition = pReciepInfo->m_sMotionSubZ.m_fVisionSubzRight;
							stPosition[17].dPosition = pReciepInfo->m_sMotionSubZ.m_fAngleSubzRight;
							stPosition[18].axis = AXIS_Z6;	nAxis++;
							//stPosition[18].dPosition = pReciepInfo->m_sMotionSubZ.m_fVisionSubzLeft;
							stPosition[18].dPosition = pReciepInfo->m_sMotionSubZ.m_fAngleSubzLeft;

							stPosition[19].axis = AXIS_Z3;	nAxis++;
							stPosition[19].dPosition = 0;
							stPosition[20].axis = AXIS_Z4;	nAxis++;
							stPosition[20].dPosition = 0;
						}
					}

					/*!
					* 이동시 Cross 축만 모션 완료 확인을 나중에 한다
					*/
					MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAngle(stPosition, nAxis, SPEED_HIGH, false);

					if (nRet == VTM_RV_SUCCESSED)
					{
						bMoveFirstDone = true;
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						// PG 번호 변경
						for (int iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true) {
								m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
							}
							else
							{
								if (iProbe == 0) {
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
								}
								else
								{
									/*!
									* 2023.03.09 change
									* Cell 하나 측정 시 다른 PG 번호를 임의로 설정
									* m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
									*/
									if (nArea == 1)
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 11);
									else
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
								}
							}

							/*if (bMeasProbe[iProbe] == true)
								m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);*/
						}
					}

					// Wait Done
					if (nRet == VTM_RV_SUCCESSED)
					{
						int axis = 0;
						for (int n = 0; n < 2; n++)
						{
							axis = stPosition[n].axis;
							nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
							if (nRet != df_MOTION_RV_SUCCESSED)
								break;
						}
					}
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_ANGLE_MOVE, false, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);
					
				if (nRet != VTM_RV_SUCCESSED) {
					bRet = false;
					m_pShareItem->SetFail(true, nRet);
					break;
				}
				
				// 검사
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_ANGLE_SAMPLE, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), i);

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] == true)
						{
							nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_ANGLE_START;
						}
					}

					//Area 정보, 측정 진행 유무
					int nAreaNum[DF_MAX_HEAD] = { 0, };

					// LYS 2026-08-12 AgingCycle R1: 에이징 반복마다 결과 행 블록 하나
					// (OCP 레이아웃, JOB_Proc.cpp:13934 계열). 사이클 비활성 시 0.
					int nAgingRow = 0;
					if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive())
						nAgingRow = nArea * nMaxStick * m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_ANGLE);
					//nAreaNum[0] = cellFirst.m_nArea + (nArea * iStick);
					//nAreaNum[1] = cellSecond.m_nArea + (nArea * iStick);
					nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + nAgingRow;
					nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1 + nAgingRow;
					m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);
					LogAgingRow(enumAgingMeasKind_ANGLE, _T("ANGLE"), nAreaNum);	// LYS 2026-08-12 AgingCycle R1
					//

					nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_ANGLE, iMeasMode, nTimeOut);
					if (nRet == VTM_RV_SUCCESSED)
					{
						if (i == 0 && bUseMeasLCR == true)
						{
							CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_LCR_SAMPLE, true, 0);

							m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("LCR"), 0);

							nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_LCR, 0);
							if (nRet == VTM_RV_SUCCESSED)
							{
								nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_LCR, 0, nTimeOutLCR);
							}

							if (nRet == VTM_RV_SUCCESSED)
							{
								nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_LCR, 0, nTimeOutLCR);
							}

							m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("LCR"), 0);

							CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_LCR_SAMPLE, false, 0);
						}
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_ANGLE, iMeasMode, nTimeOut);
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_ANGLE, iMeasMode, nTimeOut);
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true)
							{
								nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_ANGLE_END;
							}
						}
					}
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_ANGLE_SAMPLE, false, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_ANGLE, false, 0);
			}
		}
	}

	// LYS 2026-08-12 AgingCycle R1: advance the ANGLE repeat once per step run
	if (bUseMeas == true && m_pMeasSeqItemEngine != NULL)
		m_pMeasSeqItemEngine->AgingRepeatNext(enumAgingMeasKind_ANGLE);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_ANGLE End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_ANGLE, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_TSP::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 7);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_TSP_SAMPLE, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_TSP Start."));

	int bRet = true;
	bool bUseMeas = false;
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME;

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = (bool)pReciepInfo->m_sInspectItem.m_bUseTSP_7;

		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[7];
	}

	// Check Meas
	if (bUseMeas == true)
	{
		if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
		{
			bRet = false;
		}
		else
		{
			long nRet = VTM_RV_SUCCESSED;

			// TP 특성 설정
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_5);

			// 검사
			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), 0);
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_TSP, 0);

			if (nRet == VTM_RV_SUCCESSED)
			{
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_TSP, 0, nTimeOut);
			}
			
			if (nRet == VTM_RV_SUCCESSED)
			{
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_TSP, 0, nTimeOut);
			}
			
			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), 0);
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_TSP End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_TSP_SAMPLE, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	return bRet;
}

bool CDoWorkGlassMeasureStep_Measure_VISION_COLOR::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_Start, 4);

	int iStick = GetMeasureStick();
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_VISION, true, 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_VISION_COLOR Start."));

	bool bRet = true;
	bool bUseMeas = true;
	bool bUseRtestNgPass = false;
	bool bUseSampling = false;

	int nUseCELLID[df_MAX_PROBE];
	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
	bool bMeasProbe[df_MAX_PROBE];
	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nTimeOut = DEFINE_INSPECITON_WAIT_TIME;

	// Check Meas
	double dOffsetX = 0.;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseQuality_4;
		nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[4];

		bUseRtestNgPass = pReciepInfo->m_sOptionItem.m_bUseRtestNgPass;	// jsyoon 2022.06.21

		bUseSampling = pReciepInfo->m_sOptionItem.m_bUseSampling; // jsyoon 2022.07.19 add

		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;	// 2023.07.06 add
	}

	int iMeasMode = 0;

	bool bUseVisionColor = m_pShareItem->GetSystemConfigMgrPtr()->IsUseInspVisionColor();

	if (bUseMeas == true && bUseVisionColor)
	{
		// 좌표 생성
		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nMeasArea = 0;
		double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);

		dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		if (dHeadGapRcp > dHeadGap)
		{
			dHeadGap = dHeadGapRcp;
		}

		double dGlassConactPos = 0;

		S_RecipeData* pRecipeData;
		pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		int iStick = 0, nArea, nMaxStick = 0;
		iStick = GetMeasureStick();
		nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
		nMaxStick = pRecipeData->m_vecStick.size();

		dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

		// Generate 2Head Group
		bool* pbSet = new bool[nArea];
		for (int iGroup = 0; iGroup < nArea; iGroup++)
			pbSet[iGroup] = false;

		vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
		vecMeasGroup.clear();

		S_CellInfo cellFirst, cellSecond;
		S_MeasPos measPos;
		cellFirst.Clear();
		cellSecond.Clear();

		// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (MakeHeadPairByOrder 주석 참조).
		//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 필터는 아래 루프와 같은 조건을 미리 적용한다.
		int* pnPair = new int[nArea];
		{
			bool* pbUse = new bool[nArea];
			for (int iCell = 0; iCell < nArea; iCell++)
			{
				pbUse[iCell] = true;
				if (bUseSampling == true && pRecipeData->m_vecStick[iStick].m_vecCell[iCell].m_nSelectCell[M_QUALITY] != 1)
					pbUse[iCell] = false;
			}
			// LYS 2026-09-14 짝 조건에 헤드 오프셋 반영. 축 목표 = 셀 X + 오프셋이므로 두 헤드 목표 간격 = 셀 거리 + (HEAD1 - HEAD2 오프셋).
			//   인터락(dHeadGap)은 축 목표 간격을 보므로 셀 거리 조건은 dHeadGap - (o1 - o2). WAD(+168/-168)는 384, VISION(0)은 그대로,
			//   ANGLE(-272/+272)은 1264 가 되어 사실상 단독 측정으로 간다(2026-09-14 센서 배치 기준).
			double dPairGap = dHeadGap;
			{
				double dOffX = 0.0, dOffY1 = 0.0, dOffY2 = 0.0, dOffZ = 0.0;
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(0, 2, &dOffX, &dOffY1, &dOffZ);
				m_pShareItem->GetPositionConfigMgrPtr()->GetHeadOffset(1, 2, &dOffX, &dOffY2, &dOffZ);
				dPairGap = dHeadGap - (dOffY1 - dOffY2);
			}
			MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, pbUse, nArea, dPairGap, pnPair, _T("VISION_COLOR"));
			delete[] pbUse;
		}

		if (nArea > 1)
		{
			for (int iGroup = 0; iGroup < nArea; iGroup++)
			{
				if (pbSet[iGroup] != true)
				{
					cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

					if (bUseSampling == true)	// jsyoon 2022.07.19 add
					{
						//	if (cellFirst.m_nSelectCell[M_QUALITY] != true)
						if (cellFirst.m_nSelectCell[M_QUALITY] != 1)
						{
							pbSet[iGroup] = true;
							continue;
						}
					}

					cellSecond.Clear();
					for (int iSearch = 0; iSearch < nArea; iSearch++)
					{
						if (pbSet[iSearch] != true)
						{
							cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

							if (bUseSampling == true)	// jsyoon 2022.07.19 add
							{
								//	if (cellSecond.m_nSelectCell[M_QUALITY] != true)
								if (cellSecond.m_nSelectCell[M_QUALITY] != 1)
								{
									pbSet[iSearch] = true;
									continue;
								}
							}

							// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
							//if (cellFirst.m_fCellX < 50)
							if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
							{
								if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
								{
									pbSet[iGroup] = true;
									pbSet[iSearch] = true;
									measPos.Clear();
									measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
									measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
									measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fReviewZ;
									measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

									if (cellFirst.m_nArea >= nArea)
									{
										cellFirst.m_nArea -= (nArea * iStick);
									}

									if (cellSecond.m_nArea >= nArea)
									{
										cellSecond.m_nArea -= (nArea * iStick);
									}

									measPos.m_sCellInfo[0] = cellFirst;
									measPos.m_sCellInfo[1] = cellSecond;
									vecMeasGroup.push_back(measPos);

									break;
								}
							}
						}
					}
				}
			}
		}


		// 선택하고 남은것
		for (int iGroup = 0; iGroup < nArea; iGroup++)
		{
			if (pbSet[iGroup] != true)
			{
				pbSet[iGroup] = true;

				cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

				// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
				// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0. VISION 오프셋이 0 이 되어 X=0 셀을 Y3/Y1 이 맡아도 되고,
				//   ANGLE(Y3 = X-272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3 로 보내야 쉬는 Y4 Safety(-860)와 근접센서 간격(580)이 나온다.
				//if (cellFirst.m_fCellX > 50)
				// LYS 2026-09-18 담당 헤드를 셀 X 부호 대신 '갈 수 있고 쉬는 헤드와 간격이 큰 쪽'으로 고른다.
				//                구 줄은 바로 아래에 남겨 두었다(원복 시 이 블록 제거 후 주석 해제).
				//if (cellFirst.m_fCellX > 0)  // Head 2
				double dOffHeadP = 0.0, dOffHeadM = 0.0;
				GetHeadOffsetY(m_pShareItem->GetPositionConfigMgrPtr(), 2, &dOffHeadP, &dOffHeadM);
				if (PickSingleCellHead(m_pShareItem->GetPositionConfigMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(),
					cellFirst.m_fCellX, dOffHeadP, dOffHeadM, true, _T("VISION_COLOR")) != 0)  // Head 2
				{
					measPos.Clear();
					measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[1] = cellFirst;
					cellFirst.m_fCellX -= dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[0] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
				else
				{
					measPos.Clear();
					measPos.dHeadBackG = cellFirst.m_fCellY + dGlassConactPos;
					measPos.dHeadFrontG = measPos.dHeadBackG - dGantryGap;
					measPos.dHeadBackZ = pRecipeData->m_sHeadZ.m_fAngleZ;
					measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fQualityZ;

					if (cellFirst.m_nArea >= nArea)
					{
						cellFirst.m_nArea -= (nArea * iStick);
					}

					measPos.m_sCellInfo[0] = cellFirst;
					cellFirst.m_fCellX += dHeadGap;
					cellFirst.m_nArea = -1;
					measPos.m_sCellInfo[1] = cellFirst;
					vecMeasGroup.push_back(measPos);
				}
			}
		}

		delete[] pbSet;
		delete[] pnPair;	// LYS 2026-09-14

		double dGlassSize = 0; // 925 / 2;
		long nRet = df_MOTION_RV_SUCCESSED;
		nMeasArea = vecMeasGroup.size();
		bool bMoveFirst = false;
		stMotorPosition stPosition[20];

		//// 위치 이동 하여 검사 초기화 및 해제 (Cycle 시작전, Auto Proc 시작전, 검사 종료 후)
		//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_RESET);		  //화질 자동검사 시작시 초기화
		//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_WAIT_USE, TRUE); //화질 자동결과 설정

		if (nMeasArea > 0)
		{
			int nMeasStick = 0;	// jsyoon 2022.09.29 add

			// TP 설정
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_2);

			for (int i = 0; i < nMeasArea; i++)
			{
				if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
				{
					bRet = false;
					break;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Move"), i);

				nRet = VTM_RV_SUCCESSED;

				nUseCELLID[0] = vecMeasGroup[i].m_sCellInfo[0].m_nArea;
				nUseCELLID[1] = vecMeasGroup[i].m_sCellInfo[1].m_nArea;

				bMeasProbe[0] = true;
				bMeasProbe[1] = true;

				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;


				int nCellNum = 0;
				// Set PG Num
				if (nRet == VTM_RV_SUCCESSED)
				{
					/*!
					* 2023.03.09
					* 위치 이동 nCellStatus 위치는 iRotate 에 따라 표시 하는 위치가 달라져서 먼저 nMeasStick 을 구하고 상태를 읽어야 함
					*/
					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					ZeroMemory(cCellID, sizeof(cCellID));

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						int nUseHead[df_MAX_PROBE];
						ZeroMemory(nUseHead, sizeof(nUseHead));

						bool bUseProbe[df_MAX_PROBE];
						ZeroMemory(bUseProbe, sizeof(bUseProbe));

						nUseHead[0] = nUseCELLID[0];
						nUseHead[1] = nUseCELLID[0];
						bUseProbe[0] = true;
						bUseProbe[1] = true;

						nMeasStick = MakeCellID(nUseHead, bUseProbe, *cCellID, iRotate, iStick, nMaxStick);
					}
					else
					{
						nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);
					}


					// jsyoon 2022.06.21 add
					int iCellStatus = 0;
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] != true)	// 2023.03.09 add
							continue;

						nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
						iCellStatus = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick];

						// PROBE CHECK
						/*!
						* 2023.03.09
						* 측정 동작 조건 수정
						*/
						if (iCellStatus == df_CELLSTATE_PTP_SHORT ||		// PTP
							iCellStatus == df_CELLSTATE_PTP_OPEN ||		// PTP
							iCellStatus == df_CELLSTATE_OST_SHORT ||		// OST
							iCellStatus == df_CELLSTATE_OST_OPEN ||		// OST
							iCellStatus == df_CELLSTATE_AGING_NG  			// AGING
							)
						{
							bMeasProbe[iProbe] = false;
						}

						//if (iCellStatus == df_CELLSTATE_ILDE ||			// 
						//	iCellStatus == df_CELLSTATE_PTP_NOR ||		// PTP
						//	iCellStatus == df_CELLSTATE_OST_NOR ||		// OST
						//	iCellStatus == df_CELLSTATE_PASS ||			// R-TEST
						//	iCellStatus == df_CELLSTATE_AGING_OK 		// AGING
						//	)		
						//{
						//	bMeasProbe[iProbe] = true;
						//}
						//else
						//{
						//	bMeasProbe[iProbe] = false;
						//}

						if (iCellStatus == df_CELLSTATE_OVER || iCellStatus == df_CELLSTATE_UNDER)  // R-TEST 
						{
							if (bUseRtestNgPass == true)
							{
								bMeasProbe[iProbe] = true;
							}
							else
							{
								bMeasProbe[iProbe] = false;
							}
						}
					}

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						iMeasMode = 0;	// 동시 측정명령 전송 2PC 2Cam
					}
					else
					{
						if (bMeasProbe[0] == true && bMeasProbe[1] != true)
						{
							iMeasMode = 1;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
						{
							iMeasMode = 2;
						}
						else if (bMeasProbe[0] != true && bMeasProbe[1] != true)
						{
							iMeasMode = -1;
						}
						else
						{
							iMeasMode = 0;
						}
					}



					//// jsyoon 2022.09.29 change
					//int iRotate = 0;
					//if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//{
					//	iRotate = 1;
					//}

					//ZeroMemory(cCellID, sizeof(cCellID));
					//nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, iStick, nMaxStick);
					//CString strCellId;
					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	if (bMeasProbe[iProbe] == true)
					//	{
					//		nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
					////		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][iStick] = df_CELLSTATE_VISION_START;

					//		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		}
					//		else
					//		{
					//			strCellId.Format(_T("%c%02d"), 65 + iStick, nArea - nCellNum);
					//		}

					//		// strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
					//		//strCellId.Format(_T("%c%02d"), 65, nCellNum + 1);
					//		//strCellId.Format(_T("%c%02d"), 65 + nCellNum, nMaxStick - iStick);
					//		/*strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + iProbe + 1);*/
					//		_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//	}
					//}
				}

				// Move
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, true, 0);

				ZeroMemory(&stPosition, sizeof(stPosition));

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					if (nMeasArea == 1)	// 2023.05.09 add
					{
						int nUseHead[df_MAX_PROBE];
						ZeroMemory(nUseHead, sizeof(nUseHead));

						bool bUseProbe[df_MAX_PROBE];
						ZeroMemory(bUseProbe, sizeof(bUseProbe));

						nUseHead[0] = nUseCELLID[0];
						nUseHead[1] = nUseCELLID[0];
						bUseProbe[0] = true;
						bUseProbe[1] = true;

						GenMeasID(nUseHead, bUseProbe, *cCellID);
					}
					else
					{
						GenMeasID(nUseCELLID, bMeasProbe, *cCellID);
					}


					int nAxis = 0;
					double dLeft = 0., dRight = 0.;
					double dLeft2 = 0., dRight2 = 0.;
					bool bDirLtoR = true;

					if (vecMeasGroup[i].m_sCellInfo[0].m_fCellX > vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
					}
					else
					{
						dLeft = vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
						dRight = vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

						bDirLtoR = false;
					}

					if (nMeasArea == 1)
					{
						/*!
						* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
						*/
						double dHeadGapRcpF = 700.0, dHeadGapRcpB = 700.0, dMovePosF = 0.0, dMovePosB = 0.0;
						if (pReciepInfo != nullptr)
						{
							dHeadGapRcpF = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
							dHeadGapRcpB = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
						}

						dMovePosF = dHeadGapRcpF / 2;
						dMovePosB = dHeadGapRcpB / 2;

						if (bDirLtoR == true)	//  - -> +
						{
							dLeft2 = dMovePosF * -1;
							dRight2 = dMovePosF;

							dLeft = dMovePosB * -1;
							dRight = dMovePosB;
						}
						else  //  + -> -
						{
							dLeft2 = dMovePosF;
							dRight2 = dMovePosF * -1;

							dLeft = dMovePosB;
							dRight = dMovePosB * -1;
						}
					}
					else
					{
						dLeft2 = dLeft;
						dRight2 = dRight;
					}

					if (bMoveFirst == true)
					{
						stPosition[0].axis = AXIS_Y3;   nAxis++;
						stPosition[0].dPosition = dLeft2;
						stPosition[1].axis = AXIS_Y4;   nAxis++;
						stPosition[1].dPosition = dRight2;

						stPosition[2].axis = AXIS_FILTER1;   nAxis++;
						stPosition[2].dPosition = 0;
						stPosition[3].axis = AXIS_FILTER2;   nAxis++;
						stPosition[3].dPosition = 0;
					}
					else
					{
						stPosition[0].axis = AXIS_Y3;   nAxis++;
						stPosition[0].dPosition = dLeft2;
						stPosition[1].axis = AXIS_Y4;   nAxis++;
						stPosition[1].dPosition = dRight2;

						stPosition[2].axis = AXIS_Y1;   nAxis++;
						stPosition[2].dPosition = dLeft;
						stPosition[3].axis = AXIS_Y2;   nAxis++;
						stPosition[3].dPosition = dRight;
						// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (df_HEAD_Y_IDLE_SAFETY -> MoveCmdPos* 의 ApplyIdleHeadSafetyY 가 치환).
						//   FRONT 측정 첫 이동에서 BACK 헤드가 셀 좌표 + ANGLE 오프셋(MoveCmdPosVision/Angle)으로 나가 ?272 면 Y1-Y2 간격이 인터락(720) 미달로 거부된다. BACK 갠트리는 이때 SafetyHeadX1 에 있다.
						if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
						{
							stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
							stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
						}

						stPosition[4].axis = AXIS_X3;   nAxis++;
						stPosition[4].dPosition = vecMeasGroup[i].dHeadBackG + dOffsetX;	// 2023.07.05 add

						stPosition[5].axis = AXIS_Z1;   nAxis++;
						stPosition[5].dPosition = vecMeasGroup[i].dHeadFrontZ;
						stPosition[6].axis = AXIS_Z2;   nAxis++;
						stPosition[6].dPosition = vecMeasGroup[i].dHeadBackZ;

						stPosition[7].axis = AXIS_ZOOM1;   nAxis++;
						stPosition[7].dPosition = 0;
						stPosition[8].axis = AXIS_THETA1;   nAxis++;
						stPosition[8].dPosition = 0;
						stPosition[9].axis = AXIS_ZOOM2;   nAxis++;
						stPosition[9].dPosition = 0;
						stPosition[10].axis = AXIS_THETA2;   nAxis++;
						stPosition[10].dPosition = 0;

						stPosition[11].axis = AXIS_FILTER1;   nAxis++;
						stPosition[11].dPosition = 0;
						stPosition[12].axis = AXIS_FILTER2;   nAxis++;
						stPosition[12].dPosition = 0;
					}

					/*!
					* 이동시 Cross 축만 모션 완료 확인을 나중에 한다
					*/
					MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosVision(stPosition, nAxis, SPEED_HIGH, false);

					if (nRet == VTM_RV_SUCCESSED)
					{
						bMoveFirst = true;
					}

					// PG 번호 변경
					if (nRet == VTM_RV_SUCCESSED)
					{
						for (int iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true) {
								m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
							}
							else
							{
								if (iProbe == 0) {
									m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
								}
								else
								{
									/*!
									* 2023.03.09 change
									* Cell 하나 측정 시 다른 PG 번호를 임의로 설정
									* m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
									*/
									if (nArea == 1)
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 11);
									else
										m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
								}
							}
						}
					}

					if (i == 0)
					{
						// Wait Done
						if (nRet == VTM_RV_SUCCESSED)
						{
							int axis = 0;
							for (int n = 0; n < 2; n++)
							{
								axis = stPosition[n].axis;
								nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
								if (nRet != df_MOTION_RV_SUCCESSED)
									break;
							}
						}

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, false, 0);

						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);
					}
				}

				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					//20220426 add Buffer Check
					if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(NOT_LAST_ITEM, nTimeOut)) {
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityAlarm(); //알람 리턴 방안 확인 필요
						bRet = false;
					}
				}

				// 계산 완료 후 모션 이동 완료 확인
				if (i > 0)
				{
					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i - 1);

					// Wait Done
					if (nRet == VTM_RV_SUCCESSED)
					{
						int axis = 0;
						for (int n = 0; n < 2; n++)
						{
							axis = stPosition[n].axis;
							nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(axis);
							if (nRet != df_MOTION_RV_SUCCESSED)
								break;
						}
					}

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION_MOVE, false, 0);

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Move"), i);
				}


				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				// 검사
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_VISION_SAMPLE, true, 0);

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(2, _T("Meas"), i);

				if (nRet == VTM_RV_SUCCESSED && iMeasMode != -1)
				{
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (bMeasProbe[iProbe] == true)
						{
							nCellNum = nUseCELLID[iProbe]; //nCellNum = nMeasArea * iProbe + i;
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_VISION_START;
						}
					}

					// Change Recipe Done
					if (iStick == 0 && i == 0)
					{
						DWORD dwTimeOut = DEFINE_RECIPE_WAIT_TIME2; // 60; // jsyoon 2022.09.26 change Recipe Wait Time

						if (nTimeOut > DEFINE_RECIPE_WAIT_TIME2)
						{
							dwTimeOut = nTimeOut;	// 2022.12.27 add
						}

						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_VISION, dwTimeOut);
					}

					//Area 정보, 측정 진행 유무
					int nAreaNum[DF_MAX_HEAD] = { -1, -1 };

					if (nMeasArea == 1)
					{
						nAreaNum[0] = -1;
						nAreaNum[1] = nUseCELLID[0] + (nArea * nMeasStick) + 1;  // Head 2 is Master
						bMeasProbe[0] = true;
						bMeasProbe[1] = true;
					}
					else
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (nUseCELLID[iProbe] >= 0) nAreaNum[iProbe] = nUseCELLID[iProbe] + (nArea * nMeasStick) + 1;
						}
					}

					// JCY 2023.06.23 화질 면계측 동시 측정 
					/////////////////////////////////////////////////////
					m_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);

					// Color Start -> Quality Start -> Color End -> Quality End
					if (nRet == VTM_RV_SUCCESSED)
					{ 
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_COLOR, iMeasMode, 0);
					}
					
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_GRAB);
					}
					
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
					}	
					
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
					}

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_GRAB_WAIT);
					}					
					/////////////////////////////////////////////////////

					if (nRet == VTM_RV_SUCCESSED)
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							if (bMeasProbe[iProbe] == true)
							{
								nCellNum = nUseCELLID[iProbe]; // nCellNum = nMeasArea * iProbe + (i - 1);
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCellNum][nMeasStick] = df_CELLSTATE_VISION_END;
							}
						}
					}
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_SAMP_VISION_SAMPLE, false, 0);

				if (i + 1 == nMeasArea)
				{
					if (iMeasMode != -1)
					{
						// jsyoon 2022.08.18 add
						// 모든 검사 완료 후 대기, 전수 검사 아닐시 처리 방안 확인 필요
						if (iStick + 1 == m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr()->GetStickCount()) {
							if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(LAST_ITEM, nTimeOut)) {
								/*nRet = */m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityAlarm();
								bRet = false;
							}
						}
					}

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(2, _T("Meas"), i);
				}

				if (nRet != VTM_RV_SUCCESSED) {
					m_pShareItem->SetFail(true, nRet);
					bRet = false;
					break;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_STEP_VISION, false, 0);
			}
		}
	}


	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_Measure_VISION_COLOR End. TEST"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_N_PROC_VISION, false, 0);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, 4);

	return bRet;
}

bool CDoWorkGlassMeasureStep_OST::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_OST, true);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_OST_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_OST Start."));

	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseElect_1;
	}

	if (bUseMeas == true)
	{
		// LYS 2026-06-30 OST, RTEST 현재 Stick 위치
		int iStick = 0;
		iStick = GetMeasureStick();

		S_RecipeMgr* pslData = nullptr;
		pslData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		//	int nCellCnt = 0;
		//	nCellCnt = pslData->m_nCellCnt;

		int nCellCnt = 0, nStickCnt = 0;
		nCellCnt = pslData->m_nCellCnt;
		nStickCnt = pslData->m_nStickCnt;

		// jsyoon 2022.09.29 add
		int iRotate = 0;
		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
		{
			iRotate = 1;
		}
		///

		// LYS 2026-07-16 rotate=0: convert iStick to match MakeCellID nMeasStick
		if (iRotate == 0)
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - iStick - 1;
		else
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = iStick;

		//if (iRotate == 1)
		//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
		//else
		//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt;
		///


		// LYS 2026-09-07 OCP PG ages/tests the whole stick in one command - skip the Ensis EVEN round (same gate as AGING)
		bool bIsOCP = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP());
		if (!bIsOCP && nCellCnt > 10)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
			if (lRet == VTM_RV_SUCCESSED)
			{
				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_12);
			}
		}
		else
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
		}

		if (lRet == VTM_RV_SUCCESSED)
		{
			bRet = true;
			if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
			{
				Sleep(1000);
			}
		}
		else
		{
			m_pShareItem->SetFail(true, lRet);
		}
	}
	else
	{
		bRet = true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_OST End."));

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_OST, false);

	return bRet;
}

bool CDoWorkGlassMeasureStep_RTEST::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_R_TEST, true);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_RTEST_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_RTEST Start."));

	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		// bUseMeas = pReciepInfo->m_sInspectItem.m_bUseElect_1;
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseRTest;		// jsyoon 2022.08.10 change
	}

	if (bUseMeas == true)
	{
		// LYS 2026-06-30 OST, RTEST 현재 Stick 위치
		int iStick = 0;
		iStick = GetMeasureStick();

		//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;

		S_RecipeMgr* pslData = nullptr;
		pslData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		//	int nCellCnt = 0;
		//	nCellCnt = pslData->m_nCellCnt;

		int nCellCnt = 0, nStickCnt = 0;
		nCellCnt = pslData->m_nCellCnt;
		nStickCnt = pslData->m_nStickCnt;

		// jsyoon 2022.09.29 add
		int iRotate = 0;
		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
		{
			iRotate = 1;
		}
		///

		// LYS 2026-07-16 rotate=0: convert iStick to match MakeCellID nMeasStick
		if (iRotate == 0)
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - iStick - 1;
		else
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = iStick;

		//if (iRotate == 1)
		//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
		//else
		//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - 1;
		///


		// LYS 2026-09-07 OCP PG ages/tests the whole stick in one command - skip the Ensis EVEN round (same gate as AGING)
		bool bIsOCP = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP());
		if (!bIsOCP && nCellCnt > 10)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
			if (lRet == VTM_RV_SUCCESSED)
			{
				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_13);
			}
		}
		else
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
		}

		//    lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
		if (lRet == VTM_RV_SUCCESSED)
		{
			bRet = true;
			if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
			{
				Sleep(1000);
			}
		}
		else
		{
			m_pShareItem->SetFail(true, lRet);
		}
	}
	else
	{
		bRet = true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_RTEST End."));

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_R_TEST, false);

	return bRet;
}

// LYS 2026-08-06 AgingCycle E3: 에이징 리스트 항목별 패턴 점등
// (OCP DoWorkStartMeasOCP_A: TURNON_ALL -> (PTN-1) x NEXT_PTN_ALL -> PtnSec 유지 -> TurnOff)
bool CDoWorkGlassMeasureStep_AGING_PTN::Run()
{
	if (m_pMeasSeqItemEngine == NULL || m_pMeasSeqItemEngine->IsAgingCycleActive() == false)
		return true;	// standard sequence: pass-through

	const S_AgingTest* pItem = m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetItem(m_pMeasSeqItemEngine->GetAgingCurItem());
	if (pItem == NULL) return true;

	CString strLog;
	strLog.Format(_T("CDoWorkGlassMeasureStep_AGING_PTN Start. item(%d) ptn(%d) sec(%d)"),
		m_pMeasSeqItemEngine->GetAgingCurItem() + 1, pItem->m_nPtnRGB, pItem->m_nPtnSec);
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	// progress: row = current item(1-base), col 2 = lighting
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = m_pMeasSeqItemEngine->GetAgingCurItem() + 1;
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 2;
	}

	long lRet = VTM_RV_SUCCESSED;

	if (pItem->m_nPtnRGB > 0)
	{
		lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_5);	// Turn On All

		for (int i = 1; i < pItem->m_nPtnRGB && lRet == VTM_RV_SUCCESSED; i++)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_6);	// Next Pattern
		}

		if (lRet == VTM_RV_SUCCESSED)
		{
			// col 3 = holding (PtnSec)
			if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 3;

			DWORD dwS = GetTickCount();
			DWORD dwHold = (DWORD)pItem->m_nPtnSec * 1000;
			DWORD dwT = 0;
			while (dwT < dwHold)
			{
				Sleep(100);
				DWORD dwR = GetTickCount();
				dwT = (dwR >= dwS) ? (dwR - dwS) : (((DWORD)0xFFFFFFFF) - (dwS - dwR));

				if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true) break;	// user stop
			}
		}

		// Turn Off (always attempt, keep the first error)
		long lOff = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);
		if (lRet == VTM_RV_SUCCESSED) lRet = lOff;
	}

	bool bRet = true;
	if (lRet != VTM_RV_SUCCESSED)
	{
		bRet = false;
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true) bRet = false;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING_PTN End."));

	return bRet;
}

bool CDoWorkGlassMeasureStep_AGING::Run()
{
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true) Sleep(1000);
	
	 m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING Start."));

	// LYS 2026-09-28 EFA: EFA item without USE_AGING (the T1 row) skips the PG aging itself
	// (OCP _EFA: bUseAging = sAgingTest.m_bUseAging). AgingTest cycle is untouched (its NextItem only picks USE_AGING items).
	if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsEfaCycle())
	{
		const S_AgingTest* pEfaItem = m_pMeasSeqItemEngine->GetAgingCycleInfo()->GetItem(m_pMeasSeqItemEngine->GetAgingCurItem());
		if (pEfaItem != NULL && pEfaItem->m_bUseAging == FALSE)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING End. (EFA item USE_AGING=0 - skip)"));
			return true;
		}
	}

	// LYS 2026-08-06 AgingCycle E4: progress marker (col 1 = Aging)
	if (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingCycleActive()
		&& m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)
	{
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListRow = m_pMeasSeqItemEngine->GetAgingCurItem() + 1;
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingListCol = 1;
	}

    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;

    bool bUseMeas = false;
    S_RecipeInfo* pReciepInfo;
    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
    if (pReciepInfo != nullptr)
    {
        bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
    }

    if (bUseMeas == true)
    {
		// LYS 2026-06-30 (AGING) OCP mode flag
		bool bIsOCP = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP());

		// LYS 2026-06-30 (AGING) OCP: set current stick for P5
		if (bIsOCP)
		{
			int iAgingStick = GetMeasureStick();
				S_RecipeInfo* pAgingRcpInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
				// LYS 2026-07-16 rotate=0: convert iStick to match MakeCellID nMeasStick
				// LYS 2026-08-07 rotation-change: MakeCellID inverts for every rotate except 1 -> match (fixes short-side AGING S/E mirrored)
				if (pAgingRcpInfo != nullptr && pAgingRcpInfo->m_sOptionItem.m_nGlsRotate != 1)
				{
					S_RecipeData* pAgingRcpData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
					iAgingStick = (int)pAgingRcpData->m_vecStick.size() - iAgingStick - 1;
				}
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = iAgingStick;
		}

		double dCurPos = 0.;
		dCurPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X2);	// 2023.08.31 add

		// LYS 2026-08-05 AgingSet aging target-current alarm
		bool bAgingAlarmUse = false;
		std::vector<S_AgingTargetAlarm> vecAgingTarget;
		if (bIsOCP)
		{
			if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) != true)
			{
				bAgingAlarmUse = (CAgingAlarmCheck::GetAlarmUse() == TRUE);
				if (bAgingAlarmUse)
				{
					CAgingAlarmCheck::DeleteAgingResultFile();
					if (!CAgingAlarmCheck::LoadTargetRcp(m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID(), vecAgingTarget))
					{
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING target rcp open fail."));
						m_pShareItem->SetFail(true, PROPERTY_AGING_RCP_OPEN_FAIL);
						return false;
					}
				}
			}

		}

        // Start
		bool bMeasAging = false;
		if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] > -1)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			bMeasAging = true;
		}
		
		if (lRet == VTM_RV_SUCCESSED)
		{
			// LYS 2026-08-25 (AGING) OCP no longer skips the AgingPG start.
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING);
		}

		DWORD  dwS, dwR, dwT = 0;
		DWORD  dwSleep = 100;

		int iAgingTime = 0;
		if (bMeasAging == true)
		{
			iAgingTime = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectPG()->GetAgingTime();
			if (iAgingTime < 1)
				iAgingTime = 5;
		}
		else
		{
			iAgingTime = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
		}
		
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
		dwS = GetTickCount();
            
		// Move Snap Image
		if (lRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING Snap Start."));

			S_RecipeData* pRecipeData;
			pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

			S_RecipeInfo* pRecipeInfo;
			pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

			/*int iLampIntensity[2];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft; 
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;
			}*/

			int iLampIntensity[4];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft;
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;

				iLampIntensity[2] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
				iLampIntensity[3] = pRecipeInfo->m_sLampItem.m_nFrontRight;
			}

			// Set Light
		//	int iCh[2] = {0, 1 };

			// Set Light
			int iCh[4] = { 0, 1, 3, 2 };

			for (size_t i = 2; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			Sleep(1000);

			for (size_t j = 0; j < 2; j++)	// 2 Head
			{
				m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(8);
			}

			for (size_t i = 0; i < 2; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			int iStick = 0, nCell = 0, nCellL = 0, nCellR = 0;
			// Review Move
			double dLeft = 0., dRight = 0.;

			double dOffsetX = 0.;
			double dLeftReview = 0., dRightReview = 0.;
		
			if (pRecipeInfo != nullptr)
			{
				dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;

				dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft;
				dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;
			}

			// LYS 2026-06-30 (AGING) OCP: use current stick from Measure loop
			if (bIsOCP)
				iStick = GetMeasureStick();

			nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size(); 
			
			if (pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX > pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX)
			{
				dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
				dRight = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
			}
			else
			{
				dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
				dRight = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
			}

			// jsyoon 2022.05.12 add
			dLeft += dLeftReview;
			dRight += dRightReview;

			int iProbe = 0, nProbe = 0;
			int nStick = pRecipeData->m_vecStick.size();

			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}

			//for (iStick = 0; iStick < nStick; iStick++)
			//{
			//for (iStick = 1; iStick < nStick; iStick++)

			// LYS 2026-08-25 (AGING) OCP: P5 리뷰 스냅 1회만.
			// 아래 조회는 m_nAgingStartStick[4 - nProbe] 즉 P4..P1이라 슬롯 4의 P5에는 닿지 않는다.
			// OCP에서는 iStick이 이미 측정 루프의 stick을 담고 있어, 1회만 돌고 프로브별 stick 산출은 생략.
			int nProbeEnd = bIsOCP ? 2 : 5;		// OCP: nProbe == 1 only
			bool iFirst = true;
			for (nProbe = 1; nProbe < nProbeEnd; nProbe++)
			{
				if (bIsOCP != true)
				{
					if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] >= 0)
					{
						// jsyoon 2022.09.29 change
						if(iRotate == 1)
							iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
						else
							iStick = nStick - m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] - 1;

					//	iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
					}
					else
					{
						continue;
					}
				}

				if (iStick >= pRecipeData->m_vecStick.size())
				{
					continue;
				}

				double dStageY = 0.;
				dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
			//	if (dStageY > 100)
			//		continue;

				double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
				if (dStageY < dMinAxisX3)
				{
					continue;
				}

				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));

				if (iFirst == true)
				{
					stPosition[0].axis = AXIS_Y3;   nAxis++;
					stPosition[0].dPosition = dLeft;
					stPosition[1].axis = AXIS_Y4;   nAxis++;
					stPosition[1].dPosition = dRight;
					// LYS 2026-09-28 OCP: review snap heads are Y1/Y2 (BACK, cameras 0/1) and they already sit at dLeft/dRight
					// from MoveMeasureArea. Y3/Y4 (FRONT, SW limit +697) must go to Position.ini [SAFETY] instead -
					// df_HEAD_Y_IDLE_SAFETY marker, resolved by MoveCmdPosAging/ApplyIdleHeadSafetyY (same as PreMeas).
					// 2026-09-28 OOS run: 'HEAD FRONT RIGHT target 720.158 > MX 697.000' came from here.
					if (bIsOCP)
					{
						stPosition[0].axis = AXIS_Y1;	stPosition[0].dPosition = dLeft;
						stPosition[1].axis = AXIS_Y2;	stPosition[1].dPosition = dRight;
						stPosition[2].axis = AXIS_Y3;	stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;	nAxis++;
						stPosition[3].axis = AXIS_Y4;	stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;	nAxis++;
					}
					//stPosition[2].axis = AXIS_X3;   nAxis++;
					//stPosition[2].dPosition = dStageY;
					//stPosition[3].axis = AXIS_Z1;   nAxis++;
					//stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
					//stPosition[4].axis = AXIS_Z2;   nAxis++;
					//stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

					lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				}
				//else
				//{
				//	stPosition[nAxis].axis = AXIS_X3;  
				//	stPosition[nAxis].dPosition = dStageY;	nAxis++;
				//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				//}

				if (lRet == VTM_RV_SUCCESSED)
				{
					// Review Snap
					Sleep(1000);
					for (size_t j = (bIsOCP ? 0 : 2); j < (bIsOCP ? 2 : 4); j++)	// 2 Head (// LYS 2026-09-28 OCP: BACK cameras 0/1)
					{
						m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(4 + iProbe);
					}
				}
				else
				{
					break;
				}

				iProbe++;
			}

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING Snap End."));

			if (lRet == df_MOTION_RV_SUCCESSED)
			{
				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));
				stPosition[0].axis = AXIS_Y1;   nAxis++;
				stPosition[0].dPosition = dLeft;
				stPosition[1].axis = AXIS_Y2;   nAxis++;
				stPosition[1].dPosition = dRight;
				stPosition[2].axis = AXIS_Y3;   nAxis++;
				stPosition[2].dPosition = dLeft;
				stPosition[3].axis = AXIS_Y4;   nAxis++;
				stPosition[3].dPosition = dRight;
				// LYS 2026-09-28 OCP: Y3/Y4 (FRONT) to [SAFETY] - review position 720 exceeds their SW limit 697
				if (bIsOCP)
				{
					stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
					stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
				}
				//stPosition[4].axis = AXIS_X2;   nAxis++;
				//stPosition[4].dPosition = dCurPos; // pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				//stPosition[5].axis = AXIS_Z1;   nAxis++;
				//stPosition[5].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
				//stPosition[6].axis = AXIS_Z2;   nAxis++;
				//stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosReview(iStick, stPosition, nAxis, SPEED_HIGH);
			}

			for (size_t i = 0; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0); // SPO
			}

			// Complate
			if (lRet == VTM_RV_SUCCESSED)
			{
				// 2023.05.15 add For Virtual Test
				if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
				{
					do
					{
						Sleep(dwSleep);

						dwR = GetTickCount();

						if (dwR >= dwS)
							dwT = dwR - dwS;
						else
							dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

						if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
						{
							break;
						}

					} while (dwT < iAgingTime * 1000);
				}
				
				UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
				if (bMeasAging == true)
				{
					if (lRet == VTM_RV_SUCCESSED)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
					}
				}
				
				// LYS 2026-08-25 (AGING) OCP waits for the AgingPG completion as well.
				if (lRet == VTM_RV_SUCCESSED)
				{
					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
				}

			// LYS 2026-08-05 AgingSet aging target-current alarm
			if (lRet == VTM_RV_SUCCESSED && bIsOCP && bAgingAlarmUse && bMeasAging)
			{
				CString strAgingPtn;
				int nAgingCell = 0;
				int nAgingAlarm = CAgingAlarmCheck::Find(vecAgingTarget, strAgingPtn, nAgingCell);
				if (nAgingAlarm == AGING_NOT_FILE)
				{
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING result file not found."));
					lRet = PROPERTY_AGING_FILE_OPEN_FAIL;
				}
				else if (nAgingAlarm == AGING_FIND_ALARM)
				{
					CString strAgingLog;
					strAgingLog.Format(_T("CDoWorkGlassMeasureStep_AGING Spec Out : Ptn(%s) Cell(%d)"), (LPCTSTR)strAgingPtn, nAgingCell);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strAgingLog);
					lRet = PROPERTY_AGING_SPEC_OUT_ALARM;
				}
			}
			}
			else
			{
				// Aging Stop
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);	// LYS 2026-08-25 (AGING) OCP stops AgingPG too

				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}

			if (lRet == VTM_RV_SUCCESSED)
			{
				// Aging EVEN
				// LYS 2026-08-25 (AGING) OCP는 ODD 라운드만 수행하고 EVEN은 P5에서 의도적으로
				// 돌리지 않는다(2026-08-25 결정). 그래서 계속 막아둔다.
				if (!bIsOCP && nCell > 10)
				{
					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

					Sleep(500);

					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
					dwS = GetTickCount();

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING Even Start."));

					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 1);

					if (lRet == VTM_RV_SUCCESSED)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_14);
					}

					if (lRet == VTM_RV_SUCCESSED)
					{
						// 2023.05.15 add For Virtual Test
						if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
						{
							do
							{
								Sleep(dwSleep);

								dwR = GetTickCount();

								if (dwR >= dwS)
									dwT = dwR - dwS;
								else
									dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

							} while (dwT < iAgingTime * 1000);
						}

						UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
						nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
						}

						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
						}
					}

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING Even End."));
				}
			}
		}
		else
		{
			// Aging Stop
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);	// LYS 2026-08-25 (AGING) OCP stops AgingPG too

			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
		}

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
    }

    if (lRet == VTM_RV_SUCCESSED) 
    {
        bRet = true;
    }
	else
	{
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassMeasureStep_AGING End."));

	if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) != true)
		return bRet;
	else
		return true;
}