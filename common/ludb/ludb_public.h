#pragma once

#include "utilc.h"


/** 数据库类型 */
typedef enum _ludb_db_type {
    ludb_db_unknow = 0,
    ludb_db_oracle,
    ludb_db_mysql,
    ludb_db_mongo,
    ludb_db_redis
} ludb_db_type_t;

/** connect句柄 */
struct ludb_conn_t;

/** statement句柄 */
struct ludb_stmt_t;

/** resultset句柄 */
struct ludb_rest_t;

/** 批量处理句柄 */
struct ludb_batch_t;

/** 字段类型定义 */
typedef enum _column_type_ {
    column_type_char = 0,
    column_type_int,
    column_type_float,
    column_type_long,
    column_type_uint,
    column_type_blob,
    column_type_date
}column_type_t;

/** 字段定义 */
struct bind_column_t {
    const char      *name;        //< 绑定标记名称或列名称
    column_type_t    type;        //< 列类型
    int              max_len;     //< 最大长度，类型是字符串时必须设置
    bool             nullable;    //< 是否可为空
    const char      *default_value;     //< 默认值，偶尔需要
};
