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
	// LYS 2026-10-14 recipe ID length (glass ID): cell ID = GlassID.Left(L-1) + 3-char token + checksum = L+3 chars into m_szCellID[..][32]
	//   -> at most 28 chars can be typed (OnBnClickedOk checks again: the limit does not cut a longer current glass ID set above)
	((CEdit*)GetDlgItem(IDC_EDIT_GLASSID))->SetLimitText(28);

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

	// LYS 2026-10-14 recipe ID length (glass ID): the cell ID GlassID.Left(L-1) + 3-char token + checksum (L+3 chars) must fit m_szCellID[..][32]
	//   (_tcscpy_s aborts the measure thread on 32+) -> refuse a glass ID longer than 28 chars before SetGlassID. The dialog stays open
	if (strGlassID.GetLength() > 28)
	{
		CString strGlsLog;
		strGlsLog.Format(_T("[START] Glass ID too long (%d chars, max 28): %s - cycle not started"), strGlassID.GetLength(), (LPCTSTR)strGlassID);
		CLogWrapper::GetInstance()->KMLOG(WARN, strGlsLog);
		CString strGlsMsg;
		strGlsMsg.Format(_T("Glass ID가 너무 깁니다 (%d자).\nGlass ID는 28자 이하여야 합니다.\n(Cell ID = Glass ID + 셀 번호 + 체크섬, 32자 필드)\n\n[%s]\n\n시작하지 않습니다."), strGlassID.GetLength(), (LPCTSTR)strGlassID);
		AfxMessageBox(strGlsMsg, MB_ICONWARNING);
		return;
	}

	CSystemParamShare::GetInstance()->SetGlassID(strGlassID);

	CString strLog;
	strLog.Format(_T("GLASSID : %s , Inner ID : %s "), strGlassID, str);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	CString strSelectRecipeID;
	m_cmbRecipeList.GetLBText(iCurSel, strSelectRecipeID);

	// LYS 2026-10-14 recipe ID length: refuse a recipe ID longer than 31 chars before OpenRecipe - the MCC / result / ELP model fields are
	//   TCHAR[32] (a 33-char ID killed the engine thread at MCCLogMgr SetMCC_Action). Same exit as the other refusals here: the dialog stays open
	if (strSelectRecipeID.GetLength() > 31)
	{
		CString strLenLog;
		strLenLog.Format(_T("[START] Recipe ID too long (%d chars, max 31): %s - cycle not started"), strSelectRecipeID.GetLength(), (LPCTSTR)strSelectRecipeID);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLenLog);
		CString strLenMsg;
		strLenMsg.Format(_T("레시피 이름이 너무 깁니다 (%d자).\n레시피 이름은 31자 이하여야 합니다.\n(MCC / 결과 / 장치 모델명 필드 32자 제한)\n\n[%s]\n\n시작하지 않습니다. 31자 이하 이름으로 복사한 레시피를 선택하십시오."), strSelectRecipeID.GetLength(), (LPCTSTR)strSelectRecipeID);
		AfxMessageBox(strLenMsg, MB_ICONWARNING);
		return;
	}

	// LYS 2026-10-01 Hybrid Phase 4b D4b-15 (b) 0 < OpenRecipe() also accepted JOB_RECIPE_NOT_EXIST (1013): an unreadable .rcp closed the
	//   dialog and the cycle ran the previous recipe. Refuse only when the .rcp load itself failed (load serial unchanged)
	C_OutRecipeFunc* pLoadFunc = m_pRecipeEditMgr->GetRecipeFuncPtr();
	int nLoadSeq = (pLoadFunc != NULL) ? pLoadFunc->GetRcpLoadSeq() : 0;
	int nOpenRtn = m_pRecipeEditMgr->OpenRecipe(strSelectRecipeID);
	if (pLoadFunc != NULL && strSelectRecipeID.IsEmpty() != true && pLoadFunc->GetRcpLoadSeq() == nLoadSeq)
	{
		AfxMessageBox(_T("레시피 [") + strSelectRecipeID + _T("] 파일을 읽을 수 없습니다.\n파일이 없거나 손상되었습니다. (Cell ID 트레일러 등)\n\n이전 레시피로 측정하지 않도록 시작하지 않습니다."), MB_ICONWARNING);
		return;
	}
	// LYS 2026-10-14 Hybrid (14)-K at CYCLE START (user decision: hybrid only): the .rcp was read (load serial advanced) but the base .alg was
	//   not, so RecipeChange returned JOB_RECIPE_NOT_EXIST before m_strRecipeID and OpenRecipe kept the global RecipeID on the previous recipe.
	//   0 < 1013 used to start the cycle and the Recipe Change step reopened that previous recipe (or refused "UNKNOWN" after a restart).
	//   Refuse here like manual Meas Recipe Open. 1-segment recipe: the original 0 < test below (4b D4b-15 (b))
	if (nOpenRtn == JOB_RECIPE_NOT_EXIST && pLoadFunc != NULL && pLoadFunc->GetSegmentLoopCount() > 1)
	{
		CString strAlgLog;
		strAlgLog.Format(_T("[HYBRID] CYCLE START refused - %s returned %d (.alg load failed), the previous recipe is not run"), (LPCTSTR)strSelectRecipeID, nOpenRtn);
		CLogWrapper::GetInstance()->KMLOG(WARN, strAlgLog);
		AfxMessageBox(_T("레시피 [") + strSelectRecipeID + _T("]의 Align 파일(.alg)을 읽지 못했습니다.\n하이브리드 레시피는 Align 파일 없이 시작하지 않습니다.\n\nAlign 파일을 확인한 뒤 다시 시작하십시오."), MB_ICONWARNING);
		return;
	}
	if (0 < nOpenRtn)
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