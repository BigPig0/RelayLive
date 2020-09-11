#include "EncodeConvert.h"
#if defined(WINDOWS_IMPL)        /**Windows*/
#include <windows.h>
#elif defined(LINUX_IMPL)        /**Linux*/
#include <stdlib.h>
#endif
#include <string.h>

#if defined(WINDOWS_IMPL)
static std::string __ws2s(const wchar_t *pwszText, UINT uCodePage)
{
	// ��ָ������
	if(pwszText == NULL) return "";

	// �޷�������Ҫ�ĳ���.
	int nNeedSize = WideCharToMultiByte(uCodePage, 0, pwszText, -1, NULL, 0, NULL, NULL);
	if( 0 == nNeedSize ) return "";

	// ����ռ�,ת��.
	char *pRet = new char[nNeedSize + 1]; // ��Ȼ����WideCharToMultiByte�ĳ����ǰ��� null �ַ��ĳ���, ���Ƕ�+һ���ַ�.
	memset(pRet, 0, nNeedSize + 1);

	std::string strRet("");
	if ( 0 != WideCharToMultiByte(uCodePage, 0, pwszText, -1, pRet, nNeedSize, NULL, NULL) )
    {
		strRet = pRet;
	}

	delete []pRet;
	return strRet;
}

static std::wstring __s2ws(const char* pszText, UINT uCodePage)
{
	// ��ָ��
	if( pszText == NULL ) return L"";

	// ���㳤��
	int nNeedSize = MultiByteToWideChar(uCodePage, 0, pszText, -1, NULL, 0);
	if( 0 == nNeedSize ) return L"";

	// ����ռ�,ת��
	std::wstring strRet(L"");
	wchar_t *pRet = new wchar_t[nNeedSize + 1];
	memset(pRet, 0, (nNeedSize + 1) * sizeof(wchar_t));
	if( 0 != MultiByteToWideChar(uCodePage, 0, pszText, -1, pRet, nNeedSize) )
	{
		strRet = pRet;
	}
	delete []pRet;
	return strRet;
}
#elif defined(LINUX_IMPL)
static std::string __ws2s(const wchar_t *pwszText, const char *szCode)
{
    if (NULL == pwszText || wcslen(pwszText) == 0)
        return "";
    unsigned len = wcslen(pwszText) * 4 + 1;
    setlocale(LC_CTYPE, szCode);
    char *p = new char[len];
    wcstombs(p, pwszText, len);
    std::string str(p);
    delete[] p;
    return str;
}

static std::wstring __s2ws(const char* pszText, const char *szCode)
{
    if (NULL == pszText || strlen(pszText))
        return L"";
    unsigned len = strlen(pszText) + 1;
    setlocale(LC_CTYPE, szCode);
    wchar_t *p = new wchar_t[len];
    mbstowcs(p, pszText, len);
    std::wstring w_str(p);
    delete[] p;
    return w_str;
}
#endif

std::string EncodeConvert::WtoA(const std::wstring &strText)
{
#if defined(WINDOWS_IMPL)
	return __ws2s(strText.c_str(), CP_ACP);
#elif defined(LINUX_IMPL)
    return __ws2s(strText.c_str(), "");
#endif
}

std::string EncodeConvert::WtoA(const wchar_t *pwszText)
{
#if defined(WINDOWS_IMPL)
	return __ws2s(pwszText, CP_ACP);
#elif defined(LINUX_IMPL)
    return __ws2s(pwszText, "");
#endif
}

std::wstring EncodeConvert::AtoW(const std::string &strText)
{
#if defined(WINDOWS_IMPL)
	return __s2ws(strText.c_str(), CP_ACP);
#elif defined(LINUX_IMPL)
    return __s2ws(strText.c_str(), "");
#endif
}

std::wstring EncodeConvert::AtoW(const char* pszText)
{
#if defined(WINDOWS_IMPL)
	return __s2ws(pszText, CP_ACP);
#elif defined(LINUX_IMPL)
    return __s2ws(pszText, "");
#endif
}

std::string EncodeConvert::WtoUTF8(const std::wstring &strText)
{
#if defined(WINDOWS_IMPL)
	return __ws2s(strText.c_str(), CP_UTF8);
#elif defined(LINUX_IMPL)
    return __ws2s(strText.c_str(), "en_US.UTF-8");
#endif
}

std::string EncodeConvert::WtoUTF8(const wchar_t *pwszText)
{
#if defined(WINDOWS_IMPL)
	return __ws2s(pwszText, CP_UTF8);
#elif defined(LINUX_IMPL)
    return __ws2s(pwszText, "en_US.UTF-8");
#endif
}

std::wstring EncodeConvert::UTF8toW(const std::string &strText)
{
#if defined(WINDOWS_IMPL)
	return __s2ws(strText.c_str(), CP_UTF8);
#elif defined(LINUX_IMPL)
    return __s2ws(strText.c_str(), "en_US.UTF-8");
#endif
}

std::wstring EncodeConvert::UTF8toW(const char* pszText)
{
#if defined(WINDOWS_IMPL)
	return __s2ws(pszText, CP_UTF8);
#elif defined(LINUX_IMPL)
    return __s2ws(pszText, "en_US.UTF-8");
#endif
}

std::string EncodeConvert::UTF8toA(const std::string &src)
{
	return WtoA(UTF8toW(src));
}

std::string EncodeConvert::UTF8toA(const char *src)
{
	return WtoA(UTF8toW(src));
}

std::string EncodeConvert::AtoUTF8(const std::string &src)
{
	return WtoUTF8(AtoW(src));
}

std::string EncodeConvert::AtoUTF8(const char* src)
{
	return WtoUTF8(AtoW(src));
}

/*
UTF-8 ������������6���ֽ�

1�ֽ� 0xxxxxxx
2�ֽ� 110xxxxx 10xxxxxx
3�ֽ� 1110xxxx 10xxxxxx 10xxxxxx
4�ֽ� 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
5�ֽ� 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
6�ֽ� 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
*/

// ����ֵ˵��: 
// 0 -> �����ַ�������UTF-8�������
// -1 -> ��⵽�Ƿ���UTF-8�������ֽ�
// -2 -> ��⵽�Ƿ���UTF-8�ֽڱ���ĺ����ֽ�.

int EncodeConvert::IsTextUTF8(const char* pszSrc)
{
	const unsigned char* puszSrc = (const unsigned char*)pszSrc; // һ��Ҫ�޷��ŵ�,�з��ŵıȽϾͲ���ȷ��.
	// ������û��BOM��ʾ EF BB BF
	if( puszSrc[0] != 0 && puszSrc[0] == 0xEF && 
		puszSrc[1] != 0 && puszSrc[1] == 0xBB &&
		puszSrc[2] != 0 && puszSrc[2] == 0xBF)
	{
		return 0;
	}

	// ���û�� BOM��ʶ
	bool bIsNextByte = false;
	int nBytes = 0; // ��¼һ���ַ���UTF8�����Ѿ�ռ���˼����ֽ�.
	const unsigned char* pCur = (const unsigned char*)pszSrc; // ָ���α����޷����ַ���. ��Ϊ��λΪ1, ����� char ��, ���Ϊ����,�����ڱ��ʱ��ıȽϲ���.
	
	while( pCur[0] != 0 )
	{
		if(!bIsNextByte)
		{
			bIsNextByte = true;
			if ( (pCur[0] >> 7) == 0) { bIsNextByte = false; nBytes = 1; bIsNextByte = false; } // ���λΪ0, ANSI ���ݵ�.
			else if ((pCur[0] >> 5) == 0x06) { nBytes = 2; } // ����5λ���� 110 -> 2�ֽڱ����UTF8�ַ������ֽ�
			else if ((pCur[0] >> 4) == 0x0E) { nBytes = 3; } // ����4λ���� 1110 -> 3�ֽڱ����UTF8�ַ������ֽ�
			else if ((pCur[0] >> 3) == 0x1E) { nBytes = 4; } // ����3λ���� 11110 -> 4�ֽڱ����UTF8�ַ������ֽ�
			else if ((pCur[0] >> 2) == 0x3E) { nBytes = 5; } // ����2λ���� 111110 -> 5�ֽڱ����UTF8�ַ������ֽ�
			else if ((pCur[0] >> 1) == 0x7E) { nBytes = 6; } // ����1λ���� 1111110 -> 6�ֽڱ����UTF8�ַ������ֽ�
			else
			{
				nBytes = -1; // �Ƿ���UTF8�ַ���������ֽ�
				break;
			}
		}
		else
		{
			if ((pCur[0] >> 6) == 0x02) // ����,�����ֽڱ����� 10xxx ��ͷ
			{
				nBytes--;
				if (nBytes == 1) bIsNextByte = false; // �� nBytes = 1ʱ, ˵����һ���ֽ�Ӧ�������ֽ�.
			}
			else
			{
				nBytes = -2;
				break;
			}
		}

		// ����һ���ַ�
		pCur++;
	}

	if( nBytes == 1) return 0;
	else return nBytes;
}
