#pragma once

#include <atlcoll.h>

//---------------------------------------------------------------
// 데이터 베이스 - 데이터 읽기/쓰기
//---------------------------------------------------------------
class CArchiveUserDefine : public CArchive
{
public:
    template <class T> void SerializeValue(T &val)
    {
        if(IsStoring()) (*this) << val;
        else			(*this) >> val;
    }

    template <> void SerializeValue(CString &val)
    {
        UINT nDataNum = (val.IsEmpty() == true) ? -1 : 1;

        SerializeValue(nDataNum);

		if(nDataNum == 1)
		{
			if(IsStoring())	(*this) << val;
			else			(*this) >> val;
		}
    }

	void SerializeArray(LPBYTE &pData, UINT &nDataNum)
	{
        SerializeValue(nDataNum);

		if( IsStoring() )
		{
			Write(pData, nDataNum);
		}
		else
		{
			delete [] pData;
			pData = new BYTE[nDataNum];
			memset(pData, 0, nDataNum);

			Read(pData, nDataNum);
		}
	}

public:
    CArchiveUserDefine(CFile *pFile, UINT nMode) : CArchive(pFile, nMode) {}
    virtual ~CArchiveUserDefine() {}
};

//---------------------------------------------------------------
// 데이터 베이스 - 데이터 관리
//---------------------------------------------------------------
class CDataBaseBinary
{
    UINT	m_nDataOffset;
    UINT	m_nDataSize;
    LPBYTE	m_pDataBufPtr;
    
	CAtlMap<CString, CDataBaseBinary, CStringElementTraits<CString> > *m_pDataBase;

private:
	bool __LinkString(bool bSave, CString &data)
    {
        m_nDataOffset = 0;

        // 널 문자 포함한 길이
		UINT nSize = (sizeof(TCHAR) * data.GetLength()) + 1;

        if(bSave == true)
        {
            // 데이터를 저장하기 위해서 buffer 를 생성
            __SetDataSize(sizeof(UINT) + nSize);
        }

		// 문자 길이와 데이터 저장
		bool bRet = true;
		bRet &= __SetDataLink(bSave, (BYTE *)&nSize, sizeof(UINT));
		bRet &= __SetDataLinkString(bSave, data, nSize - 1);

		return bRet;
    }

	void __SetDataSize(UINT nSize)
	{
		m_nDataSize	= nSize;

		delete [] m_pDataBufPtr;
		m_pDataBufPtr = new BYTE[m_nDataSize];
		memset(m_pDataBufPtr, 0, m_nDataSize);
	}

    bool __SetDataLink(bool bSave, BYTE *pData, UINT nSize)
	{
		if((m_pDataBufPtr == nullptr) || (pData == nullptr))
			return false;

		if(bSave == true)
		{
			memcpy(m_pDataBufPtr + m_nDataOffset, pData, nSize);
			m_nDataOffset += nSize;
		}
		else
		{
			memcpy(pData, m_pDataBufPtr + m_nDataOffset, nSize);
			m_nDataOffset += nSize;
		}

		return true;
	}

	bool __SetDataLinkString(bool bSave, CString &strData, UINT nSize)
	{
		if(m_pDataBufPtr == nullptr)
			return false;

		if(bSave == true)
		{
			memcpy(m_pDataBufPtr + m_nDataOffset, (LPVOID)(TCHAR*)strData.GetString(), nSize);
			m_nDataOffset += nSize;
		}
		else
		{
			if(nSize <= 0)
				return false;

			BYTE *pbyData = new BYTE[nSize];
			memset(pbyData, 0, nSize);
			memcpy(pbyData, m_pDataBufPtr + m_nDataOffset, nSize);
			m_nDataOffset += nSize;
			

			// wchar 에 대해서 별도로 처리 해야 한다.;; 아쉽네..
			CString str_wchar = _T("");
			UINT string_count = nSize / sizeof(TCHAR);
			for (UINT n = 0; n < string_count; ++n)
			{				
				CString str;
				str.Format(_T("%c"), ((TCHAR*)pbyData)[n]);
				str_wchar += str;
			}			
			delete [] pbyData;


			strData = str_wchar;
		}

		return true;
	}

public:
    bool Serialize(CArchiveUserDefine &ar)
	{
		if(m_pDataBase)
		{
			//------------------------------------------------------
			// Load 시에는 기존의 데이터를 제거한다.
			//------------------------------------------------------
			if(ar.IsLoading() == TRUE)
			{
				m_pDataBase->RemoveAll();
			}

			//------------------------------------------------------
			// DB structs 의 정보를 string 으로 저장한다.
			//------------------------------------------------------
			UINT nCount = (UINT)m_pDataBase->GetCount();
		
			ar.SerializeValue(nCount);
		
			POSITION PosCur = m_pDataBase->GetStartPosition();

			for(UINT n = 0; n < nCount; n++)
			{
				CString	strElementTraits;

				if(ar.IsStoring())
				{
					strElementTraits = m_pDataBase->GetNextKey(PosCur);
				}

				// 하위 DB structs
				ar.SerializeValue(strElementTraits);

				(*m_pDataBase)[strElementTraits].Serialize(ar);				
			}

			//------------------------------------------------------
			// DB structs 의 실제 데이터를 저장한다.
			//------------------------------------------------------
			ar.SerializeArray(m_pDataBufPtr, m_nDataSize);

			return true;
		}

		return false;
	}

    // map data 구조를 string 으로 분리
    CDataBaseBinary &operator [] (LPCTSTR strElementTraits)
    {
        return (*m_pDataBase)[strElementTraits];
    }

public:
	//# c에는 default argument promotions라는 규칙이 있습니다.
	//# 이 규칙에 따르면 char, short 등의 int 이하 자료형은 모두 int로 진급 (만약 안되는 경우엔 unsigned int로 진급)
	//# float형은 double로 진급
	//# 결국 %f 하나로 float, double을 모두 처리 가능
	template <typename T> bool Link(bool bSave, T &data)
    {
		CString strValue;
		strValue.Format(_T("%d"), data);

        bool bRet = __LinkString(bSave, strValue);

		if(bSave == false)
		{
			data = (T)_ttoi(strValue);
		}

		return bRet;
    }

    template <> bool Link(bool bSave, bool &data)
    {
		CString strValue;
		strValue.Format(_T("%d"), (data == true) ? 1 : 0);

        bool bRet = __LinkString(bSave, strValue);

		if(bSave == false)
		{
			data = (_ttoi(strValue) == 1) ? true : false;
		}

		return bRet;
    }

	template <> bool Link(bool bSave, float &data)
    {
		CString strValue;
		strValue.Format(_T("%f"), data);

        bool bRet = __LinkString(bSave, strValue);

		if(bSave == false)
		{
			data = (float)_ttof(strValue);
		}

		return bRet;
    }

    template <> bool Link(bool bSave, double &data)
    {
		CString strValue;
		strValue.Format(_T("%f"), data);

        bool bRet = __LinkString(bSave, strValue);

		if(bSave == false)
		{
			data = _ttof(strValue);
		}

		return bRet;
    }

	template <> bool Link(bool bSave, CString &data)
    {
        return __LinkString(bSave, data);
    }

    template <> bool Link(bool bSave, CPoint &data)
    {
		CString strValue;
		strValue.Format(_T("%d_%d"), data.x, data.y);

        bool bRet = __LinkString(bSave, strValue);

		if(bSave == false)
		{
		    CString strParam1, strParam2;
			AfxExtractSubString(strParam1, strValue, 0, '_');
			AfxExtractSubString(strParam2, strValue, 1, '_');

			data = CPoint(_ttoi(strParam1), _ttoi(strParam2));
		}

		return bRet;
    }

	// image file 을 저장하기 위해서 사용하는 함수
    bool LinkArray(bool bSave, BYTE *data, UINT nNum)
    {
        m_nDataOffset = 0;

        if(bSave == true)
        {
            // 이미지 크기와 이미지 데이터를 저장해야 한다.
            __SetDataSize(sizeof(UINT) + sizeof(BYTE) * nNum);
        }

        bool bRet = true;
        bRet &= __SetDataLink(bSave, (BYTE *)&nNum,	sizeof(UINT));
        bRet &= __SetDataLink(bSave, (BYTE *)data,	sizeof(BYTE) * nNum);

        return bRet;
    }

public:
    CDataBaseBinary()
	{
		m_nDataOffset	= 0;
		m_pDataBufPtr	= nullptr;
		m_nDataSize		= 0;
		m_pDataBase		= new CAtlMap<CString, CDataBaseBinary, CStringElementTraits<CString> >;
	}
    ~CDataBaseBinary()
	{
		delete [] m_pDataBufPtr;
		delete m_pDataBase;
	}
};

//---------------------------------------------------------------
// 데이터 베이스 - 파일 연동 관련 [ binary type ]
//---------------------------------------------------------------
class CBinaryHelper
{
    CString			m_strFilePath;
    CDataBaseBinary *m_pdbBinary;

public:
    bool	IsReadEnable();
    LPCTSTR GetFilePath();
    void	SetFilePath(LPCTSTR lpszFilePath);
    bool	NewFile();
    bool	FileLoad();
    bool	FileSave();

public:
	// 일반 변수를 저장하기 위해서 사용하는 함수
    template <typename T> bool Link(bool bSave, LPCTSTR szCategory, LPCTSTR szKey, T *val, T def)
    {
        T tmp = *val;

        if(bSave == true)
        {
            return (*m_pdbBinary)[szCategory][szKey].Link(bSave, tmp);
        }
		else
		{
			if((*m_pdbBinary)[szCategory][szKey].Link(bSave, tmp) == false)
			{
				*val = def;
			}
			else
			{
				*val = tmp;
			}        
		}

        return true;
    }

    // image file 을 저장하기 위해서 사용하는 함수
    bool Link(bool bSave, LPCTSTR szCategory, LPCTSTR szKey, BYTE *val, UINT num)
    {
		return (*m_pdbBinary)[szCategory][szKey].LinkArray(bSave, val, num);
    }

public:
    CBinaryHelper();
    ~CBinaryHelper();
};
