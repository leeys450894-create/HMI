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
#include "common\CommonFunc.h"				// LYS 2026-07-07 PGM_MODE_* constants
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule/FunctionModuleProbeACC.h"

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
	m_pShareItem->GetRecipeEditMgrPtr()->OpenRecipe(strPPID); 

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

	// 
	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_nCellCnt;
	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_nStickCnt;

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
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck End."));
	}

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassRecipeStep_ProbeCheck End."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_PROBE, false);

	return bRet;

}