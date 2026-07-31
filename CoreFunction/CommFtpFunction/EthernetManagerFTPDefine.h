#pragma once

struct FtpInfo
{
    CString host;
    CString username;
    CString password;
    int		port;
    BOOL	bPassive;
};