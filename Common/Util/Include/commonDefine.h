#pragma once

#include <string>
#include <sstream>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <vector>
#include <tuple>
#include <memory>
#include <stdint.h>
using namespace std;

#define HashMap         unordered_map
#define MultiHashMap    unordered_multimap
#define HashSet         unordered_set

#define UNUSED                    UNREFERENCED_PARAMETER

#define SAFE_DELETE(p)            if(nullptr != (p)){delete (p);(p) = nullptr;}
#define SAFE_DELETE_ARRAY(p)      if(nullptr != (p)){delete[] (p);(p) = nullptr;}

#define CHECKPOINT_BOOL(p)       if(nullptr == (p)){Log::error("nullptr == "#p);return false;}
#define CHECKPOINT_VOID(p)       if(nullptr == (p)){Log::error("nullptr == "#p);return;}
#define CHECKPOINT_NULLPTR(p)    if(nullptr == (p)){Log::error("nullptr == "#p);return nullptr;}
#define CHECKPOINT_INT(p,r)      if(nullptr == (p)){Log::error("nullptr == "#p);return (r);}

typedef unsigned char      uchar;
typedef unsigned short     ushort;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef long long          long64;
typedef unsigned long long ulong64;


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
typedef shared_ptr<AutoMemory> AutoMemoryPtr;