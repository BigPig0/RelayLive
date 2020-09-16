#include "ludb_mysql.h"
#ifdef DB_MYSQL
#include "ludb_private.h"
#include <mysql.h>


#pragma comment(lib, "mariadbclient.lib")

static map<string, MYSQL*> mysql_conn_pools; //map<string, MYSQL*>

bool ludb_mysql_init() {
    mysql_library_init(0, NULL, NULL);
    return true;
}

void ludb_mysql_clean() {
    for(auto pair : mysql_conn_pools) {
        mysql_close(pair.second);
    }
    mysql_conn_pools.clear();
    mysql_library_end();
}

ludb_conn_t* ludb_mysql_connect(const char *database, const char *usr, const char *pwd) {
    string tmp = database;
    size_t pos = tmp.rfind('/');
    if(pos == string::npos)
        return NULL;
    string host = tmp.substr(0, pos);
    string dbname = tmp.substr(pos, tmp.size()-pos-1);

    MYSQL *myCont = new MYSQL();
    mysql_init(myCont);
    if (mysql_real_connect(myCont, host.c_str(), usr, pwd, dbname.c_str(), 3306, NULL, 0)) {
        ludb_mysql_conn *conn = new ludb_mysql_conn();
        conn->from_pool = false;
        conn->type = ludb_db_mysql;
        conn->conn = (void *)myCont;
        return conn;
    }
    delete myCont;
    return NULL;
}

bool ludb_mysql_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {
    string tmp = database;
    size_t pos = tmp.rfind('/');
    if(pos == string::npos)
        return false;
    string host = tmp.substr(0, pos);
    string dbname = tmp.substr(pos, tmp.size()-pos-1);

    if(mysql_conn_pools.count(tag)) {
        Log::error("mysql pool tag already exist");
        return false;
    }

    MYSQL *myCont = new MYSQL();
    mysql_init(myCont);
    if (mysql_real_connect(myCont, host.c_str(), usr, pwd, dbname.c_str(), 3306, NULL, 0)) {
        mysql_conn_pools.insert(make_pair(tag, myCont));
    }
    return true;
}

ludb_conn_t* ludb_mysql_pool_connect(const char *tag){
    if(!mysql_conn_pools.count(tag)){
        Log::error("mysql connect of tag[%s] is not exist", tag);
        return NULL;
    }
    MYSQL *pool = (MYSQL*)mysql_conn_pools[tag];
    if(pool){
        ludb_mysql_conn *conn = new ludb_mysql_conn();
        conn->from_pool = false;
        conn->type = ludb_db_mysql;
        conn->conn = (void *)pool;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_mysql_conn::ludb_mysql_conn() {
}

ludb_mysql_conn::~ludb_mysql_conn() {
    mysql_close((MYSQL *)conn);
}

ludb_stmt_t* ludb_mysql_conn::create_stmt() {
    MYSQL_STMT *sm = mysql_stmt_init((MYSQL *)conn);
    if(sm){
        ludb_mysql_stmt *stmt = new ludb_mysql_stmt();
        stmt->type = ludb_db_mysql;
        stmt->conn = this;
        stmt->stmt = (void*)sm;
        return stmt;
    }
    return NULL;
}

bool ludb_mysql_conn::ludb_commit() {
    return mysql_commit((MYSQL *)conn);
}

//////////////////////////////////////////////////////////////////////////

ludb_mysql_stmt::ludb_mysql_stmt():bindData(NULL) {

}

ludb_mysql_stmt::~ludb_mysql_stmt() {
    mysql_stmt_close((MYSQL_STMT *)stmt);
    if(bindData != NULL) {
        MYSQL_BIND *binds = (MYSQL_BIND *)bindData;
        delete[] binds;
        bindData = NULL;
    }
}

bool ludb_mysql_stmt::execute(const char *sql) {
    mysql_stmt_prepare((MYSQL_STMT *)stmt, sql, strlen(sql));
    return mysql_stmt_execute((MYSQL_STMT *)stmt) == 0;
}

bool ludb_mysql_stmt::prepare(const char *sql) {
    return mysql_stmt_prepare((MYSQL_STMT *)stmt, sql, strlen(sql));
}

bool ludb_mysql_stmt::bind_int(const char *name, int *data) {
    return false;
}

bool ludb_mysql_stmt::bind_str(const char *name, const char *data, int len) {
    return false;
}

bool ludb_mysql_stmt::bind(uint32_t col_num, column_type_t *col_type, string *col_value) {
    if(bindData != NULL) {
        MYSQL_BIND *binds = (MYSQL_BIND *)bindData;
        delete[] binds;
        bindData = NULL;
    }

    MYSQL_BIND *binds = new MYSQL_BIND[col_num];
    for(int i=0; i<col_num; i++) {
        if(col_type[i] == column_type_char)
            binds[i].buffer_type = MYSQL_TYPE_VARCHAR;
        else if(col_type[i] == column_type_int)
            binds[i].buffer_type = MYSQL_TYPE_LONG;
        else if(col_type[i] == column_type_float)
            binds[i].buffer_type = MYSQL_TYPE_FLOAT;
        else if(col_type[i] == column_type_long)
            binds[i].buffer_type = MYSQL_TYPE_LONGLONG;
        else if(col_type[i] == column_type_blob)
            binds[i].buffer_type = MYSQL_TYPE_BLOB;
        else if(col_type[i] == column_type_date)
            binds[i].buffer_type = MYSQL_TYPE_DATETIME;
        else
            binds[i].buffer_type = MYSQL_TYPE_NULL;
        binds[i].buffer = (void*)col_value[i].c_str();
        binds[i].buffer_length = (unsigned long)col_value[i].size();
        binds[i].is_null = NULL;
    }
    return mysql_stmt_bind_param((MYSQL_STMT *)stmt, binds);
}

bool ludb_mysql_stmt::execute() {
    return mysql_stmt_execute((MYSQL_STMT *)stmt)==0;
}

uint32_t ludb_mysql_stmt::affected_rows() {
    return mysql_stmt_affected_rows((MYSQL_STMT *)stmt);
}

ludb_rest_t* ludb_mysql_stmt::result() {
    MYSQL_RES *rs = mysql_store_result((MYSQL *)conn);
    if(rs) {
        ludb_mysql_rest_t *rest = new ludb_mysql_rest_t();
        rest->type = ludb_db_mysql;
        rest->stmt = this;
        rest->conn = conn;
        rest->rest = (void*)rs;
        return rest;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_mysql_rest_t::ludb_mysql_rest_t(){

}

ludb_mysql_rest_t::~ludb_mysql_rest_t(){
    mysql_free_result((MYSQL_RES *)rest);
}

bool ludb_mysql_rest_t::next(){
    row = mysql_fetch_row((MYSQL_RES *)rest);
    return row!=NULL;
}

string ludb_mysql_rest_t::get_char(uint32_t i) {
    if(row[i] == NULL)
        return "";
    char* pTmp = (char*)row[i];
    if (pTmp)
        return pTmp;
    return "";
}

int ludb_mysql_rest_t::get_int(uint32_t i) {
    return (row[i]==NULL)?0:atoi(row[i]);
}

string ludb_mysql_rest_t::get_date(uint32_t i) {
    if(row[i] == NULL)
        return "";

    int year, month, day, hour, minute, second;
    int num = sscanf(row[i], "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    if(num != 6)
        return "";

    char buff[32] = {0};
    sprintf(buff,"%04d%02d%02d%02d%02d%02d", year, month, day, hour, minute, second);

    return buff;
}

string ludb_mysql_rest_t::get_blob(uint32_t i) {
    return "";
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

ludb_mysql_batch::ludb_mysql_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : ludb_batch_t(Tag, Sql, RowNum, Interval, Binds)
{
}

ludb_mysql_batch::~ludb_mysql_batch() {
}

bool ludb_mysql_batch::insert() {
    int nRowSize = insts.size();
    if (nRowSize > row_num) 
        nRowSize = row_num; //正常不会走到这一步
    Log::debug("insert %d rows to db", nRowSize);

    // 获取数据库连接
    ludb_conn_t *conn = ludb_mysql_pool_connect(tag.c_str());
    if(!conn || !conn->conn){
        Log::error("fail to get connection: %s", tag.c_str());
        return false;
    }
    MYSQL *cn = (MYSQL*)conn->conn;
    mysql_autocommit(cn,0);//关闭自动提交

    bool ret = true;
    if(binds.empty()){
        for(auto &row : insts){
            for(string &sql : row){
                mysql_query(cn, sql.c_str());
            }
        }
        mysql_commit(cn);
    } else {
        //绑定
        MYSQL_BIND *myBinds = new MYSQL_BIND[binds.size()];
        int i = 0;
        for(auto &col : binds){
            if (col.type == column_type_char) {
                myBinds[i].buffer_type = MYSQL_TYPE_VARCHAR;
            } else if(col.type == column_type_int) {
                myBinds[i].buffer_type = MYSQL_TYPE_LONG;
            } else if(col.type == column_type_float) {
                myBinds[i].buffer_type = MYSQL_TYPE_FLOAT;
            } else if(col.type == column_type_long) {
                myBinds[i].buffer_type = MYSQL_TYPE_LONGLONG;
            } else if(col.type == column_type_uint) {
                myBinds[i].buffer_type = MYSQL_TYPE_LONG;
            } else if(col.type == column_type_date) {
                myBinds[i].buffer_type = MYSQL_TYPE_DATETIME;
            } else if(col.type == column_type_blob) {
                myBinds[i].buffer_type = MYSQL_TYPE_BLOB;
            }
            myBinds[i].is_null = 0;
            myBinds[i].buffer = (void*)col.default_value.c_str();
            myBinds[i].buffer_length = col.default_value.size();
            i++;
        }

        int column_num = binds.size();
        unsigned int count = 0;
        for(int i = 0; i < nRowSize; ++i) {
            vector<string> &row = insts[i];
            int nLineColumnSize = row.size();
            for (int j = 0; j<column_num; ++j) {
                bind_column_pri_t &col_info = binds[j];

                if(j<nLineColumnSize) {
                    myBinds[i].buffer = (void*)row[j].c_str();
                    myBinds[i].buffer_length = row[j].size();
                }
            }//for (int j = 0; j<column_num; ++j)

            MYSQL_STMT *st = mysql_stmt_init((MYSQL *)conn);
            mysql_stmt_prepare(st, sql.c_str(), sql.size());
            mysql_stmt_bind_param(st, myBinds);
            mysql_stmt_execute(st);
            column_num += mysql_stmt_affected_rows(st);
        }//for(int i = 0; i < nRowSize; ++i)
        delete[] myBinds;

        my_bool bCommit = mysql_commit(cn);
        char buff[100]={0};
        if (!bCommit || 0 == count){
            ret = false;
            Log::error("Execute %s fail bExcute: %d; bCommit: %d; count: %d" , tag.c_str(), 1, bCommit, count);
        } else {
            Log::debug("Execute %s sucess bExcute: %d; bCommit: %d; count: %d" ,tag.c_str(), 1, bCommit, count);
        }
    }
    delete conn;
    return ret;
}
#endif