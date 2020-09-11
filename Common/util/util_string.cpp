#include "util_string.h"
#include <sstream>
#include <set>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <algorithm> 

namespace util {

#ifdef LINUX_IMPL
char* _strupr(char *str)
{
	char *ptr = str;
	while (*ptr != 0x0)
	{
		if (islower(*ptr))
		{
			*ptr = toupper(*ptr);
		}
		ptr++;
	}
	return str;	
}
#endif

std::string String::upper(std::string str)
{
    std::string strRet(str);
    std::transform(strRet.begin(), strRet.end(), strRet.begin(), toupper);
    return strRet;
}

std::string String::lower(std::string str)
{
    std::string strRet(str);
    std::transform(strRet.begin(), strRet.end(), strRet.begin(), tolower);
    return strRet;
}

std::string String::replace(std::string &s, char src, char dst)
{
    std::string strRet(s);
    size_t size = strRet.size();
    for(size_t i = 0; i<size; i++) {
        if(strRet[i] == src)
            strRet[i] = dst;
    }
    return strRet;
}

std::string String::replace(std::string &str, std::string src, std::string dst)
{
    string strRet;
    size_t size = str.size();
    size_t len = src.size();
    strRet.reserve(size+1);
    size_t i = 0;
    for(; i < size-len; i++) {
        bool find = true;
        for(int n=0; n<len; n++) {
            if(str[n+i] != src[n]) {
                find = false;
                break;
            }
        }
        if(find) {
            i+=(len-1);
            strRet.append(dst);
        } else {
            strRet.push_back(str[i]);
        }
    }
    for(; i < size; i++) {
        strRet.push_back(str[i]);
    }
    return strRet;
}

std::vector<std::string> String::split(const std::string &s, const char tag)
{
	std::string res;
	std::vector<std::string> vecNum;
	for (size_t i=0; i<s.size(); i++)
	{
		char c = s[i];
		if (c != tag)
		{
			res.push_back(c);
		}
		else if(!res.empty())
		{
			vecNum.push_back(res);
			res = "";
		}
	}
	if(!res.empty())
	{
		vecNum.push_back(res);
		res = "";
	}

	return vecNum;
}

std::vector<std::string> String::split(const std::string &s, char* tag, int nLen )
{
    std::vector<std::string> pathVec;

    if (NULL == tag)
    {
        return pathVec;
    }

    int nIndexFirst = 0, nIndexSecond = 0, nMaxIndex = s.length()-1;

    // 	const char chSeparator1 = '\\';
    // 	const char chSeparator2 = '/';

    char chTemp = 0;
    bool bStartFlag = false;
    for (; nIndexFirst <= nMaxIndex; ++nIndexFirst )
    {
        chTemp = s.at(nIndexFirst);
        bool bMachFlag = false;
        for (int nIndex = 0; nIndex < nLen; ++nIndex)
        {
            if (*(tag+nIndex) == chTemp)
            {
                bMachFlag = true;
                break;
            }
        }

        if ( bMachFlag )
        {
            if (bStartFlag)
            {
                //此为结尾分隔
                pathVec.push_back(s.substr(nIndexSecond, nIndexFirst - nIndexSecond));
                //nIndexSecond = nIndexFirst;
                bStartFlag = false;
            }
            else
            {
                pathVec.push_back("");
                //此为起始分隔符，不做操作
            }
        }    
        else if (!bStartFlag)
        {
            //记录起始位置
            nIndexSecond = nIndexFirst;
            bStartFlag = true;
        }
    }

    pathVec.push_back(s.substr(nIndexSecond, nIndexFirst - nIndexSecond + 1));
    return pathVec;
}

#if 0
std::vector<std::string> String::split(const std::string &s, const std::string &tag)
{
    std::string o_str = s;
    std::vector<std::string> str_list; // 存放分割后的字符串
    int comma_n = 0;
    while(true) {
        std::string tmp_s = "";
        comma_n = o_str.find( tag.c_str() );
        if( -1 == comma_n ) {
            tmp_s = o_str.substr( 0, o_str.length() );
            str_list.push_back( tmp_s );
            break;
        }
        tmp_s = o_str.substr( 0, comma_n );
        o_str.erase( 0, comma_n+tag.size() );
        str_list.push_back( tmp_s );
    }

    return str_list;
}
#else
std::vector<std::string> String::split(const std::string &s, const std::string &tag)
{
    std::string res;
    std::vector<std::string> vecNum;
    size_t nSrcSize = s.size();
    size_t nSplSize = tag.size();
    if (nSplSize > nSrcSize) {
        vecNum.push_back(s);
        return vecNum;
    }

    size_t nEndPos = nSrcSize - nSplSize;
    for (size_t i=0; i<nSrcSize; i++) {
        char c = s[i];
        if (i <= nEndPos) {
            std::string str = s.substr(i,nSplSize);
            if (str != tag) {
                res.push_back(c);
            } else {
                i += (nSplSize - 1);
                if(!res.empty()) {
                    vecNum.push_back(res);
                    res = "";
                }
            }
        } else {
            res.push_back(c);
        }
    }
    if(!res.empty()) {
        vecNum.push_back(res);
        res = "";
    }
    return vecNum;
}
#endif

std::string String::remove(const std::string &strSrc, const std::string str)
{
    string strRet;
    size_t size = strSrc.size();
    size_t len = str.size();
    strRet.reserve(size+1);
    size_t i = 0;
    for(; i < size-len; i++) {
        bool find = true;
        for(int n=0; n<len; n++) {
            if(strSrc[n+i] != str[n]) {
                find = false;
                break;
            }
        }
        if(find) {
            i+=len;
        } else {
            strRet.push_back(strSrc[i]);
        }
    }
    for(; i < size; i++) {
        strRet.push_back(str[i]);
    }
    return strRet;
}

std::string String::remove(const std::string &strSrc, char ch)
{
    string strRet;
    strRet.reserve(strSrc.size()+1);
    for(auto c : strSrc) {
        if(c != ch) {
            strRet.push_back(c);
        }
    }
    return strRet;
}

std::string String::removeNonnumeric(std::string str)
{
	std::string strRet;
	strRet.reserve(str.size()+1);
	for(auto c : str) {
		if (c>=48 && c<=57) {
			strRet.push_back(c);
		}
	}
	return strRet;
}

// 判断字符串是否全是数字
bool String::IsNumber(const std::string &strValue)
{
	for (size_t i = 0; i < strValue.size(); i++) 
	{
		if (strValue[i] < '0' || strValue[i] > '9')
		{
			return false;
		}
	}
	return true;
}
bool String::IsNumber(const std::wstring &wstrValue)
{
	for (size_t i = 0; i < wstrValue.size(); i++) 
	{
		if (wstrValue[i] != '0'
			&& wstrValue[i] != '1'
			&& wstrValue[i] != '2'
			&& wstrValue[i] != '3'
			&& wstrValue[i] != '4'
			&& wstrValue[i] != '5'
			&& wstrValue[i] != '6'
			&& wstrValue[i] != '7'
			&& wstrValue[i] != '8'
			&& wstrValue[i] != '9')
		{
			return false;
		}
	}
	return true;
}

bool String::IsEng(std::string strIn,int nLen)
{
	bool bOk=false;
	std::string strSelData;	
	if(strIn.empty() || strIn=="NULL")
	{
		return false;
	}
	for(unsigned int i=0;i<strIn.length();i++)
	{
		if(
			(strIn[i]>='a' && strIn[i]<='z') ||
			(strIn[i]>='A' && strIn[i]<='Z')
			)
		{
			bOk = true;
		}
		else
		{
			bOk = false;
			break;
		}
	}

	if(bOk && strIn.size()==(size_t)nLen)
	{
		return true;
	}
	return false;
}

bool String::IsEng2(std::string strIn,int nLen)
{
	bool bOk=false;
	std::string strSelData;

	if(strIn.empty() || strIn=="NULL")
	{
		return false;
	}
	for(unsigned int i = 0; i < strIn.length(); i++)
	{
		if(
			(strIn[i] >= 'a' && strIn[i] <= 'z') ||
			(strIn[i] >= 'A' && strIn[i] <= 'Z')
			)
		{
			bOk = true;
		}
		else
		{
			if(-1 <= strIn[i] && 255 >= strIn[i] && isspace(strIn[i]))
			{
				continue;
			}
			bOk = false;
			break;
		}
	}

	if(bOk && strIn.size()==(size_t)nLen)
	{
		return true;
	}
	return false;
}

bool String::IsChes(std::string strIn)
{
	if(IsNumber(strIn))
	{
		return false;
	}
	else if(IsEng(strIn,strIn.size()))
	{
		return false;
	}
	
    return true;
}



bool String::HexStr2Ascii(std::string hexStr, std::string &asciiStr)
{
	// 判断输入时数字
	int len = hexStr.length();
	if (0 != len%2)
	{
		return false;
	}
	for(int i=0; i<len; i+=2)
	{
		std::string temp = hexStr.substr(i, 2);
		long ch;
		ch = strtol(temp.c_str(), NULL, 16);
		// 判断不可见字符
		if (iscntrl(ch))
		{
			return false;
		}
		asciiStr.append(1, (char)ch);
	}

	return true;
}


std::string String::StringTrimRight(const std::string strSrc, const std::string strDest)
{
	int nLen = strSrc.length();
	if (nLen <= 0)
	{
		return strSrc;
	}

	int nLen2 = strDest.length();
	if (nLen2 > nLen)
	{
		return strSrc;
	}

	std::string strSubEnd = strSrc.substr(nLen-nLen2,nLen2);
	if (strSubEnd == strDest)
	{
		return strSrc.substr(0,nLen-nLen2);
	}

	return strSrc;
}

std::string String::StringTrimRight(const std::string strSrc, const char cDest)
{
	int nLen = strSrc.length();
	if (nLen <= 0)
	{
		return strSrc;
	}

	while (strSrc.at(nLen-1) == cDest)
	{
		--nLen;
		if(nLen == 0)
		{
			return "";
		}
	}
	return strSrc.substr(0,nLen);
}

std::wstring String::StringTrimRight(const std::wstring strSrc, const std::wstring strDest)
{
	int nLen = strSrc.length();
	if (nLen <= 0)
	{
		return strSrc;
	}

	int nLen2 = strDest.length();
	if (nLen2 > nLen)
	{
		return strSrc;
	}

	std::wstring strSubEnd = strSrc.substr(nLen-nLen2,nLen2);
	if (strSubEnd == strDest)
	{
		return strSrc.substr(0,nLen-nLen2);
	}

	return strSrc;
}

std::wstring String::StringTrimRight(const std::wstring strSrc, const wchar_t cDest)
{
	int nLen = strSrc.length();
	if (nLen <= 0)
	{
		return strSrc;
	}

	while (strSrc.at(nLen-1) == cDest)
	{
		--nLen;
		if(nLen == 0)
		{
			return L"";
		}
	}
	return strSrc.substr(0,nLen);
}

std::string String::StringTrimVector(std::string &strSrc, char split)
{
	std::string res;
	std::set<std::string> setNum;
	size_t nSrcSize = strSrc.size();
	for (size_t i=0; i<nSrcSize; i++)
	{
		char c = strSrc[i];
		if (c != split)
		{
			res.push_back(c);
		}
		else if(!res.empty())
		{
			setNum.insert(res);
			res = "";
		}
	}
	if(!res.empty())
	{
		setNum.insert(res);
		res = "";
	}
	for(std::set<std::string>::iterator it = setNum.begin();
		it != setNum.end(); it++)
	{
		if (!res.empty())
		{
			res += split;
		}
		res += *it;
	}
	return res;
}

std::string String::StringTrimVector(std::string &strSrc, std::string split)
{
	size_t nSrcSize = strSrc.size();
	size_t nSplSize = split.size();
	if (nSplSize > nSrcSize)
	{
		return strSrc;
	}

	std::string res;
	std::set<std::string> setNum;
	size_t nEndPos = nSrcSize - nSplSize;
	for (size_t i=0; i<nSrcSize; i++)
	{
		char c = strSrc[i];
		if (i <= nEndPos)
		{
			std::string str = strSrc.substr(i,nSplSize);
			if (str != split)
			{
				res.push_back(c);
			}
			else
			{
				i += (nSplSize - 1);
				if(!res.empty())
				{
					setNum.insert(res);
					res = "";
				}
			}
		}
		else
		{
			res.push_back(c);
		}
	}
	if(!res.empty())
	{
		setNum.insert(res);
		res = "";
	}
	for(std::set<std::string>::iterator it = setNum.begin();
		it != setNum.end(); it++)
	{
		if (!res.empty())
		{
			res += split;
		}
		res += *it;
	}
	return res;
}


void String::utf8_cut(std::string &strContent, unsigned unLength)
{
	if (strContent.length() <= unLength)		
	{
		return;
	}

	const char *pStart = strContent.c_str();	
	const char *pEnd = strContent.c_str() + unLength;//截取长度少一位	2014/07/07	陈曙光
	char *p = NULL;	
	p = utf8_find_prev_char(pStart, pEnd);
	if (p != NULL)
	{
		strContent.assign(pStart, p - pStart);
	}	
}

char *String::utf8_find_prev_char(const char *str, const char *p)
{
	for (; p >= str; --p)
	{
		if ((*p & 0xc0) != 0x80)
			return (char *)p;
	}
	return NULL;
}

std::string String::strMakerUper( std::string str )
{
	char *chBuffer = (char*)calloc(1, str.size()+1);
	strcpy(chBuffer, str.c_str());
	std::string ret(_strupr(chBuffer));
	free(chBuffer);
	return ret;
}

std::wstring String::IPUL2STR(unsigned long ulIP)
{
	unsigned char ip[4] = {0x0};
	wchar_t tmp[16] = {0x0};
	ip[3] = (char)(ulIP>>24);
	ip[2] = (char)((ulIP&0x00FF0000)>>16);
	ip[1] = (char)((ulIP&0x0000FF00)>>8);
	ip[0] = (char)(ulIP&0x000000FF);
#ifdef _WIN32
    swprintf_s(tmp, L"%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);
#else
    swprintf(tmp, 16, L"%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);
#endif // _WIN32

	
	return tmp;
}

std::string String::GetSizeStr(unsigned long long nInt)
{
	std::stringstream ss;
	unsigned long long nsize = nInt;
	std::string str = "";
	if (nsize/1024/1024/1024 > 1)
	{
		//G
		nsize = nsize/1024/1024/1024;
		str = "G";
	}
	else if (nsize/1024/1024 > 1)
	{
		//M
		nsize = nsize/1024/1024;
		str = "M";
	}
	else if (nsize/1024 > 1)
	{
		//K
		nsize = nsize/1024;
		str = "K";
	}
	ss<<nsize;
	ss<<str;
	return ss.str();
}

std::string String::CutData(std::string strIn,int nLen)
{
	std::string data;

	//if(nLen<strIn.length())
	//{
	//	data=strIn.substr(0,nLen);
	//}
	//else
	//{
	//	data=strIn;
	//}

	if((size_t)nLen<strIn.length())
	{
		data = strIn;
		utf8_cut(data,nLen);
	}
	else
	{
		data = strIn;
	}
	return data;

}

std::string String::FilterSpecialChar(std::string &strData)
{
	std::string data;
	remove(strData,'\t');
	remove(strData,'\r');
	remove(strData,'\n');
	remove(strData,"\r\n");
	remove(strData,"\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00");
	remove(strData,"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20");
	remove(strData,"\x1A\x00");
	return strData;
}

void String::Trim(std::string &str)
{
	str.erase(str.find_last_not_of(' ') + 1, std::string::npos);
	str.erase(0, str.find_first_not_of(' '));
}

std::string String::WinPath2UnixPath(const std::string strSrc)
{
	std::string strPath = "";
	if( strPath.find("\\") )
	{
		strPath = strSrc;
		replace(strPath,"\\","/");
	}
	return strPath;
}

std::string String::dec2hex(int i)
{
    std::stringstream ioss; //定义字符串流
    std::string s_temp;     //存放转化后字符
    ioss << std::hex << i;  //以十六制形式输出
    ioss >> s_temp; 
    return s_temp;
}

bool String::isSubStr(std::string str1, std::string str2)
{
	size_t a = str1.size();
	if(a > str2.size()) a = str2.size();
	for (size_t i=0; i<a; ++i)
	{
		if(str1[i] != str2[i])
			return false;
	}
	return true;
}

std::string StringFormat(const char* format, ...) {
    size_t size = 4096;
    std::string buffer(size, '\0');
    char* buffer_p = const_cast<char*>(buffer.data());
    int expected = 0;
    va_list ap;

    while (true) {
        va_start(ap, format);
        expected = vsnprintf(buffer_p, size, format, ap);
        va_end(ap);
        if (expected>-1 && expected<=static_cast<int>(size)) {
            break;
        } else {
            /* Else try again with more space. */
            if (expected > -1)    /* glibc 2.1 */
                size = static_cast<size_t>(expected + 1); /* precisely what is needed */
            else           /* glibc 2.0 */
                size *= 2;  /* twice the old size */

            buffer.resize(size);
            buffer_p = const_cast<char*>(buffer.data());
        }
    }

    // expected不包含字符串结尾符号，其值等于：strlen(buffer_p)
    return std::string(buffer_p, expected>0?expected:0);
}

}