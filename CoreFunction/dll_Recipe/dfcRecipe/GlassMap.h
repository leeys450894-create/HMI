#pragma once

// LYS 2026-09-30 MakeCell cell overlay (IDC_STATIC_CELL_OVERLAY) - port of ICP C_GlassMap (ICPControl/GlassMap.*,
//   drawn in ICP MakeCellDlg IDC_STATIC_2). Every segment of the recipe is drawn on one physical glass.
//   Glass frame = the operate-screen overlay's frame (ALPHA DlgSubOperate::BuildHybridCellMap), so both screens agree:
//     stick s = m_fStageY + m_sMap.m_fContactOffset, cell c = m_fCellX
//     rotate 1 / 3 (long footprint)  -> centre (-c, -s), extent (CellSize.x, CellSize.y)
//     rotate 0 / 2 (short footprint) -> centre ( s, -c), extent (CellSize.y, CellSize.x)
//   x runs along the 1500 mm side (right), y along the 925 mm side drawn DOWNWARD - the operate chart's
//   SetMaxSize(..., 0, 1, 0, -1) - so a cell sits where the operate screen draws it.
//   Corner cut = IDC_STATIC_CELL_MEASURE's: ChartOrbit code 4 (fixed in DlgSubOperate::RotateGlassPath) = glass corner
//   (+750, -462.5), legs 5 % of each side - top right on screen; kept as a glass point so it turns with the canvas.
//   ICP canvas rule kept: the canvas follows the viewed segment's footprint - long (rotate 1 / 3) = landscape,
//   otherwise portrait, turned with ICP's Deg90 -> Deg0 transform (x, y) -> (-y, x).

#include <vector>

struct S_RecipeMgr;

struct S_GlassMapCell {
	double	dX, dY;			// centre in the glass frame (mm)
	double	dW, dH;			// extent along x / y (mm)
	TCHAR	szID[16];
	int		nStick, nCell;	// LYS 2026-10-12 Hybrid Phase 6 recipe vector indices (MakeCell overlay Auto carry IDs)
};

struct S_GlassMapSeg {
	int							nRotate;	// m_nGlsRotate
	std::vector<S_GlassMapCell>	vecCell;
	int							nStick, nCell0;	// LYS 2026-10-12 Hybrid Phase 6 sticks / cells of stick 0 (S_CellIdSegGeo inputs, as CellId_BuildGeo)
	S_GlassMapSeg () : nRotate (0), nStick (0), nCell0 (0) {}
};

class C_GlassMap : public CStatic
{
	DECLARE_DYNAMIC (C_GlassMap)

	public:
		C_GlassMap ();
		virtual ~C_GlassMap ();

		static void	BuildSeg (const S_RecipeMgr *pMgr, S_GlassMapSeg &sOut);		// recipe geometry -> glass frame (empty for NULL)
		void		UpdateGlassMap (const std::vector<S_GlassMapSeg> &vecSeg, int nViewSeg);

	protected:
		std::vector<S_GlassMapSeg>	m_vecSeg;
		int							m_nViewSeg;		// drawn on top in the stronger colour
		BOOL						m_bPortrait;	// ICP m_bRotate
		CRect						m_rcGlass;
		double						m_dScale;		// px per mm

		CPoint	GlassToWnd (double dX, double dY) const;

		afx_msg void	OnPaint ();
		afx_msg BOOL	OnEraseBkgnd (CDC *pDC);

	DECLARE_MESSAGE_MAP ()
};
