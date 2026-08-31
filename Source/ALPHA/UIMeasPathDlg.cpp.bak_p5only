// UIMeasPathDlg.cpp : implementation file
//

#include "pch.h"
#include "ALPHA.h"
#include "UIMeasPathDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CUIMeasPathDlg dialog


CUIMeasPathDlg::CUIMeasPathDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUIMeasPathDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUIMeasPathDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_nOldCell = 0;
}


void CUIMeasPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUIMeasPathDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUIMeasPathDlg, CDialog)
	//{{AFX_MSG_MAP(CUIMeasPathDlg)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_MESSAGE(XRTN_REPAINTED,				OnOC2Repaint)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUIMeasPathDlg message handlers

void CUIMeasPathDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	//CDialog::OnOK();
}

void CUIMeasPathDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	//CDialog::OnCancel();
}

BOOL CUIMeasPathDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//Added by Jay Hong : 2009.05.14=================================================================
	CRect rect;
	m_pParent->GetWindowRect(&rect);

	ScreenToClient(&rect);
	MoveWindow(rect.left,rect.top,rect.Width(),rect.Height());	 
    

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}



void CUIMeasPathDlg::UpdateOrbitChart(int nProbe, int nGroup, double **ppX, double **ppY)
{
	m_chartOrbit.UpdateChart(TRUE, nProbe, nGroup, ppX, ppY);
}

void CUIMeasPathDlg::SetMaxSize(double dXMax, double dXMin, double dYMax, double dYMin, BOOL bXReverse, BOOL bYReverse, BOOL bXYInvert, int iAngle/* = 0*/)
{
	m_chartOrbit.SetMaxSize(dXMax, dXMin, dYMax, dYMin, bXReverse, bYReverse, bXYInvert, iAngle);
}

BOOL CUIMeasPathDlg::AttachToView(CWnd *pParentWnd)
{
	/*	if (CUIViewDlg::AttachToView(pParentWnd))
	{
		// Chart 부 크기 조정 
		CWnd* pCtrlRect = pParentWnd->GetDlgItem(m_nViewRectID);
		ASSERT(pCtrlRect);

		CRect rect1;
		pCtrlRect->GetWindowRect(&rect1);
		pParentWnd->ScreenToClient(&rect1);

		CRect rect2;
		rect2.left = 2;
		rect2.top  = 2;
		rect2.right  = rect1.Width() - 4;
		rect2.bottom = rect1.Height() - 4;
		m_chartOrbit.MoveWindow(&rect2);

		return TRUE;
	}
	else
		return FALSE;
*/

	return TRUE;
}

void CUIMeasPathDlg::SetProbeColor(COLORREF *pColorProbe)
{
	m_chartOrbit.SetProbeColor(pColorProbe);
}


LRESULT CUIMeasPathDlg::OnPathNProbe(WPARAM wParam, LPARAM /*lParam*/)
{
	int nProbe = wParam;
	SetNProbe(nProbe);
	return 1;
}

LRESULT CUIMeasPathDlg::OnPathProbeColor(WPARAM wParam, LPARAM /*lParam*/)
{
	COLORREF* pColor = (COLORREF*)wParam;
	SetProbeColor(pColor);
	return 1;
}

/*====================================================================================================
Function : 현재까지 특별히 사용되지 않고 있음
Using	 : 
Date : 2009-05-22 
Added by:Jay Hong
=====================================================================================================*/
LRESULT CUIMeasPathDlg::OnPathMaxSize(WPARAM wParam, LPARAM lParam)
{
/*	CWorkData* pWorkData = (CWorkData*)wParam;
	CConfData* pConfData = (CConfData*)lParam;
	SetMaxSize(	pWorkData->m_dMeasPathMaxSize[0], pWorkData->m_dMeasPathMaxSize[1],
				pWorkData->m_dMeasPathMaxSize[2], pWorkData->m_dMeasPathMaxSize[3],
				(pConfData->m_iOrbitDirX < 0), (pConfData->m_iOrbitDirY < 0),
				pConfData->m_iOrbitXYInvert);	
*/
	return 1;
}

/*====================================================================================================
Function : 현재까지 특별히 사용되지 않고 있음
Using	 : 
Date : 2009-05-22 
Added by:Jay Hong
=====================================================================================================*/
LRESULT CUIMeasPathDlg::OnPathGlsSize(WPARAM wParam, LPARAM /*lParam*/)
{
/*	CWorkData* pWorkData = (CWorkData*)wParam;
	SetSampleSize(pWorkData->m_dMeasPathGlsSize[0], pWorkData->m_dMeasPathGlsSize[1],
				pWorkData->m_dMeasPathGlsSize[2], pWorkData->m_dMeasPathGlsSize[3]);	

*/
	return 1;
}

/*====================================================================================================
Function : 현재까지 특별히 사용되지 않고 있음
Using	 : 
Date : 2009-05-22 
Added by:Jay Hong
=====================================================================================================*/
LRESULT CUIMeasPathDlg::OnPathUpdateOrbit(WPARAM wParam, LPARAM /*lParam*/)
{
/*	CRecipe* pRecipe = (CRecipe*)wParam;

	int nProbe = pRecipe->GetNProbe();
	int nGroup = pRecipe->GetNMeasGroup();

	double** ppX = new double*[nProbe];
	double** ppY = new double*[nProbe];

	for(int iProbe = 0; iProbe < nProbe; iProbe++)
	{	
		ppX[iProbe] = new double[nGroup];
		ppY[iProbe] = new double[nGroup];

		for(int iGroup = 0; iGroup < nGroup; iGroup++)
		{
			ppX[iProbe][iGroup]	= pRecipe->GetPMeasGroup(iGroup)->GetPMeasArea(iProbe)->m_posCmd.x;
			ppY[iProbe][iGroup]	= pRecipe->GetPMeasGroup(iGroup)->GetPMeasArea(iProbe)->m_posCmd.y;
		}
	}

	UpdateOrbitChart(nProbe, nGroup, ppX, ppY);

	for(iProbe = 0; iProbe < nProbe; iProbe++)
	{
		delete ppX[iProbe];
		delete ppY[iProbe];
	}
	
	delete[] ppX;
	delete[] ppY;
*/
	return 1;
}

void CUIMeasPathDlg::SetParentWnd(CWnd *pWnd)
{
	m_pParent = pWnd;
}

void CUIMeasPathDlg::SetCardSize(double dXMax, double dXMin, double dYMax, double dYMin)
{
	m_chartOrbit.SetProbeCardSize(dXMax, dXMin, dYMax, dYMin);
}

void CUIMeasPathDlg::SetLoaderSize(double dXMax, double dXMin, double dYMax, double dYMin, int iLoaderCCut)
{
	m_chartOrbit.SetLoaderSize(dXMax, dXMin, dYMax, dYMin, iLoaderCCut);
}


void CUIMeasPathDlg::SetGlassSize(double dXMax, double dXMin, double dYMax, double dYMin, int iGlsCCut)
{
	m_chartOrbit.SetGlassSize(dXMax, dXMin, dYMax, dYMin, iGlsCCut);
}

void CUIMeasPathDlg::UpdateMarkCoord(double x, double y)
{
	m_chartOrbit.SetGlobalMakrPos(x, y);
}

void CUIMeasPathDlg::UpdateCurCoord(int iProbe, double x, double y, double z, double s)
{
	m_chartOrbit.SetCurPos(iProbe, TRUE, x, y);
}

//void CUIMeasPathDlg::UpdateGroupPosition(RECIPEPATH* pRecipe)
//{
//	int nGroup = pRecipe->m_aryMeasGroup.GetSize();
//
//// 	if (m_nOldCell > 0)
//// 	{
//// 		for(int iCell=0; iCell<m_nOldCell; iCell++)
//// 		{
//// 			SetRemoveGlassCellLayout(iCell);
//// 		}
//// 	}
//	
//// 	int nMaxPrbCntUsable = 0;
//// 	for (int i = 0; i < df_MAX_PROBE; i++)
//// 	{
//// //		if (m_pSystem->stSystem[VTM_PROBE1+i].m_bUse)
//// 			nMaxPrbCntUsable++;
//// 	}
//	
////	int nProbe = nMaxPrbCntUsable;
//
//	int nProbe = 1;
//
//	nProbe = pRecipe->m_nProbeCnt;	// jsyoon 20170417 add
//
//	double m_posCmd_x = 0;
//	double m_posCmd_y = 0;
//
//// 	float  iCellX;
//// 	float  iCellY;
//
//	double** ppX = new double*[nProbe];
//	double** ppY = new double*[nProbe];
//	int iProbe = 0;
//
//	MEASGROUP	stMeasGroup;
//// 	CString	strCellId = _T("");
//// 	CString	strBlockId = _T("");
//
//	for( iProbe = 0; iProbe < nProbe; iProbe++)
//	{	
//		ppX[iProbe] = new double[nGroup];
//		ppY[iProbe] = new double[nGroup];
//		
//		for(int iGroup = 0; iGroup < nGroup; iGroup++)
//		{
//			stMeasGroup = pRecipe->m_aryMeasGroup.GetAt(iGroup);
//
//		 	m_posCmd_x = stMeasGroup.stProbe[iProbe].m_dx;	//dGantry[0];
//			m_posCmd_y = stMeasGroup.stProbe[iProbe].m_dy;
//
//			if( m_posCmd_x != 0 || m_posCmd_y != 0 )
//			{
//				ppX[iProbe][iGroup]	= m_posCmd_x;
//				ppY[iProbe][iGroup]	= m_posCmd_y ;
//			}
//			else
//			{
//				ppX[iProbe][iGroup]	= XRT_HUGE_VAL;
//				ppY[iProbe][iGroup]	= XRT_HUGE_VAL;
//			}
//			
////			if(m_pSystem->stSystem[VTM_GANTRY].m_bUse)
////			{
////				ppX[iProbe][iGroup]	= m_posCmd_x;
////			}
//// 			else//Gantry를 사용 안할 경우
//// 			{
//// 				m_posCmd_x = stMeasGroup.m_stProbe[iProbe].m_ds;
//// 				ppX[iProbe][iGroup]	= m_posCmd_x;
//// 			}
////			ppY[iProbe][iGroup]	= m_posCmd_y ;
////
//// 			if( m_posCmd_x > 0 && m_posCmd_y > 0 )
//// 				strBlockId = "B";
//// 			else if(m_posCmd_x > 0 && m_posCmd_y < 0)
//// 				strBlockId = "D";
//// 			else if(m_posCmd_x < 0 && m_posCmd_y < 0)
//// 				strBlockId = "C";
//// 			else
//// 				strBlockId = "A";
//// 			
//// 			strCellId.Format(_T("%s%s%d", strBlockId, "A", iGroup);
//// 
//// 			iCellX = pRecipe->stCellInfo.CellSize.x/2;
//// 			iCellY = pRecipe->stCellInfo.CellSize.y/2;
//// 
//// 			SetGlassCellLayout(((iProbe*nGroup)+iGroup), strCellId, m_posCmd_x-iCellX, m_posCmd_y-iCellY, m_posCmd_x+iCellX, m_posCmd_y+iCellY);
//		}
//	}
//
////	m_nOldCell = nProbe*nGroup;
//	
//	UpdateOrbitChart(nProbe, 0, ppX, ppY);		// reset
//
//	UpdateOrbitChart(nProbe, nGroup, ppX, ppY);
//
//	for(iProbe = 0; iProbe < nProbe; iProbe++)
//	{
//		delete ppX[iProbe];
//		delete ppY[iProbe];
//	}
//	
//	delete[] ppX;
//	delete[] ppY;
//}

void CUIMeasPathDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
}

void CUIMeasPathDlg::SetNProbe(int nProbe,COLORREF *pColorProbe, BOOL bShowBORDER)
{
	m_chartOrbit.InitialSetup(nProbe, pColorProbe, bShowBORDER);
	//	for (int i = 0; i < nProbe; i++)
	//		m_chartOrbit.SetCurPos(i, TRUE, 0, 0);
}


void CUIMeasPathDlg::DrawChart(CWnd *pWnd)
{
	if(!m_chartOrbit.GetSafeHwnd())
	{
		CRect rect;
		pWnd->GetWindowRect(&rect);
		
		ScreenToClient(&rect);
		
		//m_chartOrbit.Create("ChartOrbit", WS_CHILD | WS_VISIBLE, rect, pWnd, 2001);
		m_chartOrbit.Create(_T("ChartOrbit"), WS_CHILD | WS_VISIBLE, rect, this, 2001);
	}
	else
	{
		SetParent(pWnd);				//현재 윈도의 부모 윈도를 pWnd으로 한다
		//m_chartOrbit.SetParent(pWnd);	//m_chartOrbit의 부모 윈도를 pWnd으로 한다		
	}	
}

/*====================================================================================================
Function : 어떻게 메세지를 받는 것인지 당췌 알 수가 없네
Using	 : 일단 무시하고 자체 코딩으로 해결하자
Date : 2009-05-22 

			DrawChart(CWnd *pWnd)이부분 확인 할 것
			//m_chartOrbit.Create("ChartOrbit", WS_CHILD | WS_VISIBLE, rect, pWnd, 2001);
			m_chartOrbit.Create("ChartOrbit", WS_CHILD | WS_VISIBLE, rect, this, 2001);
			Chart생성 할 때 "this"를 넘겨줘야 메세지를 여기서 받지.... 바부탱이

Added by:Jay Hong
=====================================================================================================*/
LRESULT CUIMeasPathDlg::OnOC2Repaint(WPARAM wParam, LPARAM lParam)
{
	
// jsyoon 20140610 remove
//	if (!m_bRepaint )	return 1;		//왜 이렇게 했는지는 나도 잘 모름, 원래 코드에 이렇게 되어 있어서
										//원래 코드도 이상함. Jay Hong : 2009.0522

	HWND hWnd = (HWND) wParam;		
	XrtCallbackStruct *cb = (XrtCallbackStruct *) lParam;

	ASSERT(m_chartOrbit.GetSafeHwnd() == hWnd);

	return m_chartOrbit.OnOC2Repaint(cb->hdc);
}


/*====================================================================================================
Function : Panel Layout 크기 파라메터 설정
Using	 : 
Date : 2009-05-22 
Added by:Jay Hong
=====================================================================================================*/
void CUIMeasPathDlg::SetPanelParam(BOOL bPanelDraw, int nPanelNx, int nPanelNy, double dPanelX0, double dPanelY0, double dPanelLx, double dPanelLy, double dPanelDx, double dPanelDy)
{
	m_chartOrbit.SetPanelParam(bPanelDraw, nPanelNx, nPanelNy, dPanelX0, dPanelY0, dPanelLx,dPanelLy, dPanelDx, dPanelDy);
}

void CUIMeasPathDlg::SetGlassCellLayout(int iIndex, CString strCellName, double dSX, double dSY, double dEX, double dEY)
{
	m_chartOrbit.SetGlassCellLayout(iIndex, strCellName, dSX, dSY, dEX, dEY);
}

void CUIMeasPathDlg::SetGlassCell(int nIndex, CStringArray* strCellName, double* dSX, double* dSY, double* dEX, double* dEY, COLORREF* pColor)
{
	m_chartOrbit.SetGlassCell(nIndex, strCellName, dSX, dSY, dEX, dEY, pColor);
}

void CUIMeasPathDlg::SetRemoveGlassCellLayout(int iIndex)
{
	m_chartOrbit.SetRemoveGlassCellLayout(iIndex);
}

//void CUIMeasPathDlg::UpdateCellPosition(RECIPEPATH *pRecipe)
//{
//	int nGroup = 0;
//	nGroup = pRecipe->m_aryMeasGroup.GetSize();
//	if( nGroup == 0)
//	{
//		SetRemoveGlassCellLayout(-1);
//		m_nOldCell = 0;
////		return; 
//	}
//	
//	if (m_nOldCell > 0 )
//	{
//		SetRemoveGlassCellLayout(-1);
//	}
//	
//	int nMaxPrbCntUsable = 0;
//	for (int i = 0; i < df_MAX_PROBE; i++)
//	{
//		nMaxPrbCntUsable++;
//	}
//	
//	int nProbe = nMaxPrbCntUsable;
//	
//	double m_posCmd_x = 0;
//	double m_posCmd_y = 0;
//	float  iCellX = 0;
//	float  iCellY = 0;
//	float  iCellDX = 0;
//	float  iCellDY = 0;
//	float  iCellOX = 0;
//	float  iCellOY = 0;
//	int    iCellNum = 0;
//	float  iStartOX = 0;
//	float  iStartOY = 0;
//
//	int    iCol = 0;
//	int    iRow = 0;
//	float  iStartX = 0;
//	float  iStartY = 0;
//
//	
//	CString	strCellId = _T("");
//	CString	strBlockId = _T("");
//
//	iCellNum = pRecipe->stCellInfo.CellNum;
//	iCellOX = pRecipe->stCellInfo.CellOffset.x;
//	iCellOY = pRecipe->stCellInfo.CellOffset.y;
//	iCellX = pRecipe->stCellInfo.CellSize.x;
//	iCellY = pRecipe->stCellInfo.CellSize.y;
//	iRow   = (int)pRecipe->stCellInfo.CellCount.x;
//	iCol   = (int)pRecipe->stCellInfo.CellCount.y;
//	iStartX = (float)(pRecipe->stCellInfo.CellEdig.x/* + m_pSystem->m_dGlobalX*/);
//	iStartY = (float)(pRecipe->stCellInfo.CellEdig.y/* + m_pSystem->m_dGlobalY*/);
//	iCellDX = pRecipe->stCellInfo.CellDistance.x;
//	iCellDY = pRecipe->stCellInfo.CellDistance.y;
//
////iStartOX = iCellDX*iRow;
//  iStartOX = (iCellDX*iRow)+iCellOX;
//
//	int iDX = 1, iDY = 1;
//	if(iStartX > 0)
//		iDX = -1;
//
//	if(iStartY > 0)
//		iDY = -1;
//
//	m_nOldCell = iCol*iRow*iCellNum;
//
//	if( m_nOldCell < 1)
//		return;
//
//	double* pSX = new double[m_nOldCell];
//	double* pSY = new double[m_nOldCell];
//	double* pEX = new double[m_nOldCell];
//	double* pEY = new double[m_nOldCell];
//
//	CStringArray strArray;
//	strArray.RemoveAll();
//
//	int iCell = 0;
//	int iX=0;
//	for (int iY=0; iY < iCol; iY++)
//	{
////		for (int iX=0; iX< iRow*iCellNum; iX++)
//		for (iCell=0; iCell< iCellNum; iCell++)
//		{
//			for (int k=0; k< iRow; k++)
//			{
//				m_posCmd_x = (iStartX + iStartOX*iCell) + ( iCellDX*iDX * k );
//				m_posCmd_y = iStartY + ( iCellDY*iDY * iY );
//				
//				iX = k+(iRow*iCell);
//				
//
//				if( FALSE)
//				{
//					if( m_posCmd_x > 0 && m_posCmd_y > 0 )
//						strBlockId = "B";
//					else if(m_posCmd_x > 0 && m_posCmd_y < 0)
//						strBlockId = "D";
//					else if(m_posCmd_x < 0 && m_posCmd_y < 0)
//						strBlockId = "C";
//					else
//						strBlockId = "A";
//					
//					if( iDY < 0)
//					{
//						if(iDX > 0) 
//							strCellId.Format(_T("%s%c%02d"), strBlockId, iX+65, iY+1);
//						else
//							strCellId.Format(_T("%s%c%02d"), strBlockId, iRow+65 - iX, iY+1);
//					}
//					else
//					{
//						if(iDX > 0)
//							strCellId.Format(_T("%s%c%02d"), strBlockId, iX+65, iCol-iY);
//						else
//							strCellId.Format(_T("%s%c%02d"), strBlockId, iRow+65 - iX, iCol-iY);
//					}
//				}
//				else
//				{
//					if( iDY > 0)
//					{
//						if(iDX > 0)		// jsyoon 20150224 change < to > 
//							strCellId.Format(_T("%c%02d"), iX+65, iY+1);
//						else
////							strCellId.Format(_T("%c%02d"), iRow+65 - iX, iY+1);
//							strCellId.Format(_T("%c%02d"), iRow*iCellNum+64 - iX, iY+1);
//					}
//					else
//					{
//						if(iDX > 0)		// jsyoon 20150224 change < to >
//							strCellId.Format(_T("%c%02d"), iX+65, iCol-iY);
//						else
//						//	strCellId.Format(_T("%c%02d"), iRow+65 - iX, iCol-iY);
//							strCellId.Format(_T("%c%02d"), iRow*iCellNum+64 - iX, iCol-iY);
//					}
//				}
//
//				
//				strArray.Add(strCellId);
////				pSX[iX+(iRow*iY)] = m_posCmd_x;
////				pSY[iX+(iRow*iY)] = m_posCmd_y;
////				pEX[iX+(iRow*iY)] = m_posCmd_x+iCellX;
////				pEY[iX+(iRow*iY)] = m_posCmd_y+iCellY;
//				pSX[iX+(iRow*iCellNum*iY)] = m_posCmd_x;
//				pSY[iX+(iRow*iCellNum*iY)] = m_posCmd_y;
//				pEX[iX+(iRow*iCellNum*iY)] = m_posCmd_x+iCellX;
//				pEY[iX+(iRow*iCellNum*iY)] = m_posCmd_y+iCellY;
//			}
//		}
//	}
//
//	SetGlassCell(m_nOldCell, &strArray, pSX, pSY, pEX, pEY);
//	
//	delete[] pSX;
//	delete[] pSY;
//	delete[] pEX;
//	delete[] pEY;
//
////	m_nOldCell = nProbe*nGroup;
//}

//void CUIMeasPathDlg::UpdateCellPosition2(RECIPEPATH *pRecipe, int iCellDir)
//{
//	int nGroup = 0;
//	nGroup = pRecipe->m_aryMeasGroup.GetSize();
//	if( nGroup == 0)
//	{
//		SetRemoveGlassCellLayout(-1);
//		m_nOldCell = 0;
//	}
//
//	nGroup = pRecipe->m_aryCellGroup.GetSize();
//	if( nGroup == 0)
//	{
//		SetRemoveGlassCellLayout(-1);
//		m_nOldCell = 0;
//		return; 
//	}
//
//	if (m_nOldCell > 0 )
//	{
//		SetRemoveGlassCellLayout(-1);
//	}
//
//	int nMaxPrbCntUsable = 0;
//	for (int i = 0; i < df_MAX_PROBE; i++)
//	{
//		nMaxPrbCntUsable++;
//	}
//
//	int nProbe = nMaxPrbCntUsable;
//
//	double m_posCmd_x = 0;
//	double m_posCmd_y = 0;
//	float  iCellX = 0;
//	float  iCellY = 0;
//	float  iCellDX = 0;
//	float  iCellDY = 0;
//	float  iCellOX = 0;
//	float  iCellOY = 0;
//	int    iBlockNum = 0;
//	float  iStartOX = 0;
//	float  iStartOY = 0;
//
//	int    iCol = 0;
//	int    iRow = 0;
//	float  iStartX = 0;
//	float  iStartY = 0;
//	int	   iCellNum=0;
//
//
//	CString	strCellId = _T("");
//	CString	strBlockId = _T("");
//
//	iBlockNum = pRecipe->stCellInfo.CellNum;
//	iCol	  = (int)pRecipe->stCellInfo.CellCount.x;
//	iRow	  = (int)pRecipe->stCellInfo.CellCount.y;
//	iStartX	  = (float)(pRecipe->stCellInfo.CellEdig.x + pRecipe->stCellInfo.GlobalMark.x);
//	iStartY	  = (float)(pRecipe->stCellInfo.CellEdig.y + pRecipe->stCellInfo.GlobalMark.y);
//
//	iCellX = pRecipe->stCellInfo.CellSize.x;
//	iCellY = pRecipe->stCellInfo.CellSize.y;
//	iCellDX = pRecipe->stCellInfo.CellDistance.x;
//	iCellDY = pRecipe->stCellInfo.CellDistance.y;
//
//
//	iStartOX = (iCellDX*iRow)+iCellOX;
//	iCellOX = pRecipe->stCellInfo.CellOffset.x;
//	iCellOY = pRecipe->stCellInfo.CellOffset.y;
//
//
//	if( pRecipe->m_nRotate == 0 )
//	{
//		iStartX;
//		iStartY;
//
//		iCellNum = iCol;
//	}
//	else if( pRecipe->m_nRotate == 1 )
//	{
//		float nTmp;
//		nTmp = iStartX;
//		iStartX = iStartY;
//		iStartY = nTmp;
//
//		iStartX *= -1;
//		iStartY;
//
//		iCellNum = iCol;
//	}
//	else if( pRecipe->m_nRotate == 2 )
//	{
//		iStartX *= -1;
//		iStartY *= -1;
//
//		iCellNum = iCol;
//	}
//	else if( pRecipe->m_nRotate == 3 )
//	{
//		float nTmp;
//		nTmp = iStartX;
//		iStartX = iStartY;
//		iStartY = nTmp;
//
//		iStartX;
//		iStartY *= -1;
//
//		iCellNum = iCol;
//	}
//
//	int iDX = 1, iDY = 1;
//	if(iStartX > 0)
//		iDX = -1;
//
//	if(iStartY > 0)
//		iDY = -1;
//
//	int iCELLIDUIDir = 0;
//	iCELLIDUIDir = iCellDir;
//
//	m_nOldCell = iCol*iRow;
//	double* pSX = new double[m_nOldCell];
//	double* pSY = new double[m_nOldCell];
//	double* pEX = new double[m_nOldCell];
//	double* pEY = new double[m_nOldCell];
//
//	CStringArray strArray;
//	strArray.RemoveAll();
//
//	int nTCell = 0, nCell=0, iBeforNum = 0;
//	CELLINFO	stCellGroup;
//	ZeroMemory(&stCellGroup, sizeof(stCellGroup));
//
//	
//	for (int iG=0; iG<nGroup; iG++)
//	{
//		stCellGroup = pRecipe->m_aryCellGroup.GetAt(iG);
//
//		if( pRecipe->m_nRotate == 0 )
//		{
//			iCol	= (int)stCellGroup.CellCount.x;
//			iRow	= (int)stCellGroup.CellCount.y;
//
//			iStartX = stCellGroup.CellOffset.x;
//			iStartY = stCellGroup.CellOffset.y;
//
//			iCellX  = stCellGroup.CellSize.x;
//			iCellY  = stCellGroup.CellSize.y;
//
//			iCellDX = stCellGroup.CellDistance.x;
//			iCellDY = stCellGroup.CellDistance.y;
//
//		}
//		else if( pRecipe->m_nRotate == 1 )
//		{
//			iRow	= (int)stCellGroup.CellCount.x;
//			iCol	= (int)stCellGroup.CellCount.y;
//
//			iStartY = -1*stCellGroup.CellOffset.x;
//			iStartX = stCellGroup.CellOffset.y;
//
//			iCellY  = -1*stCellGroup.CellSize.x;
//			iCellX  = stCellGroup.CellSize.y;
//
//			iCellDY = -1*stCellGroup.CellDistance.x;
//			iCellDX = -1*stCellGroup.CellDistance.y;
//
//		}
//		else if( pRecipe->m_nRotate == 2 )
//		{
//			iCol	= (int)stCellGroup.CellCount.x;
//			iRow	= (int)stCellGroup.CellCount.y;
//
//			iStartX = -1*stCellGroup.CellOffset.x;
//			iStartY = -1*stCellGroup.CellOffset.y;
//
//			iCellX  = -1*stCellGroup.CellSize.x;
//			iCellY  = -1*stCellGroup.CellSize.y;
//
//			iCellDX = stCellGroup.CellDistance.x;
//			iCellDY = stCellGroup.CellDistance.y;
//
//		}
//		else
//		{
//			iRow	= (int)stCellGroup.CellCount.x;
//			iCol	= (int)stCellGroup.CellCount.y;
//
//			iStartY = stCellGroup.CellOffset.x;
//			iStartX = -1*stCellGroup.CellOffset.y;
//
//			iCellY  = stCellGroup.CellSize.x;
//			iCellX  = -1*stCellGroup.CellSize.y;
//
//			iCellDY = -1*stCellGroup.CellDistance.x;
//			iCellDX = -1*stCellGroup.CellDistance.y;
//
//		}
//		
//		int iX=0, L = 65;
//		for (int k=0; k<iCol; k++)			// X	
//		{
//			for (int iY=0; iY<iRow; iY++)	// Y
//			{
//				m_posCmd_x = iStartX + ( iCellDX*iDX * k );
//				m_posCmd_y = iStartY + ( iCellDY*iDY * iY );
//
//				if( iCELLIDUIDir == 0)			// ++
//				{
//					if( pRecipe->m_nRotate == 0 || pRecipe->m_nRotate == 2 )
//					{
//						iX = iCellNum-k-1-(iBeforNum);
//
//						L = 65 + (iX/24);		// jsyoon 20190716 add
//
//						if( iX >= 24 )
//						{
//							iX -= 24;
//						}
//
//						// jsyoon 20180206 add
//						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//						if( iX >= 8 && iX < 13)	// "I" 제외
//							iX += 1;
//						else if( iX >= 13 )			// "O" 제외
//							iX += 2;
//						else
//							;
//
//						strCellId.Format(_T("%c%c%02d"), L, iX+65, iRow-iY);
//	//					strCellId.Format(_T("%c%02d", iX+65, iRow-iY);
//					}
//					else
//					{
//						iX = iY + iBeforNum;
//
//						L = 65 + (iX/24);		// jsyoon 20190716 add
//
//						if( iX >= 24 )
//						{
//							iX -= 24;
//						}
//
//						// jsyoon 20180206 add
//						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//						if( iX >= 8 && iX < 13)		// "I" 제외
//							iX += 1;
//						else if( iX >= 13 )			// "O" 제외
//							iX += 2;
//						else
//							;
//
//						strCellId.Format(_T("%c%c%02d"), L, iX+65, iCol-k);
//	//					strCellId.Format(_T("%c%02d", iX+65, iCol-k);
//					}
//
//// 					iX = iY + iBeforNum;
//// 
//// 					// jsyoon 20180206 add
//// 					// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//// 					if( iX >= 8 && iX < 13)		// "I" 제외
//// 						iX += 1;
//// 					else if( iX >= 13 )			// "O" 제외
//// 						iX += 2;
//// 					else
//// 						;
//// 
//// //					strCellId.Format(_T("%c%02d", iX+65, iY+1);
//// 					strCellId.Format(_T("%c%02d", iX+65, iCol-k); 
//
//				}
//				else if( iCELLIDUIDir == 1)		// +-
//				{
//					iX = iY + iBeforNum;
//
//					L = 65 + (iX/24);		// jsyoon 20190716 add
//
//					if( iX >= 24 )
//					{
//						iX -= 24;
//					}
//
////					iX = iCellNum-iY-1-(iBeforNum);
//
//					if( iX >= 8 && iX < 13)		// "I" 제외
//						iX += 1;
//					else if( iX >= 13 )			// "O" 제외
//						iX += 2;
//					else
//						;
//
//					strCellId.Format(_T("%c%c%02d"), L, iX+65, k+1);	// jsyoon 20190716 add
//	//				strCellId.Format(_T("%c%02d", iX+65, k+1);
//
////					strCellId.Format(_T("%c%02d",  iX+65, iCol-k); 
//				}
//				else if( iCELLIDUIDir == 2)		// -+
//				{
////					iX = k + nTCell;
//					iX = iCellNum-iY-1-(iBeforNum);
//
//					L = 65 + (iX/24);		// jsyoon 20190716 add
//
//					if( iX >= 24 )
//					{
//						iX -= 24;
//					}
//
//					if( iX >= 8 && iX < 13)		// "I" 제외
//						iX += 1;
//					else if( iX >= 13 )			// "O" 제외
//						iX += 2;
//					else
//						;
//
//					strCellId.Format(_T("%c%c%02d"), L, iX+65, k+1);	// jsyoon 20190716 add
//	//				strCellId.Format(_T("%c%02d", iX+65, k+1);
////					strCellId.Format(_T("%c%02d",  iX+65, iY+1);
//				}
//				else
//				{
//					if( pRecipe->m_nRotate == 0 || pRecipe->m_nRotate == 2 )
//					{
//						iX = iCellNum-k-1-(iBeforNum);
//
//						L = 65 + (iX/24);		// jsyoon 20190716 add
//
//						if( iX >= 24 )
//						{
//							iX -= 24;
//						}
//
//						// jsyoon 20180206 add
//						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//						if( iX >= 8 && iX < 13)	// "I" 제외
//							iX += 1;
//						else if( iX >= 13 )			// "O" 제외
//							iX += 2;
//						else
//							;
//
//						strCellId.Format(_T("%c%c%02d"), L, iX+65, iRow-iY);	// jsyoon 20190716 add
//		//				strCellId.Format(_T("%c%02d", iX+65, iRow-iY);
//					}
//					else
//					{
//						iX = iCellNum-iY-1-(iBeforNum);
//
//						L = 65 + (iX/24);		// jsyoon 20190716 add
//
//						if( iX >= 24 )
//						{
//							iX -= 24;
//						}
//
//						// jsyoon 20180206 add
//						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//						if( iX >= 8 && iX < 13)		// "I" 제외
//							iX += 1;
//						else if( iX >= 13 )			// "O" 제외
//							iX += 2;
//						else
//							;
//
//						strCellId.Format(_T("%c%c%02d"), L, iX+65, iCol-k);	// jsyoon 20190716 add
//	//					strCellId.Format(_T("%c%02d", iX+65, iCol-k);
//					}
//				}
//
//
//// 				if( iDY > 0)
//// 				{
//// 					if(iDX > 0)	
//// 					{
//// 						// jsyoon 20180206 add
//// 						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//// 						if( iX >= 8 && iX < 13)		// "I" 제외
//// 							iX += 1;
//// 						else if( iX >= 13 )			// "O" 제외
//// 							iX += 2;
//// 						else
//// 							;
//// 
//// 						strCellId.Format(_T("%c%02d", iX+65, iY+1);
//// 					}
//// 					else
//// 					{
//// 						// jsyoon 20180206 add
//// 						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//// 						iX = iCellNum-iY-1-(iBeforNum);
//// 
//// 						if( iX >= 8 && iX < 13)		// "I" 제외
//// 							iX += 1;
//// 						else if( iX >= 13 )			// "O" 제외
//// 							iX += 2;
//// 						else
//// 							;
//// 
//// 						//strCellId.Format(_T("%c%02d", iCellNum+64 - iX, iY+1);
//// 						//strCellId.Format(_T("%c%02d",  iY+65+(iBeforNum), k+1);
//// 						//strCellId.Format(_T("%c%02d",  iCellNum-iY+64-(iBeforNum), iCol-k); //lhj change 161103
//// 						strCellId.Format(_T("%c%02d",  iX+65, iCol-k); // jsyoon 20180206 change
//// 					}
//// 				}
//// 				else
//// 				{
//// 					if(iDX > 0)			// 270
//// 					{
//// 						// jsyoon 20180206 add
//// 						// BOE CELL ID "I" , "O" 사용하지 않으므로 제거 해줘야 함
//// 						iX = iCellNum-iY-1-(iBeforNum);
//// 
//// 						if( iX >= 8 && iX < 13)		// "I" 제외
//// 							iX += 1;
//// 						else if( iX >= 13 )			// "O" 제외
//// 							iX += 2;
//// 						else
//// 							;
//// 
//// 						strCellId.Format(_T("%c%02d", iX+65, iCol-k);		// jsyoon 20180206 change
//// 
//// //						strCellId.Format(_T("%c%02d", iX+65, iRow-iY);
//// //						strCellId.Format(_T("%c%02d", iY+65+(iBeforNum), k+1);
//// 					}
//// 
//// 					else				// 180
//// 					{
//// 						if( iX >= 8 && iX < 13)		// "I" 제외
//// 							iX += 1;
//// 						else if( iX >= 13 )			// "O" 제외
//// 							iX += 2;
//// 						else
//// 							;
//// 
//// 						strCellId.Format(_T("%c%02d",  iX+65, iY+1);
//// //						strCellId.Format(_T("%c%02d", iCellNum+64 - iX, iRow-iY);
//// 					}
//// 				}
//					
//				strArray.Add(strCellId);
//// 				pSX[nCell] = m_posCmd_x;
//// 				pSY[nCell] = m_posCmd_y;
//// 				pEX[nCell] = m_posCmd_x+iCellX;
//// 				pEY[nCell] = m_posCmd_y+iCellY;
//
//				// jsyoon 20160617 change
//				pSX[nCell] = m_posCmd_x-iCellX/2;
//				pSY[nCell] = m_posCmd_y-iCellY/2;
//				pEX[nCell] = m_posCmd_x+iCellX/2;
//				pEY[nCell] = m_posCmd_y+iCellY/2;
//				nCell++;
//			}
//
//		}
//
//		if( pRecipe->m_nRotate == 0 || pRecipe->m_nRotate == 2 )
//			iBeforNum += iCol;
//		else
//			iBeforNum += iRow;
//
//		nTCell += iCol;
//	}
//
//	m_nOldCell = nCell;
//
//	SetGlassCell(m_nOldCell, &strArray, pSX, pSY, pEX, pEY);
//
//	delete[] pSX;
//	delete[] pSY;
//	delete[] pEX;
//	delete[] pEY;
//
//	//	m_nOldCell = nProbe*nGroup;
//}

void CUIMeasPathDlg::UpdateCellPosition3(RECIPELAYOUT* pCellinfo)
{
	int nGroup = 0;
	nGroup = pCellinfo->m_iCellCnt;
	if( nGroup == 0)
	{
		SetRemoveGlassCellLayout(-1);
		m_nOldCell = 0;
		return; 
	}

	if (m_nOldCell > 0 )
	{
		SetRemoveGlassCellLayout(-1);
	}

	m_nOldCell = nGroup;

	double* pSX = new double[nGroup];
	double* pSY = new double[nGroup];
	double* pEX = new double[nGroup];
	double* pEY = new double[nGroup];
	COLORREF* pColor = new COLORREF[nGroup];

	CStringArray strArray;
	strArray.RemoveAll();
	CString strCellId;

	CELLLAYOUT stCellLayout;
	for (int i=0; i<nGroup; i++)
	{
		ZeroMemory(&stCellLayout, sizeof(stCellLayout));

		if( i< pCellinfo->arrCellLayout.GetSize())
			stCellLayout = pCellinfo->arrCellLayout.GetAt(i);

		strCellId.Format(_T("%s"), stCellLayout.szCellName);
		strArray.Add(strCellId);

		strCellId.Format(_T("%s"), stCellLayout.szCellText);
		strArray.Add(strCellId);

		pSX[i] = stCellLayout.dSDx;
		pSY[i] = stCellLayout.dSDy;
		pEX[i] = stCellLayout.dEDx;
		pEY[i] = stCellLayout.dEDy;
		pColor[i] = stCellLayout.color;
	}

	SetGlassCell(nGroup, &strArray, pSX, pSY, pEX, pEY, pColor);

	delete[] pSX;
	delete[] pSY;
	delete[] pEX;
	delete[] pEY;
	delete[] pColor;
}

void CUIMeasPathDlg::UpdateOrbitArea(int iMarker, int SetNum, double X, double Y)
{
	m_chartOrbit.UpdateArea(iMarker, SetNum, X, Y);
}

void CUIMeasPathDlg::SetAxisShow(BOOL bVal)
{
	m_chartOrbit.SetAxisShow(bVal);
}

void CUIMeasPathDlg::SetGlassVacuum(double* pVacuum, BOOL bUse, int nCnt)
{
	m_chartOrbit.SetGlassVacuum(pVacuum, bUse, nCnt);
}

void CUIMeasPathDlg::ClearMeasResultChart()
{
	//	m_chartOrbit.ClearResultText();
}

void CUIMeasPathDlg::UpdateMeasResultChart(int nMeasArea, int iCurMeasArea, double dXPos, double dYPos, CString strSH, int iColor)
{
	//	m_chartOrbit.Update3DMeasResultChart(nMeasArea, iCurMeasArea, dXPos, dYPos, strSH, iColor);
}

void CUIMeasPathDlg::UpdateCurCoordA(int nProbe, double* px, double* py)
{
	m_chartOrbit.SetCurPosA(nProbe, TRUE, px, py);
}

void CUIMeasPathDlg::SetGlassMeasPos(int nIndex, double* dSX, double* dSY, double* dEX, double* dEY)
{
	m_chartOrbit.SetGlassMeasPos(nIndex, dSX, dSY, dEX, dEY);
}

int CUIMeasPathDlg::SetFixtureMoveTime(double* pTime, BOOL bUse, int nCnt)
{
	m_chartOrbit.SetFixtureMoveTime(pTime, bUse, nCnt);
	return 0;
}
