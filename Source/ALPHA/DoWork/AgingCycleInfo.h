// AgingCycleInfo.h
// LYS 2026-08-06 AgingCycle E2: 에이징 사이클 레시피의 비UI 로더
// ({DIR\AGING_RCP}\{RecipeID}.rcp - CDlgSubManual_AgingTest가 기록).
// 측정 시퀀스 엔진이 사용한다(OCP DoWorkStartMeasOCP_A 대응).
// 엔진은 다이얼로그를 건드리면 안 된다(OCP는 m_vecAginList를 직접 읽었으나 HMI는 금지).
// 단일 스레드 사용: 시퀀스 엔진이 인스턴스 하나를 소유하고 실행마다 로드한다.
#pragma once

#include <vector>
#include "common\CommonFunc.h"		// S_AgingTest, dfcStr

#define DEFINE_AGING_CYCLE_STICK_MAX	100		// STICK_n scan upper bound (safety). LYS 2026-09-29 Hybrid 3b moved here from .cpp (AgingTest dialog stale-key cleanup)

// cell-use mask bit index ([MASECELL] bit0..2)
// LYS 2026-08-06: HMI has no COLOR mode - kinds are WAD/VISION/ANGLE only
// (OCP 4 kinds WAD/VISION/Color/LCR -> HMI 3 kinds, per user 2026-08-06)
enum enumAgingMeasKind
{
	enumAgingMeasKind_WAD = 0,
	enumAgingMeasKind_VISION,
	enumAgingMeasKind_ANGLE,
	enumAgingMeasKind_MAX,
};

class CAgingCycleInfo
{
public:
	CAgingCycleInfo();
	~CAgingCycleInfo();

	static CString	GetAgingRcpPath();
	static CString	GetEfaRcpPath();		// LYS 2026-09-28 EFA: 레지스트리 DIR\EFA_RCP (EFA 2-pass rcp 폴더)

	// Loads item list + [MASECELL] cell map for the recipe. FALSE if file/list missing.
	bool	Load(CString strRcpName, bool bEfa = false, int nSegCount = 1);	// LYS 2026-09-28 EFA: bEfa면 {EFA_RCP}\{rcp}.rcp를 읽는다
	void	Clear();

	// LYS 2026-09-29 Hybrid 3b A1 cell mask per segment: seg 0 = [MASECELL] (unchanged), seg k>=1 = [MASECELL_SEG{k+1}].
	//   Load(.., nSegCount) reads all sections at once. A missing/empty section = no rows = measure all (same as a file without [MASECELL]).
	static CString	GetMaskSection(int nSeg);

	int		GetItemCount() const					{ return (int)m_vecItem.size(); }
	const S_AgingTest*	GetItem(int iItem) const;

	// LYS 2026-08-12 AgingCycle R1: 해당 종류를 쓰는 활성 항목 수.
	// 측정 종류별로 예약할 결과 행 블록 크기를 정하는 데 쓴다.
	int		GetKindUseCount(int iKind) const;

	// LYS 2026-09-29 Hybrid 3b A1 nSeg = segment index (0 for 1-segment recipes), no default on purpose
	int		GetStickCount(int nSeg) const;		// 0 = no mask rows for the segment = measure all
	int		GetCellCount(int nSeg, int iStick) const;

	// kind = enumAgingMeasKind_*. Out-of-range -> false.
	bool	IsCellUse(int nSeg, int iStick, int iCell, int iKind) const;
	bool	IsStickUse(int nSeg, int iStick, int iKind) const;	// any cell of the stick

private:
	std::vector<S_AgingTest>		m_vecItem;
	std::vector<std::vector<std::vector<int>>>	m_vecSegMask;	// LYS 2026-09-29 Hybrid 3b A1 [seg][stick][cell] = 3bit mask
	void	LoadMaskSection(LPCTSTR szSection, LPCTSTR szPath, std::vector<std::vector<int>>& vecOut);
};
