#if !defined(AFX_IMAQIMAGE_H__0F0F0B42_71C4_4809_A59B_F74DEB43466C__INCLUDED_)
#define AFX_IMAQIMAGE_H__0F0F0B42_71C4_4809_A59B_F74DEB43466C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IMAQImage.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// CIMAQImage window

// Wrapper class for IMAQ image window
class CIMAQImage : public CWnd
{
// Construction
	//aaaaa

public:
	CIMAQImage();

// Attributes
public:
	BOOL	m_bWndAttached;
	int		m_iWndImg;		// Window id for image
	HWND	m_hWndImg;		// Window handle -> Do not use m_hWnd of CWnd

protected:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIMAQImage)
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CIMAQImage();
	BOOL Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle = WS_VISIBLE);
	virtual BOOL Prepare(CWnd* pParentWnd,  BOOL bChildWnd=TRUE, BOOL bShowWnd=TRUE);

	HWND	GetHWndImg();
	int		GetIWndImg();

protected:

	// Generated message map functions
protected:
	//{{AFX_MSG(CIMAQImage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAQIMAGE_H__0F0F0B42_71C4_4809_A59B_F74DEB43466C__INCLUDED_)
