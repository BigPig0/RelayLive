#pragma once

#include "util_public.h"
#include <vector>

namespace util {

class UTIL_API String
{
public:

	/**
	 * @brief:���ַ����е�������ĸ��ɴ�д
	 * @param str:ԭʼ�ַ���
	 * @return �ı����ַ���
	 */
	static std::string upper(std::string str);

    /**
	 * @brief:���ַ����е�������ĸ���Сд
	 * @param str:ԭʼ�ַ���
	 * @return �ı����ַ���
	 */
	static std::string lower(std::string str);

	/**
	 * @brief: �ַ����滻
	 * @param: str ԭʼ�ַ���
     * @param: src ��Ҫ���滻������
     * @param: dst �滻������
	 * @return:�滻����ַ���
	 */
	static std::string replace(std::string&str, char src, char dst);

	/**
	 * @brief: �ַ����滻
     * @param: str ԭʼ�ַ���
	 * @param: src ��Ҫ���滻������
	 * @param: dst �滻������
	 * @return: �滻����ַ���
	 */
	static std::string replace(std::string &str, std::string src, std::string dst);

	/**
	 * @brief:�ַ����ָ�
	 * @param:s:ԭʼ�ַ���
	 * @param:tag:�ָ����
	 * @return:std::vector<std::string>:�ָ����ַ�������
	 */
	static std::vector<std::string>  split(const std::string &s, const char tag);

	/**
	 * @brief:�ַ����ָ�
	 * @param:strSrc:ԭʼ�ַ���
	 * @param:tag:�ָ���ţ��ַ�����
	 * @param:nLen:�ָ���ŵĳ���
	 * @return:std::vector<std::string>:�ָ����ַ�������
	 */
	static std::vector<std::string>  split(const std::string &s, char* tag, int nLen);

	/**
     * @brief:�ַ����ָ�
     * @param:strSrc:ԭʼ�ַ���
     * @param:tag:�ָ���ţ��ַ�����
     * @return:std::vector<std::string>:�ָ����ַ�������
	 */
	static std::vector<std::string> split(const std::string &s, const std::string &tag);

	/**
	 * @brief:ɾ���ַ����е��ַ���
	 * @param str: ԭʼ�ַ���
	 * @param strDest: Ҫɾ�����ַ���
	 * @return ɾ������ַ���
	 */
	static std::string remove(const std::string &strSrc, const std::string str);

	/**
	 * @brief:ɾ���ַ����е��ض��ַ�
	 * @param strSrc: ԭʼ�ַ���
     * @param ch: Ҫɾ�����ַ�
	 * @return ɾ������ַ���
	 */
	static std::string remove(const std::string &strSrc, char ch);

	/**
	 * @brief ȥ���ַ����з�����
	 * @param str ԭʼ�ַ���
	 * @return �ı����ַ���
	 */
	static std::string removeNonnumeric(std::string str); 

	/**
	*@brief:ɾ���ַ������Ҳ���ַ���
	*@param:strSrc:ԭʼ�ַ���
	*@param:strDest:Ҫɾ�����ַ���
	*@return:std::string:ɾ������ַ���
	*/
	static std::string               StringTrimRight(const std::string strSrc, const std::string strDest);

	/**
	*@brief:ɾ���ַ������Ҳ���ַ�
	*@param:strSrc:ԭʼ�ַ���
	*@param:cDest:Ҫɾ�����ַ�
	*@return:std::string:ɾ������ַ���
	*/
	static std::string               StringTrimRight(const std::string strSrc, const char cDest);

	/**
	*@brief:ɾ���ַ������Ҳ���ַ���
	*@param:strSrc:ԭʼ�ַ���
	*@param:strDest:Ҫɾ�����ַ���
	*@return:std::wstring:ɾ������ַ���
	*/
	static std::wstring              StringTrimRight(const std::wstring strSrc, const std::wstring strDest);

	/**
	*@brief:ɾ���ַ������Ҳ���ַ�
	*@param:strSrc:ԭʼ�ַ���
	*@param:cDest:Ҫɾ�����ַ�
	*@return:std::wstring:ɾ������ַ���
	*/
	static std::wstring              StringTrimRight(const std::wstring strSrc, const wchar_t cDest);

	/**
	*@brief:���ַ������ض��ַ��ָ��������ų���ͬ�������ַ���
	*@param:strSrc:����ԭʼ�ַ���
	*@param:split:����ָ���
	*@return:std::string:�������ַ���
	*/
	static std::string				StringTrimVector(std::string &strSrc, char split);

	/**
	*@brief:���ַ������ض��ַ��ָ��������ų���ͬ�������ַ���
	*@param:strSrc:����ԭʼ�ַ���
	*@param:split:����ָ���
	*@return:std::string:�������ַ���
	*/
	static std::string				StringTrimVector(std::string &strSrc, std::string split);

	/**
	*@brief:�ж��ַ����Ƿ�ȫ������
	*@param:strValue:Ҫ�жϵ��ַ���
	*@return:bool:true:ȫΪ���֣�false:��ȫΪ����
	*/
	static  bool                IsNumber(const std::string &strValue);

	/**
	*@brief:�ж��ַ����Ƿ�ȫ������
	*@param:strValue:Ҫ�жϵ��ַ���
	*@return:bool:true:ȫΪ���֣�false:��ȫΪ����
	*/
	static  bool                IsNumber(const std::wstring &wstrValue);

	/**
	*@brief:�ж��ַ����Ƿ�ȫ��Ӣ��
	*@return:bool:true:ȫΪӢ�ģ�false:��ȫΪӢ��
	*/
	static bool IsEng(std::string strIn,int nLen);

	/**
	*@brief:�ж��ַ����Ƿ�ȫ��Ӣ��
	*@return:bool:true:ȫΪӢ�ģ�false:��ȫΪӢ��
	*/
	static bool IsEng2(std::string strIn,int nLen);

	/**
	*@brief:�����ֺ�Ӣ���ַ���飬�����ֺ�Ӣ�ķ���true
	*/
	static bool IsChes(std::string strIn);

	/**
	*@brief:ʮ�������ַ���ת��Ϊascii�ַ���
	*@param:hexStr:����ʮ�������ַ���
	*@param:asciiStr:���ascii�ַ���
	*@return:bool:true:ȫΪ���֣�false:��ȫΪ����
	*/
	static bool HexStr2Ascii(std::string hexStr, std::string &asciiStr);

	/**
	*@brief:Сд�ַ�ת��д�ַ�
	*/
	static std::string strMakerUper(std::string str);

	/**
	*@brief:IP��ַ����ת���ַ���
	*@param:unIP:IP��ַ����
	*@return:wstring ip��ַ�ַ���
	*/
	static std::wstring			IPUL2STR(unsigned long ulIP);

	/**
	*@brief:���ļ���С����תΪ��ʽ�����ַ���
	*@param:nInt:�ļ���С
	*@return:string �ļ���С�ַ���
	*/
    static std::string			GetSizeStr(unsigned long long nInt);
	////////////////////////////////////////////////////////////
	
	/**
	*@brief:�ض�����,�������Ļ�Ӣ��,lchen
	*/
	static std::string CutData(std::string strIn,int nStandardLen);

	/**
	*@brief:���������ַ�,\t , \r, \n,\',\"��Ҫ���˵�,lchen
	*/
	static std::string FilterSpecialChar(std::string &strData);

	/**
	*@brief:ȥ��ǰ��հף�lianghuikang
	*/
	static void Trim(std::string &str);


	static void utf8_cut(std::string &strContent, unsigned unLength);

	static std::string WinPath2UnixPath(const std::string strSrc);

    /**
     * ����ת��16�����ַ���
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