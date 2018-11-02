#pragma once

#include "ExportDefine.h"
#include <vector>

class COMMON_API StringHandle
{
public:
	StringHandle();
	~StringHandle();

	/**
	*@brief:
	*@param:
	*@return:
	*/
	static std::string          replace(std::string&s);

	/**
	*@brief:�ַ����ָ�
	*@param:s:ԭʼ�ַ���
	*@param:tag:�ָ����
	*@return:std::vector<std::string>:�ָ����ַ�������
	*/
	static std::vector<std::string>  StringSplit(const std::string s, const char tag);

	/**
	*@brief:�ַ����ָ�
	*@param:strSrc:ԭʼ�ַ���
	*@param:pChar:�ָ���ţ��ַ�����
	*@param:nLen:�ָ���ŵĳ���
	*@return:std::vector<std::string>:�ָ����ַ�������
	*/
	static std::vector<std::string>  StringCompart(std::string strSrc, char* pChar, int nLen);

	/**
	*@brief:ɾ���ַ����е��ַ���
	*@param:strSrc:ԭʼ�ַ���
	*@param:strDest:Ҫɾ�����ַ���
	*@return:std::string:ɾ������ַ���
	*/
	static std::string               StringWipe(const std::string strSrc, const std::string strDest);

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
	*@brief:���ַ����е�������ĸ��ɴ�д
	*@param:str:ԭʼ�ַ���
	*@return:std::string:�ı����ַ���
	*/
	static std::string               StringUper(std::string str);

	/**
	*@brief:ȥ���ַ����з�����
	*@param:str:ԭʼ�ַ���
	*@return:std::string:�ı����ַ���
	*/
	static std::string               RemoveUnDig(std::string str); 

    /**
     * ����ת���ַ���
     */
    template<typename T>
    static std::string               toStr(T num)
    {
        std::stringstream ss;
        ss<<num;
        return ss.str();
    }

    /**
     * ����ת���ַ���
     */
    template<typename T>
    static std::wstring              toWStr(T num)
    {
        std::wstringstream ss;
        ss<<num;
        return ss.str();
    }

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
	*@return:bool:true:ȫΪӢ�ģ�false:��ȫΪ����
	*/
	static bool IsEng(std::string strIn,int nLen);

	/**
	*@brief:�ж��ַ����Ƿ�ȫ��Ӣ��
	*@return:bool:true:ȫΪӢ�ģ�false:��ȫΪ����
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
	static bool                 HexStr2Ascii(std::string hexStr, std::string &asciiStr);

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
	*@brief:�ַ����滻
	*����string���͵��ַ�������Ҫ�滻���ַ���Ĭ��Ϊ��
	*@return:����True���ַ�����������Ҫ���滻���ַ����滻��ɣ�����û���滻�ɹ�
	*/
	static bool RemoveChar(std::string &pstr,char ch);

	/**
	*@brief:�ַ����滻
	*����string���͵��ַ�������Ҫ�滻���ַ������ַ�����Ĭ��Ϊ��
	*@return:����True���ַ�����������Ҫ���滻���ַ����滻��ɣ�����û���滻�ɹ�
	*/
	static bool RemoveEnterSymb(std::string &pstr,std::string strIn);

	/**
	*@brief:�ַ����滻
	*@param:string:strOld Դ�ַ���
	*@param:string:strNew Ŀ���ַ���
	*@return:����True���ַ�����������Ҫ���滻���ַ����滻��ɣ�����û���滻�ɹ�
	*/
	static bool replaceEnterSymb(std::string &str, std::string strOld, std::string strNew);

	
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

	/**
	*@brief:ʵ���и�
	*/
	static std::vector<std::string> SplitData(std::string &strRecData,std::string strSep);


	static void utf8_cut(std::string &strContent, unsigned unLength);

	static std::string WinPath2UnixPath(const std::string strSrc);

    /**
     * ����ת��16�����ַ���
     */
    static std::string dec2hex(int i);

private:
	static char *utf8_find_prev_char(const char *str, const char *p);

};

