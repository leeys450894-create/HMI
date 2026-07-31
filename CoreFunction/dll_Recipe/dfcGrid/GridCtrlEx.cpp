#include "stdAfx.h"
#include "GridCtrlEx.h"

IMPLEMENT_DYNAMIC (C_GridCtrl, CGridCtrl)
BEGIN_MESSAGE_MAP (C_GridCtrl, CGridCtrl)
END_MESSAGE_MAP ()

C_GridCtrl::C_GridCtrl ()
{
	SetRowResize (FALSE);
	SetColumnResize (FALSE);
}

C_GridCtrl::~C_GridCtrl ()
{	
}

BOOL	C_GridCtrl::Create(const RECT& rect, CWnd* parent, UINT nID, DWORD dwStyle, BOOL bRowResize, BOOL bColResize)
{
	//m_rect = rect;
	if (!CGridCtrl::Create (rect, parent, nID, dwStyle)) return FALSE;
	SetRowResize	(bRowResize);
	SetColumnResize (bColResize);

	return TRUE;
}

void	C_GridCtrl::SetAttribute (BOOL bEditable, BOOL bColEditable, BOOL bRowEditable)
{
	GetWindowRect (&m_rect);
	ScreenToClient (m_rect);
	m_rect.DeflateRect (3, 3);

	SetEditable		(bEditable);
	SetRowResize	(bColEditable);
	SetColumnResize (bRowEditable);
}

void	C_GridCtrl::SetFont (const CString &name, int nSize, int nWeight)
{
	CFont	font;
	font.CreateFont (nSize, 0, 0, 0, nWeight, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					   CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, name);
	CGridCtrl::SetFont (&font);	
	font.Detach ();

}

void	C_GridCtrl::SetFixedRowCol (int nRow, int nCol)
{
	SetFixedRowCount (nRow);
	SetFixedColumnCount (nCol);
}

void	C_GridCtrl::InitRowCol (int nRowSize, int nColCount, BOOL bFitSize)
{
	SetColumnCount (nColCount);
	int nColW = m_rect.Width () / nColCount;
	for (int i = 0; i < GetColumnCount () - 1; i++) {
		SetColumnWidth (i, nColW);
	}
	SetColumnWidth (nColCount - 1, m_rect.Width () - nColW * (nColCount - 1));

	if (bFitSize) {
		int nRowCount = nRowSize;
		int nRowH = (int)(m_rect.Height () / nRowCount);
		SetRowCount (nRowCount + 1);
		for (int i = 0; i < nRowCount; i++) {
			SetRowHeight (i, nRowH);
		}
		SetRowHeight (nRowCount, m_rect.Height () - nRowH * nRowCount);		
	} else {
		int nRowH = nRowSize;
		int nRowCount = (int)(m_rect.Height () / nRowH);
		SetRowCount (nRowCount + 1);
		for (int i = 0; i < nRowCount; i++) {
			SetRowHeight (i, nRowH);
		}
		SetRowHeight (nRowCount, m_rect.Height () - nRowH * nRowCount);
	}
}

void	C_GridCtrl::InitFirstCol (int nFirstColW, int nColCount)
{
	SetColumnCount (nColCount);
	SetColumnWidth (0, nFirstColW);
	int nColW = (m_rect.Width () - nFirstColW) / (nColCount-1);
	for (int i = 1; i < GetColumnCount () - 1; i++) {
		SetColumnWidth (i, nColW);
	}
	SetColumnWidth (nColCount - 1, m_rect.Width () - nFirstColW - nColW * (nColCount - 2));
}

void	C_GridCtrl::SetImageList (int nCX, int nCY)
{
	if (nCX == 0) nCX = 1;

	CImageList	*pImageList = GetImageList ();
	pImageList = new CImageList;
	pImageList->Create (nCX, nCY, ILC_COLOR, 0, nCX);
	CGridCtrl::SetImageList (pImageList);
	if (pImageList) delete pImageList;
}

void	C_GridCtrl::SetCellRange (int nRow, int nCol, double fMin, double fMax)
{
	CGridCellBase *pCell = GetCell (nRow, nCol);
	if (pCell) {
		pCell->m_fMin = fMin;
		pCell->m_fMax = fMax;
	}
}

BOOL	C_GridCtrl::IsInRange (int nRow, int nCol)
{
	CGridCellBase *pCell = GetCell (nRow, nCol);
	if (!pCell) return FALSE;
	double fVal = _wtof (pCell->GetText ());
	if (fVal >= pCell->m_fMin && fVal <= pCell->m_fMax) return TRUE;
	return FALSE;
}

double	C_GridCtrl::GetItemDouble (int nRow, int nCol)
{
	return _wtof (GetItemText (nRow, nCol));
}

int		C_GridCtrl::GetItemInt (int nRow, int nCol)
{
	return _ttoi (GetItemText (nRow, nCol));
}

int		C_GridCtrl::GetItemCurSel (int nRow, int nCol)
{
	CGridCellCombo *pCell = NULL;
	pCell = (CGridCellCombo*) GetCell (nRow, nCol);
	if (!pCell) return -1;	
	return pCell->GetCurSel ();
}

void	C_GridCtrl::SetItemCurSel (int nRow, int nCol, int nSel)
{
	CGridCellCombo *pCell = NULL;
	pCell = (CGridCellCombo*) GetCell (nRow, nCol);
	if (!pCell) return;	
	pCell->SetCurSel (nSel);

}

BOOL	C_GridCtrl::GetItemCheck (int nRow, int nCol)
{
	CGridCellCheck *pCell = NULL;
	pCell = (CGridCellCheck*) GetCell (nRow, nCol);
	if (!pCell) return FALSE;	
	return pCell->GetCheck ();
}

void	C_GridCtrl::SetItemCheck (int nRow, int nCol, BOOL bCheck)
{
	CGridCellCheck *pCell = NULL;
	pCell = (CGridCellCheck*) GetCell (nRow, nCol);
	if (!pCell) return;	
	pCell->SetCheck (bCheck);
}

