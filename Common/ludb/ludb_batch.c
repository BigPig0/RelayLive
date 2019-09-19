#include "ludb_private.h"
#include "ludb_batch.h"
#include "ludb_oracle.h"
#include "ludb_mongo.h"
#include "utilc.h"

int pointLen = sizeof(void*); //< 指针的长度，32位平台4字节，64位平台8字节

static catch_num_cb _cbCatchNum = NULL;

static void destory_row(vector_t *row){
    VECTOR_DESTORY(row, string_t*, string_destroy);
}

static void destory_bind_column_pri(bind_column_pri_t *binds) {
    string_destroy(binds->name);
    string_destroy(binds->default_value);
    free(binds);
}

static void destory_ludb_batch_handle(ludb_batch_t* h) {
    string_destroy(h->tag);
    string_destroy(h->sql);
    VECTOR_DESTORY(h->insts, bind_column_pri_t*, destory_bind_column_pri);
    QUEUE_DESTORY(h->recvs, vector_t*, destory_row);
    VECTOR_DESTORY(h->insts, vector_t*, destory_row);
    free(h);
}

/** 从接收队列中将数据移到插入队列，到插入队列插满为止 */
static int move_rows(ludb_batch_t* h) {
    int count = 0;
    while (!queue_empty(h->recvs) && vector_size(h->insts) < (size_t)h->row_num)
    {
        vector_push_back(h->insts, *(vector_t**)queue_front(h->recvs));
        queue_pop(h->recvs);
        ++count;
    }
	if(_cbCatchNum)
		_cbCatchNum(h, queue_size(h->recvs));
    return count;
}

/** 将插入队列中的数据插入数据库 */
static bool insert(ludb_batch_t* h)
{
    if(vector_empty(h->insts))
        return true;

    if(h->type == ludb_db_oracle){
		ludb_batch_insert_oracle(h);
    } else if(h->type == ludb_db_mongo) {

    }

    VECTOR_CLEAR(h->insts,vector_t*, destory_row);
    return true;
}

static void collect(void* arg){
    ludb_batch_t* h = (ludb_batch_t*)arg;
    g_log_hook("batch thread start running");
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
        if (vector_size(h->insts) >= h->row_num) {
            //Log::debug("size to insert");
            insert(h);
            h->ins_time = now; //重置计时器
        }
        sleep(100);
    }
    g_log_hook("batch thread stop running");

    //结束
    if(h->type == ludb_db_oracle){
        destory_ludb_batch_oracle(h);
    } else if(h->type == ludb_db_mongo) {

    }
    destory_ludb_batch_handle(h);
}

ludb_batch_t* create_ludb_batch(ludb_db_type_t t, const char *tag, const char *sql, int rowNum, int interval, bind_column_t* binds) {
    int i = 0;
    SAFE_MALLOC(ludb_batch_t, ret);
    ret->type       = t;
    ret->tag        = create_string();
    string_init_cstr(ret->tag, tag);
    ret->sql        = create_string();
    string_init_cstr(ret->sql, sql);
    ret->row_num    = rowNum;
    ret->interval   = interval;

    ret->binds      = create_vector(void*);
    vector_init(ret->binds);
    for(i=0; binds[i].name; i++) {
        SAFE_MALLOC(bind_column_pri_t, col_info);
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

        col_info->name = create_string();
        string_init_cstr(col_info->name, binds[i].name);
        col_info->type = binds[i].type;
        col_info->max_len = max_len;
        col_info->nullable = binds[i].nullable;
        col_info->default_value = create_string();
        string_init_cstr(col_info->default_value, binds[i].default_value?binds[i].default_value:"");

        vector_push_back(ret->binds, col_info);
    }

    ret->recvs      = create_queue(void*);
    queue_init(ret->recvs);

    ret->insts      = create_vector(void*);
    vector_init(ret->insts);

    uv_mutex_init(&ret->mutex);

    if(t == ludb_db_oracle) {
        create_ludb_batch_oracle(ret);
    } else if(t == ludb_db_mongo) {

    }

    ret->ins_time = time(NULL);

    ret->running = true;
    uv_thread_create(&ret->tid, collect, ret);

    return ret;
}

void destory_ludb_batch(ludb_batch_t* h, int clean /*= 0*/) {
    h->running = false;
}

uint64_t ludb_batch_add_row(ludb_batch_t* h, const char **row) {
    const char *col = *row;
	uint64_t ret;
    vector_t *vecrow = create_vector(void*);
    vector_init(vecrow);
    for(;col; col=*(++row)){
        string_t *strcol = create_string();
        string_init_cstr(strcol, col);
        vector_push_back(vecrow, strcol);
    }
    uv_mutex_lock(&h->mutex);
    queue_push(h->recvs, vecrow);
	ret = queue_size(h->recvs);
    uv_mutex_unlock(&h->mutex);
	return ret;
}

void ludn_batch_catch_num(catch_num_cb cb){
	_cbCatchNum = cb;
}