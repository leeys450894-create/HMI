#pragma once

#define MAX_RECEIVE_BUFFER	4096

enum eNetError
{
    SUCCESSED_OK = 0,
    NOT_OPEN_SOCKET = -1000,
};

typedef void (*CallbackClassFunction2)(LPVOID, LPVOID);
