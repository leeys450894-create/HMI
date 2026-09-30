#include "pch.h"
#include "GlassMap.h"

// LYS 2026-09-30 port of ICP C_GlassMap - see GlassMap.h for the glass frame. Colours are ICP's.
#define GM_CLR_BACK			RGB (85, 85, 85)
#define GM_CLR_GLASS		RGB (230, 210, 190)
#define GM_CLR_CELL_VIEW	RGB (255, 153, 0)		// ICP BLOCK_PROC   - viewed segment
#define GM_CLR_CELL_OTHER	RGB (255, 204, 0)		// ICP BLOCK_UNPROC - other segments
#define GM_CLR_CELL_LINE	RGB (255, 255, 255)
#define GM_CLR_TEXT			RGB (128, 128, 128)
#define GM_MIN_FONT_PX		7						// smaller cell ID text is not drawn (ICP drew it at any size)

IMPLEMENT_DYNAMIC (C_GlassMap, CStatic)
BEGIN_MESSAGE_MAP (C_GlassMap, CStatic)
	ON_WM_PAINT ()
	ON_WM_ERASEBKGND ()
END_MESSAGE_MAP ()

C_GlassMap::C_GlassMap ()
{
	m_nViewSeg	= 0;
	m_bPortrait	= FALSE;
	m_dScale	= 0.0;
	m_rcGlass.SetRectEmpty ();
}

C_GlassMap::~C_GlassMap ()
{
}

void	C_GlassMap::BuildSeg (const S_RecipeMgr *pMgr, S_GlassMapSeg &sOut)
{
	sOut.nRotate = 0;
	sOut.vecCell.clear ();
	if (pMgr == NULL) return;

	int	   nRot		= pMgr->m_sInfo.m_sOptionItem.m_nGlsRotate;
	// footprint, not MakeCellID's "only rotate 1 is direct": rotate 1 / 3 lay cells along the 1500 mm side (CheckRecipeLayout,
	// MakeCell, SelectCellDlg). 270 shares the 90 geometry the way 180 shares 0 (MakeCell basis) - unverified on a 270 run.
	BOOL   bLong	= (nRot % 2 == 1);
	double dContact	= pMgr->m_sMap.m_fContactOffset;
	double dW		= fabs (bLong ? pMgr->m_sMap.m_fCellSize.x : pMgr->m_sMap.m_fCellSize.y);
	double dH		= fabs (bLong ? pMgr->m_sMap.m_fCellSize.y : pMgr->m_sMap.m_fCellSize.x);
	sOut.nRotate = nRot;

	for (size_t i = 0; i < pMgr->m_sData.m_vecStick.size (); i++) {
		const S_StickInfo &sStick = pMgr->m_sData.m_vecStick[i];
		double dS = sStick.m_fStageY + dContact;

		for (size_t j = 0; j < sStick.m_vecCell.size (); j++) {
			const S_CellInfo &sCell = sStick.m_vecCell[j];

			S_GlassMapCell sMap;
			sMap.dX = bLong ? -sCell.m_fCellX : dS;
			sMap.dY = bLong ? -dS : -sCell.m_fCellX;
			sMap.dW = dW;
			sMap.dH = dH;
			_tcsncpy_s (sMap.szID, _countof (sMap.szID), sCell.m_chCellID, _TRUNCATE);
			sOut.vecCell.push_back (sMap);
		}
	}
}

void	C_GlassMap::UpdateGlassMap (const std::vector<S_GlassMapSeg> &vecSeg, int nViewSeg)
{
	m_vecSeg   = vecSeg;
	m_nViewSeg = nViewSeg;

	// ICP UpdateCanvasOrientation: the glass is shown as mounted while the viewed segment runs.
	// A viewed slot without cells (new segment before OPEN) keeps the landscape canvas.
	m_bPortrait = (nViewSeg >= 0 && nViewSeg < (int)m_vecSeg.size ()
				   && !m_vecSeg[nViewSeg].vecCell.empty () && (m_vecSeg[nViewSeg].nRotate % 2 == 0));	// ICP keys on footprint too

	if (GetSafeHwnd ()) Invalidate (FALSE);
}

CPoint	C_GlassMap::GlassToWnd (double dX, double dY) const
{
	double dU = dX, dV = dY;
	if (m_bPortrait) {		// ICP ApplyOrientationTransform Deg90 -> Deg0
		dU = -dY;
		dV = dX;
	}

	double dCX = (m_rcGlass.left + m_rcGlass.right) / 2.0;
	double dCY = (m_rcGlass.top + m_rcGlass.bottom) / 2.0;
	return CPoint ((int)floor (dCX + dU * m_dScale + 0.5), (int)floor (dCY + dV * m_dScale + 0.5));	// +y drawn downward (operate chart)
}

BOOL	C_GlassMap::OnEraseBkgnd (CDC *pDC)
{
	return TRUE;	// OnPaint fills everything (double buffered)
}

void	C_GlassMap::OnPaint ()
{
	CPaintDC dc (this);

	CRect rcClient;
	GetClientRect (&rcClient);
	if (rcClient.Width () <= 0 || rcClient.Height () <= 0) return;

	CDC memDC;
	memDC.CreateCompatibleDC (&dc);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap (&dc, rcClient.Width (), rcClient.Height ());
	CBitmap *pOldBmp = memDC.SelectObject (&bmp);

	memDC.FillSolidRect (rcClient, GM_CLR_BACK);

	// glass, aspect kept (ICP deflated by a fixed 89/5 px tuned to its own control)
	double dGlsW = m_bPortrait ? RCP_GLASS_SIZE_Y : RCP_GLASS_SIZE_X;
	double dGlsH = m_bPortrait ? RCP_GLASS_SIZE_X : RCP_GLASS_SIZE_Y;
	CRect  rcArea = rcClient;
	rcArea.DeflateRect (5, 5);
	double dSX = (double)rcArea.Width () / dGlsW;
	double dSY = (double)rcArea.Height () / dGlsH;
	m_dScale = (dSX < dSY) ? dSX : dSY;
	if (m_dScale <= 0.0) m_dScale = 0.01;

	int nGW = (int)(dGlsW * m_dScale);
	int nGH = (int)(dGlsH * m_dScale);
	m_rcGlass.left	 = rcArea.left + (rcArea.Width () - nGW) / 2;
	m_rcGlass.top	 = rcArea.top + (rcArea.Height () - nGH) / 2;
	m_rcGlass.right	 = m_rcGlass.left + nGW;
	m_rcGlass.bottom = m_rcGlass.top + nGH;
	memDC.FillSolidRect (m_rcGlass, GM_CLR_GLASS);

	// corner cut = IDC_STATIC_CELL_MEASURE: DlgSubOperate::RotateGlassPath always passes code 4 to ChartOrbit::SetGlassSize,
	// i.e. corner (XMax, YMin) with legs of 5 % of the width / height, and that chart reverses Y, so it shows top right.
	// Mapped like the cells, so the portrait canvas shows the same glass corner.
	{
		double dHX = RCP_GLASS_SIZE_X / 2.0, dHY = RCP_GLASS_SIZE_Y / 2.0;
		CPoint pts[3];
		pts[0] = GlassToWnd (dHX, -dHY);
		pts[1] = GlassToWnd (dHX - RCP_GLASS_SIZE_X * 0.05, -dHY);
		pts[2] = GlassToWnd (dHX, -dHY + RCP_GLASS_SIZE_Y * 0.05);
		CBrush brCut (GM_CLR_BACK);
		CPen   penCut (PS_SOLID, 1, GM_CLR_BACK);
		CBrush *pOldBr  = memDC.SelectObject (&brCut);
		CPen   *pOldPen = memDC.SelectObject (&penCut);
		memDC.Polygon (pts, 3);
		memDC.SelectObject (pOldBr);
		memDC.SelectObject (pOldPen);
	}

	// cells: other segments first, the viewed segment last so it stays on top where segments touch
	CPen   penView (PS_SOLID, 2, GM_CLR_CELL_LINE), penOther (PS_SOLID, 1, GM_CLR_CELL_LINE);
	CBrush brView (GM_CLR_CELL_VIEW), brOther (GM_CLR_CELL_OTHER);
	CBrush *pOldBr	= memDC.SelectObject (&brOther);
	CPen   *pOldPen	= memDC.SelectObject (&penOther);
	CFont  *pOldFont = NULL;
	memDC.SetBkMode (TRANSPARENT);
	memDC.SetTextColor (GM_CLR_TEXT);

	for (int nPass = 0; nPass < 2; nPass++) {
		for (int k = 0; k < (int)m_vecSeg.size (); k++) {
			BOOL bView = (k == m_nViewSeg);
			if ((nPass == 1) != bView) continue;

			const std::vector<S_GlassMapCell> &vecCell = m_vecSeg[k].vecCell;
			if (vecCell.empty ()) continue;

			memDC.SelectObject (bView ? &brView : &brOther);
			memDC.SelectObject (bView ? &penView : &penOther);

			// one font per segment - cells of a segment share one size (ICP: height = cell width / 4)
			CFont font;
			BOOL  bText = FALSE;
			{
				const S_GlassMapCell &c0 = vecCell[0];
				CPoint p1 = GlassToWnd (c0.dX - c0.dW / 2.0, c0.dY - c0.dH / 2.0);
				CPoint p2 = GlassToWnd (c0.dX + c0.dW / 2.0, c0.dY + c0.dH / 2.0);
				int nCW = abs (p2.x - p1.x), nCH = abs (p2.y - p1.y);
				int nFont = nCW / 4;
				if (nFont > nCH / 2) nFont = nCH / 2;
				if (nFont >= GM_MIN_FONT_PX) {
					font.CreateFont (-nFont, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
									 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
					CFont *pPrev = memDC.SelectObject (&font);
					if (pOldFont == NULL) pOldFont = pPrev;
					bText = TRUE;
				}
			}

			for (size_t i = 0; i < vecCell.size (); i++) {
				const S_GlassMapCell &c = vecCell[i];
				CPoint p1 = GlassToWnd (c.dX - c.dW / 2.0, c.dY - c.dH / 2.0);
				CPoint p2 = GlassToWnd (c.dX + c.dW / 2.0, c.dY + c.dH / 2.0);
				CRect rc (min (p1.x, p2.x), min (p1.y, p2.y), max (p1.x, p2.x), max (p1.y, p2.y));
				if (rc.Width () < 2)  rc.right  = rc.left + 2;
				if (rc.Height () < 2) rc.bottom = rc.top + 2;

				memDC.Rectangle (rc);
				if (bText && c.szID[0] != 0) memDC.DrawText (c.szID, -1, rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			}

			if (bText && pOldFont != NULL) memDC.SelectObject (pOldFont);	// release this segment's font before it is destroyed
		}
	}

	memDC.SelectObject (pOldBr);
	memDC.SelectObject (pOldPen);

	dc.BitBlt (0, 0, rcClient.Width (), rcClient.Height (), &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject (pOldBmp);
}
