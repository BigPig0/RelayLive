/**
 * 封装一套数据库插入、更新等操作，频繁操作大量数据，且需要批量绑定的接口
 */
#pragma once

namespace dbTool
{

#define SQLT_CHR 1 
#define SQLT_INT 3 
#define SQLT_FLT 4 
#define SQLT_LNG 8 
#define SQLT_UIN 68 
#define SQLT_BLOB 113 
#define SQLT_ODT 156 

struct bindParam
{
    char*       bindName;    //< 绑定标记名称
    uint32_t    columnType;  //< 列类型
    uint32_t    maxLen;      //< 最大长度，类型是字符串时必须设置
    bool        nullable;    //< 是否可为空
    char*       default;     //< 默认值，偶尔需要
};

struct helpHandle;

/**
 * 创建一个帮助句柄
 * @param tag 连接池标签
 * @param sql 执行的sql
 * @param rowNum 每次最多绑定的数据行数,数据达到这个值会触发执行sql
 * @param interval 最多间隔几秒，存在数据但不足rowNum时会触发sql
 * @param binds 绑定信息的数组，以bindName=NULL结束。bindName需要与sql中的绑定标记对应
 */
helpHandle* CreateHelp(string tag, string sql, int rowNum, int interval, bindParam* binds);

/**
 * 释放一个帮助句柄
 * @param clean true：需要将已经缓存的记录信息全部插入数据库；false：放弃缓存数据直接释放
 */
void FreeHelp(helpHandle* h, bool clean = false);

/**
 * 通过帮助句柄向数据库中插入一行记录。
 */
void AddRow(helpHandle* h, vector<string> row);

}