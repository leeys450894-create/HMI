// CDlgSubMaint_IOFile.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubMaint_IOFile.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
#include "Manager\IOMapControlMgr.h"
#include "SystemParamShare.h"

static CString ColumnName[] =
{
	_T("No"),
	_T("Description"),
};

static long ColumnSize[] =
{
	50,
	226,
};

enum enumColumnName
{
	enumColumnName_No = 0,
	enumColumnName_Des,	
	enumColumnName_End,
};

enum enumGridName
{
	enumGridName_Input = 0,
	enumGridName_Output,
	enumGridName_Faddr,
	enumGridName_Ready,
	enumGridName_Meas,
	enumGridName_Load,
	enumGridName_SACC,
	enumGridName_End,
};

#define _df_MAX_IO_INFO			500

// CDlgSubMaint_IOFile 대화 상자

IMPLEMENT_DYNAMIC(CDlgSubMaint_IOFile, CDialogEx)

CDlgSubMaint_IOFile::CDlgSubMaint_IOFile(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SUB_MAINT_IO, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{

}

CDlgSubMaint_IOFile::~CDlgSubMaint_IOFile()
{
}

void CDlgSubMaint_IOFile::DoDataExchange(CDataExchange* pDX)
{	
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONFIG_TITLE,	m_LabelInfo);
	DDX_Control(pDX, IDC_STATIC_CONFIG_INPUT,	m_LabelGrid[enumGridName_Input]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_OUTPUT,	m_LabelGrid[enumGridName_Output]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_FADDR,	m_LabelGrid[enumGridName_Faddr]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_READY,	m_LabelGrid[enumGridName_Ready]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_MEAS,	m_LabelGrid[enumGridName_Meas]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_LOAD,	m_LabelGrid[enumGridName_Load]);
	DDX_Control(pDX, IDC_STATIC_CONFIG_SACC,	m_LabelGrid[enumGridName_SACC]);

	DDX_Control(pDX, IDC_GRID_LIST_PLC_INPUT,	m_gridList[enumGridName_Input]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_OUTPUT,	m_gridList[enumGridName_Output]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_FADDR,	m_gridList[enumGridName_Faddr]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_READY,	m_gridList[enumGridName_Ready]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_MEAS,	m_gridList[enumGridName_Meas]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_LOAD,	m_gridList[enumGridName_Load]);
	DDX_Control(pDX, IDC_GRID_LIST_PLC_SACC,	m_gridList[enumGridName_SACC]);
}


BEGIN_MESSAGE_MAP(CDlgSubMaint_IOFile, CDialogEx)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgSubMaint_IOFile 메시지 처리기


BOOL CDlgSubMaint_IOFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	for (int n = 0; n < enumGridName_End; n++)
	{
		m_LabelGrid[n].SetBorder(TRUE, TRUE);
		m_LabelGrid[n].SetBkColor(RGB(0, 32, 0));
		m_LabelGrid[n].SetTextColor(RGB(255, 255, 192));
		m_LabelGrid[n].SetFontBold(TRUE);
	}

	//---------------------------------------------------------------------
	// 리스트의 정보를 채워준다.
	//---------------------------------------------------------------------
	for (int n = 0; n < enumGridName_End; n++)
	{
		__InitListControl(n);
		__InitialListData(n);
	}

	SetTimer(1, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSubMaint_IOFile::__InitListControl(int iGridIndex)
{
	m_gridList[iGridIndex].GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[iGridIndex].GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[iGridIndex].GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[iGridIndex].SetDefCellMargin(0);
	m_gridList[iGridIndex].SetFixedColumnSelection(FALSE);
	m_gridList[iGridIndex].SetFixedRowSelection(FALSE);
	m_gridList[iGridIndex].SetSingleColSelection(FALSE);
	m_gridList[iGridIndex].SetSingleRowSelection(FALSE);
	m_gridList[iGridIndex].SetTrackFocusCell(FALSE);
	m_gridList[iGridIndex].SetRowResize(FALSE);
	m_gridList[iGridIndex].SetColumnResize(FALSE);
}

void CDlgSubMaint_IOFile::__InitialListData(int iGridIndex)
{
	// 결과값 리스트를 갱신한다.
	m_gridList[iGridIndex].DeleteAllItems();
	m_gridList[iGridIndex].SetColumnCount(enumColumnName_End);
	m_gridList[iGridIndex].SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList[iGridIndex].SetColumnWidth(n, ColumnSize[n]);
		m_gridList[iGridIndex].SetItemText(0, n, ColumnName[n]);
		m_gridList[iGridIndex].SetItemState(0, n, GVIS_READONLY);
		m_gridList[iGridIndex].SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList[iGridIndex].SetItemFgColour(0, n, RGB(255, 255, 255));

		if (iGridIndex == enumGridName_Input)
		{
			m_gridList[iGridIndex].SetColumnWidth(enumColumnName_No, 70);
			m_gridList[iGridIndex].SetColumnWidth(enumColumnName_Des, 365);
		}
		else if (iGridIndex == enumGridName_Load || iGridIndex == enumGridName_Ready || iGridIndex == enumGridName_Meas)
		{
			m_gridList[iGridIndex].SetRowHeight(0, 18);
			m_gridList[iGridIndex].SetColumnWidth(enumColumnName_No, 42);
			m_gridList[iGridIndex].SetColumnWidth(enumColumnName_Des, 218);
		}
			
	}

	// 각 Spread별로 Row 개수를 정한다
	int nStart = 0, nEnd = 0;
	CIOMapControlMgr* pImpl = nullptr;
	pImpl = m_pAuroraMgr->GetIOMapControlMgrPtr();

	CString sConfigPath = CSystemParamShare::GetInstance()->GetConfigFolderPath();	

	switch (iGridIndex)
	{
	case enumGridName_Input:
		nStart = dfMADDRESS_RD_START, nEnd = dfMADDRESS_WT_START;
		break;
	case enumGridName_Output:
		nStart = dfMADDRESS_WT_START, nEnd = dfMADDRESS_LIMIT_COUNT;
		break;
	case enumGridName_SACC:
		nStart = dfPACCADDRESS_RD_START, nEnd = dfPACCADDRESS_LIMIT_COUNT;
		break;
	case enumGridName_Faddr:
		nStart = dfFADDRESS_RD_START, nEnd = dfFADDRESS_TOTAL_COUNT;
		break;
	case enumGridName_Load:		
		ReadFileCSV(sConfigPath + _T("\\CoreFunction\\IOMapConfig\\PLC_LOAD.csv"), iGridIndex);
		return;
	case enumGridName_Ready:
		ReadFileCSV(sConfigPath + _T("\\CoreFunction\\IOMapConfig\\PLC_READY.csv"), iGridIndex);
		return;
	case enumGridName_Meas:
		ReadFileCSV(sConfigPath + _T("\\CoreFunction\\IOMapConfig\\PLC_MEAS.csv"), iGridIndex);
		return;
	}

	int iMaxIO = 0;
	if (pImpl != nullptr)
	{
		CString strData;
		for (nStart; nStart < nEnd; nStart++)
		{
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImport(nStart));
			strData.TrimRight();
			if (strData.IsEmpty())
				continue;

			iMaxIO++;
		}
	}

	// Items + Header
	m_gridList[iGridIndex].SetEditable(TRUE);
	m_gridList[iGridIndex].SetRowCount(iMaxIO + 1);	// Axis type end 

	m_gridList[iGridIndex].SetRowHeight(0, 18);
	for (long i = 0; i < iMaxIO; i++)
	{
		m_gridList[iGridIndex].SetRowHeight(i + 1, 18);
	}

	__RefreshListData(iGridIndex);
}

void CDlgSubMaint_IOFile::__RefreshListData(int iGridIndex)
{	
	m_gridList[iGridIndex].SetRedraw(FALSE);

	bool bHomeDone = false, bChkPos = false, bChkNeg = false;
	int i = 0, nStart = 0, nEnd = 0;
	CString strData;
	CIOMapControlMgr* pImpl = nullptr;
	pImpl = m_pAuroraMgr->GetIOMapControlMgrPtr();

	switch (iGridIndex)
	{
	case enumGridName_Input:
		nStart = dfMADDRESS_RD_START, nEnd = dfMADDRESS_WT_START;
		break;
	case enumGridName_Output:
		nStart = dfMADDRESS_WT_START, nEnd = dfMADDRESS_LIMIT_COUNT;
		break;
	case enumGridName_SACC:
		nStart = dfPACCADDRESS_RD_START, nEnd = dfPACCADDRESS_LIMIT_COUNT;
		break;
	case enumGridName_Faddr:
		nStart = dfFADDRESS_RD_START, nEnd = dfFADDRESS_TOTAL_COUNT;
		break;
	case enumGridName_Load:
		nStart = dfMADDRESS_RD_START, nEnd = dfMADDRESS_WT_START;
		break;
	case enumGridName_Ready:
		nStart = dfMADDRESS_RD_START, nEnd = dfMADDRESS_WT_START;
		break;
	case enumGridName_Meas:
		nStart = dfMADDRESS_RD_START, nEnd = dfMADDRESS_WT_START;
		break;
	}

	if (pImpl != nullptr)
	{
		for (nStart; nStart < nEnd; nStart++)
		{
			// LOAD, READY, MEAS 번지 찾기 
			if (iGridIndex == enumGridName_Load || iGridIndex == enumGridName_Ready || iGridIndex == enumGridName_Meas)
			{
				UINT nAddress = m_vecnPLCInfo[iGridIndex].vecData[nStart - dfMADDRESS_RD_START].nAddress;
				if (nAddress == 9999)
					continue;
			}

			// Number 
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImport(nStart));
			strData.TrimRight();
			if (strData.IsEmpty())
				continue;		
			

			m_gridList[iGridIndex].GetCell(i + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[iGridIndex].SetItemState(i + 1, enumColumnName_No, GVIS_READONLY);
			m_gridList[iGridIndex].SetItemText(i + 1, enumColumnName_No, strData);

			// Name
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImportName(nStart));
			m_gridList[iGridIndex].GetCell(i + 1, enumColumnName_Des)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[iGridIndex].SetItemState(i + 1, enumColumnName_Des, GVIS_READONLY);
			m_gridList[iGridIndex].SetItemText(i + 1, enumColumnName_Des, strData);

			// 활성화 되어 있는 상태를 표시해준다.
			bool bRet = false;
			if (iGridIndex == enumGridName_Load || iGridIndex == enumGridName_Ready || iGridIndex == enumGridName_Meas)
			{
				bRet = pImpl->GetParam_DIOMapMemoryBitImport2(nStart);
			}
			else if (iGridIndex == enumGridName_Output )
			{
				bRet = pImpl->GetParam_DIOMapMemoryBitExport2(nStart);
			}
			else
			{
				bRet = pImpl->GetParam_DIOMapMemoryBitImport2(nStart);
			}

			if (bRet == true)
		//	if (pImpl->GetParam_DIOMapMemoryBitImport2(nStart) == true)
			{
				if (iGridIndex == enumGridName_Load || iGridIndex == enumGridName_Ready || iGridIndex == enumGridName_Meas)
				{
					if (m_vecnPLCInfo[iGridIndex].vecData[nStart - dfMADDRESS_RD_START].bUsable == true)
					{
						for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
						{
							m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
						}
					}
					else
					{
						for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
						{
							m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_LIGHT_RED);
						}
					}
				}
				else
				{
					for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
					{
						m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
					}
				}
				
			}
			else
			{
				if (iGridIndex == enumGridName_Load || iGridIndex == enumGridName_Ready || iGridIndex == enumGridName_Meas)
				{
					if (m_vecnPLCInfo[iGridIndex].vecData[nStart - dfMADDRESS_RD_START].bUsable == true)
					{
						for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
						{
							m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_LIGHT_RED);
						}
					}
					else
					{
						for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
						{
							m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
						}
					}
				}
				else
				{
					for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
					{
						m_gridList[iGridIndex].SetItemBkColour(i + 1, n, CLR_GRAY);
					}
				}				
			}

			i++;
		}
	}
	
	if(iGridIndex != enumGridName_Input)
		m_gridList[iGridIndex].ExpandColumnsToFit(TRUE);
	m_gridList[iGridIndex].SetRedraw(TRUE, TRUE);
}

bool CDlgSubMaint_IOFile::ReadFileCSV(CString strFilename, int iGridIndex)
{
	m_vecnPLCInfo[iGridIndex].vecData.resize(dfMADDRESS_HALF_COUNT);

	CStdioFile File(strFilename, CFile::modeRead);

	int iRow = 0;

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("M"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			CString strTemp, strDescription;
			strTemp.Format(_T("M%d"), nAddress);
			strLine.Delete(0, iEnd + 1);

			iEnd = strLine.Find(_T(","));
			strDescription = strLine.Mid(0, iEnd);
			strLine.Delete(0, iEnd + 1);

			//Grid Row 늘리기 
			//m_gridList[iGridIndex].SetEditable(TRUE);
			m_gridList[iGridIndex].SetRowCount(iRow + 2);	// Axis type end 
			m_gridList[iGridIndex].SetRowHeight(iRow + 1, 15);			
		

			//LOAD, READY, MEAS Vector에 값 넣기
			if (dfMADDRESS_RD_START <= nAddress && nAddress < dfMADDRESS_WT_START)
			{
				if (strLine.IsEmpty() == false)
				{
					nAddress -= dfMADDRESS_RD_START;

					m_vecnPLCInfo[iGridIndex].vecData[nAddress].bUsable = _ttoi(strLine);
					m_vecnPLCInfo[iGridIndex].vecData[nAddress].nAddress = nAddress;
					m_vecnPLCInfo[iGridIndex].vecData[nAddress].strMapNumber = strTemp;
					m_vecnPLCInfo[iGridIndex].vecData[nAddress].strDescription = strDescription;
					//m_vecRData.vecData[nAddress].nAddress = nAddress;
					//m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//m_vecRData.vecData[nAddress].strDescription = strLine;
					//m_vecRData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;

					//Grid 활성화
					// Address number
					m_gridList[iGridIndex].GetCell(iRow + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					m_gridList[iGridIndex].SetItemState(iRow + 1, enumColumnName_No, GVIS_READONLY);
					m_gridList[iGridIndex].SetItemText(iRow + 1, enumColumnName_No, strTemp);

					// Name
					m_gridList[iGridIndex].GetCell(iRow + 1, enumColumnName_Des)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					m_gridList[iGridIndex].SetItemState(iRow + 1, enumColumnName_Des, GVIS_READONLY);
					m_gridList[iGridIndex].SetItemText(iRow + 1, enumColumnName_Des, strDescription);
					iRow++;
				}
			}
		}
		else   // End file
		{
			break;
		}
	}

	File.Close();

	return true;
}

void CDlgSubMaint_IOFile::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialogEx::OnTimer(nIDEvent);

	if (nIDEvent == 1)
		for (int i = 0; i < enumGridName_End; i++)
			__RefreshListData(i);
}
