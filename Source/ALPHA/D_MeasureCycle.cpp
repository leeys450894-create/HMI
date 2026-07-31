// D_MeasureCycle.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_MeasureCycle.h"
#include "afxdialogex.h"

#include "SystemParamShare.h"
#include "Recipe\RecipeEditMgr.h"
#include "Manager\IOMapControlMgr.h"
#include "common/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _df_MAX_COLUMN			6
static CString ColumnMeas[] =
{
	_T("상부 측정"),
	_T("하부 측정"),
	_T("0"),
	_T("180"),
	_T("Meas1"),
	_T("Meas2"),
};

static CString ColumnName[] =
{
	_T("No."),
	_T("Name"),
	_T("Use"),
};
static long ColumnSize[] =
{
	40,
	100,
	30,
};
enum enumColumnName
{
	enumColumnName5_No = 0,
	enumColumnName5_Value1,
	enumColumnName5_Value2,
	enumColumnName5_End,
};

// CD_MeasureCycle 대화 상자입니다.
#define      MAIN_OPERATE_SUB_SYSTEM_MEASURECYCLE_TIMER_ID			5001

IMPLEMENT_DYNAMIC(CD_MeasureCycle, CDialogEx)

CD_MeasureCycle::CD_MeasureCycle(CRecipeEditMgr *pRecipeEditMgr, CIOMapControlMgr *pIOMapControlMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_CYCLE, pParent)
	, m_pRecipeEditMgr(pRecipeEditMgr)
	, m_pIOMapControlMgr(pIOMapControlMgr)
{

}

CD_MeasureCycle::~CD_MeasureCycle()
{
}

void CD_MeasureCycle::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CYCLE_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_COMBO_RECIPE_LIST, m_cmbRecipeList);
//	DDX_Control(pDX, IDC_STATIC_MEAS_INFO, m_LabelInfo[1]);
}


BEGIN_MESSAGE_MAP(CD_MeasureCycle, CDialogEx)
	ON_BN_CLICKED(IDOK,			&CD_MeasureCycle::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL,		&CD_MeasureCycle::OnBnClickedCancel)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CD_MeasureCycle 메시지 처리기입니다.


BOOL CD_MeasureCycle::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontSize(20);
	m_LabelInfo.SetFontBold(TRUE);

	//---------------------------------------------------------------------
	// 레시피 선택할 수 있도록 표시 해준다.
	//---------------------------------------------------------------------

	m_pRecipeEditMgr->UpdateRecipeList();

	m_cmbRecipeList.ResetContent();
	for(int i = 0; i < m_pRecipeEditMgr->GetRecipeListCount(); ++i)
	{
		m_cmbRecipeList.AddString(m_pRecipeEditMgr->GetSelectedRecipeID(i));
	}
	m_cmbRecipeList.SetCurSel(0);


	// 반복 검사 횟수를 표시해준다.
	GetDlgItem(IDC_EDIT_CYCLE_TIMES)->SetWindowText(_T("1"));
	CSystemParamShare::GetInstance()->SetCycleCount(1);	

//	GetDlgItem(IDC_EDIT_GLASSID)->SetWindowText(_T(""));
	CString strGlassID;
	strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

	GetDlgItem(IDC_EDIT_GLASSID)->SetWindowText(strGlassID);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CD_MeasureCycle::OnBnClickedOk()
{
	int iCurSel = m_cmbRecipeList.GetCurSel();
	if (iCurSel < 0)
	{
		return;
	}

	CString str;
	if (m_pIOMapControlMgr->IsOutOfService() != true)
	{
		// jsyoon 20211019 remove Muting switch
		//if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_012) == true)
		//{
		//	CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking Muting Switch On"));
		//	AfxMessageBox(_T("Please Check Muting Switch"));
		//	return;
		//}

#ifdef _USE_IOBOARD
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_011) != true)
		{
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking LIGHT CURTAIN DETECTED "));
			AfxMessageBox(_T("Please Check LIGHT CURTAIN DETECTED"));
			return;
		}

		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) != true)
		{
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking AUTO MODE KEY"));
			AfxMessageBox(_T("Please Check AUTO MODE KEY"));
			return;
		}
#else
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_LOADER_ARM_SENSOR_01) != true)
		{
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking LIGHT CURTAIN DETECTED "));
			AfxMessageBox(_T("Please Check LIGHT CURTAIN DETECTED"));
			return;
		}

		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_RUN_TEACH_SWITCH) != true)
		{
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking AUTO MODE KEY"));
			AfxMessageBox(_T("Please Check AUTO MODE KEY"));
			return;
		}
#endif // _USE_IOBOARD
	}


	GetDlgItem(IDC_EDIT_CYCLE_TIMES)->GetWindowText(str);
	const long nCycleTimes = _ttoi(str);
	if (nCycleTimes <= 0)
	{
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("Cycle Times : ") + str);
		AfxMessageBox(_T("Please enter a value greater than 0."));
		return;
	}


	// CYCLE TIME
	CSystemParamShare::GetInstance()->SetCycleCount(nCycleTimes);
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("Cycle Times : ") + str);



	// INNER ID - Current Time 으로 생성한다.  [시분초 ]
	SYSTEMTIME st;
	::GetLocalTime(&st);
	str.Format(_T("%02d%02d%02d"), st.wHour, st.wMinute, st.wSecond);
	CSystemParamShare::GetInstance()->SetInnerID(str);

	// GLASS ID
	CString strGlassID;
	GetDlgItem(IDC_EDIT_GLASSID)->GetWindowText(strGlassID);

	if (strGlassID.IsEmpty() == TRUE)
	{
		CString strTemp;
	//	strTemp.Format(_T("GLASSID%sA"), str);
		strTemp.Format(_T("GLASSID%sB"), str);

		strGlassID.Format(_T("%s"), gf_MakeMcrID(strTemp));
	}

	CSystemParamShare::GetInstance()->SetGlassID(strGlassID);

	CString strLog;
	strLog.Format(_T("GLASSID : %s , Inner ID : %s "), strGlassID, str);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	CString strSelectRecipeID;
	m_cmbRecipeList.GetLBText(iCurSel, strSelectRecipeID);

	if (0 < m_pRecipeEditMgr->OpenRecipe(strSelectRecipeID))
	{
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MeasureCycle::OnBnClickedOk2"));

		CDialogEx::OnOK();
	}
}


void CD_MeasureCycle::OnBnClickedCancel()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MeasureCycle::OnBnClickedCancel"));

	CDialogEx::OnCancel();
}



void CD_MeasureCycle::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == MAIN_OPERATE_SUB_SYSTEM_MEASURECYCLE_TIMER_ID)
	{
#ifdef _USE_IOBOARD
		if (m_pIOMapControlMgr != NULL)
		{
			// jsyoon 20211019 change
			/*if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_012) != true && 
				m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_011) == true &&
				m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) == true )*/
			// LIGHT CURTAIN MUTING STATUS
			// LIGHT CURTAIN DETECTED
			// SELECT SWITCH AUTO MODE
			if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_010) == true &&
				m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_011) == true &&
				m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) == true)
			{
				if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_114) == true && m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_115) == true)
				{
					KillTimer(MAIN_OPERATE_SUB_SYSTEM_MEASURECYCLE_TIMER_ID);

					CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MeasureCycle::TWO-HAND SWITCH ON"));
					CDialogEx::OnOK();
				}
			}
		}
#endif // _USE_IOBOARD
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CD_MeasureCycle::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(MAIN_OPERATE_SUB_SYSTEM_MEASURECYCLE_TIMER_ID);
}