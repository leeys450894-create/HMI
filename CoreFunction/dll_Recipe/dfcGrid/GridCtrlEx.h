#pragma once

#include "dfcGrid.h"

class AFX_EXT_CLASS C_GridCtrl : public CGridCtrl
{
	DECLARE_DYNAMIC (C_GridCtrl)
	public:
		C_GridCtrl ();
		virtual ~C_GridCtrl ();

	protected:
		CRect	m_rect;
		CImageList	m_imageList;

	public:
				
		virtual BOOL Create (const RECT& rect, CWnd* parent, UINT nID, DWORD dwStyle = WS_CHILD | WS_BORDER | WS_TABSTOP | WS_VISIBLE, BOOL bRowResize = FALSE, BOOL bColResize = FALSE);
		void	SetAttribute (BOOL bEditable = FALSE, BOOL bColEditable = FALSE, BOOL bRowEditable = FALSE);
		void	SetFont (const CString &name, int nSize, int nWeight = FW_MEDIUM);
		void	SetFixedRowCol (int nRow, int nCol);
		void	InitRowCol (int nRowSize, int nColCount, BOOL bFitSize = TRUE);
		void	InitFirstCol (int nFirstColW, int nColCount);
		void	SetImageList (int nCX, int nCY);
		void	SetCellRange (int nRow, int nCol, double fMin = -FLT_MAX, double fMax = FLT_MAX);
		BOOL	IsInRange (int nRow, int nCol);
		double	GetItemDouble (int nRow, int nCol);
		int		GetItemInt (int nRow, int nCol);
		int		GetItemCurSel (int nRow, int nCol);
		void	SetItemCurSel (int nRow, int nCol, int nSel);
		BOOL	GetItemCheck (int nRow, int nCol);
		void	SetItemCheck (int nRow, int nCol, BOOL bCheck);
	DECLARE_MESSAGE_MAP ()
};