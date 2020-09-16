#pragma once

#include "util_public.h"
#include <vector>

namespace util {

class UTIL_API String
{
public:

	/**
	 * @brief:将字符串中的所有字母变成大写
	 * @param str:原始字符串
	 * @return 改变后的字符串
	 */
	static std::string upper(std::string str);

    /**
	 * @brief:将字符串中的所有字母变成小写
	 * @param str:原始字符串
	 * @return 改变后的字符串
	 */
	static std::string lower(std::string str);

	/**
	 * @brief: 字符串替换
	 * @param: str 原始字符串
     * @param: src 需要被替换的内容
     * @param: dst 替换的内容
	 * @return:替换后的字符串
	 */
	static std::string replace(std::string&str, char src, char dst);

	/**
	 * @brief: 字符串替换
     * @param: str 原始字符串
	 * @param: src 需要被替换的内容
	 * @param: dst 替换的内容
	 * @return: 替换后的字符串
	 */
	static std::string replace(std::string &str, std::string src, std::string dst);

	/**
	 * @brief:字符串分割
	 * @param:s:原始字符串
	 * @param:tag:分割符号
	 * @return:std::vector<std::string>:分割后的字符串数组
	 */
	static std::vector<std::string>  split(const std::string &s, const char tag);

	/**
	 * @brief:字符串分割
	 * @param:strSrc:原始字符串
	 * @param:tag:分割符号（字符串）
	 * @param:nLen:分割符号的长度
	 * @return:std::vector<std::string>:分割后的字符串数组
	 */
	static std::vector<std::string>  split(const std::string &s, char* tag, int nLen);

	/**
     * @brief:字符串分割
     * @param:strSrc:原始字符串
     * @param:tag:分割符号（字符串）
     * @return:std::vector<std::string>:分割后的字符串数组
	 */
	static std::vector<std::string> split(const std::string &s, const std::string &tag);

	/**
	 * @brief:删除字符串中的字符串
	 * @param str: 原始字符串
	 * @param strDest: 要删除的字符串
	 * @return 删除后的字符串
	 */
	static std::string remove(const std::string &strSrc, const std::string str);

	/**
	 * @brief:删除字符串中的特定字符
	 * @param strSrc: 原始字符串
     * @param ch: 要删除的字符
	 * @return 删除后的字符串
	 */
	static std::string remove(const std::string &strSrc, char ch);

	/**
	 * @brief 去除字符串中非数字
	 * @param str 原始字符串
	 * @return 改变后的字符串
	 */
	static std::string removeNonnumeric(std::string str); 

	/**
	*@brief:删除字符串最右侧的字符串
	*@param:strSrc:原始字符串
	*@param:strDest:要删除的字符串
	*@return:std::string:删除后的字符串
	*/
	static std::string               StringTrimRight(const std::string strSrc, const std::string strDest);

	/**
	*@brief:删除字符串最右侧的字符
	*@param:strSrc:原始字符串
	*@param:cDest:要删除的字符
	*@return:std::string:删除后的字符串
	*/
	static std::string               StringTrimRight(const std::string strSrc, const char cDest);

	/**
	*@brief:删除字符串最右侧的字符串
	*@param:strSrc:原始字符串
	*@param:strDest:要删除的字符串
	*@return:std::wstring:删除后的字符串
	*/
	static std::wstring              StringTrimRight(const std::wstring strSrc, const std::wstring strDest);

	/**
	*@brief:删除字符串最右侧的字符
	*@param:strSrc:原始字符串
	*@param:cDest:要删除的字符
	*@return:std::wstring:删除后的字符串
	*/
	static std::wstring              StringTrimRight(const std::wstring strSrc, const wchar_t cDest);

	/**
	*@brief:将字符串按特定字符分割成数组后，排除相同项，再组成字符串
	*@param:strSrc:输入原始字符串
	*@param:split:输入分隔符
	*@return:std::string:重组后的字符串
	*/
	static std::string				StringTrimVector(std::string &strSrc, char split);

	/**
	*@brief:将字符串按特定字符分割成数组后，排除相同项，再组成字符串
	*@param:strSrc:输入原始字符串
	*@param:split:输入分隔符
	*@return:std::string:重组后的字符串
	*/
	static std::string				StringTrimVector(std::string &strSrc, std::string split);

	/**
	*@brief:判断字符串是否全是数字
	*@param:strValue:要判断的字符串
	*@return:bool:true:全为数字，false:不全为数字
	*/
	static  bool                IsNumber(const std::string &strValue);

	/**
	*@brief:判断字符串是否全是数字
	*@param:strValue:要判断的字符串
	*@return:bool:true:全为数字，false:不全为数字
	*/
	static  bool                IsNumber(const std::wstring &wstrValue);

	/**
	*@brief:判断字符串是否全是英文
	*@return:bool:true:全为英文，false:不全为英文
	*/
	static bool IsEng(std::string strIn,int nLen);

	/**
	*@brief:判断字符串是否全是英文
	*@return:bool:true:全为英文，false:不全为英文
	*/
	static bool IsEng2(std::string strIn,int nLen);

	/**
	*@brief:非数字和英文字符检查，非数字和英文返回true
	*/
	static bool IsChes(std::string strIn);

	/**
	*@brief:十六进制字符串转化为ascii字符串
	*@param:hexStr:输入十六进制字符串
	*@param:asciiStr:输出ascii字符串
	*@return:bool:true:全为数字，false:不全为数字
	*/
	static bool HexStr2Ascii(std::string hexStr, std::string &asciiStr);

	/**
	*@brief:小写字符转大写字符
	*/
	static std::string strMakerUper(std::string str);

	/**
	*@brief:IP地址整数转成字符串
	*@param:unIP:IP地址整数
	*@return:wstring ip地址字符串
	*/
	static std::wstring			IPUL2STR(unsigned long ulIP);

	/**
	*@brief:将文件大小整数转为格式化的字符串
	*@param:nInt:文件大小
	*@return:string 文件大小字符串
	*/
    static std::string			GetSizeStr(unsigned long long nInt);
	////////////////////////////////////////////////////////////
	
	/**
	*@brief:截断数据,包括中文或英文,lchen
	*/
	static std::string CutData(std::string strIn,int nStandardLen);

	/**
	*@brief:过滤特殊字符,\t , \r, \n,\',\"需要过滤掉,lchen
	*/
	static std::string FilterSpecialChar(std::string &strData);

	/**
	*@brief:去除前后空白，lianghuikang
	*/
	static void Trim(std::string &str);


	static void utf8_cut(std::string &strContent, unsigned unLength);

	static std::string WinPath2UnixPath(const std::string strSrc);

    /**
     * 整数转成16进制字符串
     */
    static std::string dec2hex(int i);

	/**
	 *
	 */
	static bool isSubStr(std::string str1, std::string str2);

    static std::string StringFormat(const char* format, ...);

private:
	static char *utf8_find_prev_char(const char *str, const char *p);

};


};