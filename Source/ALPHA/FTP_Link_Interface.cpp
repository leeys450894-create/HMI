#include "pch.h"
#include "FTP_Link_Interface.h"

#include "..\CoreFunction\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFTP_Link_Interface::CFTP_Link_Interface()
    : m_pIniHelper(new CIniHelper)
{
}

CFTP_Link_Interface::~CFTP_Link_Interface()
{
    if(m_pIniHelper)
    {
        delete m_pIniHelper;
        m_pIniHelper = nullptr;
    }
}

bool CFTP_Link_Interface::LinkIniFile(bool bSave)
{
	return false;
}