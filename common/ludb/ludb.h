#pragma once

#include "utilc.h"
#include "ludb_exp.h"
#include "ludb_public.h"


/**
 * 初始化oracle数据库
 * @param path oci库所在的路径，为空表示从当前目录加载
 */
LUDB_API bool ludb_init_oracle(const char *path /*= NULL*/);

/**
 * 初始化mysql数据库
 */
LUDB_API bool ludb_init_mysql();

/**
 * 初始化mongodb数据库
 */
LUDB_API bool ludb_init_mongo();

/**
 * 初始化redis数据库
 */
LUDB_API bool ludb_init_redis();

/**
 * 向lua::State实例设置ludb模块提供的接口
 * @param lua 外部建立的lua::State<>*实例
 */
LUDB_API void ludb_use_lua(void* lua);

/**
 * 清理数据库环境。每种数据库分别释放
 */
LUDB_API void ludb_clean(ludb_db_type_t t);


/**
 * 连接数据库
 * @param t 数据库类型
 * @param database 数据地址
 * @param usr 用户名
 * @param pwd 密码
 */
LUDB_API ludb_conn_t* ludb_connect(ludb_db_type_t t, const char *database, const char *usr, const char *pwd);

/**
 * 建立连接池连接数据库
 * @param t 数据库类型
 * @param tag 连接池名称
 * @param database 数据地址
 * @param usr 用户名
 * @param pwd 密码
 * @param max 最多连接数
 * @param min 最小连接数
 * @param inc 当连接不够用时，新增连接的个数
 */
LUDB_API bool ludb_create_pool(ludb_db_type_t t, const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc);

/**
 * 从指定连接池中获取一个连接
 * @param t 数据库类型
 * @param tag 连接池名称
 */
LUDB_API ludb_conn_t* ludb_pool_connect(ludb_db_type_t t, const char *tag);

/**
 * 断开一个连接，或者释放到连接池
 */
LUDB_API bool ludb_free_conn(ludb_conn_t *conn);

/**
 * 创建Statement
 */
LUDB_API ludb_stmt_t* ludb_create_stmt(ludb_conn_t *conn);

/**
 * 释放Statement
 */
LUDB_API bool ludb_free_stmt(ludb_stmt_t *stmt);

/**
 * 执行语句
 */
LUDB_API bool ludb_execute_stmt(ludb_stmt_t *stmt, const char *sql);

/**
 * 预执行语句，等待绑定
 */
LUDB_API bool ludb_prepare(ludb_stmt_t *stmt, const char *sql);
/** 绑定 ORACLE */
LUDB_API bool ludb_bind_int(ludb_stmt_t *stmt, const char *name, int *data);
LUDB_API bool ludb_bind_str(ludb_stmt_t *stmt, const char *name, const char *data, int len);
/** 绑定 MYSQL */
LUDB_API bool ludb_bind(ludb_stmt_t *stmt, uint32_t col_num, column_type_t *col_type, std::string *col_value);
/** 执行 */
LUDB_API bool ludb_execute(ludb_stmt_t *stmt);

/**
 * 返回前一次操作所影响的记录行数。
 */
LUDB_API uint32_t ludb_affected_rows(ludb_stmt_t *stmt);

/**
 * 提交任务
 */
LUDB_API bool ludb_commit(ludb_conn_t *conn);

/**
 * 获取执行结果集合
 */
LUDB_API ludb_rest_t* ludb_result(ludb_stmt_t *stmt);

/**
 * 释放执行结果集合
 */
LUDB_API void ludb_free_result(ludb_rest_t *res);

/**
 * 执行结果转到下一行
 */
LUDB_API bool ludb_result_next(ludb_rest_t *res);

/**
 * 获取字符串字段内容。字段内容为空时，返回空字符串。
 */
LUDB_API std::string ludb_rest_get_char(ludb_rest_t *res, uint32_t i);

/** 
 * 获取整数字段内容。字段内容为空时，返回0
 */
LUDB_API int ludb_rest_get_int(ludb_rest_t *res, uint32_t i);

/** 
 * 获取日期时间，转为yyyymmddhh24miss格式的字符串。
 */
LUDB_API std::string ludb_rest_get_date(ludb_rest_t *res, uint32_t i);

/** 
 * 获取二进制。字段内容为空时，返回空字符串。
 */
LUDB_API std::string ludb_rest_get_blob(ludb_rest_t *res, uint32_t i);
