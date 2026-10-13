#pragma once
#include "SegmentDefine.h"

// LYS 2026-10-01 Hybrid Phase 4b Cell ID rules shared by C_InRecipeFunc (editor / save) and C_OutRecipeFunc (ALPHA runtime). Pure functions.
//   (docs/HybridGlass_Porting_Plan.md 5.7, D4b-11) DLL internal: nothing here is exported - ALPHA reaches the rules only through the
//   exported C_OutRecipeFunc methods, so there is one copy of the rule code. Include after pch.h (MFC CString / CStdioFile, Global.h IO).
//   CellId_AutoToken is a byte copy of ALPHA CDoWorkGlassMeasureStep::MakeCellID + GenCellID (Phase 4): change both together.
struct S_CellIdSegGeo { int m_nRotate; int m_nStick; int m_nCell; };	// GLASS ROTATE, sticks, cells of stick 0 (uniform by file format)
typedef std::vector<S_CellIdSegGeo> CCellIdGeo;
typedef std::vector< std::vector< std::vector<CString> > > CCellIdGrid;	// [seg][stick][cell]

template <class T> void CellId_BuildGeo (T &rcp, CCellIdGeo &geo)		// C_InRecipeFunc / C_OutRecipeFunc (same segment API)
{
	geo.clear ();
	for (int k = 0; k < rcp.GetSegmentLoopCount (); k++) {
		S_CellIdSegGeo g = { rcp.GetSegmentRotate (k), rcp.GetSegmentStickCount (k), rcp.GetSegmentCellCount (k) };
		geo.push_back (g);
	}
}
int		CellId_BOffset (const S_CellIdSegGeo &g);		// GenCellID nMaxTop: rotate 1 -> sticks, else cells of stick 0 (D4b-6)
CString	CellId_AutoToken (const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel);	// byte copy of MakeCellID + GenCellID ("" out of range)
BOOL	CellId_DeriveB (const CString &strA, int nOffset, CString &strB);		// A [A-Z]+[0-9]+ (digits <= 9) -> letters + "%0*d" (A width kept, D4b-5)
int		CellId_Resolve (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel, CString &strTok, BOOL bAutoFallback);
																		// 1 = manual / derived from A (final token), 0 = Auto (strTok = Auto token only with bAutoFallback) / out of range
void	CellId_BuildResolved (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nPanel, CCellIdGrid &out);	// every cell, Auto included
void	CellId_BuildAuto (const CCellIdGeo &geo, int nPanel, CCellIdGrid &out);
int		CellId_TokenMax (int nGlassIdLen);				// < 0 -> M_CELLID_TOKEN_MAX, else max(0, min(M_CELLID_TOKEN_MAX, 31 - 1 - max(len - 1, 0))) (GlassID >= 31 chars -> 0, every token refused)
BOOL	CellId_Validate (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nTokenMax, std::vector<S_CellIdError> &vecErr, int *pnLongest);
																		// TRUE = OK (Auto/Auto always, no check). Errors: manual format A -> B, derived B, A+B duplicates
void	CellId_EnsureShape (S_CellIdConfig &cfg, const CCellIdGeo &geo);	// Manual -> [seg][stick][cell] (keeps values), Auto -> empty
BOOL	CellId_LoadTail (CStdioFile *pFile, const CCellIdGeo &geo, S_CellIdConfig &cfg);	// FALSE = bad trailer (cfg untouched); CException* passes through
void	CellId_SaveTail (CStdioFile *pFile, const CCellIdGeo &geo, const S_CellIdConfig &cfg);
