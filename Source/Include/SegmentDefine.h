#pragma once

// LYS 2026-09-29 Hybrid Glass segment header (docs/HybridGlass_Porting_Plan.md 5.2)
//   Kept outside preDefine.h on purpose: S_RecipeInfo/S_RecipeMgr layout (raw-pointer ABI to ALPHA) stays unchanged.
//   One segment = one S_RecipeMgr in C_In/OutRecipeFunc::m_vecSegments; this header carries only the per-segment names.
//   Hand-copied twin: CoreFunction/dll_Recipe/dfcRecipe/SegmentDefine.h == Source/Include/SegmentDefine.h (keep byte-identical).

#define M_SEGMENT_MAX			8		// segments per recipe
#define M_SEGMENT_STICK_MAX		50		// total sticks over all segments (nCellStatus[100][50])
#define M_SEG_NAME_LEN			64
#define M_SEG_EXT_LEN			256

struct S_SegmentHeader {
	wchar_t	m_chName[M_SEG_NAME_LEN];		// "Segment#1"...
	wchar_t	m_chPGName[M_SEG_EXT_LEN];		// external PG model name, blank = RecipeID
	wchar_t	m_chWADName[M_SEG_EXT_LEN];		// external WAD model name, blank = RecipeID

	S_SegmentHeader () {
		Clear ();
	}

	void Clear () {
		ZeroMemory (m_chName,    sizeof (m_chName));
		ZeroMemory (m_chPGName,  sizeof (m_chPGName));
		ZeroMemory (m_chWADName, sizeof (m_chWADName));
	}
};
