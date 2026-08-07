#if !defined(AFX_GRIDCELLBUTTON_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_)
#define AFX_GRIDCELLBUTTON_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class AFX_EXT_CLASS CGridCellButton : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCellButton)

public:
    CGridCellButton();

public:
	BOOL SetCheck(BOOL bChecked = TRUE);
	BOOL GetCheck();

// Operations
	virtual CSize GetCellExtent(CDC* pDC);
    virtual void OnClick( CPoint PointCellRelative);
	virtual void OnClickDown( CPoint PointCellRelative);
	virtual void OnDblClick( CPoint PointCellRelative) { return; };
    virtual BOOL GetTextRect( LPRECT pRect);

protected:
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
    CRect m_Rect;
    BOOL  m_bClickDown;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDCELLCHECK_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_)
