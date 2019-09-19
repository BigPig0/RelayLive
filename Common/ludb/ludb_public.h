#ifndef _LUDB_PUBLIC_H_
#define _LUDB_PUBLIC_H_

#include "utilc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 数据库类型 */
typedef enum _ludb_db_type {
    ludb_db_oracle = 0,
    ludb_db_mongo
} ludb_db_type_t;

/** connect句柄 */
typedef struct _ludb_conn_ ludb_conn_t;

/** statement句柄 */
typedef struct _ludb_stmt_ ludb_stmt_t;

/** resultset句柄 */
typedef struct _ludb_rest_ ludb_rest_t;

/** 日志回调定义 */
typedef void (*LOG_HANDLE)(char *log);

/** 批量处理句柄 */
typedef struct _ludb_batch_ ludb_batch_t;

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
typedef struct _bind_column_ {
    char*            name;        //< 绑定标记名称或列名称
    column_type_t    type;        //< 列类型
    int              max_len;     //< 最大长度，类型是字符串时必须设置
    bool             nullable;    //< 是否可为空
    char*            default_value;     //< 默认值，偶尔需要
}bind_column_t;

#ifdef __cplusplus
}
#endif
#endif