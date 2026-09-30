#pragma once

// LYS 2026-07-23 WAD 다점 측정 데이터 관리자(Phase 2). OCP 데이터 계층 이식 + 결함 교정:
//  - 고정 배열 대신 std::vector (OCP는 S_MultiMeas aData[20] 오버플로)
//  - INI 버퍼 4096자 (OCP는 MAX_PATH라 26점 부근에서 잘림)
//  - 셀 사용 맵을 여기 보관해 레시피 구조체 무침습 (OCP는 m_iUsedMeas += 16 손상)
//  - UI 스레드와 시퀀스 스레드 동시 접근에 CCriticalSection
//
// 파일 포맷은 OCP multi recipe INI와 100% 호환:
//   {RcpFolder}\{RecipeID}.rcp
//   [DATA]     CLASS,SUB_CLASS,X,Y,ENABLE,RX,GX,BY,WX,WY (필드별 콤마 병합 리스트)
//              INSP_MODE (0=Single/1=Multi), CELL_X, CELL_Y
//   [MASECELL] STICK_n = 셀별 0/1 콤마 리스트
//
// LYS 2026-09-30 Hybrid 3d 하이브리드 레시피: 세그먼트 k(>=1)는 [DATA_SEG{k+1}] / [MASECELL_SEG{k+1}] (같은 13키 / STICK_n).
//   세그먼트 0은 [DATA]/[MASECELL] 그대로(OCP 호환). 섹션이 없으면 그 세그먼트는 base 점만 측정한다.
//   평면 멤버 = 마지막으로 적재한 세그먼트 한 벌 (측정 스텝이 WAD 스텝마다 활성 세그먼트를 다시 적재).

#include <vector>
#include <afxmt.h>

struct S_WadMultiPoint
{
	CString	strMainClass;
	CString	strSubClass;

	double	dPosX;			// cell-relative X (mm) -> head Y axis offset
	double	dPosY;			// cell-relative Y (mm) -> gantry axis offset (sign inverted)

	bool	bEnable;

	double	fOffsetRX;		// R_X color coordinate offset (applied to result)
	double	fOffsetGX;		// G_X color coordinate offset (applied to result)
	double	fOffsetBY;		// B_Y color coordinate offset (applied to result)
	double	fOffsetWX;		// reserved (not applied, kept for OCP file compat)
	double	fOffsetWY;		// reserved (not applied, kept for OCP file compat)

	S_WadMultiPoint() { Clear(); }

	void Clear()
	{
		strMainClass.Empty();
		strSubClass.Empty();
		dPosX = 0.0;
		dPosY = 0.0;
		bEnable = false;
		fOffsetRX = fOffsetGX = fOffsetBY = fOffsetWX = fOffsetWY = 0.0;
	}
};

class CWadMultiMeasMgr
{
public:
	CWadMultiMeasMgr(CString strRcpFolderPath);
	~CWadMultiMeasMgr();

	//---------------------------------------------------------------
	// File IO (INI, OCP compatible)
	//---------------------------------------------------------------
	bool	LoadMeasData(CString strRecipeID);		// load [DATA] + [MASECELL]; false if file missing (= LoadMeasSegment(rcp, 0))
	bool	SaveMeasData(CString strRecipeID);		// save all from memory (= SaveMeasSegment(rcp, 0, false))

	// LYS 2026-09-30 Hybrid 3d one segment of a hybrid recipe <-> flat members
	static CString	GetDataSection(int nSeg);		// nSeg <= 0 -> "DATA",     else "DATA_SEG%d"     (nSeg + 1)
	static CString	GetMaskSection(int nSeg);		// nSeg <= 0 -> "MASECELL", else "MASECELL_SEG%d" (nSeg + 1)
	bool	LoadMeasSegment(CString strRecipeID, int nSeg);	// false only if the file is missing
	bool	SaveMeasSegment(CString strRecipeID, int nSeg, bool bDeleteStaleRows);	// true: delete STICK_n keys past the saved rows
	int		GetLoadedSegment();
	bool	IsDataFound();							// loaded segment's [DATA*] parsed >= 1 point
	bool	IsMaskFound();							// loaded segment's [MASECELL*] has >= 1 row

	void	Clear();

	//---------------------------------------------------------------
	// Path
	//---------------------------------------------------------------
	void	SetRcpFolderPath(CString strPath);
	CString	GetRcpFolderPath();
	CString	GetRcpFilePath(CString strRecipeID);

	//---------------------------------------------------------------
	// Getters (sequence side)
	//---------------------------------------------------------------
	bool	IsLoaded();
	CString	GetLoadedRecipeID();
	bool	IsMultiMode();							// INSP_MODE == 1
	int		GetPointCount();						// all points (incl. disabled)
	bool	GetPoint(int nIndex, S_WadMultiPoint& outPoint);
	int		GetUseSize();							// enabled point count (OCP GetUseSize)
	int		GetCheckNumData(int nCheckNum);			// index of nCheckNum-th enabled point (OCP GetCheckNumData)
	double	GetCellX();
	double	GetCellY();
	bool	IsMultiCell(int iStick, int iCell);		// [MASECELL] flag (out of range = false)
	int		GetCellUseStickCount();
	int		GetCellUseCellCount(int iStick);

	//---------------------------------------------------------------
	// runtime state (Phase 4) - written by measure sequence,
	// read by WAD protocol (Phase 5) and result offset (Phase 6)
	//---------------------------------------------------------------
	void	ResetRunState();
	void	SetRunActive(int iProbe, bool bActive);
	bool	IsRunActive(int iProbe);
	void	SetRunCurrent(int iProbe, int nCurrent);	// 1 = base point, 2.. = multi points
	int		GetRunCurrent(int iProbe);
	// LYS 2026-08-18 실행 중인 포인트의 명시적 인덱스(0 = base point).
	// GetCheckNumData()는 ENABLED 서수를 인덱스로 바꾸는데, base point가 비활성이면
	// 시퀀스 루프 인덱스와 어긋난다. 루프가 실제로 쓴 인덱스를 들고 다녀 모호함을 없앤다.
	void	SetRunPointIdx(int iProbe, int nIdx);
	int		GetRunPointIdx(int iProbe);

	//---------------------------------------------------------------
	// Setters (UI side, Phase 3)
	//---------------------------------------------------------------
	void	SetInspMode(bool bMulti);
	void	SetCellSize(double dCellX, double dCellY);
	void	SetPoints(const std::vector<S_WadMultiPoint>& vecPoints);
	void	GetPoints(std::vector<S_WadMultiPoint>& outPoints);
	void	SetCellUseMap(const std::vector<std::vector<int>>& vecCellUse);
	void	GetCellUseMap(std::vector<std::vector<int>>& outCellUse);

	// 검증 - 활성 점은 모두 +-CellX/2, +-CellY/2 안에 있어야 한다.
	// 유효하면 true 반환, outNgIndex = 첫 위반 점 인덱스(유효 시 -1).
	bool	IsCellRangeValid(int& outNgIndex);

private:
	// helpers (no lock inside; callers hold m_cs)
	CString	BuildJoinedList(int nField);
	void	ParseJoinedList(int nField, LPCTSTR szData);

	CCriticalSection				m_cs;

	CString							m_strRcpFolderPath;
	CString							m_strLoadedRecipeID;
	bool							m_bLoaded;
	int								m_nLoadedSeg;	// LYS 2026-09-30 Hybrid 3d segment held by the flat members
	bool							m_bDataFound;	// LYS 2026-09-30 Hybrid 3d
	bool							m_bMaskFound;	// LYS 2026-09-30 Hybrid 3d

	bool							m_bInspMode;	// INSP_MODE (0=Single, 1=Multi)
	double							m_dCellX;
	double							m_dCellY;

	std::vector<S_WadMultiPoint>	m_vecPoint;
	std::vector<std::vector<int>>	m_vecCellUse;	// [stick][cell] 0/1

	// runtime state (max 2 heads)
	bool							m_bRunActive[2];
	int								m_nRunCurrent[2];
	int								m_nRunPointIdx[2];	// LYS 2026-08-18 point index in m_vecPoint
};
