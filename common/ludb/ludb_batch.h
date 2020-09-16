#pragma once

#include "ludb_exp.h"
#include "ludb_public.h"


/**
 * ����һ������������
 * @param t ���ݿ�����
 * @param tag ���ӳر�ǩ
 * @param sql ִ�е�sql
 * @param rowNum ÿ�����󶨵���������,���ݴﵽ���ֵ�ᴥ��ִ��sql
 * @param interval ��������룬�������ݵ�����rowNumʱ�ᴥ��sql
 * @param binds ����Ϣ�����飬��bindName=NULL������bindName��Ҫ��sql�еİ󶨱�Ƕ�Ӧ
 */
LUDB_API ludb_batch_t* create_ludb_batch(ludb_db_type_t t, const char *tag, const char *sql, int rowNum, int interval, bind_column_t* binds);

/**
 * �ͷ�һ������������
 * @param clean 1����Ҫ���Ѿ�����ļ�¼��Ϣȫ���������ݿ⣻0��������������ֱ���ͷ�
 */
LUDB_API void destory_ludb_batch(ludb_batch_t* h, int clean /*= 0*/);

/**
 * ͨ��������������ݿ��в���һ�м�¼��
 * @param row ��NULL��β���ַ���ָ�����飬ÿ���ַ�������һ���ֶ����ݡ�
 * @return ���������
 */
LUDB_API uint64_t ludb_batch_add_row(ludb_batch_t* h, const char **row);

/**
 * ���ûص���֪ͨ����ļ�¼��
 */
typedef void(*catch_num_cb)(ludb_batch_t*, uint64_t);
LUDB_API void ludn_batch_catch_num(catch_num_cb cb);
