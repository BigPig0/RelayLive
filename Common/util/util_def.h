#pragma once
#include <memory>

#define SAFE_DELETE(p)            if(nullptr != (p)){delete (p);(p) = nullptr;}
#define SAFE_DELETE_ARRAY(p)      if(nullptr != (p)){delete[] (p);(p) = nullptr;}


// 智能释放的内存
struct AutoMemory
{
    char*   pBuff;
    int     nLen;
    AutoMemory()
        : pBuff(nullptr)
        , nLen(0)
    {}
    ~AutoMemory()
    {
        if (nullptr != pBuff)
        {
            free(pBuff);
            pBuff = nullptr;
        }
    }
};
typedef std::shared_ptr<AutoMemory> AutoMemoryPtr;