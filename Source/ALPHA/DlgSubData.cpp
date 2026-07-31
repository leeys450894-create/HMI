// CDlgSubData.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubData.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"

#include "Manager/ResultMgr.h"
#include "CommTcpIpObjectCIM.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "SystemParamShare.h"
#include "common/CommonFunc.h"

//#include "..\ALPHA\"
// CDlgSubData 대화 상자

IMPLEMENT_DYNAMIC(CDlgSubData, CMainSubDlgWin)

CDlgSubData::CDlgSubData(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=nullptr*/)
	: CMainSubDlgWin(IDD_DLG_SUB_DATA, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{

}

CDlgSubData::~CDlgSubData()
{
}

void CDlgSubData::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_DATA_TITLE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CDlgSubData, CMainSubDlgWin)
	ON_BN_CLICKED(IDC_BUTTON_EXPORT_SE_APD, &CDlgSubData::OnBnClickedButtonExportSeApd)
	ON_BN_CLICKED(IDC_BUTTON_SEND_APD_TO_FTP, &CDlgSubData::OnBnClickedButtonSendApdToFtp)
	ON_BN_CLICKED(IDC_BUTTON_SEND_RCPLIST_TO_FTP, &CDlgSubData::OnBnClickedButtonSendRcplistToFtp)
END_MESSAGE_MAP()


// CDlgSubData 메시지 처리기



BOOL CDlgSubData::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);


	CRect rect;
	GetDlgItem(IDC_STATIC_DATA_TITLE)->GetWindowRect(&rect);
	ScreenToClient(&rect);

	CResultMgr::GetInstance()->Initialize(GetSafeHwnd(), rect);

	CString strConfig;
	strConfig.Format(_T("%s%s"), CSystemParamShare::GetInstance()->GetConfigFolderPath(), _T("Config.ini"));
	CResultMgr::GetInstance()->SetConfigPath(strConfig);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSubData::OnBnClickedButtonExportSeApd()
{
	::API_Result_SaveResult(true, true, gf_GetCurTimeString(4));
}


void CDlgSubData::OnBnClickedButtonSendApdToFtp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SendAPDFileToFTP();
}


void CDlgSubData::OnBnClickedButtonSendRcplistToFtp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SendRecipeListToFTP(); 
}
