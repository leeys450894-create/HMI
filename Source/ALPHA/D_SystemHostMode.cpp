// D_SystemHostMode.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "D_SystemHostMode.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
#include "SystemParamShare.h"
#include "common\CommonFunc.h"
#include "Manager\ShareMemoryMgr.h"
#include "Manager\CommTcpIpFactoryMgr.h"
#include "Manager\MaintControlMgr.h"
#include "CommTcpIpObjectCIM.h"

// CD_SystemHostMode 대화 상자

IMPLEMENT_DYNAMIC(CD_SystemHostMode, CDialogEx)

CD_SystemHostMode::CD_SystemHostMode(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POPUP_SYSTEM_HOSTMODE, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{
	m_nState = df_HOST_OFFLINE; // default
}

CD_SystemHostMode::~CD_SystemHostMode()
{
}

void CD_SystemHostMode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SYSTEM_HOST_MESSAGE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CD_SystemHostMode, CDialogEx)
	ON_BN_CLICKED(IDC_SYSTEM_HOSTMODE_OFFLINE, &CD_SystemHostMode::OnBnClickedSystemHostmodeOffline)
	ON_BN_CLICKED(IDC_SYSTEM_HOSTMODE_ONLINE, &CD_SystemHostMode::OnBnClickedSystemHostmodeOnline)
	ON_BN_CLICKED(IDC_CTBUTTONCTRL_SHTTER_OPEN, &CD_SystemHostMode::OnBnClickedCtbuttonctrlShtterOpen)
	ON_BN_CLICKED(IDC_CTBUTTONCTRL_SHTTER_CLOSE, &CD_SystemHostMode::OnBnClickedCtbuttonctrlShtterClose)
	ON_BN_CLICKED(IDOK, &CD_SystemHostMode::OnBnClickedOk)
END_MESSAGE_MAP()


// CD_SystemHostMode 메시지 처리기


BOOL CD_SystemHostMode::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontSize(20);
	m_LabelInfo.SetFontBold(TRUE);

	CenterWindow();

	CheckHostMode(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CD_SystemHostMode::OnBnClickedSystemHostmodeOffline()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_nState = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode();

	if (m_nState == df_HOST_OFFLINE)
	{
		CheckHostMode(false);
	}
	else
	{
		CheckHostMode(true);

		//m_nState = df_HOST_OFFLINE;
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 3, NULL);  // Offline

//		::PostMessage(m_pDlgOperate->GetSafeHwnd(), WM_HOSTMODE, (WPARAM)1, (LPARAM)df_HOST_OFFLINE);
	}

	CDialog::OnOK();
}


void CD_SystemHostMode::OnBnClickedSystemHostmodeOnline() 
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (CheckValid() == false) { CheckHostMode(true);	 return; }

	m_nState = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode();

	if (m_nState == df_HOST_ONLINE_REMOTE)
	{
		CheckHostMode(false);
	}
	else
	{
		CheckHostMode(true);

		//uhkang 20131118 Show Message Box when Machine is Not Initialized
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk != TRUE)
		{
			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

			AfxMessageBox(_T("Machine is Not Initialized!!"));
			CDialog::OnCancel();
			return;
		}

		if (m_pAuroraMgr->GetMaintControlMgr()->IsInitializedMC() != true)
		{
			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

			AfxMessageBox(_T("Machine is ReInitialized!!"));
			CDialog::OnCancel();
			return;
		}

		if (m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->IsConnected() != true)
		{
			if (m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->IsOutOfService() != true)
			{
				m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

				AfxMessageBox(_T("PLC do not Connect!!"));
				CDialog::OnCancel();
				return;
			}
		}
		else
		{
			// Check Arm
			if (m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_LOADER_ARM_SENSOR_01) != true)	// 
			{
				m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

				AfxMessageBox(_T("ARM is Detected!!"));
				//20140217 add
				CDialog::OnCancel();
				return;	// jsyoon 20140218 add
			}

			// jsyoon 20200215 add
			// Shutter Open
			if (m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE) != true )
			{
				m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);

				AfxMessageBox(_T("Loader Shutter Open!!"));
				CDialog::OnCancel();
				return;
			}

			if (m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_UNLOAD_SENSOR01) != true || 
				m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_UNLOAD_SENSOR02) != true ||
				m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_UNLOAD_SENSOR03) != true ||
				m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_LIFT_PIN_SENSOR_UP01) != true )
			{
				// Safety로 이동 후 온라인 변경
				//m_pSystem->m_bForceChangeMode = TRUE;
				m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE);


				
				//m_pDlgOperate->DoStartGlassLoadState();

				//20140217 add
				CDialog::OnOK();
				return;	// jsyoon 20140218 add
			}
		}

		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_ONLINE_REMOTE);

		//m_nState = df_HOST_ONLINE_REMOTE;
		//::PostMessage(m_pDlgOperate->GetSafeHwnd(), WM_HOSTMODE, (WPARAM)1, (LPARAM)df_HOST_ONLINE_REMOTE);
	}

	CDialog::OnOK();
}


void CD_SystemHostMode::OnBnClickedCtbuttonctrlShtterOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CD_SystemHostMode::OnBnClickedCtbuttonctrlShtterClose()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CD_SystemHostMode::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CD_SystemHostMode::CheckHostMode(bool bEnable)
{
	// TODO: 여기에 구현 코드 추가.

	m_nState = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode();

	if (m_nState == df_HOST_ONLINE_REMOTE)
	{
		GetDlgItem(IDC_SYSTEM_HOSTMODE_ONLINE)->EnableWindow(bEnable);
	}
	else if (m_nState == df_HOST_OFFLINE)
	{
		GetDlgItem(IDC_SYSTEM_HOSTMODE_OFFLINE)->EnableWindow(bEnable);
	}
}


bool CD_SystemHostMode::CheckValid()
{
	// TODO: 여기에 구현 코드 추가.
	if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->IsConnected() != true)
	{
		return false;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_IDLE)
	{
		return true;
	}

	return false;
}
