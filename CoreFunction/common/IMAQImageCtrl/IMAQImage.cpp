// IMAQImage.cpp : implementation file
//

#include "pch.h"
#include "IMAQImage.h"
#include "IMAQImageCtrl.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIMAQImage

CIMAQImage::CIMAQImage()
{
	m_bWndAttached = FALSE;
	m_hWndImg = 0;
	m_iWndImg = -1;
}

CIMAQImage::~CIMAQImage()
{
	if (m_bWndAttached)	Detach();
}


BEGIN_MESSAGE_MAP(CIMAQImage, CWnd)
	//{{AFX_MSG_MAP(CIMAQImage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CIMAQImage message handlers

// Do not use this function.
// But, it you ever want to use this function, modify this one
// by refering to Prepare function
BOOL CIMAQImage::Create(CWnd* /*pParentWnd*/, const RECT& /*rect*/, UINT /*nID*/, 
						DWORD /*dwStyle*/ /*=WS_VISIBLE*/)
{
	ASSERT (FALSE);

	return FALSE;
}

// Use this function for preparing IMAQ image display window.
// But, make sure to resize window after calling this function. 
BOOL CIMAQImage::Prepare(CWnd* pParentWnd, BOOL bChildWnd, BOOL bShowWnd)
{
	if (!m_hWndImg)
	{
		imaqGetWindowHandle(&m_iWndImg);	// Window is already created at IMAQ
		m_hWndImg = (HWND)imaqGetSystemWindowHandle(m_iWndImg);

		// cf. CWorkImage::Prepare
		// Use Attach instead of SubclassWindow for CIMAQImage
		//SubclassWindow(m_hWndImg);

		ASSERT(!m_bWndAttached);
		if (!Attach(m_hWndImg))
		{
			AfxMessageBox(_T("Window Attaching Failed!"));
			return FALSE;
		}
		else
			m_bWndAttached = TRUE;

		::SetParent(m_hWndImg, pParentWnd->GetSafeHwnd());
		if(bChildWnd)
			::SetWindowLong(m_hWndImg, GWL_STYLE, WS_CHILD | WS_VISIBLE | WS_OVERLAPPED);
		else
			::SetWindowLong(m_hWndImg, GWL_STYLE, WS_VISIBLE | WS_OVERLAPPED);

		imaqShowWindow(m_iWndImg, bShowWnd);
		//::MoveWindow(m_hWndImg, rect.left, rect.top, rect.Width(), rect.Height(), false);
	}

	return TRUE;
}

HWND CIMAQImage::GetHWndImg()	
{ 
	return m_hWndImg; 
}

int CIMAQImage::GetIWndImg()	
{ 
	return m_iWndImg; 
}
