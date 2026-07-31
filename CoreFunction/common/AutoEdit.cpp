// AutoEdit.cpp: Implementierungsdatei
//

#include "pch.h"
#include "AutoEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAutoEdit

CAutoEdit::CAutoEdit()
{
    // set some defaults
    m_validateProc	= NULL;
    m_nExitChar		= _T('.');
    m_nValidChar	= NULL;
    m_nMaxChar		= -1;
    m_bSignOnce		= false;
    m_bSignInFront	= false;
    m_bNullPad		= false;
    m_sDefault.Empty();

    // create the little window for the error messages
    //m_pInfoWnd = new CInfoWnd();
	m_pInfoWnd = NULL;
}

CAutoEdit::~CAutoEdit()
{
    // free all attached resources
    if (m_pInfoWnd)		
		delete m_pInfoWnd;
    if (m_nValidChar)	
		delete[] m_nValidChar;
}


BEGIN_MESSAGE_MAP(CAutoEdit, CEdit)
	//{{AFX_MSG_MAP(CAutoEdit)
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen f? Nachrichten CAutoEdit 

void CAutoEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
    // filter each character and see that it fits the allowed characters,
    CString s;
	// TODO: Code f? die Behandlungsroutine f? Nachrichten hier einf?en und/oder Standard aufrufen
    if (nChar == m_nExitChar) {
        // if the exit character is pressed, translate it to a TAB
        ((CDialog*)GetParent())->NextDlgCtrl();
        return;
    }

    if ((m_nValidChar == NULL)
    || ((m_nValidChar != NULL)                    // do I have to check if the char is valid ?
    &&  (strchr(m_nValidChar,nChar) != NULL))) {  // is it a 'valid' character ?
        if ((nChar == _T('-'))
        ||  (nChar == _T('+'))) {
            if ((m_bSignOnce)
            && (CountSigns() > 0)) {
                int nStart,nEnd;
                GetSel(nStart,nEnd);
                if (!((nStart == 0) && (nEnd >= LineLength(-1)))) { // not everything selected ?
                    ::MessageBeep((UINT)-1);
                    return;
                }
            }
        }
        CEdit::OnChar(nChar, nRepCnt, nFlags); // give it to the base class to handle

        if (m_bSignInFront) { // put the sign in front if required
           ArrangeSign();
        } 

        // if the max Char flag is set, check the length and exit if field full
        if ((m_nMaxChar != -1) 
        &&   (LineLength() >= m_nMaxChar)) {
            GetWindowText(s); // if the maximum number of characters is reached, simulate a TAB
            ((CDialog*)GetParent())->NextDlgCtrl();
            return;
        }
    }
    else {
        // if it is a non displayable character, process it anyway (e.g. CTRL+C etc.)
        if (!_istprint(nChar)) {
           CEdit::OnChar(nChar, nRepCnt, nFlags); // let the base class handle it
        }
        else {
            // beep !
            ::MessageBeep((UINT)-1); // tell the user that this key is not allowed
        }
    }
}

int CAutoEdit::CountNoOfChars()
{
    CString s;
    GetWindowText(s);
    return(s.GetLength());
}

int CAutoEdit::CountSigns()
{
    // ensure that only one sign is possible 
    CString s;
    int n = 0;
    GetWindowText(s);
    for (int i = 0;i < s.GetLength(); i++) {
        if ((s.GetAt(i) == _T('-'))
        || (s.GetAt(i) == _T('+'))) {
            n++;
        }
    }
    return(n);
}

void CAutoEdit::ArrangeSign()
{
    // see that the sign is always in front of the value
    CString s;
    GetWindowText(s);
    if (s.FindOneOf(_T("+-")) != -1) {
        s = _T(' ') + s;
        for (int i = 1;i < s.GetLength(); i++) {
            if ((s.GetAt(i) == _T('-'))
            || (s.GetAt(i) == _T('+'))) {
                s.SetAt(0,s.GetAt(i));
                s = s.Left(i) + s.Right(s.GetLength() - i - 1);
            }
        }
        SetWindowText(s);
        // make sure that typying commends at the end of the string
        SetSel(s.GetLength(),s.GetLength(),TRUE);
    }
}

void CAutoEdit::OnKillFocus(CWnd* pNewWnd) 
{
    CEdit::OnKillFocus(pNewWnd); // ALWAYS call this !

    CString s;
    CWnd* pWnd = GetParent();
    // check if the cancel button is being pressed, this must be treated differently
    // since the user must be allowed to cancel in case of an error
    // NOTE : this only works if the cancel button is named IDCANCEL (as defined by the standard)
    if (pWnd) pWnd = pWnd->GetDlgItem(IDCANCEL);
    if (pWnd == NULL) { // if no cancel button has been found
        pWnd = GetParent(); // see if there is a cancel button futher up in the stack
        if (pWnd) pWnd = pWnd->GetParent(); // this is in case we are using subdialogs (see codeproject)
        if (pWnd) pWnd = pWnd->GetDlgItem(IDCANCEL); 
    }
    if (pWnd != pNewWnd) { // not the cancel button
       if (m_validateProc != NULL) { // a validation procedure has been defined
          GetWindowText(s);
          CString strError;
          strError = m_validateProc->validate(s); // validate the field and return the error message and the formatted contents
          SetWindowText(s); // display the reformatted contents
          if (!strError.IsEmpty()) { // keep it here if not valid
              SetFocus(); // stay in this field
              SetSel(0,-1); // select the contents, for easier correction
              m_pInfoWnd->Hide(); // fix : make sure that the newest error message is displayed
              m_pInfoWnd->Show(this,strError); // show the error message in a little window
             
          }
          else {
              m_pInfoWnd->Hide(); // field is correct, get rid of the litte error message window
          }
       }
    }  
}

void CAutoEdit::SetText(const CString *str)
{
    if (str == NULL) 
        SetWindowText(m_sDefault);
    else 
        SetWindowText((*str));
}

void CAutoEdit::PreSubclassWindow() 
{
	CEdit::PreSubclassWindow();

    m_pInfoWnd = new CInfoWnd();
    m_pInfoWnd->Create(GetParent()); // have to create here, since it is too early in the constructor
                                     // attach the window to the parent (the dialog)   
}

void CAutoEdit::SetValidChar(char* pChar)   
{ 
    if (m_nValidChar) delete [] m_nValidChar;
    m_nValidChar = new char[strlen(pChar) + 1];
    strcpy(m_nValidChar,pChar);
} 

//YR
BOOL CAutoEdit::Validate()
{
	if (NULL == m_validateProc) 
	{
		ASSERT(FALSE);
		return FALSE;
	}

	CString str;
    GetWindowText(str);
    CString strError = m_validateProc->validate(str); // validate the field and return the error message and the formatted contents

	return (strError.IsEmpty());
}

CValidateDbl::CValidateDbl(double dMin, double dMax) : CEditValidate()
{
	ASSERT(dMax > dMin);
	m_dMin = dMin;
	m_dMax = dMax;
	m_strFormat = "%.4f";
}

void CValidateDbl::SetMinMax(double dMin, double dMax)
{
	ASSERT(dMax > dMin);
	m_dMin = dMin;
	m_dMax = dMax;
}

CString CValidateDbl::validate(CString& str)
{
	double val = _ttoi(str);
	CString ret;
	CString format;

	if (val < m_dMin)
	{
		format.Format(_T("Enter a number bigger than %s !"), m_strFormat);
		ret.Format(format, m_dMin);
	}
	else if (val > m_dMax)
	{
		format.Format(_T("Enter a number smaller than %s !"), m_strFormat);
		ret.Format(format, m_dMax);
	}
	else
		ret = _T("");

	return ret;
}

void CValidateDbl::SetFormatStr(char* fmtStr)
{
	m_strFormat = fmtStr;
}

CValidateInt::CValidateInt(int iMin, int iMax) : CEditValidate()
{
	ASSERT(iMax > iMin);
	m_iMin = iMin;
	m_iMax = iMax;
}

void CValidateInt::SetMinMax(int iMin, int iMax)
{
	ASSERT(iMax > iMin);
	m_iMin = iMin;
	m_iMax = iMax;
}

CString CValidateInt::validate(CString& str)
{
	int val = _ttoi(str);
	CString ret;

	if (val < m_iMin)
		ret.Format(_T("Enter a number bigger than %d !"), m_iMin);
	else if (val > m_iMax)
		ret.Format(_T("Enter a number smaller than %d !"), m_iMax);
	else
		ret = _T("");

	return ret;
}

CAutoEditDbl::CAutoEditDbl() : CAutoEdit()
{
	SetValidChar("-0123456789.");
	SetExitChar(',');

	m_pValidate = NULL;
	m_strFormat = "%.4f";
}

CAutoEditDbl::CAutoEditDbl(double dMin, double dMax) : CAutoEdit()
{
	ASSERT (dMax > dMin);

	SetValidChar("-0123456789.");
	SetExitChar(',');

	m_pValidate = new CValidateDbl(dMin, dMax);
    SetValidationHandler(m_pValidate);

	m_strFormat = "%.4f";
}

CAutoEditDbl::~CAutoEditDbl()
{
	if (m_pValidate)
		delete m_pValidate;
}

void CAutoEditDbl::SetMinMax(double dMin, double dMax, double dIni)
{
	ASSERT (dMax > dMin);

	if (NULL == m_pValidate)
	{
		m_pValidate = new CValidateDbl(dMin, dMax);
	    SetValidationHandler(m_pValidate);
	}
	else
		m_pValidate->SetMinMax(dMin, dMax);

	CString str;
	str.Format(m_strFormat, dIni);
	SetWindowText(str);
}

void CAutoEditDbl::SetValue(double val, LPCTSTR format)
{
	CString str;
	str.Format(format, val);
	SetText(&str);
}

double CAutoEditDbl::GetValue()
{
	CString str;
	GetWindowText(str);
	double d = _ttof(str);
	return d;
}

void CAutoEditDbl::SetFormatStr(char* fmtStr)
{
	m_strFormat = fmtStr;
}

CAutoEditInt::CAutoEditInt() : CAutoEdit()
{
	SetValidChar("-0123456789");

	m_pValidate = NULL;
}

CAutoEditInt::CAutoEditInt(int iMin, int iMax) : CAutoEdit()
{
	ASSERT (iMax > iMin);

	SetValidChar("-0123456789");

	m_pValidate = new CValidateInt(iMin, iMax);
    SetValidationHandler(m_pValidate);
}

CAutoEditInt::~CAutoEditInt()
{
	if (m_pValidate)
		delete m_pValidate;
}

void CAutoEditInt::SetMinMax(int iMin, int iMax, int iIni)
{
	ASSERT (iMax > iMin);

	if (NULL == m_pValidate)
	{
		m_pValidate = new CValidateInt(iMin, iMax);
	    SetValidationHandler(m_pValidate);
	} 
	else
		m_pValidate->SetMinMax(iMin, iMax);

	CString str;
	str.Format(_T("%d"), iIni);
	SetWindowText(str);
}

void CAutoEditInt::SetValue(int val, LPCTSTR format)
{
	CString str;
	str.Format(format, val);
	SetText(&str);
}

int CAutoEditInt::GetValue()
{
	CString str;
	GetWindowText(str);
	int i = _ttoi(str);
	return i;
}

