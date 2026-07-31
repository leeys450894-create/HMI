// StaticSpread.cpp : implementation file
//

#include "pch.h"
#include "StaticSpread.h"
//#include "../ExModule/FarPoint64/SSDLLMFC.CPP"





#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticSpread

CStaticSpread::CStaticSpread()
{
}

CStaticSpread::~CStaticSpread()
{
}


BEGIN_MESSAGE_MAP(CStaticSpread, CStatic)
	//{{AFX_MSG_MAP(CStaticSpread)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticSpread message handlers

void CStaticSpread::PreSubclassWindow() 
{

	CRect rRect;
	GetClientRect(&rRect);

	int nID = this->GetDlgCtrlID();
	m_Spread.Create(WS_BORDER | WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,rRect, this, nID);

	m_Spread.SetMaxRows(10);
	m_Spread.SetMaxCols(10);
	m_Spread.SetSelBlockOptions(0);



	m_Spread.SetBool(SSB_HORZSCROLLBAR, FALSE);
	m_Spread.SetBool(SSB_VERTSCROLLBAR, FALSE);
	m_Spread.SetGrayAreaColor(RGB(255,255,255),RGB(255,255,255));
	m_Spread.SetGridColor(RGB(0,0,0));
	
	COLORREF lpShadowColor;
	COLORREF lpShadowText ;
	COLORREF lpShadowDark ;
	COLORREF lpShadowLight;

	
	COLORREF cHeader = RGB(220,220,220);
	m_Spread.GetShadowColor(&lpShadowColor,&lpShadowText ,&lpShadowDark ,&lpShadowLight); 
	m_Spread.SetShadowColor(cHeader,lpShadowText ,RGB(0,0,0) ,cHeader); 




	CStatic::PreSubclassWindow();
}

LRESULT CStaticSpread::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if(SSM_DBLCLK <=message && message<=SSM_BEFOREROWMOVE)
		this->GetParent()->SendMessage(message,wParam,lParam);

	if(message == WM_COMMAND && m_Spread.m_hWnd == (HWND) lParam )
    {
        int iMsgData = HIWORD(wParam);

        if(iMsgData == SSN_BLOCKSELECTED ||iMsgData == SSN_BLOCKCHANGED)
        {
            
            SS_CELLCOORD pStart,pEnd;
            m_Spread.GetSelectBlock(&pStart,&pEnd);
            //sLog(LOG_LEVEL_NORMAL,_T("Selected block..(%d,%d)~(%d,%d)"),pStart.Col,pStart.Row,pEnd.Col,pEnd.Row);
            //m_Spread.SetColorRange(pStart.Col,pStart.Row,
            //    (pEnd.Col-pStart.Col),(pEnd.Row-pStart.Row),RGB(0,255,0),RGB(0,0,0));
            m_Spread.SetColor(pStart.Col,pStart.Row,RGB(0,255,0),RGB(0,0,0));
            m_Spread.SetColor(pEnd.Col,pEnd.Row,RGB(0,255,0),RGB(0,0,0));
        }

        if(iMsgData == SSN_BLOCKDESELECTED)
        {
            m_Spread.SetColorRange(-1,-1,-1,-1,RGB(255,255,255),RGB(0,0,0));
        }

        if( SSN_SETFOCUS <= iMsgData  && iMsgData <= SSN_BLOCKDESELECTED)
            this->GetParent()->SendMessage(message,wParam,lParam);
    }

	return CStatic::WindowProc(message, wParam, lParam);
}


BOOL CStaticSpread::SetData(int x, int y, const TCHAR *format, ...)
{

    va_list argptr;
    TCHAR *buffer;//[1024];

    va_start(argptr, format);

    size_t numberOfElements = _vsctprintf(format,argptr)+1;
    buffer = (TCHAR *)malloc(numberOfElements*sizeof(TCHAR));
    memset(buffer, 0x00, numberOfElements*sizeof(TCHAR));
    _vstprintf_s(buffer,numberOfElements,format,argptr);

    va_end(argptr);

    CString sVal;
    sVal.Format(_T("%s"),buffer);
    m_Spread.SetData(x,y,sVal.GetBuffer(sVal.GetLength()));
    free(buffer);

    return TRUE;

}

double CStaticSpread::GetData(int x, int y)
{
	//CString buf;

	//SetCol(x);
	//SetRow(y);
	//buf = GetText();
    //return _tstof((LPTSTR)(LPCTSTR)buf);

    double dVal;
    m_Spread.GetFloat(x,y,&dVal);
    return dVal;
	
}
CString CStaticSpread::GetDataString(int x, int y)
{
	//CString buf;

	//SetCol(x);
	//SetRow(y);
	//buf = GetText();
	//return _tstof((LPTSTR)(LPCTSTR)buf);

	CString sData;
	TCHAR * sCellData = new TCHAR[255];
	SSGetData(m_Spread.GetSafeHwnd(), x,y, sCellData);
	sData.Format(_T("%s"),sCellData);
	delete [] sCellData;

	return sData;
}

BOOL CStaticSpread::SetColWidth(int iCol, double dWidth)
{
	return m_Spread.SetColWidth(iCol,dWidth);
}

BOOL CStaticSpread::SetRowHeight(int iRow, double dHeight)
{
	return m_Spread.SetRowHeight(iRow,dHeight);
}

void CStaticSpread::SetWindowPos(CRect rect)
{
	((CWnd *) this)->SetWindowPos(NULL,rect.left,rect.top,rect.Width(),rect.Height(),SWP_NOZORDER);
	CRect r;
	GetClientRect(r);
	m_Spread.SetWindowPos(NULL,r.left,r.top,r.Width(),r.Height(),SWP_NOZORDER);
}

void CStaticSpread::SetMaxCols(int nCols)
{
	m_Spread.SetMaxCols(nCols);
}

void CStaticSpread::SetMaxRows(int nRows)
{
	m_Spread.SetMaxRows(nRows);
}




BOOL CStaticSpread::SetSelectBlock(int iColS, int iRowS, int iColE, int iRowE)
{
	SS_CELLCOORD CellUL={iColS, iRowS} ;
	SS_CELLCOORD CellLR={iColE, iRowS} ;
	return m_Spread.SetSelectBlock(&CellUL, &CellLR);
}

BOOL CStaticSpread::SetActiveCell(int Col, int Row)
{
	return m_Spread.SetActiveCell(Col, Row);
}

BOOL CStaticSpread::GetActiveCell(int *pCol, int *pRow)
{
	SS_COORD Col=0;
	SS_COORD Row=0;
	BOOL bRet =m_Spread.GetActiveCell(&Col, &Row); 
	*pCol = Col;
	*pRow = Row;
	return bRet;
}
BOOL CStaticSpread::SaveCSVFile(CString sFileName)
{
    return m_Spread.ExportToTextFile(sFileName,_T(""),_T(","),_T(""),
        SS_EXPORTTEXT_CREATE|SS_EXPORTTEXT_COLHEADERS|SS_EXPORTTEXT_ROWHEADERS,_T(""));
}

BOOL CStaticSpread::SaveExcelFile(CString sFileName)
{
    return m_Spread.ExportExcelBook(sFileName,_T(""));
}

BOOL CStaticSpread::ImportExeclFile(CString sFileName)
{
	return m_Spread.ImportExcelBook(sFileName, _T(""));
}