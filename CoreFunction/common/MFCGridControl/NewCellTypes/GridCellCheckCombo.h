#if !defined(AFX_GridCellCheckCombo_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_)
#define AFX_GridCellCheckCombo_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// GridCellCheckCombo.h : header file
//////////////////////////////////////////////////////////////////////


#include "../GridCtrl_src/GridCell.h"

#include "CheckComboBox.h"

class CGridCellCheckCombo : public CGridCell
{
    friend class CGridCtrl;
    DECLARE_DYNCREATE(CGridCellCheckCombo)

public:
    CGridCellCheckCombo();

// editing cells
public:
    virtual BOOL  Edit(int nRow, int nCol, CRect rect, CPoint point, UINT nID, UINT nChar);
    virtual CWnd* GetEditWnd() const;
    virtual void  EndEdit();

// Operations
public:
	virtual CSize GetCellExtent(CDC* pDC);

// CGridCellCheckCombo specific calls
public:
	CStringArray* GetOptions();
    void  SetOptions(CStringArray& ar);
    void  SetStyle(DWORD dwStyle)           { m_dwStyle = dwStyle; }
    DWORD GetStyle()                        { return m_dwStyle;    }

	CArray<BOOL>* GetCheckList();				// Jeong : Add GetCheckList();
	void  SetCheckList(CArray<BOOL>& ar);		// Jeong : Add SetCheckList();
	void  SetCheck(INT nIndex, BOOL bFlag);		// Jeong : Add SetCheck();
	BOOL  GetCheck(INT nIndex);					// Jeong : Add GetCheck();
	void  SelectAll(BOOL bCheck = TRUE);		// Jeong : Add SelectAll();

protected:
    virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

    CStringArray m_Strings;
    DWORD        m_dwStyle;

	CArray<BOOL> m_arrCheck;		// Jeong : checkbox check option.
	CString      m_strSaveText;		// Jeong : Save previous text.
};



/////////////////////////////////////////////////////////////////////////////
// CCheckComboEdit window

#define IDC_COMBOEDIT 1001

class CCheckComboEdit : public CEdit
{
// Construction
public:
	CCheckComboEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCheckComboEdit)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCheckComboEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCheckComboEdit)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CInPlaceCheckList window

class CInPlaceCheckList : public CCheckComboBox
{
    friend class CCheckComboEdit;

// Construction
public:
	CInPlaceCheckList(CWnd* pParent,         // parent
                 CRect& rect,           // dimensions & location
                 DWORD dwStyle,         // window/combobox style
                 UINT nID,              // control ID
                 int nRow, int nColumn, // row and column
                 COLORREF crFore, COLORREF crBack,  // Foreground, background colour
				 CStringArray& Items,   // Items in list
				 CArray<BOOL>& chklist, // Jeong : check status in list
                 CString sInitText,     // initial selection
				 UINT nFirstChar);      // first character to pass to control

// Attributes
public:
   CCheckComboEdit m_edit;  // subclassed edit control

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInPlaceCheckList)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInPlaceCheckList();
    void EndEdit();

protected:
    int GetCorrectDropWidth();

// Generated message map functions
protected:
	//{{AFX_MSG(CInPlaceCheckList)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDropdown();
	afx_msg UINT OnGetDlgCode();
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG
	//afx_msg void OnSelendOK();

	DECLARE_MESSAGE_MAP()

private:
	int		 m_nNumLines;
	CString  m_sInitText;
	int		 m_nRow;
	int		 m_nCol;
 	UINT     m_nLastChar; 
	BOOL	 m_bExitOnArrows; 
    COLORREF m_crForeClr, m_crBackClr;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GridCellCheckCombo_H__ECD42822_16DF_11D1_992F_895E185F9C72__INCLUDED_)
