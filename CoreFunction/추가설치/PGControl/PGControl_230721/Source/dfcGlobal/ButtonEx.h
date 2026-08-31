#pragma once
#include <afxbutton.h>

class AFX_EXT_CLASS C_ButtonEx : public CMFCButton
{
	DECLARE_DYNAMIC (C_ButtonEx)
	public:
		C_ButtonEx ();
		virtual ~C_ButtonEx ();
	protected:
		afx_msg	void	OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg	void	OnLButtonUp (UINT nFlags, CPoint point);
		virtual void	DrawItem (LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP ()
};