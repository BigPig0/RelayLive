#ifndef _LUDB_HELP_H_
#define _LUDB_HELP_H_

#include "ludb_exp.h"
#include "ludb_public.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 创建一个批量处理句柄
 * @param t 数据库类型
 * @param tag 连接池标签
 * @param sql 执行的sql
 * @param rowNum 每次最多绑定的数据行数,数据达到这个值会触发执行sql
 * @param interval 最多间隔几秒，存在数据但不足rowNum时会触发sql
 * @param binds 绑定信息的数组，以bindName=NULL结束。bindName需要与sql中的绑定标记对应
 */
LUDB_API ludb_batch_t* create_ludb_batch(ludb_db_type_t t, const char *tag, const char *sql, int rowNum, int interval, bind_column_t* binds);

/**
 * 释放一个批量处理句柄
 * @param clean 1：需要将已经缓存的记录信息全部插入数据库；0：放弃缓存数据直接释放
 */
LUDB_API void destory_ludb_batch(ludb_batch_t* h, int clean /*= 0*/);

/**
 * 通过批量句柄向数据库中插入一行记录。
 * @param row 以NULL结尾的字符串指针数组，每个字符串代表一个字段数据。
 * @return 缓存的行数
 */
LUDB_API uint64_t ludb_batch_add_row(ludb_batch_t* h, const char **row);

/**
 * 设置回调，通知缓存的记录数
 */
typedef void(*catch_num_cb)(ludb_batch_t*, uint64_t);
LUDB_API void ludn_batch_catch_num(catch_num_cb cb);

#ifdef __cplusplus
}
#endif
#endif