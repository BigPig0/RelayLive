#pragma once

#include "ludb_exp.h"
#include "ludb_public.h"
#include <vector>

/**
 * ����һ������������
 * @param t ���ݿ�����
 * @param tag ���ӳر�ǩ
 * @param sql ִ�е�sql
 * @param rowNum ÿ�����󶨵���������,���ݴﵽ���ֵ�ᴥ��ִ��sql
 * @param interval ��������룬�������ݵ�����rowNumʱ�ᴥ��sql��intervalΪ0ʱ��������ʱ������Ҫ�û��ֶ�ִ�С�
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
 * intervalΪ0ʱ�����Զ�ִ����⣬��Ҫ�û����ø÷����ֶ���⡣
 * @return ִ�гɹ�
 */
LUDB_API bool ludb_batch_run_sync(ludb_batch_t* h);

/**
 * ���ûص���֪ͨ����ļ�¼��
 */
typedef void(*catch_num_cb)(ludb_batch_t*, uint64_t);
LUDB_API void ludb_batch_catch_num(catch_num_cb cb);

/**
 * ���ûص���֪ͨʧ�ܵļ�¼����
 */
typedef void(*failed_data_cb)(ludb_batch_t*, std::vector<std::vector<std::string>>);
LUDB_API void ludb_batch_failed_data(failed_data_cb cb);
