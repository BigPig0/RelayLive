#pragma once
#include "ExportDefine.h"
#include <string>
using namespace std;

namespace Settings
{
    /**
     * 加载配置文件
     * @param strFileName[in] 配置文件路径
     * @return 成功true,失败false;
     */
    bool   COMMON_API loadFromProfile(const string &strFileName);

    /**
     * 读取配置值
     * @param section[in] 配置段落
     * @param key[in] 配置关键字
     * @return 配置值
     */
    string COMMON_API getValue(const string &section,const string &key);

    /**
     * 读取配置值
     * @param section[in] 配置段落
     * @param key[in] 配置关键字
     * @param default[in] 默认值
     * @return 配置值
     */
    string COMMON_API getValue(const string &section,const string &key,const string &default);

    /**
     * 读取整数配置值
     * @param section[in] 配置段落
     * @param key[in] 配置关键字
     * @param default[in] 默认值
     * @return 配置值
     */
    int COMMON_API getValue(const string &section,const string &key,const int &default);
};
