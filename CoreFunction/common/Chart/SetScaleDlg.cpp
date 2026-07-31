// SetScaleDlg.cpp : implementation file
//

#include "pch.h"
//#include "metrosys.h"
#include "SetScaleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDlg dialog


CSetScaleDlg::CSetScaleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSetScaleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSetScaleDlg)
	m_editXMax = _T("");
	m_editXMin = _T("");
	m_editYMax = _T("");
	m_editYMin = _T("");
	//}}AFX_DATA_INIT
}


void CSetScaleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetScaleDlg)
	DDX_Text(pDX, IDC_EDIT_XMAX, m_editXMax);
	DDX_Text(pDX, IDC_EDIT_XMIN, m_editXMin);
	DDX_Text(pDX, IDC_EDIT_YMAX, m_editYMax);
	DDX_Text(pDX, IDC_EDIT_YMIN, m_editYMin);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetScaleDlg, CDialog)
	//{{AFX_MSG_MAP(CSetScaleDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDlg message handlers
