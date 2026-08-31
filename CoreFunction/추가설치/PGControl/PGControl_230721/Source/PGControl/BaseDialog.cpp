#include "pch.h"
#include "BaseDialog.h"

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC (C_BaseDialog, CDialogEx)
BEGIN_MESSAGE_MAP (C_BaseDialog, CDialogEx)
	ON_WM_ERASEBKGND ()
	ON_WM_CTLCOLOR ()
END_MESSAGE_MAP ()

C_BaseDialog::C_BaseDialog (UINT nID, CWnd *pParent) : CDialogEx (nID, pParent)
{
	m_bModal = FALSE;
	m_font.CreateFont (13, 0, 0, 0, FW_BOLD, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, dfcGetFontName ());

}

C_BaseDialog::~C_BaseDialog ()
{
	m_font.DeleteObject ();
}

void	C_BaseDialog::DoDataExchange (CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BOOL C_BaseDialog::OnEraseBkgnd (CDC* pDC)
{
	CRect rc = dfcClientRect (this);
	pDC->FillSolidRect (&rc, COLOR_USER_FRAME);
	return FALSE;
}

HBRUSH	C_BaseDialog::OnCtlColor (CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	static	CBrush	brush (COLOR_USER_FRAME);

	if (nCtlColor == CTLCOLOR_STATIC || nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_LISTBOX || nCtlColor == CTLCOLOR_SCROLLBAR) {
		pDC->SetBkColor	  (COLOR_USER_FRAME);
		pDC->SetTextColor (COLOR_WHITE);
	}

	return brush;
}


INT_PTR C_BaseDialog::DoModal ()
{
	m_bModal = TRUE;
	return CDialogEx::DoModal();
}

void	C_BaseDialog::EnableControls (BOOL bEnable)
{
	CWnd* pWndChild = GetWindow (GW_CHILD);

	TCHAR lpszClassName [MAX_CLASS_NAME];
	while (pWndChild != NULL) {
		ASSERT_VALID (pWndChild);
		::GetClassName(pWndChild->GetSafeHwnd(), lpszClassName, MAX_CLASS_NAME);
		CString strClass = lpszClassName;

		if (strClass == BUTTON_CLASS		|| 
			strClass == EDIT_CLASS			|| 
			strClass == LIST_CLASS			|| 
			strClass == COMBOBOX_CLASS		|| 
			strClass == SCROLLBAR_CLASS		|| 
			strClass == MFCEDITBROWSE_CLASS ||
			strClass == GRID_CLASS)  {
				pWndChild->EnableWindow (bEnable);
		}

		if (strClass == BUTTON_CLASS) {
			/*if (pWndChild->GetSafeHwnd ()) {
				CMFCButton *pButton = dynamic_cast<CMFCButton*> (pWndChild);
				if (pButton) {
					pButton->SetFaceColor (bEnable ? COLOR_FRAME : COLOR_GRAY);
					pButton->SetTextColor (bEnable ? COLOR_WHITE : COLOR_GRAY_DARK);
				}
			}*/
		}
		pWndChild = pWndChild->GetNextWindow ();
	}

	Invalidate (FALSE);
}

void	C_BaseDialog::OnCancel ()
{
	m_bModal ? CDialogEx::OnCancel () : ShowWindow (SW_HIDE);
}

void	C_BaseDialog::OnOK ()
{
	m_bModal ? CDialogEx::OnOK () : ShowWindow (SW_HIDE);
}


void	C_BaseDialog::InitButtons (CMFCButton &button)
{
	button.SetFont (&m_font);
	button.m_nFlatStyle = CMFCButton::BUTTONSTYLE_SEMIFLAT;
	button.m_bDrawFocus = FALSE;
	button.SetFaceColor		(COLOR_USER_BUTTON);
	button.SetTextColor		(COLOR_GRAY_BRIGHT);
	button.SetTextHotColor	(COLOR_YELLOW_DARK);
}

//void	C_BaseDialog::InitGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, BOOL bSmall, BOOL bHeader, int nPixCell)
void	C_BaseDialog::InitGridUI(C_GridCtrl& Grid, int nDlgItem, int nRow, int nCol, int nGridMode)
{
	CRect rc;
	GetDlgItem (nDlgItem)->GetWindowRect (&rc);
	ScreenToClient (&rc);
	DWORD dwStyle = WS_CHILD | WS_TABSTOP | WS_VISIBLE;

	Grid.SetFont(dfcGetFontName(), 15, FW_BOLD);

	Grid.SetColumnCount(nCol);
	Grid.SetRowCount(nRow);
	Grid.EnableToolTips(FALSE);
	Grid.EnableTitleTips(FALSE);
	Grid.GetWindowRect(&rc);

	int	nWidth = 0;
	if (nGridMode == RESULT_GRID) {
		Grid.SetFixedRowCol(0, 7);
		Grid.SetColumnWidth(0, 32);
		Grid.SetColumnWidth(1, 90);
		Grid.SetColumnWidth(2, 90);
		Grid.SetColumnWidth(3, 70);
		Grid.SetColumnWidth(4, 70);
		Grid.SetColumnWidth(5, 90);
		Grid.SetColumnWidth(6, 133);
		for (int i = 7; i < nCol; i++) Grid.SetColumnWidth(i, 62);
	}
	else if (nGridMode == SETTING_GRID) {
		Grid.SetFixedRowCol(0, 1);
		Grid.SetColumnWidth(0, 120);
		Grid.SetColumnWidth(1, 225);
		Grid.SetColumnWidth(2, 50);
	}
	else if (nGridMode == SETTING_GRID_SMALL) {
		Grid.SetFixedRowCol(0, 2);
		Grid.SetColumnWidth(0, 73);
		Grid.SetColumnWidth(1, 170);
	}
	else {
		Grid.SetFixedRowCol(0, 2);

		if (nCol < 2) {
			nWidth = (rc.Width () - 4) / nCol;
			for (int i = 0; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);
		}
		else {
			nWidth = (rc.Width () - 4) / 2;
			Grid.SetColumnWidth (0, nWidth);
			nWidth = nWidth / (nCol - 1);
			for (int i = 1; i < nCol; i++) Grid.SetColumnWidth (i, nWidth);
		}
	}

	for (int i = 0; i < nRow; i++) Grid.SetRowHeight(i, 22);			

	for (int i = 0; i < nRow; i++) {
		for (int j = 0; j < nCol; j++) {
			Grid.SetItemFormat (i, j, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}
	}
	Grid.SetFixedTextColor (COLOR_WHITE);
	Grid.SetFixedBkColor   (COLOR_USER_BUTTON);
}