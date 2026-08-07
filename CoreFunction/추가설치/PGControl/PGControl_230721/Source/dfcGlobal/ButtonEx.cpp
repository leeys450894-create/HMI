#include "pch.h"
#include "ButtonEx.h"

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC (C_ButtonEx, CMFCButton)

BEGIN_MESSAGE_MAP (C_ButtonEx, CMFCButton)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
END_MESSAGE_MAP ()

C_ButtonEx::C_ButtonEx ()
{
}

C_ButtonEx::~C_ButtonEx ()
{
}

void	C_ButtonEx::DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CMFCButton::DrawItem (lpDrawItemStruct);
	CDC	*pDC = CDC::FromHandle (lpDrawItemStruct->hDC);
	CRect rcItem = lpDrawItemStruct->rcItem, rcClient = rcItem;
	if (GetCheck ()) {
		rcClient.DeflateRect (0, 0, 1, 1);
		CPen pen, *pOldPen;
		pen.CreatePen (PS_SOLID, 4, RGB(128, 0, 0));
		pOldPen = (CPen*)pDC->SelectObject (&pen);			
		pDC->MoveTo (rcClient.left, rcClient.top);
		pDC->LineTo (rcClient.left, rcClient.bottom);
		pDC->LineTo (rcClient.right, rcClient.bottom);
		pDC->LineTo (rcClient.right, rcClient.top);
		pDC->LineTo (rcClient.left, rcClient.top);
	}
}

void	C_ButtonEx::OnLButtonDown (UINT nFlags, CPoint point) 
{
	CMFCButton::OnLButtonDown (nFlags, point);
	GetParent ()->SendMessage (WM_LBUTTONDOWN, GetDlgCtrlID ());
}

void	C_ButtonEx::OnLButtonUp (UINT nFlags, CPoint point) 
{
	CMFCButton::OnLButtonUp (nFlags, point);
	if(!IsWindow (this->m_hWnd)) return;
	GetParent ()->SendMessage (WM_LBUTTONUP, GetDlgCtrlID ());
}
