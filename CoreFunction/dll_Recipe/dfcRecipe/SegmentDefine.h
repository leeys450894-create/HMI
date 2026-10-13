#pragma once
#include <vector>	// LYS 2026-10-01 Hybrid Phase 4b S_CellIdConfig

// LYS 2026-09-29 Hybrid Glass segment header (docs/HybridGlass_Porting_Plan.md 5.2)
//   Kept outside preDefine.h on purpose: S_RecipeInfo/S_RecipeMgr layout (raw-pointer ABI to ALPHA) stays unchanged.
//   One segment = one S_RecipeMgr in C_In/OutRecipeFunc::m_vecSegments; this header carries only the per-segment names.
//   Hand-copied twin: CoreFunction/dll_Recipe/dfcRecipe/SegmentDefine.h == Source/Include/SegmentDefine.h (keep byte-identical).

#define M_SEGMENT_MAX			8		// segments per recipe
#define M_SEGMENT_STICK_MAX		50		// total sticks over all segments (nCellStatus[100][50])
#define M_RESULT_AREA_MAX		2500	// LYS 2026-09-29 Hybrid 3b result rows per glass = dResult APD AREA_MAX_CNT
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

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B Auto/Manual (docs/HybridGlass_Porting_Plan.md 5.7, D-5)
//   Recipe-global modes + per-cell manual IDs, kept out of S_CellInfo (preDefine.h raw-pointer ABI to ALPHA).
//   Invariant: an Auto panel has no manual IDs (empty vector); a Manual panel holds [seg][stick][cell] of the recipe geometry.
#define M_CELLID_PANEL_MAX		2		// 0 = A, 1 = B
#define M_CELLID_MODE_AUTO		0
#define M_CELLID_MODE_MANUAL	1
#define M_CELLID_TOKEN_MAX		20		// manual / derived token chars at save: GlassID 11 - 1 + token 20 + checksum 1 = 31
#define M_CELLID_ID_MAX			31		// final ID chars: stLotInfo.m_szCellID[..][32], WAD MCRID[2][32] (_tcscpy_s aborts on 32+)
#define M_CELLID_ERR_EMPTY		1
#define M_CELLID_ERR_LENGTH		2
#define M_CELLID_ERR_FORMAT		3
#define M_CELLID_ERR_DERIVE		4
#define M_CELLID_ERR_DUP		5

struct S_CellIdConfig {
	int	m_nMode[M_CELLID_PANEL_MAX];												// M_CELLID_MODE_*
	std::vector< std::vector< std::vector<CString> > > m_vecManual[M_CELLID_PANEL_MAX];	// [panel][seg][stick][cell]
	S_CellIdConfig () { Clear (); }
	void Clear () { for (int p = 0; p < M_CELLID_PANEL_MAX; p++) { m_nMode[p] = M_CELLID_MODE_AUTO; m_vecManual[p].clear (); } }
	BOOL IsDefault () const { return m_nMode[0] == M_CELLID_MODE_AUTO && m_nMode[1] == M_CELLID_MODE_AUTO; }
};

struct S_CellIdError {
	int		m_nCode;									// M_CELLID_ERR_*
	int		m_nSeg, m_nStick, m_nCell, m_nPanel;		// 0-based location
	int		m_nSeg2, m_nStick2, m_nCell2, m_nPanel2;	// M_CELLID_ERR_DUP: first location
	int		m_nLimit;									// M_CELLID_ERR_LENGTH: allowed chars
	CString	m_strToken;									// offending text (DERIVE: the A source)
	S_CellIdError () : m_nCode (0), m_nSeg (-1), m_nStick (-1), m_nCell (-1), m_nPanel (0),
					   m_nSeg2 (-1), m_nStick2 (-1), m_nCell2 (-1), m_nPanel2 (0), m_nLimit (0) {}
};

// LYS 2026-10-01 Hybrid Phase 4b bijective base-26 (0 A .. 25 Z, 26 AA, 702 AAA; negative -> "@") - moved here from
//   CRecipeEditMgr::CellIdLetterPrefix (Phase 4) so the DLL editor and ALPHA use one copy (D4b-11)
inline CString SegCellIdLetterPrefix (int nIndex)
{
	CString strPrefix;
	if (nIndex < 0) return CString (_T("@"));
	do { strPrefix.Insert (0, (TCHAR)(_T('A') + (nIndex % 26))); nIndex = nIndex / 26 - 1; } while (nIndex >= 0);
	return strPrefix;
}
