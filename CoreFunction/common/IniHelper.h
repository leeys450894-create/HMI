#pragma once

class CIniHelper
{
    CString m_strFilePath;
public:
    bool	IsReadEnable();
    void	SetFilePath(LPCTSTR lpszFilePath);
    LPCTSTR GetFilePath();

	// 내부적으로 사용되는 함수
private:
	CString		TrimSymbol(LPCTSTR szName);

	short		__LoadSHORT(LPCTSTR szCategory, LPCTSTR szKey, short nDefValue = 0);
	bool		__LoadBOOL(LPCTSTR szCategory, LPCTSTR szKey, bool nDefValue = 0);
	int			__LoadINT(LPCTSTR szCategory, LPCTSTR szKey, int nDefValue = 0);
	CString		__LoadSTRING(LPCTSTR szCategory, LPCTSTR szKey, CString nDefValue = _T(""));
	float		__LoadFLOAT(LPCTSTR szCategory, LPCTSTR szKey, float dDefValue = 0.f);
	double		__LoadDOUBLE(LPCTSTR szCategory, LPCTSTR szKey, double dDefValue = 0.f);
	CPoint		__LoadPOINT(LPCTSTR szCategory, LPCTSTR szKey, CPoint nDefValue = CPoint(0, 0));
	CRect		__LoadRECT(LPCTSTR szCategory, LPCTSTR szKey, CRect nDefValue = CRect(0, 0, 0, 0));
	ULONGLONG	__LoadULONGLONG(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG nDefValue = 0);
	CString		__LoadTCHAR(LPCTSTR szCategory, LPCTSTR szKey, TCHAR nDefValue[] = {0,});
	void		__SaveSHORT(LPCTSTR szCategory, LPCTSTR szKey, short nValue);
	void		__SaveBOOL(LPCTSTR szCategory, LPCTSTR szKey, bool nValue);
	void		__SaveINT(LPCTSTR szCategory, LPCTSTR szKey, int nValue);
	void		__SaveSTRING(LPCTSTR szCategory, LPCTSTR szKey, CString strValue);
	void		__SaveFLOAT(LPCTSTR szCategory, LPCTSTR szKey, float dValue);
	void		__SaveDOUBLE(LPCTSTR szCategory, LPCTSTR szKey, double dValue);
	void		__SavePOINT(LPCTSTR szCategory, LPCTSTR szKey, CPoint ptValue);
	void		__SaveRECT(LPCTSTR szCategory, LPCTSTR szKey, CRect rtValue);
	void		__SaveULONGLONG(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG nValue);

	//------------------------
private:
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, short *nValue, short nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue, bool nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, int *nValue, int nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG *nValue, ULONGLONG nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, long *nValue, long nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, unsigned short *nValue, unsigned short nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue, CString strDefValue = _T(""));
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, float *nValue, float nDefValue = 0);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, double *fValue, double fDefValue = 0.f);
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, CPoint *ptValue, CPoint ptDefValue = CPoint(0, 0));
	void		Read(LPCTSTR szCategory, LPCTSTR szKey, CRect *rtValue, CRect rtDefValue = CRect(0, 0, 0, 0));
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, short *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, int *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, long *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, unsigned short *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, float *nValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, double *fValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, CPoint *ptValue);
	void		Write(LPCTSTR szCategory, LPCTSTR szKey, CRect *rtValue);


public:
	template <class T>
	void Link(bool bSave, LPCTSTR szCategory, LPCTSTR szKey, T *value, T init_value)
	{
		if(bSave)	Write(szCategory, szKey, value);
		else		Read(szCategory, szKey, value, init_value);
	}

	void Delete(LPCTSTR szCategory);

public:
    CIniHelper(void);
    ~CIniHelper(void);
};
