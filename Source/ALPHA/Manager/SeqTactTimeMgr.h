#pragma once

#include "..\AuroraSubMgr.h"
#include "common\TimeChecker.h"

// Step MeasInfo
struct STactTimeInfo
{
	int cycle_index;
	int Dowork_index;
	int sitem_index;
	CString strName;
	CString	fStart;
	CString	fEnd;
	float	fElapsed;
	CTime lStart;
	CTime lEnd;
	float fmsec;


	STactTimeInfo()
	{
		cycle_index = -1;
		Dowork_index = -1;
		sitem_index = -1;
		fElapsed = 0.f;

		lStart = 0;
		lEnd = 0;
		fmsec = 0.f;
	}
};

// Glass MeasInfo
struct STactTimeMeasInfo
{
	int sitem_index;
	CString strName;
	CString	fStart;
	CString	fEnd;
	float	fElapsed;
	CString	strGlassID;	// jsyoon 20211014 add GlassID
	CString	strRecipeID; // jsyoon 20211014 add RecipeID

	STactTimeMeasInfo()
	{
		sitem_index = -1;
		fElapsed = 0.f;
	}
};

class CLogFileAccessor;
class CSeqTactTimeMgr : public CAuroraSubMgr
{
	CString			m_strLogPath;
	CLogFileAccessor	*m_pLogFileAccessor;
	std::vector<STactTimeInfo>	m_vecData;
	std::vector<STactTimeMeasInfo>	m_vecData2;
	CTimeChecker	m_timecheck_total;
	CTimeChecker	m_timecheck_cycle;
	CTimeChecker	m_timecheck_sitem;
	int				m_nCycleIndex;
	CString			m_strGlassID;
	CString			m_strStartTime;
	CString			m_strStartTime2;
	float			m_fCycleTime_ms;

public:
	void SetDataClear();
	long GetDataCount();
	void PushData(STactTimeInfo data);
	STactTimeInfo GetData(int index);
	void SetDataClear2();
	long GetDataCount2();
	void PushData2(STactTimeMeasInfo data);
	STactTimeMeasInfo GetData2(int index);
	CString GetStartTime();
	CString GetStartTime2();
	CString GetEndTime();
	float   GetElapsed();

	void SetTotalSeqTimeST();
	void SetTotalSeqTimeED();
	void SetCycleTimeST(int cycle_index, CString strGlassID, CString strReciepID);
	void SetCycleTimeED(int cycle_index);
	void SetSeqItemTimeST(int cycle_index, CString strSeqName, int sitem_index);
	void SetSeqItemTimeED(int cycle_index, CString strSeqName, int sitem_index);
	void SetCycleIndex(int cycle_index);
	void SetLogFile(CString strLog);

	CLogFileAccessor *GetLogFileAccessorPtr();

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
private:
	virtual void Finalize() override;
public:
	CSeqTactTimeMgr(CString strLogPath);
	virtual ~CSeqTactTimeMgr();
protected:
	unsigned int m_nMeasCount;
};

