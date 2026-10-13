#include "pch.h"
#include "DoWorkGlassRecipeStep.h"
#include "..\Manager\SystemConfigMgr.h"	// LYS 2026-06-26 OCP option

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"

#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\ResultMgr.h"
#include "..\Manager\ResumeInfoMgr.h"	// LYS 2026-07-07 Resume raw data reload
#include "..\Manager\ResumeJournal.h"		// LYS 2026-10-08 Hybrid Phase 5 (MarkCellStatusReset)
#include "common\CommonFunc.h"				// LYS 2026-07-07 PGM_MODE_* constants
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule/FunctionModuleProbeACC.h"
#include "AgingCycleInfo.h"					// LYS 2026-09-29 Hybrid 3b capacity pre-check (K)
#include "..\Manager\WadMultiMeasMgr.h"		// LYS 2026-09-29 Hybrid 3b capacity pre-check (P)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassRecipeStep::GetComplete()
{
	return m_pShareItem->GetComplete();
}

bool CDoWorkGlassRecipeStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None End."));

	// LYS 2026-10-08 Hybrid Phase 5 journal (Format=2) bookmark: the grid is rebuilt by the journal replay at measure start (RS1 below is for
	//   format-1 bookmarks only). The mark lets Measure restore nCellStatus only after the UI reset Recipe Change posts (GenGlassPath counter)
	CResumeInfoMgr* pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	if (CSystemParamShare::GetInstance()->IsResumeMode() && pResumeT != NULL && pResumeT->IsTupleBookmark())
	{
		CResultMgr::GetInstance()->ClearData();
		CResumeJournal::GetInstance()->MarkCellStatusReset();
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("Resume mode (journal): result grid cleared - journal replay at measure start"));
	}
	else
	// LYS 2026-07-03 Resume: skip ResultClear to preserve previous stick results
	if (CSystemParamShare::GetInstance()->IsResumeMode())
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("Resume mode: ResultClear skipped"));

		// LYS 2026-07-07 Reload previous stick raw data into FPSpread for complete APD
		CResumeInfoMgr* pResume = m_pShareItem->GetResumeInfoMgrPtr();
		if (pResume && !pResume->GetResultPath().IsEmpty())
		{
			CString strBasePath;
			strBasePath.Format(_T("%sRawData\\%s\\%s\\"),
				pResume->GetResultPath(), pResume->GetInspDate(), pResume->GetGlassID());

			// Verify base path exists
			if (!PathFileExists(strBasePath))
			{
				CString strLog;
				strLog.Format(_T("Resume reload SKIP: path not found [%s]"), strBasePath);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
			}
			else
			{
				S_RecipeData* pRD = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
				int nLastStick = pResume->GetLastStick();
				int nCellsPerStick = (pRD && pRD->m_vecStick.size() > 0) 
					? (int)pRD->m_vecStick[0].m_vecCell.size() : 0;
				int nPrevCells = nCellsPerStick * (nLastStick + 1);
				int nMaxStick = pRD ? (int)pRD->m_vecStick.size() : 0;

				// LYS 2026-07-21 Get glass rotation for iMeasArea calculation
				S_RecipeInfo* pRI = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
				int nGlsRotate = pRI ? pRI->m_sOptionItem.m_nGlsRotate : 1;

				if (nPrevCells > 0 && nCellsPerStick > 0)
				{
					struct ReloadType { LPCTSTR folder; LPCTSTR ext; int mode; };
					ReloadType types[] = {
						{ _T("WAD"),    _T("*.dat"), PGM_MODE_LUMIN },
						{ _T("ANGLE"),  _T("*.ini"), PGM_MODE_ANGLE },
						{ _T("VISION"), _T("*.txt"), PGM_MODE_VISION },
					};

					for (int t = 0; t < 3; t++)
					{
						CString strTypePath = strBasePath + types[t].folder + _T("\\");
						if (!PathFileExists(strTypePath))
							continue;

						CFileFind finder;
						CStringArray arrFiles;
						BOOL bFound = finder.FindFile(strTypePath + types[t].ext);
						while (bFound)
						{
							bFound = finder.FindNextFile();
							if (!finder.IsDirectory())
								arrFiles.Add(finder.GetFileName());
						}
						finder.Close();

						// Sort by filename (InnerID is timestamp-based, alpha sort = chronological)
						for (INT_PTR si = 0; si < arrFiles.GetSize() - 1; si++)
							for (INT_PTR sj = si + 1; sj < arrFiles.GetSize(); sj++)
								if (arrFiles[si] > arrFiles[sj])
								{ CString s = arrFiles[si]; arrFiles[si] = arrFiles[sj]; arrFiles[sj] = s; }

						int nReload = min((int)arrFiles.GetSize(), nPrevCells);
						for (int i = 0; i < nReload; i++)
						{
							// LYS 2026-07-21 일반 측정과 동일한 방식으로 iMeasArea를 계산한다.
							// 파일은 측정 순서(스틱 0,1,2... 각 스틱당 nCellsPerStick개)로 정렬돼 있다.
							int iStick = i / nCellsPerStick;
							int iCell  = i % nCellsPerStick;
							int nMeasStick;
							if (nGlsRotate == 1)
								nMeasStick = iStick;               // 90 deg: direct
							else
								nMeasStick = nMaxStick - iStick - 1; // 0 deg: reversed

							RSRITEMDATA rd;
							ZeroMemory(&rd, sizeof(rd));
							rd.nMeasMode = types[t].mode;
							rd.iMeasArea = iCell + (nCellsPerStick * nMeasStick) + 1;
							_tcscpy_s(rd.szLotID, pResume->GetLotID());
							_tcscpy_s(rd.szGlassID, pResume->GetGlassID());
							_tcscpy_s(rd.szRecipeID, pResume->GetRecipeID());
							_tcscpy_s(rd.szResultFile, arrFiles[i]);
							_tcscpy_s(rd.szResultPath, strTypePath);
							rd.m_nMeasAreaCnt = nPrevCells;
							CResultMgr::GetInstance()->Updata_Data(&rd);
						}

						CString strLog;
						strLog.Format(_T("Resume reload: %s %d/%d files loaded (rotate=%d)"), types[t].folder, nReload, (int)arrFiles.GetSize(), nGlsRotate);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, strLog);

						if (nReload < nPrevCells)
						{
							strLog.Format(_T("Resume reload WARNING: %s expected %d files, found %d"), types[t].folder, nPrevCells, (int)arrFiles.GetSize());
							m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
						}
					}
				}
			}
		}
	}
	else
	{
		CResultMgr::GetInstance()->ClearData();
	}

	return true;
}


bool CDoWorkGlassRecipeStep_Idle::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
	ResetEvent(m_pShareItem->GetStartEventHandle());

	if(m_pShareItem->GetFail() != true)
		m_pShareItem->SetComplete(true);   // 동작 완료

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	return false;
}

// LYS 2026-09-29 자동 측정 전 측정 위치 SW 리밋 사전 검사 (하이브리드 세그먼트 2 헤드 Y -1492.668 사고 재발 방지)
//   MoveMeasureArea와 같은 식으로 모든 세그먼트·스틱의 Stage X / Stage R / 헤드 Y1·Y2 리뷰 목표를 계산해 모터 SW 리밋과 비교한다.
//     Stage X : OCP = 스틱 StageY + USER OFFSET StageY, HMI = 스틱0 StageY - 스틱 StageY + 오프셋 (스틱 0은 CellDistance.y*0 + 오프셋)
//     Stage R : 스틱 R + USER OFFSET StageRotate
//     Y1 = max(첫/마지막 셀 X) + REVIEW RIGHT, Y2 = min(첫/마지막 셀 X) + REVIEW LEFT
//   Align 오프셋은 이 시점에 모르므로 제외한다(수 mm 이내). 반환 0 = 통과, 아니면 알람 ID(3901/3902), strLog = 첫 위반 내용.
//   수식을 바꾸면 DoWorkGlassMeasureStep.cpp CDoWorkGlassMeasureStep_MoveMeasureArea::Run 과 같이 바꿀 것.
// LYS 2026-09-30 뒤쪽 헤드 Y1/Y2 간격도 검사한다. PreMeas_Move·MoveMeasureArea가 두 헤드를 스틱 양 끝 셀에 동시에 보내므로
//   |Y1 - Y2| < INTERLOCKHEADGAPB면 모션 계층이 3027(MOTION_INTERLOCK_7)로 거부한다 - 셀이 한쪽에 몰린 반쪽 세그먼트에서 발생
//   (2026-09-30 11:28 9_OCP_TEST_Hybrid_Half_260930 세그먼트 1: 간격 531 < 780). 위반 시 3027로 자동 시작 거부(모션 전).
static long CheckMeasureTargetSWLimit(CRecipeEditMgr* pRcp, CMaintControlMgr* pMaint, bool bOCP, CString& strLog)
{
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	int nSeg = (pFunc != NULL && pFunc->GetSegmentCount() > 0) ? pFunc->GetSegmentCount() : 1;
	double dHeadGapB = 0.0;	// LYS 2026-09-30 back head Y1/Y2 minimum gap (CoreFunction.ini INTERLOCKHEADGAPB)
	pMaint->GetInterlockPos(nullptr, nullptr, nullptr, &dHeadGapB);

	for (int k = 0; k < nSeg; k++)
	{
		S_RecipeMgr* pMgr = (pFunc != NULL && pFunc->GetSegmentCount() > 0) ? pFunc->GetSegmentMgr(k) : pRcp->GetRecipeMgrPtr();
		if (pMgr == nullptr) continue;

		const S_RecipeInfo& sInfo = pMgr->m_sInfo;
		const S_RecipeData& sData = pMgr->m_sData;
		double dOffX = sInfo.m_sMotionOffsetPos.m_fStageY;
		double dOffR = sInfo.m_sMotionOffsetPos.m_fStageRotate;

		for (size_t s = 0; s < sData.m_vecStick.size(); s++)
		{
			const S_StickInfo& sStick = sData.m_vecStick[s];
			double dX = bOCP ? (sStick.m_fStageY + dOffX)
							 : ((s > 0) ? (sData.m_vecStick[0].m_fStageY - sStick.m_fStageY + dOffX) : (pMgr->m_sMap.m_fCellDistance.y * s + dOffX));
			double dR = sStick.m_fStageR + dOffR;

			int	   nAxis[4]	  = { AXIS_X, AXIS_LOADER_R, AXIS_Y1, AXIS_Y2 };
			double dTarget[4] = { dX, dR, 0.0, 0.0 };
			const TCHAR* szName[4] = { _T("STAGE X"), _T("STAGE R"), _T("HEAD Y1(BACK RIGHT)"), _T("HEAD Y2(BACK LEFT)") };
			int nCheck = 2;

			size_t nCell = sStick.m_vecCell.size();
			if (nCell > 0)
			{
				double dA = sStick.m_vecCell[0].m_fCellX, dB = sStick.m_vecCell[nCell - 1].m_fCellX;
				dTarget[2] = ((dA > dB) ? dA : dB) + sInfo.m_sMotionOffsetPos.m_fHeadReviewRight;
				dTarget[3] = ((dA > dB) ? dB : dA) + sInfo.m_sMotionOffsetPos.m_fHeadReviewLeft;
				nCheck = 4;
			}

			for (int j = 0; j < nCheck; j++)
			{
				double dMax = pMaint->GetMaxSWLimit(nAxis[j]);
				double dMin = pMaint->GetMinSWLimit(nAxis[j]);
				if (dMax <= dMin) continue;	// 리밋 미설정 축

				if (dTarget[j] > dMax || dTarget[j] < dMin)
				{
					strLog.Format(_T("[PRECHECK] segment %d/%d stick %d %s target %.3f out of SW limit [%.3f, %.3f] - auto start refused"),
						k + 1, nSeg, sStick.m_nStickNum, szName[j], dTarget[j], dMin, dMax);
					return (dTarget[j] > dMax) ? MOTION_SW_OVER_LIMIT_POS : MOTION_SW_OVER_LIMIT_NEG;
				}
			}

			// LYS 2026-09-30 back heads go to both end cells at once (same |Y1 - Y2| test as the motion layer's 3027)
			if (nCheck == 4 && dHeadGapB > 0.0 && fabs(dTarget[2] - dTarget[3]) < dHeadGapB)
			{
				strLog.Format(_T("[PRECHECK] segment %d/%d stick %d HEAD Y1 %.3f / Y2 %.3f gap %.3f < INTERLOCKHEADGAPB %.3f - auto start refused"),
					k + 1, nSeg, sStick.m_nStickNum, dTarget[2], dTarget[3], fabs(dTarget[2] - dTarget[3]), dHeadGapB);
				return MOTION_INTERLOCK_7;
			}
		}
	}
	return 0;
}

// LYS 2026-09-29 Hybrid Phase 3b A7/A8 capacity pre-check (hybrid recipes only, called from _Change before any motion)
//   total sticks <= M_SEGMENT_STICK_MAX : nCellStatus[100][50] columns, each segment owns its own column range (A8)
//   G x max(K_wad x P, K_vis, K_ang) <= M_RESULT_AREA_MAX : AgingTest repeat / WAD Multi Point blocks are G rows each (A7)
//     G = sum of cells x sticks over all segments, K = AgingTest items using the kind (1 when AgingTest is off),
//     P = WAD Multi Point count (1 when no segment uses it). Returns false = refuse auto start, strLog = reason.
static bool CheckHybridCapacity(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc, CString& strLog)
{
	int nSeg = pFunc->GetSegmentCount();
	int nTotalStick = pFunc->GetSegmentStickBase(nSeg);
	if (nTotalStick > M_SEGMENT_STICK_MAX)
	{
		strLog.Format(_T("[HYBRID] total sticks %d > %d (cell status columns) - auto start refused"), nTotalStick, M_SEGMENT_STICK_MAX);
		return false;
	}

	int nG = 0;
	bool bAnyMulti = false;
	for (int k = 0; k < nSeg; k++)
	{
		nG += pFunc->GetSegmentCellCount(k) * pFunc->GetSegmentStickCount(k);
		S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
		if (pSeg != nullptr && pSeg->m_sInfo.m_sInspectItem.m_bUseSpec_Multi == TRUE) bAnyMulti = true;
	}

	int nK[enumAgingMeasKind_MAX] = { 1, 1, 1 };
	S_RecipeMgr* pSeg0 = pFunc->GetSegmentMgr(0);
	bool bOCP = (pShare->GetSystemConfigMgrPtr() != nullptr && pShare->GetSystemConfigMgrPtr()->IsOCP());
	// LYS 2026-09-30 Hybrid 3c EFA glass: the runtime runs EFA instead of AgingTest (ThreadRun bEfaMode ? AgingCycleBegin(nPass) : AgingCycleBegin(-1)).
	//   An EFA pass keeps one item per stick, so VISION / ANGLE stay in block 0 (T1 overwrites T0), but WAD Multi Point still reserves
	//   GetAgingKindMax(WAD) blocks first (= the EFA rcp's USE_AGING + USE_PROPERTY items) -> K wad = max(1, that count), K vis/ang = 1.
	//   G x K x P stays an upper bound (K + P - 1 <= K x P). Recipe EFA flag = EFA for sure (the AgingRcp K is not used);
	//   shared flag only = either may run at measure start -> K wad = max(AgingRcp, EFA rcp), conservative.
	int nEfaPass = 0, nEfaKWad = 1;
	bool bEfaRecipe = false;
	if (bOCP && pSeg0 != nullptr)
	{
		bEfaRecipe = (pSeg0->m_sInfo.m_sInspectItem.m_bUseEFA != FALSE);
		bool bEfaFlag = (pShare->GetShareMemoryMgrPtr() != nullptr && pShare->GetShareMemoryMgrPtr()->GetSys_NewAgingMode());
		if (bEfaRecipe || bEfaFlag)
		{
			CAgingCycleInfo efa;
			if (efa.Load(pShare->GetRecipeEditMgrPtr()->GetRecipeID(), true) && efa.GetItemCount() > 0)
			{
				nEfaPass = efa.GetItemCount();
				int n = efa.GetKindUseCount(enumAgingMeasKind_WAD);
				nEfaKWad = (n > 0) ? n : 1;
			}
		}
	}
	bool bEfaSure = (bEfaRecipe && nEfaPass > 0);
	if (bOCP && pSeg0 != nullptr && pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test != FALSE && bEfaSure == false)
	{
		CAgingCycleInfo info;
		if (info.Load(pShare->GetRecipeEditMgrPtr()->GetRecipeID()))
		{
			for (int j = 0; j < enumAgingMeasKind_MAX; j++)
			{
				int n = info.GetKindUseCount(j);
				nK[j] = (n > 0) ? n : 1;
			}
		}
	}
	int nRcpKWad = nK[enumAgingMeasKind_WAD];	// LYS 2026-09-30 AgingRcp (or 1) K for the K x P layout below
	if (bEfaSure || (nEfaPass > 0 && nEfaKWad > nK[enumAgingMeasKind_WAD]))
		nK[enumAgingMeasKind_WAD] = nEfaKWad;	// EFA for sure: vis/ang stay 1 (AgingRcp block skipped)

	// LYS 2026-09-30 Hybrid 3d B10 P = max over the segments using WAD Multi Point, each read from its own section.
	//   Rows stay inside G x K x max P_k because a segment's block index is at most K x P_k - 1 (D3d-2: each segment keeps its own stride)
	int nP = 1;
	CString strPerSeg;
	CWadMultiMeasMgr* pMultiShared = pShare->GetWadMultiMeasMgrPtr();
	if (bAnyMulti && pMultiShared != nullptr)
	{
		CWadMultiMeasMgr multi(pMultiShared->GetRcpFolderPath());	// local copy - the shared manager keeps its run state
		for (int k = 0; k < nSeg; k++)
		{
			S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
			int nPk = 1;
			if (pSeg != nullptr && pSeg->m_sInfo.m_sInspectItem.m_bUseSpec_Multi == TRUE
				&& multi.LoadMeasSegment(pShare->GetRecipeEditMgrPtr()->GetRecipeID(), k) && multi.IsMultiMode() && multi.GetUseSize() > 0 && multi.GetPointCount() > 1)
				nPk = multi.GetPointCount();
			if (nPk > nP) nP = nPk;
			CString strOne;
			strOne.Format(_T("%s%d"), (k > 0) ? _T("/") : _T(""), nPk);
			strPerSeg += strOne;
		}
	}

	// LYS 2026-09-30 Hybrid 3c WAD blocks by layout. AgingRcp: rows K + rep x (P - 1) + ip - 1 -> K x P blocks.
	//   EFA: the base point stays in block 0 (one item per stick, no loop-back) and Multi point ip goes to block K + ip - 1
	//   -> K + P - 1 blocks when P > 1, 1 block when P = 1 (K x P over-counted and could refuse a glass that fits).
	int nEfaBlock = (nP > 1) ? nEfaKWad + nP - 1 : 1;
	int nBlock = bEfaSure ? nEfaBlock : nRcpKWad * nP;
	if (!bEfaSure && nEfaPass > 0 && nEfaBlock > nBlock) nBlock = nEfaBlock;	// EFA flag only: either may run at measure start
	if (nK[enumAgingMeasKind_VISION] > nBlock) nBlock = nK[enumAgingMeasKind_VISION];
	if (nK[enumAgingMeasKind_ANGLE] > nBlock)  nBlock = nK[enumAgingMeasKind_ANGLE];

	int nRows = nG * nBlock;
	strLog.Format(_T("[HYBRID] capacity: sticks %d/%d, result rows %d/%d (G %d x block %d; K wad/vis/ang %d/%d/%d, WAD points %d)"),
		nTotalStick, M_SEGMENT_STICK_MAX, nRows, M_RESULT_AREA_MAX, nG, nBlock,
		nK[enumAgingMeasKind_WAD], nK[enumAgingMeasKind_VISION], nK[enumAgingMeasKind_ANGLE], nP);
	if (strPerSeg.IsEmpty() == FALSE)	// LYS 2026-09-30 Hybrid 3d
		strLog += _T(" (per segment ") + strPerSeg + _T(")");
	if (nEfaPass > 0)	// LYS 2026-09-30 Hybrid 3c
	{
		CString strEfa;
		if (bEfaSure) strEfa.Format(_T(" (EFA %d passes: K from the EFA rcp, T1 overwrites T0)"), nEfaPass);
		else          strEfa.Format(_T(" (EFA flag only, %d passes: K wad = max(AgingRcp, EFA rcp %d))"), nEfaPass, nEfaKWad);
		strLog += strEfa;
	}
	if (nRows > M_RESULT_AREA_MAX)
	{
		strLog += _T(" - auto start refused");
		return false;
	}
	return true;
}

// LYS 2026-09-30 Hybrid Phase 3d B11 WAD Multi Point data per segment (hybrid recipes only). WARN only, never refuses (D3d-3):
//   the runtime is safe by itself (a cell outside the mask rows = base point only), the dialog save check is the first defence.
//   For each segment k using WAD Multi: section present / INSP_MODE, mask rows x width = segment sticks x cells,
//   CELL_X/Y vs |m_fCellSize.x/.y| (X = head Y direction, InRecipeFunc CheckRecipeLayout), active points ip >= 1 inside +-cell/2.
static void CheckHybridWadMulti(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc)
{
	CWadMultiMeasMgr* pMultiShared = pShare->GetWadMultiMeasMgrPtr();
	if (pMultiShared == nullptr) return;
	CWadMultiMeasMgr multi(pMultiShared->GetRcpFolderPath());	// local copy - the shared manager keeps its run state
	CString strRcpID = pShare->GetRecipeEditMgrPtr()->GetRecipeID();
	int nSeg = pFunc->GetSegmentCount();
	for (int k = 0; k < nSeg; k++)
	{
		S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
		if (pSeg == nullptr || pSeg->m_sInfo.m_sInspectItem.m_bUseSpec_Multi != TRUE) continue;
		double dRX = fabs(pSeg->m_sMap.m_fCellSize.x), dRY = fabs(pSeg->m_sMap.m_fCellSize.y);
		int nStick = pFunc->GetSegmentStickCount(k), nCell = pFunc->GetSegmentCellCount(k);
		CString strData = CWadMultiMeasMgr::GetDataSection(k), strMask = CWadMultiMeasMgr::GetMaskSection(k), strLog;

		if (multi.LoadMeasSegment(strRcpID, k) == false)
		{
			strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: file %s not found - base point only"), k + 1, nSeg, multi.GetRcpFilePath(strRcpID).GetString());
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
			continue;
		}
		if (multi.IsDataFound() == false || multi.IsMultiMode() == false)
		{
			strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: [%s] %s - base point only"), k + 1, nSeg, strData.GetString(), multi.IsDataFound() ? _T("INSP_MODE 0") : _T("not found"));
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
			continue;
		}
		if (multi.IsMaskFound() == false)
		{
			strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: [%s] not found - every cell uses the base point only"), k + 1, nSeg, strMask.GetString());
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
		else
		{
			int nRows = multi.GetCellUseStickCount(), nBad = 0;
			for (int i = 0; i < nRows && i < nStick; i++)
				if (multi.GetCellUseCellCount(i) != nCell) nBad++;
			if (nRows != nStick || nBad > 0)
			{
				strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: [%s] rows %d / sticks %d, %d row(s) not %d cells - cells outside the rows use the base point only"),
					k + 1, nSeg, strMask.GetString(), nRows, nStick, nBad, nCell);
				pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
			}
		}
		double dCX = multi.GetCellX(), dCY = multi.GetCellY();
		if (fabs(dCX - dRX) > 0.5 || fabs(dCY - dRY) > 0.5)
		{
			bool bSwap = (fabs(dCX - dRY) <= 0.5 && fabs(dCY - dRX) <= 0.5);
			strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: [%s] CELL %.4f x %.4f != recipe cell %.4f x %.4f%s"), k + 1, nSeg, strData.GetString(),
				dCX, dCY, dRX, dRY, bSwap ? _T(" (X/Y swapped? points are in this segment's frame)") : _T(""));
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
		int nOut = 0, nFirstOut = -1;
		for (int ip = 1; ip < multi.GetPointCount(); ip++)
		{
			S_WadMultiPoint pt;
			if (multi.GetPoint(ip, pt) == false || pt.bEnable == false) continue;
			if (fabs(pt.dPosX) >= dRX / 2.0 || fabs(pt.dPosY) >= dRY / 2.0)
			{
				if (nFirstOut < 0) nFirstOut = ip;
				nOut++;
			}
		}
		if (nOut > 0)
		{
			strLog.Format(_T("[HYBRID] WAD Multi segment %d/%d: %d active point(s) outside +-recipe cell/2 (first: point %d)"), k + 1, nSeg, nOut, nFirstOut + 1);
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
	}
}

// LYS 2026-09-30 Hybrid Phase 3c AgingRcp / EFA rcp cell masks per segment, checked before the glass moves (hybrid recipes only).
//   WARN only, never refuses (D4 / D3c-5): a segment without its mask section measures every cell of each item's kinds -
//   2026-09-30 13:03 segment 2 did that without a log line. Also A12 (the PTN index goes to PG models that differ between
//   segments) and a per-segment m_bUseAgingTest_2 mismatch (the AGING step reads the active segment's flag).
//   Gates = the runtime's: EFA = OCP + (shared flag or segment-0 recipe EFA flag) + an EFA rcp with items (GetEfaPassCount),
//   otherwise AgingTest = segment 0 m_bUseAging_Test (AgingGlassLatch) + an aging rcp with items.
static void CheckHybridAgingMask(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc)
{
	if (pShare->GetSystemConfigMgrPtr() == nullptr || pShare->GetSystemConfigMgrPtr()->IsOCP() == false) return;
	S_RecipeMgr* pSeg0 = pFunc->GetSegmentMgr(0);
	if (pSeg0 == nullptr) return;
	CString strRcpID = pShare->GetRecipeEditMgrPtr()->GetRecipeID();
	int nSeg = pFunc->GetSegmentCount();

	CAgingCycleInfo info;
	bool bFlag = (pShare->GetShareMemoryMgrPtr() != nullptr && pShare->GetShareMemoryMgrPtr()->GetSys_NewAgingMode());
	bool bEfa = (bFlag || pSeg0->m_sInfo.m_sInspectItem.m_bUseEFA != FALSE)
		&& info.Load(strRcpID, true, nSeg) && info.GetItemCount() > 0;
	if (bEfa == false)
	{
		if (pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test == FALSE) return;
		if (info.Load(strRcpID, false, nSeg) == false || info.GetItemCount() < 1) return;	// the runtime logs "aging rcp not found"
	}
	LPCTSTR szKind = bEfa ? _T("EFA") : _T("AgingRcp");

	CString strLog, strOne, strWarn;
	strLog.Format(_T("[HYBRID] %s mask rows per segment:"), szKind);
	for (int k = 0; k < nSeg; k++)
	{
		int nRows = info.GetStickCount(k), nSticks = pFunc->GetSegmentStickCount(k);
		CString strSec = CAgingCycleInfo::GetMaskSection(k);
		if (nRows > 0) strOne.Format(_T(" seg%d [%s] %d/%d"), k + 1, strSec.GetString(), nRows, nSticks);
		else           strOne.Format(_T(" seg%d [%s] none"), k + 1, strSec.GetString());
		strLog += strOne;
		strWarn.Empty();
		if (nRows == 0)
			strWarn.Format(_T("[HYBRID] %s mask [%s] not found - segment %d/%d measures every cell of each item's kinds (no mask = measure all)"), szKind, strSec.GetString(), k + 1, nSeg);
		else if (nRows != nSticks)
			strWarn.Format(_T("[HYBRID] %s mask [%s] rows %d != segment %d/%d sticks %d - %s"), szKind, strSec.GetString(), nRows, k + 1, nSeg, nSticks,
				(nRows < nSticks) ? _T("sticks without a row are skipped") : _T("rows past the last stick are unused"));
		if (strWarn.IsEmpty() == FALSE) pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strWarn);
	}
	strLog += _T(" (none = every cell measured)");
	pShare->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	bool bPtn = false, bAgingItem = false;
	for (int i = 0; i < info.GetItemCount(); i++)
	{
		const S_AgingTest* pItem = info.GetItem(i);
		if (pItem == nullptr) continue;
		if (pItem->m_bUseAging != FALSE) bAgingItem = true;
		// EFA AGING_PTN lights the pass item's pattern even when USE_AGING = 0; an AgingTest cycle runs USE_AGING items only
		if (pItem->m_nPtnRGB > 0 && (bEfa || pItem->m_bUseAging != FALSE)) bPtn = true;
	}
	if (bPtn)	// A12 the PTN index is relative to the PG model of the segment (PGEvent_0 szModel = segment header, blank = RecipeID)
	{
		CString strPG0, strList;
		bool bDiff = false;
		for (int k = 0; k < nSeg; k++)
		{
			CString strPG = pFunc->GetSegmentHeader(k).m_chPGName;
			strPG.Trim();
			if (strPG.IsEmpty()) strPG = strRcpID;
			if (k == 0) strPG0 = strPG; else if (strPG != strPG0) bDiff = true;
			strOne.Format(_T("%sseg%d [%s]"), (k > 0) ? _T(" / ") : _T(""), k + 1, strPG.GetString());
			strList += strOne;
		}
		if (bDiff)
		{
			strLog.Format(_T("[HYBRID] %s PTN index goes to PG models %s - their pattern tables must match (A12)"), szKind, strList.GetString());
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
	}
	if (bAgingItem)	// the AGING step reads the ACTIVE segment's m_bUseAgingTest_2 (DoWorkGlassMeasureStep AGING bUseMeas); ValidateSegments does not compare it
	{
		bool b0 = (pSeg0->m_sInfo.m_sInspectItem.m_bUseAgingTest_2 != FALSE);
		CString strDiff;
		for (int k = 1; k < nSeg; k++)
		{
			S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
			if (pSeg == nullptr || (pSeg->m_sInfo.m_sInspectItem.m_bUseAgingTest_2 != FALSE) == b0) continue;
			strOne.Format(_T("%s%d"), strDiff.IsEmpty() ? _T("") : _T(","), k + 1);
			strDiff += strOne;
		}
		if (strDiff.IsEmpty() == FALSE)
		{
			strLog.Format(_T("[HYBRID] %s: AGING (m_bUseAgingTest_2) of segment(s) %s differs from segment 1 (%d) - aging runs only where it is on%s"),
				szKind, strDiff.GetString(), (int)b0, bEfa ? _T(", T1 still re-measures every segment") : _T(""));
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
	}
}

// LYS 2026-10-01 Hybrid Phase 4 (4-3) glass-wide result items, latched once per glass on this (sequence) thread - the segment vectors
//   are read only here (same place as CheckHybridCapacity / CheckHybridAgingMask). Hybrid recipes only.
//   (1) result save gate (user policy 2026-09-29): any segment with an optical item (Spec_3 / Quality_4 / Color_5 / Angle_6 /
//       TSP_7 / LCR_8) or AgingTest_2 -> save; electrical only -> no file + Unload CIM bypass. Result and Unload read the latch.
//   (2) APD header CELL_X_CNT / CELL_Y_CNT = G x 1 (stGlassInfo -> WAD result stData[4]/[5] -> dResult): X x Y = row block size G
//   (3) cell ID letter range and the longest ID vs m_szCellID[..][32] - log only (GenCellID cuts and warns at run time)
//   (4) D4-14 estimate (review A2): dResult ExportAsRawDateICP merges sheets 1..4 (WAD 1 / VISION 2 / ANGLE 3) with the MaxRows of the
//       LAST non-empty sheet, so rows above the top sheet's last row drop out of the merged sheet and the APD. Recipe flags only
//       (AgingRcp/EFA masks, sampling and AgingCycle blocks are not counted) - WARN, nothing is refused
//       LYS 2026-10-01 Hybrid Phase 4 (fix) loss = top sheet ends before the last row any optical sheet reaches (not G)
static void LatchHybridResultItems(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc)
{
	CRecipeEditMgr* pRcp = pShare->GetRecipeEditMgrPtr();
	int nSeg = pFunc->GetSegmentCount();

	bool bSave = false, bSave0 = false;
	int nLetters = 0, nRowEnd = 0;
	int nSheetRow[4] = { 0, 0, 0, 0 };	// last reachable T0 row per dResult sheet: [1] WAD Spec_3, [2] VISION Quality_4, [3] ANGLE Angle_6
	CString strGate = _T("[HYBRID] result save gate:"), strOne;
	for (int k = 0; k < nSeg; k++)
	{
		nLetters += CRecipeEditMgr::SegCellIdLetterExtent(pFunc, k);
		nRowEnd += pFunc->GetSegmentCellCount(k) * pFunc->GetSegmentStickCount(k);
		S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
		if (pSeg == nullptr) continue;
		const S_InspectItem& sItem = pSeg->m_sInfo.m_sInspectItem;
		bool bOptical = (sItem.m_bUseSpec_3 == TRUE || sItem.m_bUseQuality_4 == TRUE || sItem.m_bUseColor_5 == TRUE
			|| sItem.m_bUseAngle_6 == TRUE || sItem.m_bUseTSP_7 == TRUE || sItem.m_bUseLCR_8 == TRUE);
		bool bAging2 = (sItem.m_bUseAgingTest_2 == TRUE);
		if (bOptical || bAging2) { bSave = true; if (k == 0) bSave0 = true; }
		if (sItem.m_bUseSpec_3 == TRUE)    nSheetRow[1] = nRowEnd;
		if (sItem.m_bUseQuality_4 == TRUE) nSheetRow[2] = nRowEnd;
		if (sItem.m_bUseAngle_6 == TRUE)   nSheetRow[3] = nRowEnd;
		strOne.Format(_T("%s seg%d [%s] optical %d aging2 %d"), (k > 0) ? _T(",") : _T(""), k + 1, pFunc->GetSegmentName(k).GetString(), bOptical ? 1 : 0, bAging2 ? 1 : 0);
		strGate += strOne;
	}
	strGate += bSave ? _T(" -> save") : _T(" -> bypass (electrical only)");
	if (bSave && bSave0 == false) strGate += _T(" (segment 1 alone: bypass)");
	pRcp->SetResultSaveLatch(bSave ? 1 : 0);
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strGate);

	int nTopSheet = 3;
	while (nTopSheet > 0 && nSheetRow[nTopSheet] == 0) nTopSheet--;
	// LYS 2026-10-01 Hybrid Phase 4 (fix) compare with the last row any optical sheet reaches, not G: rows past every optical
	//   sheet (e.g. a trailing electrical-only segment) hold no WAD / VISION / ANGLE data, so the merge loses nothing there
	int nOptEnd = (nSheetRow[1] > nSheetRow[2]) ? nSheetRow[1] : nSheetRow[2];
	if (nSheetRow[3] > nOptEnd) nOptEnd = nSheetRow[3];
	if (nTopSheet > 0 && nSheetRow[nTopSheet] < nOptEnd)
	{
		CString strMerge;
		strMerge.Format(_T("[HYBRID] APD merge keeps rows 1..%d of %d (top sheet %s ends at row %d, dResult D4-14) - later rows miss the merged sheet / APD"),
			nSheetRow[nTopSheet], nOptEnd, (nTopSheet == 3) ? _T("ANGLE") : (nTopSheet == 2) ? _T("VISION") : _T("WAD"), nSheetRow[nTopSheet]);
		pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strMerge);
	}

	int nG = pRcp->GetGlassRowBlock(0, 0);
	CString strApd;
	strApd.Format(_T("[HYBRID] APD header CELL_X_CNT %d CELL_Y_CNT 1 (glass rows G x 1; segment 1 alone %d x %d)"), nG,
		pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount, pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount);
	pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount = nG;
	pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount = 1;
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strApd);

	CString strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	CString strLast = CRecipeEditMgr::CellIdLetterPrefix(nLetters - 1);
	int nGidLen = strGlassID.GetLength();
	int nLongest = ((nGidLen > 0) ? nGidLen - 1 : 0) + strLast.GetLength() + 2 + 1;	// GlassID - 1 + letters + 2 digits + checksum
	CString strCid;
	strCid.Format(_T("[HYBRID] cell ID letters A..%s over %d segments, longest ID %d/31 chars (GlassID length %d)"), strLast.GetString(), nSeg, nLongest, nGidLen);
	pShare->GetLogFileAccessorPtr()->KMLOG(nLongest > 31 ? WARN : OPER, strCid);
}

// LYS 2026-10-01 Hybrid Phase 4b D-15 (docs/HybridGlass_Porting_Plan.md 5.7) English text of one Cell ID error for the GLASS log
static CString CellIdErrText(const S_CellIdError& e, int nGlassIdLen)
{
	CString strWhere, strWhy;
	strWhere.Format(_T("Segment %d Stick %d Cell %d (%c)"), e.m_nSeg + 1, e.m_nStick + 1, e.m_nCell + 1, e.m_nPanel ? _T('B') : _T('A'));
	switch (e.m_nCode)
	{
	case M_CELLID_ERR_EMPTY:  strWhy = _T("empty"); break;
	case M_CELLID_ERR_LENGTH: strWhy.Format(_T("'%s' longer than %d chars (cell ID max 31, GlassID length %d)"), e.m_strToken.GetString(), e.m_nLimit, nGlassIdLen); break;
	case M_CELLID_ERR_FORMAT: strWhy.Format(_T("'%s' is not A-Z letters + digits"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DERIVE: strWhy.Format(_T("B cell ID cannot be derived from A manual ID '%s'"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DUP:    strWhy.Format(_T("duplicate '%s' (Segment %d Stick %d Cell %d (%c))"), e.m_strToken.GetString(),
								  e.m_nSeg2 + 1, e.m_nStick2 + 1, e.m_nCell2 + 1, e.m_nPanel2 ? _T('B') : _T('A')); break;
	default:                  strWhy.Format(_T("error %d"), e.m_nCode); break;
	}
	return strWhere + _T(": ") + strWhy;
}

// LYS 2026-10-01 Hybrid Phase 4b D-15 Cell ID A/B re-check at every auto start, right after the recipe reload (Glass ID final).
//   Auto/Auto (every recipe saved before 4b): latch 0, no log. false = refuse without an alarm (caller SetFail(true, 0))
static bool CheckCellIdConfig(CBaseSequenceImpl* pShare)
{
	CRecipeEditMgr* pRcp = pShare->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	// LYS 2026-10-01 Hybrid Phase 4b review fix: load serial taken BEFORE the check - a CIM PPID / DLL TestMode reload that lands after the
	//   check must not bind latch 1 to a config nobody validated (GetManualCellToken then sees another serial -> one WARN, Auto IDs)
	int nLoadSeq = 0, nLoadTry = 0;
	BOOL bLoadBusy = FALSE;
	if (pFunc != NULL)
	{
		nLoadTry = pFunc->GetRcpLoadTry();	// try count first, then the busy count (LoadRcpFile raises them the other way round)
		bLoadBusy = pFunc->IsRcpLoadBusy();
		nLoadSeq = pFunc->GetRcpLoadSeq();
	}
	if (pFunc == NULL || pFunc->IsCellIdDefault())
	{
		pRcp->SetCellIdLatch(0);
		return true;
	}
	CString strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	CString strGidLog = strGlassID.IsEmpty() ? CString(_T("(none)")) : strGlassID;	// AgingTest / AgingNew START right after boot: no Glass ID (panel A)
	int nGid = strGlassID.GetLength(), nLongest = 0;
	std::vector<S_CellIdError> vecErr;
	CString strLog;
	if (pFunc->ValidateCellIdConfig(nGid, vecErr, &nLongest) != TRUE)
	{
		for (size_t i = 0; i < vecErr.size() && i < 10; i++)
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, _T("[CELLID] ") + CellIdErrText(vecErr[i], nGid));
		if (vecErr.size() > 10)
		{
			strLog.Format(_T("[CELLID] ... %d more"), (int)vecErr.size() - 10);
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
		strLog.Format(_T("[CELLID] cell ID check failed (%d errors, GlassID %s) - auto start refused"), (int)vecErr.size(), strGidLog.GetString());
		pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		pRcp->SetCellIdLatch(-1);
		return false;
	}
	// LYS 2026-10-01 Hybrid Phase 4b review fix: a recipe load ran while the config was read -> what passed is unknown, refuse like a failed check
	if (bLoadBusy == TRUE || pFunc->IsRcpLoadBusy() == TRUE || pFunc->GetRcpLoadTry() != nLoadTry)
	{
		strLog.Format(_T("[CELLID] recipe reloaded during the cell ID check (recipe load %d/%d, GlassID %s) - auto start refused"), nLoadSeq, pFunc->GetRcpLoadSeq(), strGidLog.GetString());
		pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		pRcp->SetCellIdLatch(-1);
		return false;
	}
	pRcp->SetCellIdLatch(1, nLoadSeq, strGlassID);	// bound to the load the check started on + this Glass ID (GetManualCellToken compares both)
	bool bManA = (pFunc->GetCellIdMode(0) == M_CELLID_MODE_MANUAL), bManB = (pFunc->GetCellIdMode(1) == M_CELLID_MODE_MANUAL);
	bool bPanelB = (strGlassID.Mid(nGid - 2, 1).CompareNoCase(_T("B")) == 0);	// same rule as GenCellID
	strLog.Format(_T("[CELLID] manual cell ID: A %s, B %s, glass panel %c (GlassID %s), longest ID %d/31 chars"),
		bManA ? _T("MANUAL") : _T("AUTO"), bManB ? _T("MANUAL") : (bManA ? _T("AUTO (from A)") : _T("AUTO")),
		bPanelB ? _T('B') : _T('A'), strGidLog.GetString(), ((nGid > 0) ? nGid - 1 : 0) + nLongest + 1);
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strLog);
	return true;
}

bool CDoWorkGlassRecipeStep_Change::Run()
{
	//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_Change Start."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_RECIPE, true);

	bool bMeasMode[enumMEASURE_MODE_END];
	ZeroMemory(&bMeasMode, sizeof(bMeasMode));

	// jsyoon 2022.07.19 add
	CString strPPID;
	strPPID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetRecipeID());
	// LYS 2026-10-01 Hybrid Phase 4b D4b-15 (b) the .rcp itself could not be read (missing / corrupt, e.g. a broken Cell ID trailer):
	//   OpenRecipe left the previous in-memory recipe -> refuse instead of running it. A readable recipe (load serial advanced) keeps
	//   the original path, including the "base .alg missing" return (JOB_RECIPE_NOT_EXIST after a good .rcp load)
	C_OutRecipeFunc* pLoadFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	int nLoadSeq = (pLoadFunc != NULL) ? pLoadFunc->GetRcpLoadSeq() : 0;
	m_pShareItem->GetRecipeEditMgrPtr()->OpenRecipe(strPPID); 
	if (pLoadFunc != NULL && strPPID.IsEmpty() != true && pLoadFunc->GetRcpLoadSeq() == nLoadSeq)
	{
		CString strRcpLog;
		strRcpLog.Format(_T("[RECIPE] recipe %s could not be read (file missing or corrupt) - auto start refused, the previous in-memory recipe is not used"), strPPID.GetString());
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strRcpLog);
		m_pShareItem->SetFail(true, JOB_RECIPE_NOT_EXIST);	// alarm 1013, same as the hybrid .alg refusal (:672)
		return false;
	}

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

	// 
	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_nCellCnt;
	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_nStickCnt;

	// LYS 2026-09-29 Hybrid Phase 3 세그먼트 .alg 가 하나라도 없으면 자동 시작 거부 (ICP와 같음, JOB_RECIPE_NOT_EXIST 알람)
	//                stGlassInfo(결과 헤더 CELL/STICK MAX)는 세그먼트 0 값 그대로 둔다 - 세그먼트 경계에서 바꾸지 않는다
	{
		C_OutRecipeFunc* pSegFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		if (pSegFunc != NULL && pSegFunc->GetSegmentCount() > 1)
		{
			bool bAlignOK = true;
			for (int k = 0; k < pSegFunc->GetSegmentCount(); k++)
			{
				if (pSegFunc->IsSegmentAlignLoaded(k) == FALSE)
				{
					CString strSegLog;
					strSegLog.Format(_T("[HYBRID] Segment %d [%s] align recipe [%s] not loaded - auto start refused"),
						k + 1, pSegFunc->GetSegmentName(k).GetString(), pSegFunc->GetSegmentAlignName(k).GetString());
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strSegLog);
					bAlignOK = false;
				}
			}
			// LYS 2026-09-29 Hybrid Phase 3b A7/A8 capacity pre-check (same refusal path as the .alg check above)
			CString strCapLog;
			if (CheckHybridCapacity(m_pShareItem, pSegFunc, strCapLog))
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strCapLog);
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strCapLog);
				bAlignOK = false;
			}
			// LYS 2026-09-30 Hybrid 3d B11 WAD Multi Point data per segment - WARN only (D3d-3)
			CheckHybridWadMulti(m_pShareItem, pSegFunc);
			// LYS 2026-09-30 Hybrid 3c AgingRcp / EFA rcp mask sections per segment, A12, per-segment AGING flag - WARN only (D3c-5)
			CheckHybridAgingMask(m_pShareItem, pSegFunc);
			LatchHybridResultItems(m_pShareItem, pSegFunc);	// LYS 2026-10-01 Hybrid Phase 4 (4-3) save gate latch + APD header G x 1 (stGlassInfo: replaces the segment-0 values set above) + cell ID letters (hybrid only)
			if (bAlignOK != true)
			{
				m_pShareItem->SetFail(true, JOB_RECIPE_NOT_EXIST);
				return false;
			}
		}
	}

	// LYS 2026-10-01 Hybrid Phase 4b D-15 Cell ID A/B re-check (any segment count - the manual mode is recipe-global). Auto/Auto -> no log
	if (CheckCellIdConfig(m_pShareItem) != true)
	{
		m_pShareItem->SetFail(true, 0);	// D-15: no alarm (the Alarm DLL ignores id < 1000) - WARN lines + "Fail Exit" + "The Inspection is Fail.!!"
		return false;
	}

	// LYS 2026-09-29 측정 위치 SW 리밋 사전 검사 - 모션 전에 거부한다 (세그먼트 1을 다 잰 뒤 세그먼트 2에서 멈추던 사고 재발 방지)
	{
		CString strLimitLog;
		bool bOCP = (m_pShareItem->GetSystemConfigMgrPtr() != nullptr && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP());
		long nLimitAlarm = CheckMeasureTargetSWLimit(m_pShareItem->GetRecipeEditMgrPtr(), m_pShareItem->GetMaintControlMgrPtr(), bOCP, strLimitLog);
		if (nLimitAlarm != 0)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strLimitLog);
			m_pShareItem->SetFail(true, (int)nLimitAlarm);
			return false;
		}
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[PRECHECK] measure targets within SW limits"));
	}

#ifdef _USE_TEST
	if (pReciepInfo != nullptr)
	{
		pReciepInfo->m_sInspectItem.m_bUsePre_0 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseElect_1 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseAgingTest_2 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseSpec_3 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseQuality_4 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseColor_5 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseAngle_6 = TRUE;
		pReciepInfo->m_sInspectItem.m_bUseTSP_7 = TRUE;

		pReciepInfo->m_sContactPos.m_fPreContactPos = 5.;
		pReciepInfo->m_sContactPos.m_fEndContactPos = 6.;
		pReciepInfo->m_sContactPos.m_fAddOD = 0.2;
	}

	S_RecipeMap* pRecipeMap;
	pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
	if (pRecipeMap != nullptr)
	{
		pRecipeMap->m_fCellDistance.x = 150.;
		pRecipeMap->m_fCellDistance.y = 200.;
	}

	if (pRecipeData != nullptr)
	{
		pRecipeData->m_vecStick.clear();

		pRecipeData->m_sHeadZ.m_fSpecZ = 500.;

		S_StickInfo stickInfo;
		stickInfo.Clear();
		for (size_t i = 0; i < 5; i++)
		{
			stickInfo.m_nStickNum = i;
			stickInfo.m_fStageR = 0;
			stickInfo.m_fStageY = 400. - (pRecipeMap->m_fCellDistance.y * i);
			stickInfo.m_vecCell.clear();

			S_CellInfo cellInfo;
			cellInfo.Clear();
			for (size_t j = 0; j < 10; j++)
			{
				cellInfo.m_nArea = (i*10) + j;
				cellInfo.m_fCellX = -800. + (pRecipeMap->m_fCellDistance.x * j);
				stickInfo.m_vecCell.push_back(cellInfo);
			}

			pRecipeData->m_vecStick.push_back(stickInfo);
		}
	}

#endif // _USE_TEST

	if(pReciepInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE) bMeasMode[enumMEASURE_MODE_AGING] = true;
	if(pReciepInfo->m_sInspectItem.m_bUseSpec_3 == TRUE)      bMeasMode[enumMEASURE_MODE_WAD] = true;
	if(pReciepInfo->m_sInspectItem.m_bUseQuality_4 == TRUE)   bMeasMode[enumMEASURE_MODE_VISION] = true;
	if(pReciepInfo->m_sInspectItem.m_bUseColor_5 == TRUE)     bMeasMode[enumMEASURE_MODE_COLOR] = true;
	if(pReciepInfo->m_sInspectItem.m_bUseAngle_6 == TRUE)     bMeasMode[enumMEASURE_MODE_ANGLE] = true;
	if(pReciepInfo->m_sInspectItem.m_bUseTSP_7 == TRUE)       bMeasMode[enumMEASURE_MODE_TSP] = true;
	

	if ( (pReciepInfo->m_sInspectItem.m_bUsePre_0 == TRUE ) | (pReciepInfo->m_sInspectItem.m_bUseElect_1 == TRUE) | (pReciepInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE) )
		bMeasMode[enumMEASURE_MODE_PG] = true;
	// LYS 2026-10-01 Hybrid Phase 4 (4-7) open every device any segment uses (D4-18) - segment 0 alone opened no WAD / VISION for 1_OCP_TEST_Long_P1RT. 1 segment: no-op, no line
	{
		CString strDevLog;
		if (m_pShareItem->GetRecipeEditMgrPtr()->OrSegmentMeasMode(bMeasMode, &strDevLog))
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, CString(_T("[HYBRID] Recipe Change device recipe open (all segments):")) + strDevLog);
	}
	//	bMeasMode[enumMEASURE_MODE_LCR] = true;

	// 위치 이동 하여 검사 초기화 및 해제 (Cycle 시작전, Auto Proc 시작전, 검사 종료 후)
	m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_RESET);		  //화질 자동검사 시작시 초기화
	m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkVisionCmd(EXE_QUALITY_WAIT_USE, TRUE); //화질 자동결과 설정

	// Recipe Open
	int nRet = VTM_RV_SUCCESSED;

	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	if (bMeasMode[enumMEASURE_MODE_VISION]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkSetParameter(enumTCPIpFactorytID_VISION);  // Vistion Motion Homing
	//}

	if (bMeasMode[enumMEASURE_MODE_PG]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_AGING]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_AGING);
	}

	// 특성 Change Recipe
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_WAD]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_WAD);
	}

	// LYS 2026-07-02 OCP 모드 LCR/TSP Not Use
	if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP() == false)
	{
		// Change Recipe
		if (nRet == VTM_RV_SUCCESSED)
		{
			if (bMeasMode[enumMEASURE_MODE_LCR]) { nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_LCR); }
		}

		// Change Recipe
		if (nRet == VTM_RV_SUCCESSED)
		{
			if (bMeasMode[enumMEASURE_MODE_TSP]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_TSP);
		}
	}

	// Change Recipe
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_VISION]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_VISION);
	}

	//// Change Recipe
	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	if (bMeasMode[enumMEASURE_MODE_COLOR]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_COLOR);
	//}

	//// Change Recipe
	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	if (bMeasMode[enumMEASURE_MODE_ANGLE]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_ANGLE);
	//}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_Change End."));

	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetFail(true, nRet);
		return false;
	}

	return true;
}


bool CDoWorkGlassRecipeStep_ProbeCheck::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck Start."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_PROBE, true);

	// LYS 2026-09-29 OOS(IO 또는 모션)면 ProbeCheck 전체를 건너뛴다 - 실 PLC Probe ID(D2100/D2150)가 없어
	//                레시피 Probe ID와 비교하면 교체 요청으로 빠지고, ACX SACC 이동도 의미가 없다
	if (m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->IsOutOfService() == true ||
		m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("CDoWorkGlassRecipeStep_ProbeCheck Skip : OOS mode"));
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_PROBE, false);
		return true;
	}

	int nRet = VTM_RV_SUCCESSED;
	bool bRet = true;
	bool bChangeProbe = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	CString strProbeName, strData;
	// 현재 Probe 확인
	int Priority = 0;
	Priority = pReciepInfo->m_sProbeInfo.m_nProbePriority;

	// LYS 2026-06-26 OCP: only P5 (index 4)
	int nProbeStart_RC = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()) ? 4 : 0;
	for (int iProbe = nProbeStart_RC; iProbe < 5; iProbe++)
	{
		bChangeProbe = false;
	
		switch (Priority)
		{
		case 0:
		{
			strProbeName.Format(_T("%s"), pReciepInfo->m_sProbeInfo.m_chProbeFirst[iProbe]);		// recipe
		}
		break;

		case 1:
		{
			strProbeName.Format(_T("%s"), pReciepInfo->m_sProbeInfo.m_chProbeSecond[iProbe]);	// recipe
		}
		break;
		default:
			break;
		}

		if (strProbeName.IsEmpty() == true)
		{
			continue;
		}

		strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[iProbe]);
		strData.TrimRight();
		if (strProbeName.CompareNoCase(strData) != 0 )
		{
			bChangeProbe = true;
		}

		if (bChangeProbe == true)
		{

			int iStartBit = 0;
			switch (iProbe)
			{
			case 0:
				iStartBit = PLC_R_PROBE_UNIT01_CARD_CHK_01;
				break;
			case 1:
				iStartBit = PLC_R_PROBE_UNIT02_CARD_CHK_01;
				break;
			case 2:
				iStartBit = PLC_R_PROBE_UNIT03_CARD_CHK_01;
				break;
			case 3:
				iStartBit = PLC_R_PROBE_UNIT04_CARD_CHK_01;
				break;
			case 4:
				iStartBit = PLC_R_PROBE_UNIT05_CARD_CHK_01;
				break;
			default:
				break;
			}

			bool bExistProbe = false;
			for (int iProbeExist = 0; iProbeExist < 4; iProbeExist++)
			{
				int nBit = m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(iStartBit + iProbeExist);
				if (nBit != -1)
				{
					if (nBit == 1)
						bExistProbe |= true;
				}
			}

			CString strLog;
			strLog.Format(_T("CDoWorkGlassRecipeStep_ProbeCheck Change. %d"), iProbe);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);


			bool bRunStart = false;

			if (bExistProbe != true)
			{
				// Probe 가 없으면 Loading
				int iLoadCassette = -1;
				for (int iCassette = 0; iCassette < 7; iCassette++)		// LYS 2026-07-03 Cassette 5->7
				{
					strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szCassettePROBEID[iCassette]);
					strData.TrimRight();
					if (strProbeName.CompareNoCase(strData) == 0)
					{
						iLoadCassette = iCassette;
						break;
					}
				}

				if (iLoadCassette >= 0)
				{
					bRunStart = true;
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_LODING);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe + df_PROBE_AGING1);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette + 1);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
				}
			}
			else
			{
				// Probe 가 있으면 Exchange
				// Probe 가 없으면 Loading
				int iLoadCassette = -1, iUnLoadCassette = -1;
				for (int iCassette = 0; iCassette < 7; iCassette++)		// LYS 2026-07-03 Cassette 5->7
				{
					strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szCassettePROBEID[iCassette]);
					strData.TrimRight();
					if (strProbeName.CompareNoCase(strData) == 0)
					{
						iLoadCassette = iCassette;
						iUnLoadCassette = iLoadCassette;
						break;
					}
				}

				if(iLoadCassette >= 0)
				{
					bRunStart = true;
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_EXCHANGE);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe + df_PROBE_AGING1);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette + 1);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetUnLoadCassette(iUnLoadCassette + 1);
					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
				}
			}

			Sleep(1000);

			int nLoop = 0;
			bool bRet = false;
			nRet = VTM_RV_FAILED;

			if (bRunStart == true)
			{
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_WORK_SACC_Start, NULL);

				do
				{
					Sleep(100);

					bRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_Fail();
					if (bRet == true)
					{
						nRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetFailNum();
						break;
					}

					bRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_Complete();
					if (bRet == true)
					{
						nRet = VTM_RV_SUCCESSED;
						break;
					}

					nLoop++;

				} while (nLoop < 10 * 60 * 30); // 30min
			}
			
			if (nRet != VTM_RV_SUCCESSED)
			{
				break;
			}
		}
	} 

	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetFail(true, nRet);
		bRet = false;

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck Fail."));
	}
	else
	{
		// LYS 2026-09-08 ProbeCheck 후 ACX SACC에 꽂기
		int nBit = 0;

		// LYS 2026-09-28 OOS: 모션 OOS면 AutoChange X 안전 비트(MOTION_INTERLOCK_11)를 보지 않는다
		bool bMotionOOS = (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true);
		if (bMotionOOS) m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck : motion OOS - skip ACX safety bit check"));

		if (bMotionOOS || (nBit = m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_AUTO_CHANGE_X0_POSITION_03)))		// Safety 위치
		{
			double dACXPos[3];
			ZeroMemory(&dACXPos, sizeof(dACXPos));

			m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(&dACXPos[0], &dACXPos[1], &dACXPos[2]);

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_AC_X, true);

			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveOrgPosA(AXIS_X4, dACXPos[2], true, SPEED_HIGH, 0, 0);

			//if (m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->IsOutOfService() == true)
				m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_AUTO_CHANGE_X0_POSITION_04, true);		// SACC 위치

			if (nRet == true)
			{
				
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_AC_X, false);

				double CurrAC_X = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4);

				double AbsPos;
				AbsPos = CurrAC_X - dACXPos[2];

				if (abs(AbsPos) < 0.1) m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_AUTO_CHANGE_X0_POSITION_04, true);		// SACC Position OK -> Set bit on

				bRet = true;
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck Fail : ACX Move to SACC Fail."));
				bRet = false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck Fail : ACX Position is not Safety."));
			nRet = MOTION_INTERLOCK_11;
			m_pShareItem->SetFail(true, nRet);
			bRet = false;
		}

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck End."));
	}

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck End."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_PROBE, false);

	return bRet;

}