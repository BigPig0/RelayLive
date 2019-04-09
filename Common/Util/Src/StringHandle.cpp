#include "StringHandle.h"
#include <sstream>
#include <set>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __QZ_LINUX__
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

#else

#endif


StringHandle::StringHandle()
{
}

StringHandle::~StringHandle()
{
}

std::string StringHandle::replace(std::string &s)
{
	size_t i = 0,pos = std::string::npos;
	std::string olds = "^#",news = "\\D";
	while((i = s.find_last_of(olds,pos)) != std::string::npos)
	{
		s =  s.replace(i,news.length(),news);
		pos = i - 1;
	}

	olds = "#",news = "\\d";
	pos = std::string::npos;
	while((i = s.find_last_of(olds,pos)) != std::string::npos)
	{
		s = s.replace(i,news.length(),news);
		pos = i - 1;
	}
	return s;
}



std::string StringHandle::StringUper(std::string str )
{
	char chBuffer[255] = {0};
	strcpy_s(chBuffer, str.c_str());

    char *ptr = chBuffer;
    while (*ptr != '\0')
    {
        if (isascii(*ptr))
        {
            if (islower(*ptr))
            {
                *ptr = toupper(*ptr);
            }
        }
        ptr++;
    }

	//return std::string(strupr(chBuffer));
    return std::string(chBuffer);
}


std::vector<std::string> StringHandle::StringCompart(std::string strSrc, char* pChar, int nLen )
{
	std::vector<std::string> pathVec;

	if (NULL == pChar)
	{
		return pathVec;
	}

	int nIndexFirst = 0, nIndexSecond = 0, nMaxIndex = strSrc.length()-1;

	// 	const char chSeparator1 = '\\';
	// 	const char chSeparator2 = '/';

	char chTemp = 0;
	bool bStartFlag = false;
	for (; nIndexFirst <= nMaxIndex; ++nIndexFirst )
	{
		chTemp = strSrc.at(nIndexFirst);
		bool bMachFlag = false;
		for (int nIndex = 0; nIndex < nLen; ++nIndex)
		{
			if (*(pChar+nIndex) == chTemp)
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
				pathVec.push_back(strSrc.substr(nIndexSecond, nIndexFirst - nIndexSecond));
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

	pathVec.push_back(strSrc.substr(nIndexSecond, nIndexFirst - nIndexSecond + 1));
	return pathVec;
}

std::string StringHandle::StringWipe( const std::string strSrc, const std::string strDest )
{
	size_t nIndexStart = 0, nIndexEnd = 0;

	std::string strTmp = strSrc;
	while(true)
	{
		nIndexStart = strTmp.find(strDest);

		if (std::string::npos == nIndexStart )
		{
			break;
		}
		nIndexEnd = nIndexStart + strDest.length() - 1;

		std::string strFirst = strTmp.substr(0, nIndexStart);
		std::string strSecond = strTmp.substr(nIndexEnd+1, strTmp.length()-nIndexEnd-1);

		strTmp = strFirst + strSecond;
	}

	return strTmp;
}

std::vector<std::string> StringHandle::StringSplit(const std::string s, const char tag)
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

std::string StringHandle::RemoveUnDig(std::string str)
{
	std::string newStr;

	int len = str.length();
	if (0 == len)
	{
		return "";
	}
	char *p = (char*)str.c_str();

	newStr.reserve(len+1);

	for(int i=0; i<len; i++)
	{
		if (p[i]>=48 && p[i]<=57)
		{
			newStr.append(1, p[i]);
		}
	}

	return newStr;
}

// 判断字符串是否全是数字
bool StringHandle::IsNumber(const std::string &strValue)
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
bool StringHandle::IsNumber(const std::wstring &wstrValue)
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

bool StringHandle::IsEng(std::string strIn,int nLen)
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

	if(bOk && strIn.length()==nLen)
	{
		return true;
	}
	return false;
}

bool StringHandle::IsEng2(std::string strIn,int nLen)
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

	if(bOk && strIn.length()==nLen)
	{
		return true;
	}
	return false;
}

bool StringHandle::IsChes(std::string strIn)
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



bool StringHandle::HexStr2Ascii(std::string hexStr, std::string &asciiStr)
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


std::string StringHandle::StringTrimRight(const std::string strSrc, const std::string strDest)
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

std::string StringHandle::StringTrimRight(const std::string strSrc, const char cDest)
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

std::wstring StringHandle::StringTrimRight(const std::wstring strSrc, const std::wstring strDest)
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

std::wstring StringHandle::StringTrimRight(const std::wstring strSrc, const wchar_t cDest)
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

std::string StringHandle::StringTrimVector(std::string &strSrc, char split)
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

std::string StringHandle::StringTrimVector(std::string &strSrc, std::string split)
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


void StringHandle::utf8_cut(std::string &strContent, unsigned unLength)
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

char *StringHandle::utf8_find_prev_char(const char *str, const char *p)
{
	for (p; p >= str; --p)
	{
		if ((*p & 0xc0) != 0x80)
			return (char *)p;
	}
	return NULL;
}

std::string StringHandle::strMakerUper( std::string str )
{
	char chBuffer[255] = {0};
	strcpy_s(chBuffer, str.c_str());
	return std::string(_strupr(chBuffer));
}

std::wstring StringHandle::IPUL2STR(unsigned long ulIP)
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

std::string StringHandle::GetSizeStr(unsigned long long nInt)
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

bool StringHandle::RemoveChar(std::string &str,char ch)
{
	if(str.empty() || "NULL"==str)
		return false;
	std::string::size_type npos(0);
	while(npos<str.length())
	{
		npos=str.find(ch);
		if(npos<0 || npos>str.length())
			break;
		else
			str.erase(npos,1);
	}
	return true;
}

bool StringHandle::RemoveEnterSymb(std::string &str, std::string strIn)
{
	if(str.empty() || "NULL"==str)
		return false;
	std::string::size_type npos(0);
	while(npos < str.length())
	{
		npos = str.find(strIn);
		if(npos < 0 || npos > str.length())
			break;
		else
			str.erase(npos, strIn.length());	//删除匹配字符
	}
	return true;
}

bool StringHandle::replaceEnterSymb(std::string &str, std::string strOld, std::string strNew)
{
	if(str.empty() || "NULL"==str)
		return false;
	std::string::size_type npos(0);

	while(npos < str.length())
	{
		npos = str.find(strOld);
		if(npos < 0 || npos > str.length())
			break;
		else
		{
			str.replace(npos, strOld.length(), strNew);	//替换字符
		}
	}
	return true;
}

std::string StringHandle::CutData(std::string strIn,int nLen)
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

	if(nLen<strIn.length())
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

std::string StringHandle::FilterSpecialChar(std::string &strData)
{
	std::string data;
	RemoveChar(strData,'\t');
	RemoveChar(strData,'\r');
	RemoveChar(strData,'\n');
	RemoveEnterSymb(strData,"\r\n");
	RemoveEnterSymb(strData,"\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00\x20\x00");
	RemoveEnterSymb(strData,"\x20\x20\x20\x20\x20\x20\x20\x20\x20\x20");
	RemoveEnterSymb(strData,"\x1A\x00");
	//CUtilsModule::RemoveEnterSymb(strData,"\x1A");
	//CUtilsModule::RemoveChar(strData,'\'');
	//CUtilsModule::RemoveChar(strData,'\"');
	return strData;
}

void StringHandle::Trim(std::string &str)
{
	str.erase(str.find_last_not_of(' ') + 1, std::string::npos);
	str.erase(0, str.find_first_not_of(' '));
}


std::vector<std::string> StringHandle::SplitData(std::string &strRecData,std::string strSep)
{
	std::vector<std::string> vecFieldData;
	std::string o_str;
	o_str = strRecData;
	std::vector<std::string> str_list; // 存放分割后的字符串
	int comma_n = 0;
	do
	{
		std::string tmp_s = "";
		//comma_n = o_str.find( "\t" );
		comma_n = o_str.find( strSep.c_str() );
		if( -1 == comma_n )
		{
			tmp_s = o_str.substr( 0, o_str.length() );
			str_list.push_back( tmp_s );
			break;
		}
		tmp_s = o_str.substr( 0, comma_n );
		o_str.erase( 0, comma_n+1 );
		str_list.push_back( tmp_s );
	}
	while(true);

	vecFieldData = str_list;

	if(vecFieldData.size()==1 && vecFieldData[0] == strRecData)
	{
		vecFieldData.clear();
	}

	return vecFieldData;
}

std::string StringHandle::WinPath2UnixPath(const std::string strSrc)
{
	std::string strPath = "";
	if( strPath.find("\\") )
	{
		strPath = strSrc;
		replaceEnterSymb(strPath,"\\","/");
	}
	return strPath;
}

std::string StringHandle::dec2hex(int i)
{
    std::stringstream ioss; //定义字符串流
    std::string s_temp;     //存放转化后字符
    ioss << std::hex << i;  //以十六制形式输出
    ioss >> s_temp; 
    return s_temp;
}

bool StringHandle::isSubStr(std::string str1, std::string str2)
{
	int a = str1.size();
	if(a > str2.size()) a = str2.size();
	for (int i=0; i<a; ++i)
	{
		if(str1[i] != str2[i])
			return false;
	}
	return true;
}