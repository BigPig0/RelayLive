#include "ludb_redis.h"
#include "ludb_private.h"
#include "hiredis.h"

#ifdef WINDOWS_IMPL
#pragma comment(lib, "hiredis.lib")
#endif

static map<string, redisContext*> redis_conn_pools; //map<string, redisContext*>

struct redis_stmt_t {
    string      cmd;       // 执行的命令
    int         rsid;      // 记录读取结果取出情况
    redisReply *reply;     // 执行结果
	redis_stmt_t()
		: rsid(0)
		, reply(nullptr)
	{}
};

bool ludb_redis_init() {
#ifdef WINDOWS_IMPL
    static WSADATA wsadata;
    int err = WSAStartup(MAKEWORD(2,2), &wsadata);
#endif
    return true;
}

void ludb_redis_clean() {

}

ludb_conn_t* ludb_redis_connect(const char *database, const char *usr, const char *pwd) {
    char host[50] = {0};    //服务ip
    int port = 6379;        //服务端口
    char *p;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    strcpy(host, database);
    p = strstr(host, ":");
    if(p) {
        host[p-host] = 0;
        port = atoi(p+1);
    }

    redisContext *c = redisConnectWithTimeout(host, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            Log::error("Connection error: %s", c->errstr);
            redisFree(c);
        } else {
            Log::error("Connection error: can't allocate redis context");
        }
        return NULL;
    }

    char cmd[20] = {0};
    sprintf(cmd, "SELECT %s", usr);
    redisReply *reply = (redisReply*)redisCommand(c, cmd);
    Log::debug("%s: %s", cmd, reply->str);
    freeReplyObject(reply);

    ludb_redis_conn *conn = new ludb_redis_conn();
    conn->from_pool = false;
    conn->type = ludb_db_redis;
    conn->conn = (void *)c;
    return conn;
}

bool ludb_redis_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    if(redis_conn_pools.count(tag)) {
        Log::error("redis pool tag already exist");
        return false;
    } 
    char host[50] = {0};    //服务ip
    int port = 6379;        //服务端口
    char *p;
    struct timeval timeout = { 1, 500000 }; // 1.5 seconds

    strcpy(host, database);
    p = strstr(host, ":");
    if(p) {
        host[p-host] = 0;
        port = atoi(p+1);
    }
    redisContext *c = redisConnectWithTimeout(host, port, timeout);
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context");
        }
        return NULL;
    }
    char cmd[20] = {0};
    sprintf(cmd, "SELECT %s", usr);
    redisReply *reply = (redisReply*)redisCommand(c, cmd);
    printf("%s: %s", cmd, reply->str);
    freeReplyObject(reply);

    redis_conn_pools.insert(make_pair(tag, c));
    return true;
}

ludb_conn_t* ludb_redis_pool_connect(const char *tag){
    if(!redis_conn_pools.count(tag)){
        Log::error("redis connect of tag[%s] is not exist", tag);
    }
    redisContext *c = redis_conn_pools[tag];
    
    ludb_redis_conn *conn = new ludb_redis_conn();
    conn->from_pool = true;
    conn->type = ludb_db_redis;
    conn->conn = (void *)c;
    return conn;
}

//////////////////////////////////////////////////////////////////////////

ludb_redis_conn::ludb_redis_conn(){}

ludb_redis_conn::~ludb_redis_conn(){
    redisContext *c = (redisContext *)conn;
    if(from_pool){

    } else {
        redisFree(c);
    }
}

ludb_stmt_t* ludb_redis_conn::create_stmt(){
    redis_stmt_t *st = new redis_stmt_t();
    ludb_redis_stmt *stmt = new ludb_redis_stmt();
    stmt->type = ludb_db_redis;
    stmt->conn = this;
    stmt->stmt = (void *)st;
    return stmt;
}

bool ludb_redis_conn::ludb_commit(){
    return true;
}

//////////////////////////////////////////////////////////////////////////

ludb_redis_stmt::ludb_redis_stmt(){}

ludb_redis_stmt::~ludb_redis_stmt(){
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    delete st;
}


bool ludb_redis_stmt::execute(const char *sql){
    redisContext *c = (redisContext*)conn->conn;
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    st->reply = (redisReply*)redisCommand(c, sql);
    if(st->reply->type == REDIS_REPLY_ERROR)
        return false;
    return true;
}

bool ludb_redis_stmt::prepare(const char *sql){
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    st->cmd = sql;
    return true;
}

bool ludb_redis_stmt::bind_int(const char *name, int *data){
    return false;
}

bool ludb_redis_stmt::bind_str(const char *name, const char *data, int len){
    return false;
}

bool ludb_redis_stmt::execute(){
    redisContext *c = (redisContext*)conn->conn;
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    st->reply = (redisReply*)redisCommand(c, st->cmd.c_str());
    if(st->reply->type == REDIS_REPLY_ERROR)
        return false;
    return true;
}

uint32_t ludb_redis_stmt::affected_rows(){
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    if(st->reply->type == REDIS_REPLY_ERROR)
        return 0;
    else if(st->reply->type == REDIS_REPLY_STRING)
        return 1;
    else if(st->reply->type == REDIS_REPLY_INTEGER)
        return st->reply->integer;
    else if(st->reply->type == REDIS_REPLY_ARRAY)
        return st->reply->elements;

    return 0;
}

ludb_rest_t* ludb_redis_stmt::result(){
    redis_stmt_t *st = (redis_stmt_t*)stmt;
    ludb_redis_rest_t *rest = new ludb_redis_rest_t();
    rest->type = ludb_db_redis;
    rest->stmt = this;
    rest->conn = conn;
    rest->rest = (void*)st->reply;
    return rest;
}

//////////////////////////////////////////////////////////////////////////

ludb_redis_rest_t::ludb_redis_rest_t(){}

ludb_redis_rest_t::~ludb_redis_rest_t(){}

bool ludb_redis_rest_t::next(){
    redisReply *reply = (redisReply *)rest;
    redis_stmt_t *st = (redis_stmt_t*)stmt->stmt;
    if(reply->type == REDIS_REPLY_ERROR)
        return false;
    if(reply->type == REDIS_REPLY_ARRAY){
        return st->rsid++ < reply->elements;
    }
    return st->rsid++ < 1;
}

string ludb_redis_rest_t::get_char(uint32_t i){
    redisReply *reply = (redisReply *)rest;
    redis_stmt_t *st = (redis_stmt_t*)stmt->stmt;
    if(reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_ERROR) {
        string ret(reply->str, reply->len);
        return ret;
    } else if (reply->type == REDIS_REPLY_INTEGER){
        char buff[20]={0};
        sprintf(buff,"%d",reply->integer);
        return buff;
    } else if (reply->type == REDIS_REPLY_ARRAY) {
        if(st->rsid > reply->elements){
            return "";
        }
        redisReply *repele = reply->element[st->rsid-1];
        if(repele->type == REDIS_REPLY_STRING || repele->type == REDIS_REPLY_ERROR) {
            string ret(repele->str, repele->len);
            return ret;
        } else if (repele->type == REDIS_REPLY_INTEGER){
            char buff[20]={0};
            sprintf(buff,"%d",repele->integer);
            return buff;
        }
    }
    return "";
}

int ludb_redis_rest_t::get_int(uint32_t i){
    redisReply *reply = (redisReply *)rest;
    redis_stmt_t *st = (redis_stmt_t*)stmt->stmt;
    if(reply->type == REDIS_REPLY_STRING || reply->type == REDIS_REPLY_ERROR) {
        string ret(reply->str, reply->len);
        return stoi(ret);
    } else if (reply->type == REDIS_REPLY_INTEGER){
        return reply->integer;
    } else if (reply->type == REDIS_REPLY_ARRAY) {
        if(st->rsid > reply->elements){
            return 0;
        }
        redisReply *repele = reply->element[st->rsid-1];
        if(repele->type == REDIS_REPLY_STRING || repele->type == REDIS_REPLY_ERROR) {
            string ret(repele->str, repele->len);
            return stoi(ret);
        } else if (repele->type == REDIS_REPLY_INTEGER){
            return repele->integer;
        }
    }
    return 0;
}

string ludb_redis_rest_t::get_date(uint32_t i){
    Log::error("Redis doesn't has this interface");
    return "";
}

string ludb_redis_rest_t::get_blob(uint32_t i){
    Log::error("Redis doesn't has this interface");
    return "";
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

ludb_redis_batch::ludb_redis_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : ludb_batch_t(Tag, Sql, RowNum, Interval, Binds)
{

}

ludb_redis_batch::~ludb_redis_batch() {

}

bool ludb_redis_batch::insert() {
    return false;
}