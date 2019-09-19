#ifndef _LUDB_PRIVATE_H_
#define _LUDB_PRIVATE_H_

#include "ludb_public.h"
#include "cstl.h"
#include "cstl_easy.h"
#include "uv.h"

#ifdef __cplusplus
extern "C" {
#endif

/** 连接句柄 */
typedef struct _ludb_conn_ {
    ludb_db_type_t type;
    bool           from_pool;
    void          *conn;
}ludb_conn_t;

/** statement句柄 */
typedef struct _ludb_stmt_ {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    void          *stmt;
}ludb_stmt_t;

/** resultset句柄 */
typedef struct _ludb_rest_ {
    ludb_db_type_t type;
    ludb_conn_t   *conn;
    ludb_stmt_t   *stmt;
    void          *rest;
}ludb_rest_t;

/** 连接池句柄 */
typedef struct _ludb_pool_ {
    ludb_db_type_t type;
    void          *pool;
}ludb_pool_t;

/** 字段定义 */
typedef struct _bind_column_pri_ {
    string_t        *name;        //< 绑定标记名称或列名称
    column_type_t    type;        //< 列类型
    int              max_len;     //< 最大长度，类型是字符串时必须设置
    bool             nullable;    //< 是否可为空
    string_t        *default_value;     //< 默认值，偶尔需要
}bind_column_pri_t;

/** 批量处理句柄 */
typedef struct _ludb_batch_ {
    ludb_db_type_t type;      //< 数据库类型
    string_t      *tag;       //< 连接池标签
    string_t      *sql;       //< 执行的sql语句
    int            row_num;   //< 数据缓存达到这么多行就执行
    int            interval;  //< 从上次执行经过这么多秒，缓存数据不为空就执行。0 不启动定时器
    vector_t      *binds;     //< vector<bind_column_t>
    queue_t       *recvs;     //< queue<vector<string>> 数据接收队列
    vector_t      *insts;     //< vector<vector<string>> 待入库的数据
    uv_mutex_t     mutex;     //< 上面这两个队列切换的锁
    bool           running;   //< 是否正在执行
    time_t         ins_time;  //< 上一次提交的时间
    uv_thread_t    tid;       //< 执行线程ID
    void          *handle;    //< 根据不通类型分别定义各自的结构
}ludb_batch_t;

extern LOG_HANDLE g_log_hook;
extern int pointLen;

#ifdef __cplusplus
}
#endif
#endif