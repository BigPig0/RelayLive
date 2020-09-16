#pragma once

#include "utilc.h"
#include "ludb_exp.h"
#include "ludb_public.h"


/**
 * ��ʼ��oracle���ݿ�
 * @param path oci�����ڵ�·����Ϊ�ձ�ʾ�ӵ�ǰĿ¼����
 */
LUDB_API bool ludb_init_oracle(const char *path /*= NULL*/);

/**
 * ��ʼ��mongodb���ݿ�
 */
LUDB_API bool ludb_init_mongo();

/**
 * ��ʼ��redis���ݿ�
 */
LUDB_API bool ludb_init_redis();

/**
 * ��lua::Stateʵ������ludbģ���ṩ�Ľӿ�
 * @param lua �ⲿ������lua::State<>*ʵ��
 */
LUDB_API void ludb_use_lua(void* lua);

/**
 * �������ݿ⻷����ÿ�����ݿ�ֱ��ͷ�
 */
LUDB_API void ludb_clean(ludb_db_type_t t);


/**
 * �������ݿ�
 * @param t ���ݿ�����
 * @param database ���ݵ�ַ
 * @param usr �û���
 * @param pwd ����
 */
LUDB_API ludb_conn_t* ludb_connect(ludb_db_type_t t, const char *database, const char *usr, const char *pwd);

/**
 * �������ӳ��������ݿ�
 * @param t ���ݿ�����
 * @param tag ���ӳ�����
 * @param database ���ݵ�ַ
 * @param usr �û���
 * @param pwd ����
 * @param max ���������
 * @param min ��С������
 * @param inc �����Ӳ�����ʱ���������ӵĸ���
 */
LUDB_API bool ludb_create_pool(ludb_db_type_t t, const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc);

/**
 * ��ָ�����ӳ��л�ȡһ������
 * @param t ���ݿ�����
 * @param tag ���ӳ�����
 */
LUDB_API ludb_conn_t* ludb_pool_connect(ludb_db_type_t t, const char *tag);

/**
 * �Ͽ�һ�����ӣ������ͷŵ����ӳ�
 */
LUDB_API bool ludb_free_conn(ludb_conn_t *conn);

/**
 * ����Statement
 */
LUDB_API ludb_stmt_t* ludb_create_stmt(ludb_conn_t *conn);

/**
 * �ͷ�Statement
 */
LUDB_API bool ludb_free_stmt(ludb_stmt_t *stmt);

/**
 * ִ�����
 */
LUDB_API bool ludb_execute_stmt(ludb_stmt_t *stmt, const char *sql);

/**
 * Ԥִ����䣬�ȴ���
 */
LUDB_API bool ludb_prepare(ludb_stmt_t *stmt, const char *sql);
/** �� */
LUDB_API bool ludb_bind_int(ludb_stmt_t *stmt, const char *name, int *data);
LUDB_API bool ludb_bind_str(ludb_stmt_t *stmt, const char *name, const char *data, int len);
/** ִ�� */
LUDB_API bool ludb_execute(ludb_stmt_t *stmt);

/**
 * ����ǰһ�β�����Ӱ��ļ�¼������
 */
LUDB_API uint32_t ludb_affected_rows(ludb_stmt_t *stmt);

/**
 * �ύ����
 */
LUDB_API bool ludb_commit(ludb_conn_t *conn);

/**
 * ��ȡִ�н������
 */
LUDB_API ludb_rest_t* ludb_result(ludb_stmt_t *stmt);

/**
 * �ͷ�ִ�н������
 */
LUDB_API void ludb_free_result(ludb_rest_t *res);

/**
 * ִ�н��ת����һ��
 */
LUDB_API bool ludb_result_next(ludb_rest_t *res);

/**
 * ��ȡ�ַ����ֶ����ݡ��ֶ�����Ϊ��ʱ�����ؿ��ַ�����
 */
LUDB_API std::string ludb_rest_get_char(ludb_rest_t *res, uint32_t i);

/** 
 * ��ȡ�����ֶ����ݡ��ֶ�����Ϊ��ʱ������0
 */
LUDB_API int ludb_rest_get_int(ludb_rest_t *res, uint32_t i);

/** 
 * ��ȡ����ʱ�䣬תΪyyyymmddhh24miss��ʽ���ַ�����
 */
LUDB_API std::string ludb_rest_get_date(ludb_rest_t *res, uint32_t i);

/** 
 * ��ȡ�����ơ��ֶ�����Ϊ��ʱ�����ؿ��ַ�����
 */
LUDB_API std::string ludb_rest_get_blob(ludb_rest_t *res, uint32_t i);
