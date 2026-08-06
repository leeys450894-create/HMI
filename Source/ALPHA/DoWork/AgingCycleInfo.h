// AgingCycleInfo.h
// LYS 2026-08-06 AgingCycle E2: non-UI loader for the aging cycle recipe
// ({DIR\AGING_RCP}\{RecipeID}.rcp - written by CDlgSubManual_AgingTest).
// Used by the measure sequence engine (OCP DoWorkStartMeasOCP_A counterpart);
// the engine must NOT touch the dialog (OCP read m_vecAginList directly - HMI forbids).
// Single-thread use: one instance owned by the sequence engine, loaded per run.
#pragma once

#include <vector>
#include "common\CommonFunc.h"		// S_AgingTest, dfcStr

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

	// Loads item list + [MASECELL] cell map for the recipe. FALSE if file/list missing.
	bool	Load(CString strRcpName);
	void	Clear();

	int		GetItemCount() const					{ return (int)m_vecItem.size(); }
	const S_AgingTest*	GetItem(int iItem) const;

	int		GetStickCount() const					{ return (int)m_vecCellMask.size(); }
	int		GetCellCount(int iStick) const;

	// kind = enumAgingMeasKind_*. Out-of-range -> false.
	bool	IsCellUse(int iStick, int iCell, int iKind) const;
	bool	IsStickUse(int iStick, int iKind) const;	// any cell of the stick

private:
	std::vector<S_AgingTest>		m_vecItem;
	std::vector<std::vector<int>>	m_vecCellMask;		// [stick][cell] = 4bit mask
};
