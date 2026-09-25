#include "pch.h"
#include "MySpreadDll.h"

#if !defined(USER_COLOR_T_DEFINED)
#define USER_COLOR_T_DEFINED

// standard color
#define RGB_WHITE			RGB(255, 255, 255)
#define RGB_BLACK			RGB(0, 0, 0)
#define RGB_RED				RGB(255, 0, 0)
#define RGB_GREEN			RGB(0, 255, 0)
#define RGB_BLUE			RGB(0, 0, 255)
#define RGB_CYAN			RGB(0, 255, 255)
#define RGB_MAGENTA			RGB(255, 0, 255)
#define RGB_YELLOW			RGB(255, 255, 0)
#define RGB_DEEP_GREEN		RGB(0, 128, 0)
#define	RGB_GRAY			RGB(128, 128, 128)
#define	RGB_LIGHT_GRAY		RGB(192, 192, 192)
#define	RGB_DARK_GRAY		RGB(70, 70, 70)
#define	RGB_ORANGE			RGB(255, 128, 0)

// custom color
#define RGB_MAIN			RGB(225, 231, 253)	// main back.	  (흐린파랑부분)
#define RGB_MAIN2			RGB(119, 161, 224)	// 테두리 프레임. (진한파랑부분)
#define RGB_CHILD			RGB(240, 219, 251)	// light-pink : Child Dialog

#define RGB_LIGHT_BLUE1		RGB(190, 210, 240)	// grid header
#define RGB_LIGHT_BLUE2		RGB(218, 230, 248)
#define RGB_LIGHT_BLUE3		RGB(239, 242, 254)	// grid back
#define RGB_DARK_BLUE1		RGB(24, 44, 123)	// text
#define RGB_DARK_BLUE2		RGB(25, 65, 139)
#define RGB_DARK_BLUE3		RGB(60, 101, 193)
#define RGB_LIGHT_PINK		RGB(226, 197, 245)	// grid Sel.

#endif	// USER_COLOR_T_DEFINED


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMySpreadDll::CMySpreadDll(UINT uComponentID, UINT uid, CWnd* pParentWnd, BOOL bUseMessage/*=FALSE*/)
{
	m_pParentWnd	= pParentWnd;
	m_bUseMessage	= bUseMessage;
	m_uComponentID	= uComponentID;

	CRect rect;
	CWnd* pWnd = pParentWnd->GetDlgItem(uComponentID);
	pWnd->GetClientRect(&rect);

	Create(0x50010000, rect, pWnd, uid);
}

CMySpreadDll::~CMySpreadDll()
{
}

BEGIN_MESSAGE_MAP(CMySpreadDll, TSpread)
	//{{AFX_MSG_MAP(CMySpreadDll)
	//spreadDll message
	ON_MESSAGE(SSM_CLICK,		OnFPSpreadClick)
	ON_MESSAGE(SSM_DBLCLK,		OnFPSpreadDblClick)
	ON_MESSAGE(SSM_DATACHANGE,	OnFPSpreadDataChange)
	//}}AFX_MSG_MAP	
END_MESSAGE_MAP()

void CMySpreadDll::InitMySpread_SACCIO(int nFilter)
{
	//
	SetMaxCols(4);
	SetMaxRows(10);
	SetColWidth(SS_ALLCOLS, 14.2);
	SetRowHeight(SS_ALLROWS, 12.5);
	SetColWidth(SS_HEADER, 5.1);
	SetRowHeight(SS_HEADER, 14.5);

	CString strtemp;

	//Header
	SetData(1, SS_HEADER, _T("SACC"));
	SetData(2, SS_HEADER, _T("SACC"));
	SetData(3, SS_HEADER, _T("EQ"));
	SetData(4, SS_HEADER, _T("EQ"));

	//
	SetData(1, 1, _T("Auto Mode"));
	SetData(1, 2, _T("Manual Mode"));
	SetData(1, 3, _T("Alarm"));
	SetData(1, 4, _T("Ready"));
	SetData(1, 5, _T("Load Buffer High"));
	SetData(1, 6, _T("Load Buffer Low"));
	SetData(1, 7, _T("UnLoad Buffer High"));
	SetData(1, 8, _T("UnLoad Buffer Low"));

	//
	SetData(2, 1, _T("CMD Check Ack"));
	SetData(2, 2, _T("CMD Check Nak"));
	SetData(2, 3, _T("Busy"));
	SetData(2, 4, _T("Entry Request"));
	SetData(2, 5, _T("Complete Ack"));
	SetData(2, 6, _T("Handfold Ack"));
	SetData(2, 10, _T("Sequence Done"));

	//
	SetData(3, 1, _T("Auto Mode"));
	SetData(3, 2, _T("Manual Mode"));
	SetData(3, 3, _T("Load Slot #1"));
	SetData(3, 4, _T("Load Slot #2"));
	SetData(3, 5, _T("Load Slot #3"));
	SetData(3, 6, _T("Load Slot #4"));
	SetData(3, 7, _T("UnLoad Slot #1"));
	SetData(3, 8, _T("UnLoad Slot #2"));
	SetData(3, 9, _T("UnLoad Slot #3"));
	SetData(3, 10, _T("UnLoad Slot #4"));

	//
	SetData(4, 1, _T("Loding"));
	SetData(4, 2, _T("UnLoding"));
	SetData(4, 3, _T("Exchange"));
	SetData(4, 7, _T("CMD Start"));
	SetData(4, 8, _T("Busy"));
	SetData(4, 9, _T("Complete"));	
}

void CMySpreadDll::InitMySpread_SACCBF(int nFilter)
{
	//
	SetMaxCols(3);
	SetMaxRows(2);
	SetColWidth(SS_ALLCOLS, 8);
	SetColWidth(SS_HEADER, 5.1);
	SetColWidth(1, 21.5);

	SetRowHeight(SS_ALLROWS, 14.5);	

	SetData(1, SS_HEADER, _T("PROBE ID"));
	SetData(2, SS_HEADER, _T("UP"));
	SetData(3, SS_HEADER, _T("DOWN"));
}

void CMySpreadDll::InitMySpread_SACCID(int nFilter)
{
	//
	SetMaxCols(3);
	// LYS 2026-09-16 Probe Cassette 5 -> 7 slots (CDlgSACC re-reads [PROBESLOT] MAXPROBE and calls SetMaxRows again)
	//SetMaxRows(5);
	SetMaxRows(7);
	SetColWidth(SS_HEADER, 5.1);
	SetColWidth(1, 23);
	SetColWidth(2, 7);
	SetColWidth(3, 7);
//	SetColWidth(4, 7);

	SetRowHeight(SS_HEADER, 25);
	SetRowHeight(SS_ALLROWS, 12.8);
	//SetRowHeight(SS_ALLROWS, 12.0);	// LYS 2026-09-16 7 rows must fit the (enlarged) control without scrolling

	SetData(1, SS_HEADER, _T("PROBE ID"));
	SetData(2, SS_HEADER, _T("LOAD"));
	SetData(3, SS_HEADER, _T("UNLOAD"));
//	SetData(4, SS_HEADER, _T("SACC \n현위치"));
}

void CMySpreadDll::InitMySpread_InsideProbe(int nFilter)
{
	//
	SetMaxCols(1);
	SetMaxRows(5);
	SetColWidth(SS_HEADER, 5.1);
	SetColWidth(1, 23);
	SetRowHeight(SS_ALLROWS, 14.5);
	SetData(1, SS_HEADER, _T("PROBE ID"));
}

void CMySpreadDll::InitMySpread_AGINGSET(int nFilter)
{
	SetMaxCols(5);
	SetMaxRows(0);
	SetColWidth(SS_ALLCOLS, 10);
	SetColWidth(SS_HEADER, 5.1);

	SetData(0, SS_HEADER, _T("No"));
	SetData(1, SS_HEADER, _T("Use"));
	SetData(2, SS_HEADER, _T("점등패턴"));
	SetData(3, SS_HEADER, _T("기준값(mA)"));
	SetData(4, SS_HEADER, _T("상한값(mA)"));
	SetData(5, SS_HEADER, _T("하한값(mA)"));
}

void CMySpreadDll::InitMySpread_AGINGTEST_MEAS(int nFilter)
{
	SetMaxCols(10);
	SetMaxRows(5);
	SetColWidth(SS_ALLCOLS, 8);
	SetColWidth(SS_HEADER, 5);
	SetRowHeight(SS_ALLROWS, 20);

	CString sTemp;
	char cHeader = 'A';
	for (int iCol = 0; iCol < GetMaxCols(); iCol++)
	{
		sTemp.Format(_T("%c"), cHeader + iCol);
		SetData(iCol + 1, 0, sTemp);
	}
}

void CMySpreadDll::InitMySpread_AGINGTEST_LIST(int nFilter)
{
	SetMaxCols(6);	// LYS 2026-08-06 Phase 2 +ANGLE
	SetMaxRows(0);
	SetColWidth(SS_ALLCOLS, 10);
	SetColWidth(SS_HEADER, 5.1);

	SetData(0, SS_HEADER, _T("No"));
	SetData(1, SS_HEADER, _T("Aging"));
	SetData(2, SS_HEADER, _T("점등패턴"));
	SetData(3, SS_HEADER, _T("점등시간(Sec)"));
	SetData(4, SS_HEADER, _T("특성"));
	SetData(5, SS_HEADER, _T("화질"));
	SetData(6, SS_HEADER, _T("시야각"));	// LYS 2026-08-06 Phase 2
}

void CMySpreadDll::InitMySpread_AUTOCONTACT(int nFilter)
{
	SetMaxCols(1);
	SetMaxRows(8);
	SetColWidth(SS_ALLCOLS, 10);
	SetColWidth(SS_HEADER, 30);
	SetRowHeight(SS_ALLROWS, 14.5);

	//COL
	SetData(0, SS_HEADER, _T("항목"));
	SetData(1, SS_HEADER, _T("설정값"));

	//ROW
	SetData(SS_HEADER, 1, _T("Ready Position(mm)"));
	SetData(SS_HEADER, 2, _T("1차 Limit Position(mm)"));
	SetData(SS_HEADER, 3, _T("1차 Move Gap(mm)"));
	SetData(SS_HEADER, 4, _T("2차 Move Gap(mm)"));
	SetData(SS_HEADER, 5, _T("OD Max(mm)"));
	SetData(SS_HEADER, 6, _T("R-TEST 성공률(%)"));
	SetData(SS_HEADER, 7, _T("신호기 연결이상"));
	SetData(SS_HEADER, 8, _T("Add OD(mm)"));
}

void CMySpreadDll::InitMySpread_MEAS_PROGRESS(int nFilter)
{
	int iCurRow = 1;
	CString strTemp;

	double dMaxCol = 168;
	double dMaxRow = 450;

	SetMaxCols(nFilter + 1);
	SetMaxRows(15);

	double dColSize = (double)(dMaxCol / GetMaxCols());
	double dRowSize = (double)(dMaxRow / GetMaxRows());

	SetColWidth(SS_ALLCOLS,  dColSize);
	SetRowHeight(SS_ALLROWS, dRowSize);

	for (int iRow = 1; iRow < GetMaxRows(); iRow++)
	{
		if(iRow % 3 != 0)
			SetBorderRange(2, iRow, GetMaxCols(), iRow, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, RGB_BLACK); //셀 사이 라인 색(BOTTOM)		

		//if(iRow % 3 == 0)
		//	SetBorderRange(2, iRow, GetMaxCols(), iRow, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, RGB_BLACK); //셀 사이 라인 색(BOTTOM)		
	}

	for(int iCol = 1; iCol < GetMaxCols(); iCol++)
		SetBorderRange(iCol, 1, iCol, GetMaxRows(), SS_BORDERTYPE_RIGHT, SS_BORDERSTYLE_SOLID, RGB_GRAY_120); //셀 사이 라인 색(RIGHT)
	
	//---------------------------------------------------------------------
	// FONT Type
	//---------------------------------------------------------------------
	HFONT hfData;
	MakeFont(&hfData, _T("Tahoma"), 15, TRUE);	
	SetFont(SS_ALLCOLS, SS_ALLROWS, hfData, TRUE);
	
	for (int iRow = 1; iRow <= GetMaxRows() / 3; iRow++)
	{
		//---------------------------------------------------------------------
		//Spread CELL Color 지정 
		//---------------------------------------------------------------------
		SetColorRange(1, iCurRow, 1, iCurRow + 2, RGB_LIGHT_GRAY, RGB_DARK_PINK);		  //Probe 부분 
		SetColorRange(2, iCurRow, GetMaxCols(), iCurRow + 2, RGB_BLACK, RGB(210,210,0));  //CELL  부분 					
		SetBorderRange(1, iCurRow + 2, GetMaxCols(), iCurRow + 2, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, RGB_GRAY_120); //셀 사이 라인 색(BOTTOM)

		//---------------------------------------------------------------------
		//PROBE Number 
		//---------------------------------------------------------------------
		strTemp.Format(_T("PROBE %d"), iRow);
		SetData(1, iCurRow, strTemp);

		//---------------------------------------------------------------------
		//CELL Number 
		//---------------------------------------------------------------------
		for (int iCol = 2; iCol <= GetMaxCols(); iCol++)
		{
			strTemp.Format(_T("%d"), iCol - 1);
			SetData(iCol, iCurRow, strTemp);
		}
		iCurRow += 3;
	}	
}

void CMySpreadDll::InitMySpread_RESULT(int nFilter)
{
	//SetMaxCols(1);
	//SetMaxRows(8);
	//SetColWidth(SS_ALLCOLS, 10);
	//SetColWidth(SS_HEADER, 30);
	//SetRowHeight(SS_ALLROWS, 14.5);
	//
	////COL
	//SetData(0, SS_HEADER, _T("항목"));
	//SetData(1, SS_HEADER, _T("설정값"));
	//
	////ROW
	//SetData(SS_HEADER, 1, _T("Ready Position(mm)"));
	//SetData(SS_HEADER, 2, _T("1차 Limit Position(mm)"));
	//SetData(SS_HEADER, 3, _T("1차 Move Gap(mm)"));
	//SetData(SS_HEADER, 4, _T("2차 Move Gap(mm)"));
	//SetData(SS_HEADER, 5, _T("OD Max(mm)"));
	//SetData(SS_HEADER, 6, _T("R-TEST 성공률(%)"));
	//SetData(SS_HEADER, 7, _T("신호기 연결이상"));
	//SetData(SS_HEADER, 8, _T("Add OD(mm)"));
}

//===========================================================================
// event-message
//===========================================================================
LRESULT CMySpreadDll::OnFPSpreadClick(WPARAM wParam, LPARAM lParam)
{
	if(!m_bUseMessage) return 0;
	m_pParentWnd->SendMessage(SSM_CLICK, (WPARAM)m_uComponentID, (LPARAM)lParam);
	return 0;
}

LRESULT CMySpreadDll::OnFPSpreadDataChange(WPARAM wParam, LPARAM lParam)
{
	if(!m_bUseMessage) return 0;
	m_pParentWnd->SendMessage(SSM_DATACHANGE, (WPARAM)m_uComponentID, (LPARAM)lParam);
	return 0;
}

LRESULT CMySpreadDll::OnFPSpreadDblClick(WPARAM wParam, LPARAM lParam)
{
	if(!m_bUseMessage) return 0;
	m_pParentWnd->SendMessage(SSM_DBLCLK, (WPARAM)m_uComponentID, (LPARAM)lParam);
	return 0;
}

//===========================================================================
// common
//===========================================================================
CString CMySpreadDll::GetDataString(int col, int row)
{
	TCHAR data[256];
	GetData(col, row, data);
	return (CString)data;
}

int CMySpreadDll::GetDataInt(int col, int row)
{
	TCHAR data[256];
	GetData(col, row, data);
	return _ttoi(data);
}

double CMySpreadDll::GetDataDbl(int col, int row)
{
	TCHAR data[256];
	GetData(col, row, data);
	return _ttof(data);
}

void CMySpreadDll::SetDataString(int col, int row, CString data)
{
	SetData(col, row, data);
}

void CMySpreadDll::MakeFont(HFONT* pFont, CString strName/*="Tahoma"*/, int iSize/*=14*/, BOOL bBold/*=FALSE*/, BOOL bItalic/*=FALSE*/)
{
	*pFont = CreateFont(iSize,				// nHeight
						0,					// nWidth
						0,					// nEscapement
						0,					// nOrientation
						(bBold?FW_BOLD:FW_NORMAL),		// fnWeight
						bItalic,			// tbItalic
						FALSE,				// tbUnderline
						FALSE,				// tbStrike
						DEFAULT_CHARSET,		// tbCharSet
						OUT_DEFAULT_PRECIS,		// tbOutputPrecision
						CLIP_DEFAULT_PRECIS,	// tbClipPrecision
						DEFAULT_QUALITY,		// tbQuality
						DEFAULT_PITCH | FF_DONTCARE,	// tbPitchAndFamily
						strName					// lpszFace
						);
}

// blue 2009.12.01 ========================================
// 
// 읽기모드
// scroll : when needed
// cell move, resize : x
//=========================================================
void CMySpreadDll::InitDefault()
{
	// 무조건 fix...
	SetAppearance(SS_APPEARANCE_FLAT);			// Control's Border
	SetCursor(SS_CURSORTYPE_DEFAULT, SS_CURSOR_ARROW);
	SetGrayAreaColor(RGB_WHITE, RGB_WHITE);		// Foreground parameter has no effect.

	// 나중에 봐서.. 뺄라믄 빼구..
	SetOperationMode(SS_OPMODE_NORMAL);
	SetBackColorStyle(SS_BACKCOLORSTYLE_UNDERGRID);

	SetBool(SSB_SCROLLBAREXTMODE,	TRUE);		// show only when needed.
	SetBool(SSB_HSCROLLBARTRACK,	TRUE);
	SetBool(SSB_VSCROLLBARTRACK,	TRUE);
	SetBool(SSB_ALLOWCOLMOVE,		FALSE);
	SetBool(SSB_ALLOWROWMOVE,		FALSE);
	SetBool(SSB_ALLOWUSERRESIZE,	FALSE);

	SetGridColor(RGB_GRAY_200);	// 데이터부분 라인
	//SetShadowColor(RGB(233,238,244), RGB_DARK_GRAY, RGB_GRAY_190, RGB(233,238,244)); // 헤더부분 배경, 글씨, 라인_어두운부분(주), 라인_밝은부분
	SetShadowColor(RGB_CHILD, RGB_DARK_GRAY, RGB_GRAY_190, RGB_LIGHT_BEIGE2);
	SetColor(SS_ALLCOLS, SS_ALLROWS, RGB_LIGHT_BEIGE, RGB_DARK_GRAY); // 데이터부분 배경, 글씨
	SetSelColor(RGB_LIGHT_BLUE4, RGB_BLACK); // 선택라인	

	MakeFont(&m_hfont);
}

void CMySpreadDll::InitDefault_Progress()
{
	// 무조건 fix...
	SetAppearance(SS_APPEARANCE_DEFAULT);			// Control's Border
	SetCursor(SS_CURSORTYPE_DEFAULT, SS_CURSOR_ARROW);
	SetGrayAreaColor(RGB_GRAY_240, RGB_GRAY_240);		// Foreground parameter has no effect.

	// 나중에 봐서.. 뺄라믄 빼구..
	SetOperationMode(SS_OPMODE_READONLY);
	SetBackColorStyle(SS_BACKCOLORSTYLE_UNDERGRID);

	SetBool(SSB_SCROLLBAREXTMODE, TRUE);		// show only when needed.
	SetBool(SSB_HSCROLLBARTRACK, TRUE);
	SetBool(SSB_VSCROLLBARTRACK, TRUE);
	SetBool(SSB_ALLOWCOLMOVE, FALSE);
	SetBool(SSB_ALLOWROWMOVE, FALSE);
	SetBool(SSB_ALLOWUSERRESIZE, FALSE);
	SetBool(SSB_SHOWCOLHEADERS, FALSE);
	SetBool(SSB_SHOWROWHEADERS, FALSE);

	SetGridColor(RGB_GRAY_200);	// 데이터부분 라인
	SetShadowColor(RGB_BLACK, RGB_DARK_GRAY, RGB_BLACK, RGB_BLACK); // 헤더부분 배경, 글씨, 라인_어두운부분(주), 라인_밝은부분
	//SetColor(SS_ALLCOLS, SS_ALLROWS, RGB_DARK_GRAY, RGB_DARK_GRAY); // 데이터부분 배경, 글씨
	SetSelColor(RGB_LIGHT_BLUE4, RGB_BLACK); // 선택라인		

	MakeFont(&m_hfont);
}

//===========================================================================
// for MetroSys
//===========================================================================

//=========================================================
// CRecipeView
void CMySpreadDll::InitMySpread_RecipeList() 
{
	InitDefault();
	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	SS_CELLTYPE cellType;
	SetTypeStaticText(&cellType, SS_TEXT_VCENTER | SS_TEXT_LEFT);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title
	SetData(0, SS_HEADER, _T("No."));
	SetData(1, SS_HEADER, _T("File Name"));

	// grid size
	SetMaxCols(1);
	SetMaxRows(1);

	SetRowHeight(SS_ALLROWS, 18.);
	SetColWidth(0, 5.0);
	SetColWidth(1, 48.0);
}

//=========================================================
// CAlignView
void CMySpreadDll::InitMySpread_AlignPos() 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_ROWMODE);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);


	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);
	
	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_RIGHT, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title
	SetData(0, SS_HEADER, _T("No."));
	SetData(1, SS_HEADER, _T("X"));
	SetData(2, SS_HEADER, _T("Y"));
	SetData(3, SS_HEADER, _T("Z"));

	// grid size
	SetMaxCols(3);
	SetMaxRows(1);

	SetRowHeight(SS_ALLROWS, 18.5);
	SetColWidth(0, 5.0);
	SetColWidth(1, 12.0);
	SetColWidth(2, 12.0);
	SetColWidth(3, 12.0);
}

void CMySpreadDll::InitMySpread_AreaPos() 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_ROWMODE);
	//SetOperationMode(SS_OPMODE_NORMAL);	// 이건 grid에서 포커스가 사라지믄, highlight도 사라진다.

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
// 	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);

	SetBool(SSB_AUTOCLIPBOARD,	TRUE);
	SetClipboardOptions(SS_CLIP_NOHEADERS);


	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_RIGHT, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title
	SetData(0, SS_HEADER, _T("No."));
	SetData(1, SS_HEADER, _T("X"));
	SetData(2, SS_HEADER, _T("Y"));
	SetData(3, SS_HEADER, _T("Z"));

	// grid size
	SetMaxCols(3);
	SetMaxRows(1);

	SetRowHeight(SS_ALLROWS, 16.0);
	SetColWidth(0, 5.1);
	SetColWidth(1, 11.2);
	SetColWidth(2, 11.2);
	SetColWidth(3, 11.2);
}

//=========================================================
// COperateView
void CMySpreadDll::InitMySpread_Result() 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_SINGLESEL);
	
	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_RIGHT, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title & width
	SetColHeaderRows(2);
	SetRowHeight(SS_ALLROWS, 16.0);
	double dw1 = 7.3;
	double dw2 = 7.5;

	int icol = -1;
	AddCellSpan(SS_HEADER, SS_HEADER, 1, 2);
	SetData(++icol, SS_HEADER, _T("No."));				SetColWidth(icol, 5.0);

	AddCellSpan	(icol+1, SS_HEADER, 3, 1);
	SetData		(icol+1, SS_HEADER, _T("Aperture Ratio"));
	SetData(++icol, SS_HEADER+1, _T("R"));				SetColWidth(icol, dw1);
	SetData(++icol, SS_HEADER+1, _T("G"));				SetColWidth(icol, dw1);
	SetData(++icol, SS_HEADER+1, _T("B"));				SetColWidth(icol, dw1);

	AddCellSpan	(icol+1, SS_HEADER, 3, 1);
	SetData		(icol+1, SS_HEADER, _T("0 Degree"));
	SetData(++icol, SS_HEADER+1, _T("Y"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER+1, _T("x"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER+1, _T("y"));				SetColWidth(icol, dw2);

	AddCellSpan	(icol+1, SS_HEADER, 3, 1);
	SetData		(icol+1, SS_HEADER, _T("30 Degree"));
	SetData(++icol, SS_HEADER+1, _T("Y"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER+1, _T("x"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER+1, _T("y"));				SetColWidth(icol, dw2);

	AddCellSpan(icol + 1, SS_HEADER, 3, 1);
	SetData(icol + 1, SS_HEADER, _T("45 Degree"));
	SetData(++icol, SS_HEADER + 1, _T("Y"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER + 1, _T("x"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER + 1, _T("y"));				SetColWidth(icol, dw2);

	AddCellSpan(icol + 1, SS_HEADER, 3, 1);
	SetData(icol + 1, SS_HEADER, _T("60 Degree"));
	SetData(++icol, SS_HEADER + 1, _T("Y"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER + 1, _T("x"));				SetColWidth(icol, dw2);
	SetData(++icol, SS_HEADER + 1, _T("y"));				SetColWidth(icol, dw2);	

	// grid size
	SetMaxCols(icol);
	SetMaxRows(0);

	// oddEvenCol color
	SetColorRange(4, SS_ALLROWS, 6, SS_ALLROWS, RGB_LIGHT_BEIGE2, RGB_DARK_GRAY);
	SetColorRange(10, SS_ALLROWS, 12, SS_ALLROWS, RGB_LIGHT_BEIGE2, RGB_DARK_GRAY);
}

void CMySpreadDll::InitMySpread_MeasCCD() 
{
	InitDefault();
	//SetOperationMode(SS_OPMODE_SINGLESEL);
	SetOperationMode(SS_OPMODE_ROWMODE);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);


	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_RIGHT, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title
// 	int icol = -1;
// 	SetData(++icol, SS_HEADER, _T("Src"));
// 	SetData(++icol, SS_HEADER, _T("W"));
// 	SetData(++icol, SS_HEADER, _T("R"));
// 	SetData(++icol, SS_HEADER, _T("G"));
// 	SetData(++icol, SS_HEADER, _T("B"));

	SetData(0, SS_HEADER, _T(""));
	SetData(1, SS_HEADER, _T("Exp.Time"));
	SetData(2, SS_HEADER, _T("Gain"));
	
	// grid size
	SetMaxCols(2);
	SetMaxRows(0);

	SetRowHeight(SS_ALLROWS, 16.0);
	SetColWidth(0, 7.0);
	SetColWidth(1, 8.0);
	SetColWidth(2, 7.0);
}

// MeasPBView에 있는거. 현재 결과값 표시하는거.
// void CMySpreadDll::InitMySpread_MeasPBResult_curr() 
// {
// 	InitDefault();
// 	SetOperationMode(SS_OPMODE_READONLY);
// 
// 	SetBool(SSB_HORZSCROLLBAR,	FALSE);
// 	SetBool(SSB_VERTSCROLLBAR,	FALSE);
// 	SetBool(SSB_AUTOSIZE,		TRUE);
// 
// 	//SetBool(SSB_SHOWCOLHEADERS,	FALSE);
// 	SetBool(SSB_SHOWROWHEADERS,	FALSE);
// 
// 	//SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);
// 	
// 
// 	SS_CELLTYPE cellType;
// 	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 10, NULL, SS_CASE_NOCASE);
// 	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);
// 
// 
// 	SetShadowColor(RGB_CHILD, RGB_DARK_GRAY, RGB_GRAY_190, RGB_LIGHT_BEIGE2);
// 
// 	// grid size
// 	SetMaxCols(3);
// 	SetMaxRows(1);
// 
// 	SetRowHeight(SS_HEADER, 16.0);
// 	SetRowHeight(1, 28.0);
// 
// 	SetColWidth(SS_ALLCOLS, 13.0);
// 
// 	// title
// 	//SetColor(1, SS_ALLROWS, clrTitle, RGB_BLACK);
// 	SetFont	(SS_ALLCOLS, SS_HEADER, m_hfont, TRUE);
// 
// 	// data
// 	HFONT hfData;
// 	MakeFont(&hfData, "Tahoma", 28, TRUE);
// 	SetColor(1, 1, RGB_LIGHT_BEIGE, RGB_RED_TXT);
// 	SetColor(2, 1, RGB_LIGHT_BEIGE, RGB_GREEN_TXT);
// 	SetColor(3, 1, RGB_LIGHT_BEIGE, RGB_BLUE_TXT);
// 	SetFont (SS_ALLCOLS, 1, hfData, TRUE);
// 
// 	SetData(1, SS_HEADER, _T("R"));
// 	SetData(2, SS_HEADER, _T("G"));
// 	SetData(3, SS_HEADER, _T("B"));
// }

// blue 2018.01.22 세로로..
void CMySpreadDll::InitMySpread_MeasPBResult_curr() 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_READONLY);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);

	SetBool(SSB_SHOWCOLHEADERS,	FALSE);
	//SetBool(SSB_SHOWROWHEADERS,	FALSE);

	//SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);


	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);


	SetShadowColor(RGB_CHILD, RGB_DARK_GRAY, RGB_GRAY_190, RGB_LIGHT_BEIGE2);

	// grid size
	SetMaxCols(1);
	SetMaxRows(3);

	SetRowHeight(SS_ALLROWS, 30.0);
	//SetRowHeight(1, 28.0);

	SetColWidth(SS_ALLCOLS, 13.0);

	// title
	//SetColor(1, SS_ALLROWS, clrTitle, RGB_BLACK);
	SetFont	(SS_ALLCOLS, SS_HEADER, m_hfont, TRUE);

	// data
	HFONT hfData;
	MakeFont(&hfData, _T("Tahoma"), 28, TRUE);
	SetColor(1, 1, RGB_LIGHT_BEIGE, RGB_RED_TXT);
	SetColor(1, 2, RGB_LIGHT_BEIGE, RGB_GREEN_TXT);
	SetColor(1, 3, RGB_LIGHT_BEIGE, RGB_BLUE_TXT);
	SetFont (1, SS_ALLROWS, hfData, TRUE);

	SetData(SS_HEADER, 1, _T("R"));
	SetData(SS_HEADER, 2, _T("G"));
	SetData(SS_HEADER, 3, _T("B"));
}

// MeasPBView에 있는거. 현재 결과값 표시하는거.
void CMySpreadDll::InitMySpread_MeasSpecResult_curr(int nCol/*=1*/) 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_READONLY);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);

// 	SetBool(SSB_SHOWCOLHEADERS,	FALSE);
// 	SetBool(SSB_SHOWROWHEADERS,	FALSE);

	//SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);


	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	SetSelColor(RGB_LIGHT_BEIGE, RGB_BLACK);	// 헤더선택하믄..선택색상으로 바껴서..바꿔주자..
	SetShadowColor(RGB_CHILD, RGB_DARK_GRAY, RGB_GRAY_190, RGB_LIGHT_BEIGE2);

	// grid size
	SetMaxCols(nCol);
	SetMaxRows(3);
// 	SetMaxCols(3);
// 	SetMaxRows(nRow);

	SetRowHeight(SS_HEADER, 20.0);
	SetRowHeight(SS_ALLROWS, 32.0);

	SetColWidth(SS_HEADER, 11.0);
	SetColWidth(SS_ALLCOLS, 15.0);

	// title
	//SetColor(1, SS_ALLROWS, clrTitle, RGB_BLACK);
	//SetFont	(SS_ALLCOLS, SS_HEADER, m_hfont, TRUE);	// row-header

	HFONT hfData;	// col-header
	MakeFont(&hfData, _T("Tahoma"), 18, FALSE);
	SetFont (SS_ALLCOLS, SS_HEADER, hfData, TRUE);

	MakeFont(&hfData, _T("Tahoma"), 20, FALSE);
	SetFont (SS_HEADER, SS_ALLROWS, hfData, TRUE);

	// data	
	MakeFont(&hfData, _T("Tahoma"), 25, TRUE);
	SetColor(SS_ALLCOLS, SS_ALLROWS, RGB_LIGHT_BEIGE, RGB_BLACK);
	SetFont (SS_ALLCOLS, SS_ALLROWS, hfData, TRUE);


	SetData(SS_HEADER, 1, _T("Y (cd/㎡)"));
	SetData(SS_HEADER, 2, _T("x"));
	SetData(SS_HEADER, 3, _T("y"));

	// 3x2
// 	SetData(1, SS_HEADER, _T("Y (cd/㎡)"));
// 	SetData(2, SS_HEADER, _T("x"));
// 	SetData(3, SS_HEADER, _T("y"));
// 
// 	SetData(SS_HEADER, 1, _T(STR_SPEC_NAME_CH1));
// 	SetData(SS_HEADER, 2, _T(STR_SPEC_NAME_CH2));
}


//=========================================================
// CSetup_UserDlg, CPasswordDlg
void CMySpreadDll::InitMySpread_User() 
{
	InitDefault();
	SetBool(SSB_PROCESSTAB, TRUE);
	//SetBool(SSB_AUTOSIZE,	TRUE);
	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	// header title
	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_LEFT, 20, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	SetData(0, SS_HEADER, _T("No."));
	SetData(1, SS_HEADER, _T("User Mode"));

	// grid size
	SetMaxCols(1);
	SetMaxRows(1);

	SetRowHeight(SS_ALLROWS, 20.0);
	SetColWidth(0, 5.0);
	SetColWidth(1, 21.0);
}

//=========================================================
// TitleView에서 사용하는 작은거..
//		: 2x2, color & font fixed, no scrollbar
//=========================================================
void CMySpreadDll::InitMySpread_Info()
{
	InitDefault();
	SetOperationMode(SS_OPMODE_READONLY);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_VERTSCROLLBAR,	FALSE);
	SetBool(SSB_SHOWCOLHEADERS,	FALSE);
	SetBool(SSB_SHOWROWHEADERS,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);

	COLORREF clrTitle	= RGB(202,210,223);
	COLORREF clrData	= RGB(224,231,245);

	SetGridColor(RGB_WHITE);
	SetSelColor(clrData, RGB_BLACK);

	HFONT hfData;
	MakeFont(&hfData, _T("Tahoma"), 16, FALSE);

	SS_CELLTYPE cellType;
	//SetTypeStaticText(&cellType, SS_TEXT_VCENTER | SS_TEXT_CENTER);
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 25, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// col = 2, row = 2 로 고정..
	SetMaxRows(2);
	SetMaxCols(2);

	// title
	SetColor(1, SS_ALLROWS, clrTitle, RGB_BLACK);
	SetFont	(1, SS_ALLROWS, m_hfont, TRUE);
	SetColWidth(1, 7.0);

	// data
	SetColor(2, SS_ALLROWS, clrData, RGB_BLACK);
	SetFont (2, SS_ALLROWS, hfData, TRUE);
	SetColWidth(2, 23.0);

	SetRowHeight(SS_ALLROWS, 20.0);
}

void CMySpreadDll::InitMySpread_Matrix() 
{
	InitDefault();
	SetOperationMode(SS_OPMODE_ROWMODE);

	SetBool(SSB_HORZSCROLLBAR,	FALSE);
	SetBool(SSB_AUTOSIZE,		TRUE);

	SetBool(SSB_AUTOCLIPBOARD,	TRUE);
	SetClipboardOptions(SS_CLIP_NOHEADERS);


	SetFont(SS_ALLCOLS, SS_ALLROWS, m_hfont, TRUE);

	SS_CELLTYPE cellType;
	SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_RIGHT, 10, NULL, SS_CASE_NOCASE);
	SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

	// header title
	SetData(0, SS_HEADER, _T("No."));
	SetData(1, SS_HEADER, _T("X"));
	SetData(2, SS_HEADER, _T("Y"));

	// grid size
	SetMaxCols(2);
	SetMaxRows(1);

	SetRowHeight(SS_ALLROWS, 16.0);
	SetColWidth(0, 5.1);
	SetColWidth(1, 11.2);
	SetColWidth(2, 11.2);
	SetColWidth(3, 11.2);
}