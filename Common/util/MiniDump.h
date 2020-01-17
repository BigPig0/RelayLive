#pragma once
#include "util_public.h"

class UTIL_API CMiniDump
{
public:
    CMiniDump(string szFileName);
    ~CMiniDump(void);
    void SetCallback(void (*p)());
};

