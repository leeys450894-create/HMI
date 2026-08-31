#pragma once

#define PIX_UNIFORMITY		1
#define PIX_NON_UNIFORMITY  2
#define PIX_COORD_ITEM		3

class CMFCButton;

class C_BaseDialog : public CDialogEx
{
	DECLARE_DYNAMIC (C_BaseDialog)

	public:
		C_BaseDialog (UINT nID, CWnd *pParent = NULL);
		virtual ~C_BaseDialog ();
	
	private:
		CFont	m_font;
		BOOL	m_bModal;
	
	protected:
		virtual void	DoDataExchange	(CDataExchange* pDX);
		virtual	void	OnCancel ();
		virtual void	OnOK ();
		virtual void	InitButtons (CMFCButton &button);
		//virtual void	InitGridUI(C_GridCtrl& Grid, int nDlgItem, int nRow, int nCol, BOOL bSmall = FALSE, BOOL bHeader = FALSE, int nPixCell = -1);
		virtual void	InitGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, int nGridMode);

	public:
		virtual void	EnableControls (BOOL bEnable = TRUE);

	public:
		virtual INT_PTR DoModal ();


	protected:
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg HBRUSH	OnCtlColor (CDC *pDC, CWnd *pWnd, UINT nCtlColor);
		
	DECLARE_MESSAGE_MAP ()

};