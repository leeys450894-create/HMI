// GridCellNumeric.cpp: implementation of the CGridCellNumeric class.
//
// Written by Andrew Truckle [ajtruckle@wsatkins.co.uk]
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCellNumeric.h"
#include "../inplaceedit.h"
#include "../GridCtrl.h"

IMPLEMENT_DYNCREATE(CGridCellNumeric, CGridCell)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Create a control to do the editing
BOOL CGridCellNumeric::Edit(int nRow, int nCol, CRect rect, CPoint /* point */, UINT nID, UINT nChar)
{
    m_bEditing = TRUE;
    
    // CInPlaceEdit auto-deletes itself
  //  m_pEditWnd = new CInPlaceEdit(GetGrid(), rect, /*GetStyle() |*/ ES_NUMBER, nID, nRow, nCol,
		//GetText(), nChar);
    m_pEditWnd = new CInPlaceEdit(GetGrid(), rect, ES_LEFT, nID, nRow, nCol,
		GetText(), nChar);
 
	SendMessage (m_pEditWnd->GetSafeHwnd (), EM_SETSEL, 0, 100);
	return TRUE;
}

// Cancel the editing.
void CGridCellNumeric::EndEdit()
{
	if (m_pEditWnd) {
        ((CInPlaceEdit*)m_pEditWnd)->EndEdit();
	}
}

void CGridCellNumeric::OnEndEdit()
{
	CString string, tmp = GetText();
	if (tmp.IsEmpty()) SetText (string);
	else {
		if (m_bUser) string.Format (_T("%.3lf"), _wtof(tmp));
		else string.Format (_T("%lf"), _wtof(tmp));

		SetText (string);
	}
	CGridCell::OnEndEdit();
}
