#include "pch.h"
#include "BinaryHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBinaryHelper::CBinaryHelper()
{
	m_pdbBinary = new CDataBaseBinary;
}

CBinaryHelper::~CBinaryHelper()
{
	if(m_pdbBinary)
	{
		delete m_pdbBinary;
		m_pdbBinary = nullptr;
	}
}

LPCTSTR CBinaryHelper::GetFilePath()
{
    return m_strFilePath;
}

void CBinaryHelper::SetFilePath(LPCTSTR lpszFilePath)
{
    m_strFilePath = lpszFilePath;
}

bool CBinaryHelper::IsReadEnable()
{
    return (PathFileExists(m_strFilePath) == TRUE) ? true : false;
}

bool CBinaryHelper::NewFile()
{
    CFile file;
    CFileException fe;

    if(file.Open(m_strFilePath, CFile::modeCreate, &fe) ==  FALSE)
    {
        return false;
    }

    CArchiveUserDefine ar(&file, CArchive::store);

	if(m_pdbBinary)
	{
		delete m_pdbBinary;
		m_pdbBinary = new CDataBaseBinary;	
	}

    ar.Close();
    file.Close();

    return true;
}

bool CBinaryHelper::FileLoad()
{
    CFile file;
    CFileException fe;

    if(file.Open(m_strFilePath, CFile::modeRead, &fe) == FALSE)
    {
        return false;
    }

    CArchiveUserDefine ar(&file, CArchive::load);

	BOOL bReturn = FALSE;
	if(m_pdbBinary)
	{
		delete m_pdbBinary;
		m_pdbBinary = new CDataBaseBinary;

	    bReturn = m_pdbBinary->Serialize(ar);
	}

    ar.Close();
    file.Close();

    return (bReturn == TRUE) ? true : false;
}

bool CBinaryHelper::FileSave()
{
    CFile file;
    CFileException fe;

    if(file.Open(m_strFilePath, CFile::modeWrite | CFile::modeCreate, &fe) ==  FALSE)
    {
        return false;
    }

    CArchiveUserDefine ar(&file, CArchive::store);

	BOOL bReturn = FALSE;
	if(m_pdbBinary)
	{
		bReturn = m_pdbBinary->Serialize(ar);
	}

    ar.Close();
    file.Close();

    return (bReturn == TRUE) ? true : false;
}