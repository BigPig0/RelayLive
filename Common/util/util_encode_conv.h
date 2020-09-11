/**
 * ���ļ�����һ�����������ַ�����ת������
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

namespace util {

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

// ���һ���� null ��β���ַ����Ƿ���UTF-8, �������0, Ҳֻ��ʾ������պ÷���UTF8�ı������.
// ����ֵ˵��: 
// 1 -> �����ַ�������UTF-8�������
// -1 -> ��⵽�Ƿ���UTF-8�������ֽ�
// -2 -> ��⵽�Ƿ���UTF-8�ֽڱ���ĺ����ֽ�.
static int IsTextUTF8(const char* pszSrc); 
};
};