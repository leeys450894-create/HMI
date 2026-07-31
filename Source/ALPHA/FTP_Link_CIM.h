#pragma once

#include "FTP_Link_Interface.h"

class CFTP_Link_CIM : public CFTP_Link_Interface
{

public:
    virtual bool LinkIniFile(bool bSave) override;

public:
    CFTP_Link_CIM(CString strConfigPath);
    virtual ~CFTP_Link_CIM();
};