#include "EncodeConvert.h"
#include <Windows.h>


std::string __do_w_to_a_utf8(const wchar_t *pwszText, UINT uCodePage)
{
	// 空指针输入
	if(pwszText == NULL) return "";

	// 无法计算需要的长度.
	int nNeedSize = WideCharToMultiByte(uCodePage, 0, pwszText, -1, NULL, 0, NULL, NULL);
	if( 0 == nNeedSize ) return "";

	// 分配空间,转换.
	char *pRet = new char[nNeedSize + 1]; // 虽然返回WideCharToMultiByte的长度是包含 null 字符的长度, 还是多+一个字符.
	memset(pRet, 0, nNeedSize + 1);

	std::string strRet("");
	if ( 0 == WideCharToMultiByte(uCodePage, 0, pwszText, -1, pRet, nNeedSize, NULL, NULL) )
	{
	}
	else
	{
		strRet = pRet;
	}

	delete []pRet;
	return strRet;
}

std::wstring __do_a_utf8_to_w(const char* pszText, UINT uCodePage)
{
	// 空指针
	if( pszText == NULL ) return L"";

	// 计算长度
	int nNeedSize = MultiByteToWideChar(uCodePage, 0, pszText, -1, NULL, 0);
	if( 0 == nNeedSize ) return L"";

	// 分配空间,转换
	std::wstring strRet(L"");
	wchar_t *pRet = new wchar_t[nNeedSize + 1];
	memset(pRet, 0, (nNeedSize + 1) * sizeof(wchar_t));
	if( 0 == MultiByteToWideChar(uCodePage, 0, pszText, -1, pRet, nNeedSize) )
	{
	}
	else
	{
		strRet = pRet;
	}
	delete []pRet;
	return strRet;
}

std::string EncodeConvert::WtoA(const std::wstring &strText)
{
	return __do_w_to_a_utf8(strText.c_str(), CP_ACP);
}

std::string EncodeConvert::WtoA(const wchar_t *pwszText)
{
	return __do_w_to_a_utf8(pwszText, CP_ACP);
}

std::wstring EncodeConvert::AtoW(const std::string &strText)
{
	return __do_a_utf8_to_w(strText.c_str(), CP_ACP);
}

std::wstring EncodeConvert::AtoW(const char* pszText)
{
	return __do_a_utf8_to_w(pszText, CP_ACP);
}

std::string EncodeConvert::WtoUTF8(const std::wstring &strText)
{
	return __do_w_to_a_utf8(strText.c_str(), CP_UTF8);
}

std::string EncodeConvert::WtoUTF8(const wchar_t *pwszText)
{
	return __do_w_to_a_utf8(pwszText, CP_UTF8);
}

std::wstring EncodeConvert::UTF8toW(const std::string &strText)
{
	return __do_a_utf8_to_w(strText.c_str(), CP_UTF8);
}

std::wstring EncodeConvert::UTF8toW(const char* pszText)
{
	return __do_a_utf8_to_w(pszText, CP_UTF8);
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
UTF-8 编码最多可以有6个字节

1字节 0xxxxxxx
2字节 110xxxxx 10xxxxxx
3字节 1110xxxx 10xxxxxx 10xxxxxx
4字节 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
5字节 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
6字节 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

*/

// 返回值说明: 
// 0 -> 输入字符串符合UTF-8编码规则
// -1 -> 检测到非法的UTF-8编码首字节
// -2 -> 检测到非法的UTF-8字节编码的后续字节.

int EncodeConvert::IsTextUTF8(const char* pszSrc)
{
	const unsigned char* puszSrc = (const unsigned char*)pszSrc; // 一定要无符号的,有符号的比较就不正确了.
	// 看看有没有BOM表示 EF BB BF
	if( puszSrc[0] != 0 && puszSrc[0] == 0xEF && 
		puszSrc[1] != 0 && puszSrc[1] == 0xBB &&
		puszSrc[2] != 0 && puszSrc[2] == 0xBF)
	{
		return 0;
	}

	// 如果没有 BOM标识
	bool bIsNextByte = false;
	int nBytes = 0; // 记录一个字符的UTF8编码已经占用了几个字节.
	const unsigned char* pCur = (const unsigned char*)pszSrc; // 指针游标用无符号字符型. 因为高位为1, 如果用 char 型, 会变为负数,不利于编程时候的比较操作.
	
	while( pCur[0] != 0 )
	{
		if(!bIsNextByte)
		{
			bIsNextByte = true;
			if ( (pCur[0] >> 7) == 0) { bIsNextByte = false; nBytes = 1; bIsNextByte = false; } // 最高位为0, ANSI 兼容的.
			else if ((pCur[0] >> 5) == 0x06) { nBytes = 2; } // 右移5位后是 110 -> 2字节编码的UTF8字符的首字节
			else if ((pCur[0] >> 4) == 0x0E) { nBytes = 3; } // 右移4位后是 1110 -> 3字节编码的UTF8字符的首字节
			else if ((pCur[0] >> 3) == 0x1E) { nBytes = 4; } // 右移3位后是 11110 -> 4字节编码的UTF8字符的首字节
			else if ((pCur[0] >> 2) == 0x3E) { nBytes = 5; } // 右移2位后是 111110 -> 5字节编码的UTF8字符的首字节
			else if ((pCur[0] >> 1) == 0x7E) { nBytes = 6; } // 右移1位后是 1111110 -> 6字节编码的UTF8字符的首字节
			else
			{
				nBytes = -1; // 非法的UTF8字符编码的首字节
				break;
			}
		}
		else
		{
			if ((pCur[0] >> 6) == 0x02) // 首先,后续字节必须以 10xxx 开头
			{
				nBytes--;
				if (nBytes == 1) bIsNextByte = false; // 当 nBytes = 1时, 说明下一个字节应该是首字节.
			}
			else
			{
				nBytes = -2;
				break;
			}
		}

		// 下跳一个字符
		pCur++;
	}

	if( nBytes == 1) return 0;
	else return nBytes;
}
