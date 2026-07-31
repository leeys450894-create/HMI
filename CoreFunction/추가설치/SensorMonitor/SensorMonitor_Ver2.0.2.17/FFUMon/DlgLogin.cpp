// DlgLogin.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "DlgLogin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgLogin dialog


CDlgLogin::CDlgLogin(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgLogin::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgLogin)
	m_strInputPW = _T("");
	//}}AFX_DATA_INIT
}


void CDlgLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgLogin)
	DDX_Text(pDX, IDC_EDIT_INSERT, m_strInputPW);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgLogin, CDialog)
	//{{AFX_MSG_MAP(CDlgLogin)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CDlgLogin::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgLogin message handlers

CString CDlgLogin::GetInput()
{
	return m_strInputPW;
}


void CDlgLogin::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
