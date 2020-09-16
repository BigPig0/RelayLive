#include "ludb_private.h"
#include "ludb_batch.h"
#include "ludb_oracle.h"
#include "ludb_mysql.h"
#include "ludb_mongo.h"
#include "ludb_redis.h"
#include "utilc.h"

int pointLen = sizeof(void*); //< ָ��ĳ��ȣ�32λƽ̨4�ֽڣ�64λƽ̨8�ֽ�

static catch_num_cb _cbCatchNum = NULL;
static failed_data_cb _cbFailedData = NULL;

/** �ӽ��ն����н������Ƶ�������У���������в���Ϊֹ */
static int move_rows(ludb_batch_t* h) {
    int count = 0;
    while (!h->recvs.empty() && h->insts.size() < (size_t)h->row_num)
    {
        h->insts.push_back(h->recvs.front());
        h->recvs.pop();
        ++count;
    }
	if(_cbCatchNum)
		_cbCatchNum(h, h->recvs.size());
    return count;
}

/** ����������е����ݲ������ݿ� */
static bool insert(ludb_batch_t* h)
{
    if(h->insts.empty())
        return true;

    bool ret = h->insert();
    if(!ret) {
        // ִ��ʧ�ܣ���ʧ�ܵ���������
        if(_cbFailedData) {
            _cbFailedData(h, h->insts);
        }
    }
    h->insts.clear();

    return ret;
}

static void collect(void* arg){
    ludb_batch_t* h = (ludb_batch_t*)arg;
    Log::debug("batch thread start running");
    while (h->running) {
        int move_count;
        time_t now;
        uv_mutex_lock(&h->mutex);
        move_count = move_rows(h);
        uv_mutex_unlock(&h->mutex);
        //Log::debug("move %d rows",move_count);
        now = time(NULL);
        if (difftime(now, h->ins_time) > h->interval) {
            //Log::debug("interval to insert");
            insert(h);
            h->ins_time = now; //���ü�ʱ��
        }
        //Log::debug("catch rows:%d/%d",m_vecInsertRows.size(),m_nMaxRows);
        if (h->insts.size() >= h->row_num) {
            //Log::debug("size to insert");
            insert(h);
            h->ins_time = now; //���ü�ʱ��
        }
        sleep(100);
    }
    Log::debug("batch thread stop running");

    //����
    delete h;
}

ludb_batch_t::ludb_batch_t(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : tag(Tag)
    , sql(Sql)
    , row_num(RowNum)
    , interval(Interval)
    , running(true)
    , ins_time(time(NULL))
{
    for(int i=0; Binds[i].name; i++) {
        int max_len = Binds[i].max_len;
        switch (Binds[i].type)
        {
        case column_type_char:
            if (max_len == 0)
                max_len = 16;
            break;
        case column_type_int:
        case column_type_uint:
            max_len = sizeof(int32_t);
            break;
        case column_type_float:
            max_len = sizeof(double);
            break;
        case column_type_long:
            max_len = sizeof(uint64_t);
            break;
        case column_type_blob:
        case column_type_date:
            max_len = pointLen;
            break;
        }

        bind_column_pri_t col_info;
        col_info.name          = Binds[i].name;
        col_info.type          = Binds[i].type;
        col_info.max_len       = max_len;
        col_info.nullable      = Binds[i].nullable;
        col_info.default_value = Binds[i].default_value?binds[i].default_value:"";

        binds.push_back(col_info);
    }

    uv_mutex_init(&mutex);

    if(interval > 0)
        uv_thread_create(&tid, collect, this);
}

ludb_batch_t::~ludb_batch_t(){

}

ludb_batch_t* create_ludb_batch(ludb_db_type_t t, const char *tag, const char *sql, int rowNum, int interval, bind_column_t* binds) {
    ludb_batch_t *ret = NULL;
    if(t == ludb_db_oracle) {
#ifdef DB_ORACLE
        ret = new ludb_oracle_batch(tag,sql,rowNum,interval,binds);
#endif
    } else if(t == ludb_db_mysql) {
#ifdef DB_MYSQL
        ret = new ludb_mysql_batch(tag,sql,rowNum,interval,binds);
#endif
    } else if(t == ludb_db_mongo) {
#ifdef DB_MONGO
        ret = new ludb_mongo_batch(tag,sql,rowNum,interval,binds);
#endif
    } else if(t == ludb_db_redis) {
#ifdef DB_REDIS
        ret = new ludb_redis_batch(tag,sql,rowNum,interval,binds);
#endif
    }
    ret->type = t;
    return ret;
}

void destory_ludb_batch(ludb_batch_t* h, int clean /*= 0*/) {
    h->running = false;
    if(h->interval > 0)
        delete h;
}

uint64_t ludb_batch_add_row(ludb_batch_t* h, const char **row) {
    const char *col = *row;
	uint64_t ret;
    vector<string> vecrow;
    for(;col; col=*(++row)){
        vecrow.push_back(col);
    }
    uv_mutex_lock(&h->mutex);
    h->recvs.push(vecrow);
	ret = h->recvs.size();
    uv_mutex_unlock(&h->mutex);
	return ret;
}

bool ludb_batch_run_sync(ludb_batch_t* h) {
    uv_mutex_lock(&h->mutex);
    move_rows(h);
    uv_mutex_unlock(&h->mutex);
    return insert(h);
}

void ludb_batch_catch_num(catch_num_cb cb){
	_cbCatchNum = cb;
}

void ludb_batch_failed_data(failed_data_cb cb){
    _cbFailedData = cb;
}