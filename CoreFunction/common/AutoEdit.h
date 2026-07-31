#if !defined(AFX_AUTOEDIT_H__056ECEAA_9283_11D3_8599_00105A744766__INCLUDED_)
#define AFX_AUTOEDIT_H__056ECEAA_9283_11D3_8599_00105A744766__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AutoEdit.h : Header-Datei
//

#include "InfoWnd.h"

/////////////////////////////////////////////////////////////////////////////
class CEditValidate
{
public:
    CEditValidate() {};
    ~CEditValidate() {};
    virtual CString validate(CString& /*str*/) { return(_T(""));};
};

/////////////////////////////////////////////////////////////////////////////
class CValidateDbl : public CEditValidate 
{
public:
	CValidateDbl(double dMin, double dMax);
    virtual CString validate(CString& str);

	void SetMinMax(double dMin, double dMax);
	void	SetFormatStr(char* fmtStr);

protected:
	double	m_dMin;
	double	m_dMax;
	CString	 m_strFormat;
};

/////////////////////////////////////////////////////////////////////////////
class CValidateInt : public CEditValidate 
{
public:
	CValidateInt(int iMin, int iMax);
    virtual CString validate(CString& str);

	void SetMinMax(int iMin, int iMax);

protected:
	int		m_iMin;
	int		m_iMax;
};

/////////////////////////////////////////////////////////////////////////////
// Fenster CAutoEdit 

class CAutoEdit : public CEdit
{
// Konstruktion
public:
    CAutoEdit();

// Attribute
public:

// Operationen
public:

// ?erschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktions?erschreibungen
	//{{AFX_VIRTUAL(CAutoEdit)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementierung
public:
	void SetText(const CString* str = NULL);
    void SetExitChar(int nChar = '.') { m_nExitChar = nChar;}; // Sets the Character used to exit this field
    void SetValidChar(char* pChar);                             // Set the array with valid characters
    void SetMaxChars(int nMax = -1)  {m_nMaxChar = nMax;LimitText(nMax);};      // Set the max characters, if set the field is skipped when the last character is typed
    void SetSignHandler(bool bOnce = true ,bool bInFront = true) {m_bSignOnce = bOnce; m_bSignInFront = bInFront;};
    void SetDefaultValue(LPCSTR p) {m_sDefault = p;};
    void SetNullPadding(bool b = true) {m_bNullPad = b;};
    void SetValidationHandler(CEditValidate* p) {m_validateProc = p;};
    int CountNoOfChars();
    int CountSigns();

	BOOL	Validate();		//YR

    virtual ~CAutoEdit();

	// Generierte Nachrichtenzuordnungsfunktionen
protected:
	//{{AFX_MSG(CAutoEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    void ArrangeSign();
protected:
	UINT	m_nExitChar;   // This character acts like a TAB, e.g a '.' in a numeric field
    int		m_nMaxChar;     // -1 unused, > 0 leaves the field automatically
	char*	m_nValidChar; // NULL or a array of valid keys, eg. "0123456789" fo numeric only !
    bool	m_bSignOnce;
    bool	m_bSignInFront;
    bool	m_bNullPad;
    CString m_sDefault;
    CEditValidate* m_validateProc;
    CInfoWnd* m_pInfoWnd;
};

/////////////////////////////////////////////////////////////////////////////
class CAutoEditDbl : public CAutoEdit
{
public:
    CAutoEditDbl();
    CAutoEditDbl(double dMin, double dMax);
	~CAutoEditDbl();

	void	SetMinMax(double dMin, double dMax, double dIni = 0.0);
	void	SetValue(double val, LPCTSTR format = _T("%.4f"));
	void	SetFormatStr(char* fmtStr);
	double	GetValue();

	CValidateDbl*	m_pValidate;
	CString	 m_strFormat;
};

/////////////////////////////////////////////////////////////////////////////
class CAutoEditInt : public CAutoEdit
{
public:
    CAutoEditInt();
    CAutoEditInt(int iMin, int iMax);
	~CAutoEditInt();

	void	SetMinMax(int iMin, int iMax, int iIni = 0);
	void	SetValue(int val, LPCTSTR format = _T("%d"));
	int		GetValue();

	CValidateInt*	m_pValidate;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ f?t unmittelbar vor der vorhergehenden Zeile zus?zliche Deklarationen ein.

#endif // AFX_AUTOEDIT_H__056ECEAA_9283_11D3_8599_00105A744766__INCLUDED_
