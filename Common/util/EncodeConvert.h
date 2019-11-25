/**
 * 该文件包含一个用来进行字符编码转换的类
 */
#pragma once
#include "util_public.h"

#if defined(_UNICODE) || defined(UNICODE)
#define TtoA EncodeConvert::WtoA
#define AtoT EncodeConvert::AtoW
#define WtoT(a) (a)
#define TtoW(a) (a)
typedef std::wstring _tstring;
#else
#define TtoA(a) (a)
#define AtoT(a) (a)
#define WtoT EncodeConvert::WtoA
#define TtoW EncodeConvert::AtoW
typedef std::string _tstring;
#endif

class UTIL_API EncodeConvert
{
public:
static string WtoA(const wchar_t* pwszSrc);
static string WtoA(const std::wstring &strSrc);

static wstring AtoW(const char* pszSrc);
static wstring AtoW(const std::string &strSrc);

static string WtoUTF8(const wchar_t* pwszSrc);
static string WtoUTF8(const std::wstring &strSrc);

static wstring UTF8toW(const char* pszSrc);
static wstring UTF8toW(const std::string &strSr);

static string AtoUTF8(const char* src);
static string AtoUTF8(const std::string &src);

static string UTF8toA(const char* src);
static string UTF8toA(const std::string &src);

// 检测一个以 null 结尾的字符串是否是UTF-8, 如果返回0, 也只表示这个串刚好符合UTF8的编码规则.
// 返回值说明: 
// 1 -> 输入字符串符合UTF-8编码规则
// -1 -> 检测到非法的UTF-8编码首字节
// -2 -> 检测到非法的UTF-8字节编码的后续字节.
static int IsTextUTF8(const char* pszSrc); 
};