#if !defined(AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_)
#define AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Label.h : header file
//

#define	NM_LINKCLICK	WM_APP + 0x200
#define	WM_UPDATETEXT	WM_APP + 0x201	// jsyoon 20150126 add

/////////////////////////////////////////////////////////////////////////////
// CLabel window

class CLabel : public CStatic
{
    // Construction
public:
    static enum LinkStyle { LinkNone, HyperLink, MailLink }eLinkStyle;
    static enum FlashType {None, Text, Background }eFlashType;
    static enum Type3D { Raised, Sunken}eType3D;
    static enum BackFillMode { Normal, Gradient }eBackFillMode;

    CLabel();
    virtual CLabel &SetBkColor(COLORREF crBkgnd, COLORREF crBkgndHigh = 0, BackFillMode mode = Normal);
    virtual CLabel &SetTextColor(COLORREF crText);
    virtual CLabel &SetText(const CString &strText);
    virtual CLabel &SetFontBold(BOOL bBold);
    virtual CLabel &SetFontName(const CString &strFont, BYTE byCharSet = ANSI_CHARSET);
    virtual CLabel &SetFontUnderline(BOOL bSet);
    virtual CLabel &SetFontItalic(BOOL bSet);
    virtual CLabel &SetFontSize(int nSize);
    virtual CLabel &SetSunken(BOOL bSet);
    virtual CLabel &SetBorder(BOOL bSet, BOOL bExSet = FALSE);
    virtual CLabel &SetClient(BOOL bSet);
    virtual CLabel &SetTransparent(BOOL bSet);
    virtual CLabel &FlashText(BOOL bActivate);
    virtual CLabel &FlashBackground(BOOL bActivate);
    virtual CLabel &SetLink(BOOL bLink, BOOL bNotifyParent);
    virtual CLabel &SetLinkCursor(HCURSOR hCursor);
    virtual CLabel &SetFont3D(BOOL bSet, Type3D type = Raised);
    virtual CLabel &SetRotationAngle(UINT nAngle, BOOL bRotation);
    virtual CLabel &SetText3DHiliteColor(COLORREF cr3DHiliteColor);
    virtual CLabel &SetFont(LOGFONT lf);
    virtual CLabel &SetMailLink(BOOL bEnable, BOOL bNotifyParent);
    virtual CLabel &SetHyperLink(const CString &sLink);
    virtual COLORREF GetBkColor();
    void	SetInvalidate(BOOL bSet) { m_bInvalidate = bSet; }	// blue 2009.09.14

    // Attributes
public:
protected:
    void UpdateSurface();
    void ReconstructFont();
    void DrawGradientFill(CDC *pDC, CRect *pRect, COLORREF crStart, COLORREF crEnd, int nSegments);
    COLORREF		m_crText;
    COLORREF		m_cr3DHiliteColor;
    HBRUSH			m_hwndBrush;
    HBRUSH			m_hBackBrush;
    LOGFONT			m_lf;
    CFont			m_font;
    BOOL			m_bState;
    BOOL			m_bTimer;
    LinkStyle		m_Link;
    BOOL			m_bTransparent;
    BOOL			m_bFont3d;
    BOOL			m_bToolTips;
    BOOL			m_bNotifyParent;
    BOOL			m_bRotation;
    FlashType		m_Type;
    HCURSOR			m_hCursor;
    Type3D			m_3dType;
    BackFillMode	m_fillmode;
    COLORREF		m_crHiColor;
    COLORREF		m_crLoColor;
    CString			m_sLink;

    // blue 2009.09.14 화면 깜빡임 없앨라고..
    CString			m_strText;
    BOOL			m_bInvalidate;	// default : TRUE - 기존 코드.
    // SetInvalidate(FALSE) 로 설정하면 타이머 돌때 깜빡임 제거.
    CString			m_strTimeText;

    // Operations
public:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CLabel)
protected:
    virtual void PreSubclassWindow();
    virtual BOOL PreCreateWindow(CREATESTRUCT &cs);
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CLabel();

    // Generated message map functions
protected:
    //{{AFX_MSG(CLabel)
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
    afx_msg void OnSysColorChange();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC *pDC);
    //}}AFX_MSG
	afx_msg LRESULT OnUpdateEvent_Time(WPARAM wp, LPARAM lp);
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LABEL_H__A4EABEC5_2E8C_11D1_B79F_00805F9ECE10__INCLUDED_)
