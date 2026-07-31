// DlgSMarkUI.cpp : implementation file
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSMarkUI.h"
#include "afxdialogex.h"
#include "common/CommonFunc.h"

// CDlgSMarkUI dialog

IMPLEMENT_DYNAMIC(CDlgSMarkUI, CDialog)

CDlgSMarkUI::CDlgSMarkUI(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSMarkUI::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);			// pys 20200521 add
}

CDlgSMarkUI::~CDlgSMarkUI()
{
}

BOOL CDlgSMarkUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CString strPath;
//	strPath.Format(dfSMARK_IMAGE_PATH);
//	strPath = "D:\\KMAC\\SYSTEM\\";
	gf_Initialize();
	strPath = gf_GetFilePath(_T("SMARK.png")); 
	
	if (m_image.IsNull()) {  
		m_image.Load(strPath); 
		((CStatic *)GetDlgItem(IDC_STC_SMARK_IMAGE))->SetBitmap(m_image);
	}

	return TRUE;  
}

void CDlgSMarkUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSMarkUI, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CDlgSMarkUI message handlers

// pys 20200521 add
void CDlgSMarkUI::OnPaint()
{
	if (IsIconic()){
		CPaintDC dc(this);
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	} else {
		CDialog::OnPaint();
	}
}
