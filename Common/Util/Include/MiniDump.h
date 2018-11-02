#pragma once
#include "ExportDefine.h"

class COMMON_API CMiniDump
{
public:
    CMiniDump(LPCTSTR szFileName);
    ~CMiniDump(void);
    void SetCallback(void (*p)());
};

