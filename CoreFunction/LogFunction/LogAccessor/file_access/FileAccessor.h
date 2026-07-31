#pragma once

#include <string>

class CFileAccessor
{
	FILE *m_pStream;

public:
	bool CloseStream();
	bool Open_Stream(CString strPath);
	bool WriteStream(CString strMsg);


public:
	CFileAccessor(void);
	~CFileAccessor(void);
};

