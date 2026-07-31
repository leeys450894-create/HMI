//#include "stdafx.h"
#include "pch.h"
#include "SeqTactTimeMgr.h"

//#include "WindowEventHandler.h"
#include "..\AuroraFuncDefine.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "common/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSeqTactTimeMgr::CSeqTactTimeMgr(CString strLogPath)
	: m_pLogFileAccessor(new CLogFileAccessor)
	, m_nMeasCount(0)
	, m_nCycleIndex(0)
	, m_fCycleTime_ms(0)
{
	SetName(_T("SEQ_TACT_TIME_MGR"));

    const CString strFolderPath	= strLogPath + _T("TACT TIME\\");
    const CString strLogFile	= strFolderPath + _T("tact_time.log");
    MakeDir(strFolderPath);

	m_strLogPath = strFolderPath;
	
    m_pLogFileAccessor->Initialize(strLogFile);

	m_strGlassID.Empty();
}

CSeqTactTimeMgr::~CSeqTactTimeMgr()
{
	Finalize();
}

void CSeqTactTimeMgr::Finalize()
{
	//----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }
}


void CSeqTactTimeMgr::SetDataClear()
{
    //초기화 한다.
    m_vecData.clear();
}

long CSeqTactTimeMgr::GetDataCount()
{
    return (long)m_vecData.size();
}

void CSeqTactTimeMgr::PushData(STactTimeInfo data)
{
    m_vecData.push_back(data);
}

STactTimeInfo CSeqTactTimeMgr::GetData(int index)
{
    if ((0 <= index) && (index < GetDataCount()))
    {
        return m_vecData[index];
    }

    return STactTimeInfo();
}

void CSeqTactTimeMgr::SetDataClear2()
{
	//초기화 한다.
	m_vecData2.clear();

	m_nMeasCount = 0;
}

long CSeqTactTimeMgr::GetDataCount2()
{
	return (long)m_vecData2.size();
}

void CSeqTactTimeMgr::PushData2(STactTimeMeasInfo data)
{
	m_vecData2.push_back(data);
}

STactTimeMeasInfo CSeqTactTimeMgr::GetData2(int index)
{
	if ((0 <= index) && (index < GetDataCount2())) 
	{
		return m_vecData2[index];
	}

	return STactTimeMeasInfo();
}

void CSeqTactTimeMgr::SetTotalSeqTimeST()
{
    m_timecheck_total.Reset();
	m_fCycleTime_ms = 0;
	m_strStartTime = m_timecheck_total.GetSTtimeString();
	m_strStartTime2 = gf_GetCurTimeString(4);

    CString strLog = _T("--------------- Total Seq Start ---------------");
    m_pLogFileAccessor->KMLOG(OPER, strLog);
}

void CSeqTactTimeMgr::SetTotalSeqTimeED()
{
	float fCycleTime_ms = m_timecheck_total.Elapsed_ms();
	CString strEndTime = m_timecheck_total.GetEDtimeString();

    CString strLog = _T("--------------- Total Seq End ---------------");
    m_pLogFileAccessor->KMLOG(OPER, strLog);

    //-------------------------------------------------------------------
    // tact time histrogy 를 저장한다.
    //-------------------------------------------------------------------
    std::vector<CString> vecHistrogy;

    CString str;
    str.Format(_T("Cycle, No., Item Name, Start, End, Time[sec]\r\n"));
    vecHistrogy.push_back(str);

	str.Format(_T("%d, %d, %s, %s, %s, %.3f\r\n"), 1, 0, m_strGlassID,
		m_strStartTime, strEndTime, fCycleTime_ms/*GetElapsed()*/ / 1000.f);		// PPID ~ 2107
	vecHistrogy.push_back(str);

	strLog.Format(_T("%s, %s, %s, %.3f"), m_strGlassID, m_strStartTime, strEndTime, fCycleTime_ms/*GetElapsed()*/ / 1000.f);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

    for(size_t i = 0; i < m_vecData.size(); ++i)
    {
        CString str;
//        str.Format(_T("%d, %d, %s, %s, %s, %.3f\r\n"), m_vecData[i].cycle_index, m_vecData[i].sitem_index, m_vecData[i].strName.GetString(),
		str.Format(_T("%d, %d, %s, %s, %s, %.3f\r\n"), m_vecData[i].cycle_index, i+1, m_vecData[i].strName.GetString(),
														m_vecData[i].fStart.GetString(), m_vecData[i].fEnd.GetString(), m_vecData[i].fElapsed / 1000.f);
        vecHistrogy.push_back(str);
    }

	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d-%02d-%02d_%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour,st.wMinute, st.wSecond);

	CString strFilePath, strFileFullPath;
	strFilePath.Format(_T("%s\\history\\%d\\"), m_strLogPath.GetString(), st.wDay);
	MakeDir(strFilePath);
	strFileFullPath.Format(_T("%stact_time_history_%s.txt"), strFilePath, sTime.GetString());
	
	if(WriteDataFile(strFileFullPath, vecHistrogy) == TRUE)
	{
		CString strLog;
		strLog.Format(_T("Success : tact time history save [ %s ]"), strFileFullPath.GetString());
	    m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else
	{
		CString strLog;
		strLog.Format(_T("Fail : tact time history save [ %s ]"), strFileFullPath.GetString());
	    m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
}

void CSeqTactTimeMgr::SetCycleTimeST(int cycle_index, CString strGlassID, CString strReciepID)
{
    m_timecheck_cycle.Reset();

	if (m_nMeasCount > 50)
		SetDataClear2();

	STactTimeMeasInfo data;
	data.sitem_index = m_nMeasCount;
	data.strGlassID = strGlassID;		// jsyoon 20211014 add
	data.strRecipeID = strReciepID;		// jsyoon 20211014 add
	data.fStart = m_timecheck_cycle.GetSTtimeString();
	PushData2(data);

	m_strGlassID = strGlassID;

    CString strLog;
    strLog.Format(_T("Cycle Seq Start [ %08d, GlassID: %s, RecipeID: %s]"), cycle_index, strGlassID, strReciepID);
    m_pLogFileAccessor->KMLOG(OPER, strLog);

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_MEASURS_ELAPSED_TIME, NULL, true);
}

void CSeqTactTimeMgr::SetCycleTimeED(int cycle_index)
{
 //   m_timecheck_cycle.Elapsed_ms();

	const float fElapsed_ms = m_timecheck_cycle.Elapsed_ms();
	m_fCycleTime_ms = fElapsed_ms;

	for (int i = (int)(m_vecData2.size() - 1); i >= 0; --i)
	{
		if (m_vecData2[i].sitem_index == m_nMeasCount)
		{
		//	m_vecData2[i].fEnd = m_timecheck_cycle.GetEDtimeString();
		//	m_vecData2[i].fElapsed = fElapsed_ms;
			if (m_vecData2[i].fEnd.IsEmpty() == true)
			{
				m_vecData2[i].fEnd = m_timecheck_cycle.GetEDtimeString();
				m_vecData2[i].fElapsed = fElapsed_ms;

				m_nMeasCount++;		// jsyoon 2022.08.29 change
			//	break;
			}
			break;
		}
	}

//	m_nMeasCount++;

    CString strLog;
    strLog.Format(_T("Cycle Seq End [ %08d ]"), cycle_index);
    m_pLogFileAccessor->KMLOG(OPER, strLog);

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_MEASURS_ELAPSED_TIME, NULL, false);
}

void CSeqTactTimeMgr::SetSeqItemTimeST(int cycle_index, CString strSeqName, int sitem_index)
{
	if (strSeqName.IsEmpty() == TRUE)
		return;

    m_timecheck_sitem.Reset();

    STactTimeInfo data;
	data.cycle_index = m_nCycleIndex; // cycle_index;
	data.Dowork_index = cycle_index;
    data.sitem_index = sitem_index;
    data.strName = strSeqName;
    data.fStart = m_timecheck_sitem.GetSTtimeString();
	

	SYSTEMTIME tCurr;
	GetLocalTime(&tCurr);
	CTime tData(tCurr.wYear, tCurr.wMonth, tCurr.wDay, tCurr.wHour, tCurr.wMinute, tCurr.wSecond);

	tData.GetTime();
	data.lStart = tData;
	data.fmsec = tCurr.wMilliseconds;

    PushData(data);

    CString strLog;
    strLog.Format(_T("Cycle Seq Item : %s - Start"), strSeqName.GetString());
    m_pLogFileAccessor->KMLOG(OPER, strLog);

    CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME, NULL, NULL);
}

void CSeqTactTimeMgr::SetSeqItemTimeED(int cycle_index, CString strSeqName, int sitem_index)
{
	if (strSeqName.IsEmpty() == TRUE)
		return;

  //  const float fElapsed_ms = m_timecheck_sitem.Elapsed_ms();
	float fElapsed_ms = m_timecheck_sitem.Elapsed_ms();
    for(int i = (int)(m_vecData.size() - 1); i >= 0; --i)
    {
        if( (m_vecData[i].Dowork_index == cycle_index) &&
            (m_vecData[i].sitem_index == sitem_index) &&
            (m_vecData[i].strName == strSeqName) )
        {
            m_vecData[i].fEnd = m_timecheck_sitem.GetEDtimeString();
			SYSTEMTIME tCurr;
			GetLocalTime(&tCurr);
			CTime tData(tCurr.wYear, tCurr.wMonth, tCurr.wDay, tCurr.wHour, tCurr.wMinute, tCurr.wSecond);
			m_vecData[i].lEnd = tData;
			CTimeSpan tmSpan = m_vecData[i].lEnd - m_vecData[i].lStart;
			fElapsed_ms = tmSpan.GetTotalSeconds() * 1000 + (tCurr.wMilliseconds-m_vecData[i].fmsec);
            m_vecData[i].fElapsed = fElapsed_ms;
            break;
        }
    }

    CString strLog;
    strLog.Format(_T("Cycle Seq Item : %s - Finish, %.3f sec"), strSeqName.GetString(), fElapsed_ms / 1000.f);
    m_pLogFileAccessor->KMLOG(OPER, strLog);

    CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME, NULL, NULL);
}

void CSeqTactTimeMgr::SetCycleIndex(int cycle_index)
{
	m_nCycleIndex = cycle_index;
}

CString CSeqTactTimeMgr::GetStartTime()
{
	return m_timecheck_cycle.GetSTtimeString();
}

CString CSeqTactTimeMgr::GetStartTime2()
{
	return m_strStartTime2;
}

CString CSeqTactTimeMgr::GetEndTime()
{
	return m_timecheck_cycle.GetEDtimeString();
}

float CSeqTactTimeMgr::GetElapsed()
{
	const float fElapsed_ms = m_timecheck_cycle.Elapsed_ms();
	return fElapsed_ms;
}

void CSeqTactTimeMgr::SetLogFile(CString strLog)
{
	m_pLogFileAccessor->KMLOG(OPER, strLog);
}

CLogFileAccessor* CSeqTactTimeMgr::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}