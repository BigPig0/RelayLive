#include "util_minidump.h"

static string g_szDumpFileName;

static void (*callback)() = nullptr;

#ifdef WIN32
#include <windows.h>  
#include <DbgHelp.h>  
#include <stdlib.h>
#pragma comment(lib, "dbghelp.lib")  


static inline BOOL IsDataSectionNeeded(const WCHAR* pModuleName)  
{  
    if(pModuleName == 0)  
    {  
        return FALSE;  
    }  

    WCHAR szFileName[_MAX_FNAME] = L"";  
    _wsplitpath_s(pModuleName,NULL,0,NULL,0, szFileName,_MAX_FNAME,NULL,0);  

    if(_wcsicmp(szFileName, L"ntdll") == 0)  
        return TRUE;  

    return FALSE;  
}  

static inline BOOL CALLBACK MiniDumpCallback(PVOID                            pParam,  
                                      const PMINIDUMP_CALLBACK_INPUT   pInput,  
                                      PMINIDUMP_CALLBACK_OUTPUT        pOutput)  
{  
    if(pInput == 0 || pOutput == 0)  
        return FALSE;  

    switch(pInput->CallbackType)  
    {  
    case ModuleCallback:  
        if(pOutput->ModuleWriteFlags & ModuleWriteDataSeg)  
            if(!IsDataSectionNeeded(pInput->Module.FullPath))  
                pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);  
    case IncludeModuleCallback:  
    case IncludeThreadCallback:  
    case ThreadCallback:  
    case ThreadExCallback:  
        return TRUE;  
    default:;  
    }  

    return FALSE;  
}  

static inline void CreateMiniDump(PEXCEPTION_POINTERS pep, string strFileName)  
{  
    HANDLE hFile = CreateFileA(strFileName.c_str(), GENERIC_READ | GENERIC_WRITE,  
        FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);  

    if((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))  
    {  
        MINIDUMP_EXCEPTION_INFORMATION mdei;  
        mdei.ThreadId           = GetCurrentThreadId();  
        mdei.ExceptionPointers  = pep;  
        mdei.ClientPointers     = NULL;  

        MINIDUMP_CALLBACK_INFORMATION mci;  
        mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;  
        mci.CallbackParam       = 0;  

        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);  

        CloseHandle(hFile);  
    }  
    if (callback != nullptr)
    {
        callback();
    }
}  

static LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)  
{  
    CreateMiniDump(pExceptionInfo, g_szDumpFileName);  

    return EXCEPTION_EXECUTE_HANDLER;  
}  

// 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效  
static void DisableSetUnhandledExceptionFilter()  
{  
    void* addr = (void*)GetProcAddress(LoadLibraryA("kernel32.dll"), "SetUnhandledExceptionFilter");  
    if (addr)  
    {  
        unsigned char code[16];  
        int size = 0;  

        code[size++] = 0x33;  
        code[size++] = 0xC0;  
        code[size++] = 0xC2;  
        code[size++] = 0x04;  
        code[size++] = 0x00;  

        DWORD dwOldFlag, dwTempFlag;  
        VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);  
        WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);  
        VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);  
    }  
}  

static void InitMinDump()  
{  
    //注册异常处理函数  
    SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);  

    //使SetUnhandledExceptionFilter  
    DisableSetUnhandledExceptionFilter();  
}

#endif

namespace util {

CMiniDump::CMiniDump(string szFileName)
{
    g_szDumpFileName = szFileName;
#ifdef WIN32
    InitMinDump();
    SetErrorMode(SEM_NOGPFAULTERRORBOX);
#endif
}

CMiniDump::~CMiniDump()
{

}

void CMiniDump::SetCallback(void (*p)())
{
    callback = p;
}

}