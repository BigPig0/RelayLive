#include "ludb_private.h"
#include "ludb_batch.h"
#include "ludb_oracle.h"
#include "ludb_mongo.h"
#include "ludb_redis.h"
#include "utilc.h"

int pointLen = sizeof(void*); //< 指针的长度，32位平台4字节，64位平台8字节

static catch_num_cb _cbCatchNum = NULL;

/** 从接收队列中将数据移到插入队列，到插入队列插满为止 */
static int move_rows(ludb_batch_t* h) {
    int count = 0;
    while (!h->recvs.empty() && h->insts.size() < (size_t)h->row_num)
    {
        h->insts.push_back(h->recvs.front());
        h->recvs.front();
        ++count;
    }
	if(_cbCatchNum)
		_cbCatchNum(h, h->recvs.size());
    return count;
}

/** 将插入队列中的数据插入数据库 */
static bool insert(ludb_batch_t* h)
{
    if(h->insts.empty())
        return true;

    h->insert();
    h->insts.clear();

    return true;
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
            h->ins_time = now; //重置计时器
        }
        //Log::debug("catch rows:%d/%d",m_vecInsertRows.size(),m_nMaxRows);
        if (h->insts.size() >= h->row_num) {
            //Log::debug("size to insert");
            insert(h);
            h->ins_time = now; //重置计时器
        }
        sleep(100);
    }
    Log::debug("batch thread stop running");

    //结束
    delete h;
}

ludb_batch_t::ludb_batch_t(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : tag(Tag)
    , sql(Sql)
    , row_num(RowNum)
    , interval(Interval)
    , ins_time(time(NULL))
    , running(true)
{
    for(int i=0; Binds[i].name; i++) {
        int max_len = binds[i].max_len;
        switch (binds[i].type)
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
    uv_thread_create(&tid, collect, this);
}

ludb_batch_t::~ludb_batch_t(){

}

ludb_batch_t* create_ludb_batch(ludb_db_type_t t, const char *tag, const char *sql, int rowNum, int interval, bind_column_t* binds) {
    ludb_batch_t *ret = NULL;
    if(t == ludb_db_oracle)
        ret = new ludb_oracle_batch(tag,sql,rowNum,interval,binds);
    else if(t == ludb_db_mongo)
        ret = new ludb_mongo_batch(tag,sql,rowNum,interval,binds);
    else if(t == ludb_db_redis)
        ret = new ludb_redis_batch(tag,sql,rowNum,interval,binds);
    
    ret->type = t;
    return ret;
}

void destory_ludb_batch(ludb_batch_t* h, int clean /*= 0*/) {
    h->running = false;
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

void ludn_batch_catch_num(catch_num_cb cb){
	_cbCatchNum = cb;
}