#if !defined(AFX_LEDBUTTON_H__A21BE56D_3313_40E9_9A77_5610124E271B__INCLUDED_)
#define AFX_LEDBUTTON_H__A21BE56D_3313_40E9_9A77_5610124E271B__INCLUDED_

// LedButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLedButton window

#define DEFAULT_MARGIN	20

/*
  class CLedButton : public CButton (version 1.0)

  Description:
    class that displays a LED that has 2 states: ON or OFF.
    The LED can have some text on its right.
    The LED is read-only (by user) and can not have focus (you should disable "tab stop" in the resource),
    The disabled state is not implemented: it is like the enable state.

  How to use:
    To your dialog add either a button, a checkbox or a radio button,
    bind it to a member variable (of type Control) using ClassWizard,
    then change in the header file the class type to CLedButton.

    In InitDialog bind your bitmap resource to the button:
    m_greenLed.SetImage( IDB_LEDBUTTON_GREEN, 15 ); //15 is the width of one image in the bitmap

    //You can set or retreive the state of the LED (ON or OFF) using
    m_greenLed.Depress(TRUE);  //ON  (second image in bitmap)
    m_greenLed.Depress(FALSE); //OFF (first image in bitmap)

    These bitmaps are given with this source code:
    IDB_LEDBUTTON_GREEN.bmp
    IDB_LEDBUTTON_RED.bmp

  Copyright:
    Completely Free (as long as this header remains like this) !
    Please visit http://welcome.to/softbird for updates, other sources and more !
    (c)Benjamin Mayrargue 2000

Example of use:

	in OnInitDialog():
	//Set images
	m_greenLed.SetImage( IDB_GREENBUTTON, 15 );
	m_redLed.SetImage( IDB_REDBUTTON, 15 );
	m_redLed2.SetImage( IDB_REDBUTTON, 15 );

	// set the initial state of buttons
	m_greenLed.Depress(true);
	m_redLed.Depress(true);

	//Add a handler to a switch button (for example)
	void CExampleDlg::OnClikSwitchRedLed()
	{
		m_redLed.Depress(!m_redLed.IsDepressed());
	}
*/

class CRGBImageList
{
public:
    CRGBImageList()
    {
		m_dcMem.CreateCompatibleDC(NULL);
		m_old = NULL;
    }
    virtual ~CRGBImageList()
    {
		if( m_hImage )
		{
			m_dcMem.SelectObject(m_old);
			m_old = NULL;
			m_bmp.Detach();
			::DeleteObject(m_hImage);
			m_hImage = NULL;			// YR
		}
		m_dcMem.DeleteDC();
    }
    
    BOOL Create( UINT idBitmap, UINT nWidth )
    {
		m_hImage = (HBITMAP)::LoadImage( AfxGetInstanceHandle(), MAKEINTRESOURCE(idBitmap), IMAGE_BITMAP, 0, 0, LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS );
		if( !m_hImage ) return FALSE;
		m_bmp.Attach(m_hImage);
		m_width = nWidth;
      
		//CSize size = m_bmp.GetBitmapDimension(); //Must be set by SetBitmapDimension: unusable here.
		m_bmp.GetObject(sizeof(BITMAP),&m_bmpInfo);
		m_height = m_bmpInfo.bmHeight;

		m_old = m_dcMem.SelectObject(&m_bmp);
		return TRUE;
    }

	BOOL Draw(CDC* pDC, int nImage, POINT pt)
    {
		// blue 2009.04.14 pt.x => pt.x-1 : 그려지는게..안이뻐서.. ㅡ.ㅡ
		return  pDC->BitBlt( pt.x-1, pt.y, m_width, m_height, &m_dcMem, nImage*m_width, 0, SRCCOPY );
    }

    CSize GetImageDimension() { CSize s(m_width,m_bmpInfo.bmHeight); return s; }

protected:
    CDC		m_dcMem;
    HBITMAP m_hImage;
    CBitmap m_bmp;
    UINT	m_width, m_height;
    CGdiObject *m_old;
    BITMAP	m_bmpInfo;
};


class CLedButton : public CButton
{
// Construction
public:
    //ReadOnly does not work: it is ALWAYS read-only.
	CLedButton( BOOL bReadOnly=TRUE, BOOL bDepressed=FALSE, BOOL bCenterAlign=TRUE ) { Init(bReadOnly,bDepressed,bCenterAlign); }
    virtual ~CLedButton() {}
    void Init( BOOL bReadOnly=TRUE, BOOL bDepressed=FALSE, BOOL bCenterAlign=TRUE );

    //Call "SetImage" once before displaying the control.
    void SetImage(UINT idBmp, UINT nWidthOfOneImage);

    //Does not work
	// void SetReadOnly(BOOL bReadOnly=TRUE) { m_bReadOnly = bReadOnly; }

	BOOL Depress( BOOL Down ); //Set button state (TRUE=pushed, FALSE=depressed)
	BOOL IsDepressed();        //Get button state

	void SetCheck( int nCheck );
	int GetCheck( );
	
	// blue 2009.09.17 
	void	SetFontName(const CString& strName);
	void	SetFontBold(BOOL bBold);
	void	SetFontSize(int nSize);
	void	SetTextColor(COLORREF crOff, COLORREF crOn);

    // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLedButton)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	int		m_nMargin;
	int		m_nRadius;
	CPoint	m_ptCenter;
	CRect	m_textRect;
	BOOL	m_bDepressed, m_bCenterAlign, m_bReadOnly; //m_bReadOnly does not work
    
	CRGBImageList m_imgList;
    CSize	m_rcImage;

	// blue 2009.09.17
	CFont		m_font;
	LOGFONT		m_lf;
	COLORREF	m_colorOff, m_colorOn;
	void		ReconstructFont();

	//{{AFX_MSG(CLedButton)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_LEDBUTTON_H__A21BE56D_3313_40E9_9A77_5610124E271B__INCLUDED_)
