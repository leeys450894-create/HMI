#include "stdafx.h"
#include "../GridCell.h"
#include "../GridCtrl.h"

#include "GridCellButton.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CGridCellButton, CGridCell)

CGridCellButton::CGridCellButton() : CGridCell()
{
    m_bClickDown = FALSE;
    //m_Rect.IsRectNull();
}

CSize CGridCellButton::GetCellExtent(CDC* pDC)
{
    // Using SM_CXHSCROLL as a guide to the size of the checkbox
    int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2*GetMargin();	
    CSize	cellSize = CGridCell::GetCellExtent(pDC);	
    cellSize.cx += nWidth;	
    cellSize.cy = max (cellSize.cy, nWidth);	
    return  cellSize;
}

// i/o:  i=dims of cell rect; o=dims of text rect
BOOL CGridCellButton::GetTextRect( LPRECT pRect)
{
    return CGridCell::GetTextRect(pRect);
}

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL CGridCellButton::Draw(CDC* pDC, int nRow, int nCol, CRect rect,  BOOL bEraseBkgnd /*=TRUE*/)
{
    BOOL bResult = CGridCell::Draw(pDC, nRow, nCol, rect,  bEraseBkgnd);
	m_Rect = rect;

	UINT nState = DFCS_BUTTONPUSH | DFCS_BUTTON3STATE;
	m_bClickDown ? nState |= DFCS_PUSHED : nState &=~DFCS_PUSHED;
	pDC->DrawFrameControl(m_Rect, DFC_BUTTON, nState);
	pDC->SetBkColor (GetSysColor(COLOR_BTNFACE));
	
    CFont *pOldFont = NULL,
          *pFont = GetFontObject();
    if (pFont)
        pOldFont = pDC->SelectObject(pFont);

	pDC->DrawText (GetText (), m_Rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

    if (pOldFont)
        pDC->SelectObject(pOldFont);

    return bResult;
}

void CGridCellButton::OnClick(CPoint PointCellRelative)
{
	PointCellRelative += m_Rect.TopLeft();
	m_bClickDown = FALSE;
	GetGrid()->InvalidateRect(m_Rect);
}

void CGridCellButton::OnClickDown( CPoint PointCellRelative)
{
	PointCellRelative += m_Rect.TopLeft();
	m_bClickDown = TRUE;
	GetGrid()->InvalidateRect(m_Rect);

}

