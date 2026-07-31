#pragma once
#include "..\dfcGrid\GridCtrlEx.h"

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
		virtual void	InitGridUI (C_GridCtrl &Grid, int nDlgItem, int nRow, int nCol, BOOL bSmall = FALSE, BOOL bHeader = FALSE, int nPixCell = -1);

	public:
		virtual void	EnableControls (BOOL bEnable = TRUE);

	public:
		virtual INT_PTR DoModal ();


	protected:
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg HBRUSH	OnCtlColor (CDC *pDC, CWnd *pWnd, UINT nCtlColor);
		
	DECLARE_MESSAGE_MAP ()

};