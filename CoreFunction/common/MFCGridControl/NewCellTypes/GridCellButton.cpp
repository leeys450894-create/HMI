#include "pch.h"  
#include "GridCellButton.h"  
#include "../GridCtrl_src/GridCell.h"  
#include "../GridCtrl_src/GridCtrl.h"

#ifdef _DEBUG  
#define new DEBUG_NEW  
#endif  

IMPLEMENT_DYNCREATE(CGridCellButton, CGridCell)

CGridCellButton::CGridCellButton() : CGridCell()
{
	m_bPushing = FALSE;
}

CGridCellButton::~CGridCellButton(void)
{
}

BOOL CGridCellButton::Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /* = TRUE */)
{
	m_rect = rect;
	rect.DeflateRect(GetMargin(), 0);

	pDC->SetBkMode(TRANSPARENT);

	UINT state = (m_bPushing == TRUE) ? DFCS_BUTTONPUSH | DFCS_PUSHED : DFCS_BUTTONPUSH;
	pDC->DrawFrameControl(rect, DFC_BUTTON, state);

	CFont* pOldFont = pDC->SelectObject(GetFontObject());
	COLORREF ColorCurrent = pDC->GetTextColor();
	pDC->SetTextColor(::GetSysColor(COLOR_BTNTEXT));
	pDC->DrawText(GetText(), -1, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	pDC->SetTextColor(ColorCurrent);
	pDC->SelectObject(pOldFont);
	
	return TRUE;
}

//void CGridCellButton::OnClick(CPoint PointCellRelative)
//{
//	m_bPushing = !m_bPushing;
//
//	GetGrid()->InvalidateRect(m_rect);
//}

void CGridCellButton::OnClickDown(CPoint PointCellRelative)
{
	m_bPushing = !m_bPushing;

	GetGrid()->InvalidateRect(m_rect);
}

void CGridCellButton::OnClickUp(CPoint PointCellRelative)
{
	m_bPushing = !m_bPushing;

	GetGrid()->InvalidateRect(m_rect);
}
