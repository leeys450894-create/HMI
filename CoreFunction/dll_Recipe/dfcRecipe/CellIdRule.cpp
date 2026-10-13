#include "pch.h"
#include "CellIdRule.h"
#include <map>
#include <climits>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B rule core (docs/HybridGlass_Porting_Plan.md 5.7, plan p4b (2)(3)).
//   Pure functions over a geometry snapshot (CellId_BuildGeo). No std::vector::at; every index is range-checked and a container is
//   never sized beyond the recipe geometry, so a corrupt trailer cannot drive an allocation.

// manual token, "" when out of shape (an Auto panel holds none)
static CString CellId_ManualAt (const S_CellIdConfig &cfg, int nPanel, int nSeg, int nStick, int nCell)
{
	if (nPanel < 0 || nPanel >= M_CELLID_PANEL_MAX) return CString ();
	const CCellIdGrid &grid = cfg.m_vecManual[nPanel];
	if (nSeg < 0 || nSeg >= (int)grid.size ()) return CString ();
	if (nStick < 0 || nStick >= (int)grid[nSeg].size ()) return CString ();
	if (nCell < 0 || nCell >= (int)grid[nSeg][nStick].size ()) return CString ();
	return grid[nSeg][nStick][nCell];
}

static BOOL CellId_InGeo (const CCellIdGeo &geo, int nSeg, int nStick, int nCell)
{
	if (nSeg < 0 || nSeg >= (int)geo.size ()) return FALSE;
	return (nStick >= 0 && nStick < geo[nSeg].m_nStick && nCell >= 0 && nCell < geo[nSeg].m_nCell) ? TRUE : FALSE;
}

static int CellId_NonNeg (int n) { return (n < 0) ? 0 : n; }

// [A-Z]+[0-9]+ ; *pnLetters = length of the letter run
static BOOL CellId_IsCanonical (const CString &str, int *pnLetters = NULL)
{
	int nLen = str.GetLength (), nLet = 0;
	while (nLet < nLen && str[nLet] >= _T('A') && str[nLet] <= _T('Z')) nLet++;
	if (nLet == 0 || nLet == nLen) return FALSE;
	for (int c = nLet; c < nLen; c++) {
		if (str[c] < _T('0') || str[c] > _T('9')) return FALSE;
	}
	if (pnLetters) *pnLetters = nLet;
	return TRUE;
}

static void CellId_AddError (std::vector<S_CellIdError> &vecErr, int nCode, int nSeg, int nStick, int nCell, int nPanel, const CString &strToken, int nLimit)
{
	S_CellIdError e;
	e.m_nCode = nCode; e.m_nSeg = nSeg; e.m_nStick = nStick; e.m_nCell = nCell; e.m_nPanel = nPanel;
	e.m_strToken = strToken; e.m_nLimit = nLimit;
	vecErr.push_back (e);
}

int		CellId_BOffset (const S_CellIdSegGeo &g)
{
	// GenCellID nMaxTop (DoWorkGlassMeasureStep.cpp): GLASS ROTATE 1 -> m_vecStick.size(), else m_vecStick[0].m_vecCell.size() (rot 3 too)
	return (g.m_nRotate == 1) ? g.m_nStick : g.m_nCell;
}

CString	CellId_AutoToken (const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel)
{
	if (!CellId_InGeo (geo, nSeg, nStick, nCell)) return CString ();
	const S_CellIdSegGeo &g = geo[nSeg];
	BOOL bHybrid = (geo.size () > 1) ? TRUE : FALSE;		// = ALPHA GetCellIdLetterBase() >= 0 (segment count > 1)
	int nBase = 0;											// RecipeEditMgr SyncCellStatusStickBase: SegCellIdLetterExtent sum in segment index order
	for (int q = 0; q < nSeg; q++) nBase += (geo[q].m_nRotate == 1) ? geo[q].m_nCell : geo[q].m_nStick;
	int nLetter = (g.m_nRotate == 1) ? nCell : g.m_nStick - nStick - 1;		// MakeCellID: rotate 1 -> letter = cell, else nMaxStick - iStick - 1
	int nNum	= (g.m_nRotate == 1) ? nStick + 1 : nCell + 1;					//             rotate 1 -> number = iStick + 1, else nCellNum + 1
	CString strTok;
	if (bHybrid) strTok.Format (_T("%s%02d"), (LPCTSTR)SegCellIdLetterPrefix (nBase + nLetter), nNum);	// Phase 4 letter carry lines
	else		 strTok.Format (_T("%c%02d"), 65 + nLetter, nNum);											// original lines (>= 26 letters -> '[' etc., as today)
	if (nPanel != 1) return strTok;

	CString strTmp = strTok + _T(" "), strBuf;		// GenCellID strTmp.Format("%s ", cCellID) then its B-panel branch
	int nMaxTop = CellId_BOffset (g);
	if (bHybrid) {
		int nPfx = 0;
		while (nPfx < strTmp.GetLength () && strTmp[nPfx] >= _T('A') && strTmp[nPfx] <= _T('Z')) nPfx++;
		strBuf.Format (_T("%s%02d"), (LPCTSTR)strTmp.Left (nPfx), nMaxTop + _ttoi (strTmp.Mid (nPfx)));
	}
	else strBuf.Format (_T("%s%02d"), (LPCTSTR)strTmp.Mid (0, 1), nMaxTop + _ttoi (strTmp.Mid (1, 2)));	// number >= 100 cut to 2 digits, as today
	return strBuf;
}

BOOL	CellId_DeriveB (const CString &strA, int nOffset, CString &strB)
{
	strB.Empty ();
	CString strCan = strA;
	strCan.Trim ();
	strCan.MakeUpper ();
	int nLet = 0;
	if (strCan != strA || !CellId_IsCanonical (strCan, &nLet)) return FALSE;
	CString strLet = strCan.Left (nLet), strDig = strCan.Mid (nLet);
	if (strDig.GetLength () > 9) return FALSE;
	__int64 nVal = _ttoi64 (strDig) + (__int64)nOffset;
	if (nVal < 0 || nVal > INT_MAX) return FALSE;
	strB.Format (_T("%s%0*d"), (LPCTSTR)strLet, strDig.GetLength (), (int)nVal);
	return TRUE;
}

int		CellId_Resolve (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel, CString &strTok, BOOL bAutoFallback)
{
	strTok.Empty ();
	if (nPanel < 0 || nPanel >= M_CELLID_PANEL_MAX || !CellId_InGeo (geo, nSeg, nStick, nCell)) return 0;
	if (cfg.m_nMode[nPanel] == M_CELLID_MODE_MANUAL) {
		strTok = CellId_ManualAt (cfg, nPanel, nSeg, nStick, nCell);
		return 1;
	}
	if (nPanel == 1 && cfg.m_nMode[0] == M_CELLID_MODE_MANUAL) {		// B Auto + A Manual -> B derived from the manual A (failure -> "")
		CString strB;
		if (CellId_DeriveB (CellId_ManualAt (cfg, 0, nSeg, nStick, nCell), CellId_BOffset (geo[nSeg]), strB)) strTok = strB;
		return 1;
	}
	if (bAutoFallback) strTok = CellId_AutoToken (geo, nSeg, nStick, nCell, nPanel);
	return 0;
}

void	CellId_BuildResolved (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nPanel, CCellIdGrid &out)
{
	out.clear ();
	out.resize (geo.size ());
	for (int k = 0; k < (int)geo.size (); k++) {
		int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
		out[k].assign (nStick, std::vector<CString> (nCell));
		for (int i = 0; i < nStick; i++)
			for (int j = 0; j < nCell; j++) CellId_Resolve (cfg, geo, k, i, j, nPanel, out[k][i][j], TRUE);
	}
}

void	CellId_BuildAuto (const CCellIdGeo &geo, int nPanel, CCellIdGrid &out)
{
	out.clear ();
	out.resize (geo.size ());
	for (int k = 0; k < (int)geo.size (); k++) {
		int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
		out[k].assign (nStick, std::vector<CString> (nCell));
		for (int i = 0; i < nStick; i++)
			for (int j = 0; j < nCell; j++) out[k][i][j] = CellId_AutoToken (geo, k, i, j, nPanel);
	}
}

int		CellId_TokenMax (int nGlassIdLen)
{
	if (nGlassIdLen < 0) return M_CELLID_TOKEN_MAX;
	// final ID = Glass ID without its last char + token + checksum char <= M_CELLID_ID_MAX
	int nMax = M_CELLID_ID_MAX - 1 - ((nGlassIdLen > 1) ? nGlassIdLen - 1 : 0);
	if (nMax > M_CELLID_TOKEN_MAX) nMax = M_CELLID_TOKEN_MAX;
	if (nMax < 0) nMax = 0;
	return nMax;
}

BOOL	CellId_Validate (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nTokenMax, std::vector<S_CellIdError> &vecErr, int *pnLongest)
{
	vecErr.clear ();
	if (pnLongest) *pnLongest = 0;
	if (cfg.IsDefault ()) return TRUE;		// Auto/Auto (every recipe saved before 4b): never checked -> no popup, no D-15 log

	int nSegCnt = (int)geo.size (), nLongest = 0;
	BOOL bManA = (cfg.m_nMode[0] == M_CELLID_MODE_MANUAL) ? TRUE : FALSE;
	BOOL bManB = (cfg.m_nMode[1] == M_CELLID_MODE_MANUAL) ? TRUE : FALSE;
	std::vector< std::vector<char> > vecBadA (nSegCnt);	// A cells that failed step 1 -> no derived-B report (ICP X7 double report fixed)
	for (int k = 0; k < nSegCnt; k++) vecBadA[k].assign ((size_t)CellId_NonNeg (geo[k].m_nStick) * (size_t)CellId_NonNeg (geo[k].m_nCell), 0);

	// 1. manual panels: format (panel A -> B, segment -> stick -> cell)
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
		if (cfg.m_nMode[p] != M_CELLID_MODE_MANUAL) continue;
		for (int k = 0; k < nSegCnt; k++) {
			int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
			for (int i = 0; i < nStick; i++) {
				for (int j = 0; j < nCell; j++) {
					CString strRaw = CellId_ManualAt (cfg, p, k, i, j);
					CString strCan = strRaw;
					strCan.Trim ();
					strCan.MakeUpper ();
					int nCode = 0;
					if (strCan.IsEmpty ())								nCode = M_CELLID_ERR_EMPTY;
					else if (strCan.GetLength () > nTokenMax)			nCode = M_CELLID_ERR_LENGTH;
					else if (!CellId_IsCanonical (strCan) || strCan != strRaw)	nCode = M_CELLID_ERR_FORMAT;
					if (nCode != 0) {
						CellId_AddError (vecErr, nCode, k, i, j, p, strRaw, (nCode == M_CELLID_ERR_LENGTH) ? nTokenMax : 0);
						if (p == 0) vecBadA[k][(size_t)i * (size_t)nCell + (size_t)j] = 1;
						continue;
					}
					if (strRaw.GetLength () > nLongest) nLongest = strRaw.GetLength ();
				}
			}
		}
	}

	// 2. B derived from the manual A (A Manual + B Auto)
	if (bManA && !bManB) {
		for (int k = 0; k < nSegCnt; k++) {
			int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
			for (int i = 0; i < nStick; i++) {
				for (int j = 0; j < nCell; j++) {
					if (vecBadA[k][(size_t)i * (size_t)nCell + (size_t)j]) continue;
					CString strB;
					CellId_Resolve (cfg, geo, k, i, j, 1, strB, FALSE);
					if (strB.IsEmpty ()) { CellId_AddError (vecErr, M_CELLID_ERR_DERIVE, k, i, j, 1, CellId_ManualAt (cfg, 0, k, i, j), 0); continue; }
					if (strB.GetLength () > nTokenMax) { CellId_AddError (vecErr, M_CELLID_ERR_LENGTH, k, i, j, 1, strB, nTokenMax); continue; }
					if (strB.GetLength () > nLongest) nLongest = strB.GetLength ();
				}
			}
		}
	}

	// 3. duplicates over panel A (all segments) then panel B (all segments), resolved values incl. Auto, empty skipped
	std::map<CString, S_CellIdError> mapSeen;
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
		for (int k = 0; k < nSegCnt; k++) {
			int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
			for (int i = 0; i < nStick; i++) {
				for (int j = 0; j < nCell; j++) {
					CString strTok;
					CellId_Resolve (cfg, geo, k, i, j, p, strTok, TRUE);
					if (strTok.IsEmpty ()) continue;
					std::map<CString, S_CellIdError>::const_iterator it = mapSeen.find (strTok);
					if (it != mapSeen.end ()) {
						S_CellIdError e;
						e.m_nCode = M_CELLID_ERR_DUP; e.m_nSeg = k; e.m_nStick = i; e.m_nCell = j; e.m_nPanel = p;
						e.m_nSeg2 = it->second.m_nSeg; e.m_nStick2 = it->second.m_nStick; e.m_nCell2 = it->second.m_nCell; e.m_nPanel2 = it->second.m_nPanel;
						e.m_strToken = strTok;
						vecErr.push_back (e);
					}
					else {
						S_CellIdError first;
						first.m_nSeg = k; first.m_nStick = i; first.m_nCell = j; first.m_nPanel = p;
						mapSeen.insert (std::make_pair (strTok, first));
					}
				}
			}
		}
	}

	if (pnLongest) *pnLongest = nLongest;
	return vecErr.empty () ? TRUE : FALSE;
}

void	CellId_EnsureShape (S_CellIdConfig &cfg, const CCellIdGeo &geo)
{
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
		if (cfg.m_nMode[p] != M_CELLID_MODE_MANUAL) {
			cfg.m_nMode[p] = M_CELLID_MODE_AUTO;
			cfg.m_vecManual[p].clear ();
			continue;
		}
		CCellIdGrid &grid = cfg.m_vecManual[p];
		grid.resize (geo.size ());
		for (int k = 0; k < (int)geo.size (); k++) {
			int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
			grid[k].resize (nStick);
			for (int i = 0; i < nStick; i++) grid[k][i].resize (nCell);
		}
	}
}

// trimmed line of decimal digits only (counts / indices in the trailer). FALSE = anything else
static BOOL CellId_ReadCount (CStdioFile *pFile, int &nVal)
{
	CString str = ReadString (pFile).Trim ();
	if (str.IsEmpty () || str.GetLength () > 9) return FALSE;
	for (int c = 0; c < str.GetLength (); c++) {
		if (str[c] < _T('0') || str[c] > _T('9')) return FALSE;
	}
	nVal = _ttoi (str);
	return TRUE;
}

// _CELL_ID_PANEL_ / modeA / modeB / nBlock (0 | nSeg) / { _CELL_ID_MANUAL_ k nStick nCell {A line, B line} x nStick } x nBlock / _CELL_ID_END_
//   Shape (nStick / nCell) is compared with the recipe geometry BEFORE anything is allocated. Manual panel tokens are kept as read
//   (no content check - editor / SaveFile / D-15 judge them); Auto panel lines are counted and dropped (invariant: Auto = empty).
//   std::exception -> FALSE; CException* (CFileException, CMemoryException) goes to LoadRcpFile's handler, which deletes it.
BOOL	CellId_LoadTail (CStdioFile *pFile, const CCellIdGeo &geo, S_CellIdConfig &cfg)
{
	try {
		if (ReadString (pFile).Trim () != _T("_CELL_ID_PANEL_")) return FALSE;
		S_CellIdConfig tmp;
		for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
			CString strMode = ReadString (pFile).Trim ();
			if (strMode == _T("0"))		 tmp.m_nMode[p] = M_CELLID_MODE_AUTO;
			else if (strMode == _T("1")) tmp.m_nMode[p] = M_CELLID_MODE_MANUAL;
			else return FALSE;
		}
		int nSegCnt = (int)geo.size (), nBlock = -1;
		if (!CellId_ReadCount (pFile, nBlock)) return FALSE;
		BOOL bAnyManual = (tmp.m_nMode[0] == M_CELLID_MODE_MANUAL || tmp.m_nMode[1] == M_CELLID_MODE_MANUAL) ? TRUE : FALSE;
		if (nBlock != 0 && nBlock != nSegCnt) return FALSE;
		if (bAnyManual != ((nBlock > 0) ? TRUE : FALSE)) return FALSE;

		for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
			if (tmp.m_nMode[p] == M_CELLID_MODE_MANUAL) tmp.m_vecManual[p].resize (nBlock);
		}
		CStringArray arrTok;
		for (int k = 0; k < nBlock; k++) {
			if (ReadString (pFile).Trim () != _T("_CELL_ID_MANUAL_")) return FALSE;
			int nIdx = -1, nStick = -1, nCell = -1;
			if (!CellId_ReadCount (pFile, nIdx) || nIdx != k) return FALSE;
			if (!CellId_ReadCount (pFile, nStick) || nStick != geo[k].m_nStick) return FALSE;
			if (!CellId_ReadCount (pFile, nCell) || nCell != geo[k].m_nCell) return FALSE;
			for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
				if (tmp.m_nMode[p] == M_CELLID_MODE_MANUAL) tmp.m_vecManual[p][k].assign (nStick, std::vector<CString> (nCell));
			}
			for (int i = 0; i < nStick; i++) {
				for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {		// stick i: A line, then B line
					dfcArryParsing (&arrTok, ReadString (pFile));		// comma-terminated tokens, each trimmed, empty tokens kept
					if (arrTok.GetSize () != (INT_PTR)nCell) return FALSE;
					if (tmp.m_nMode[p] != M_CELLID_MODE_MANUAL) continue;
					for (int j = 0; j < nCell; j++) tmp.m_vecManual[p][k][i][j] = arrTok.GetAt (j);
				}
			}
		}
		if (ReadString (pFile).Trim () != _T("_CELL_ID_END_")) return FALSE;
		cfg = tmp;
		return TRUE;
	}
	catch (std::exception &) {
		return FALSE;
	}
}

// Always written (N = 1 and Auto/Auto too: 5 lines). Lines / tokens follow the current geometry, so they always match the recipe body.
void	CellId_SaveTail (CStdioFile *pFile, const CCellIdGeo &geo, const S_CellIdConfig &cfg)
{
	BOOL bManual[M_CELLID_PANEL_MAX];
	for (int p = 0; p < M_CELLID_PANEL_MAX; p++) bManual[p] = (cfg.m_nMode[p] == M_CELLID_MODE_MANUAL) ? TRUE : FALSE;
	int nBlock = (bManual[0] || bManual[1]) ? (int)geo.size () : 0;		// = IsDefault () ? 0 : nSeg (modes are 0 / 1 only)

	WriteString (pFile, _T("_CELL_ID_PANEL_"));
	WriteInt	(pFile, bManual[0] ? M_CELLID_MODE_MANUAL : M_CELLID_MODE_AUTO);
	WriteInt	(pFile, bManual[1] ? M_CELLID_MODE_MANUAL : M_CELLID_MODE_AUTO);
	WriteInt	(pFile, nBlock);
	for (int k = 0; k < nBlock; k++) {
		int nStick = CellId_NonNeg (geo[k].m_nStick), nCell = CellId_NonNeg (geo[k].m_nCell);
		WriteString (pFile, _T("_CELL_ID_MANUAL_"));
		WriteInt	(pFile, k);
		WriteInt	(pFile, nStick);
		WriteInt	(pFile, nCell);
		for (int i = 0; i < nStick; i++) {
			for (int p = 0; p < M_CELLID_PANEL_MAX; p++) {
				CString strLine;
				for (int j = 0; j < nCell; j++) {
					if (bManual[p]) strLine += CellId_ManualAt (cfg, p, k, i, j);
					strLine += _T(",");
				}
				WriteString (pFile, strLine);
			}
		}
	}
	WriteString (pFile, _T("_CELL_ID_END_"));
}
