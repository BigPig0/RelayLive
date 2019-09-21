#include "ludb_oracle.h"
#include "ludb_private.h"
#include "ocilib.h"

#pragma comment(lib, "ocilib.lib")

static map<string, OCI_ConnPool*> oracle_conn_pools; //map<string, OCI_ConnPool*>

static void oci_error_handler( OCI_Error *err ) {
    OCI_Connection * conn = OCI_ErrorGetConnection(err);
    char error_info[2000] = {0};
    sprintf(error_info, "Error[ORA-%05d] - msg[%s] - database[%s] - user[%s] - sql[%s]"
        , OCI_ErrorGetOCICode(err)
        , OCI_ErrorGetString(err)
        , OCI_GetDatabase(conn)
        , OCI_GetUserName(conn)
        , OCI_GetSql(OCI_ErrorGetStatement(err)));
    Log::error(error_info);
};

bool ludb_oracle_init(const char *path /*= NULL*/) {
    if(!OCI_Initialize(oci_error_handler, path, OCI_ENV_DEFAULT | OCI_ENV_THREADED))
        return false;

    return true;
}

void ludb_oracle_clean() {
    for(auto pair : oracle_conn_pools) {
        OCI_PoolFree(pair.second);
    }
    oracle_conn_pools.clear();
    OCI_Cleanup();
}

ludb_conn_t* ludb_oracle_connect(const char *database, const char *usr, const char *pwd) {
    OCI_Connection *cn = OCI_CreateConnection(database, usr, pwd, OCI_SESSION_DEFAULT);
    if(cn){
        ludb_oracle_conn *conn = new ludb_oracle_conn();
        conn->from_pool = false;
        conn->type = ludb_db_oracle;
        conn->conn = (void *)cn;
        return conn;
    }
    return NULL;
}

bool ludb_oracle_create_pool(const char *tag, const char *database, const char *usr, const char *pwd, uint32_t max, uint32_t min, uint32_t inc) {

    if(oracle_conn_pools.count(tag)) {
        Log::error("oracle pool tag already exist");
        return false;
    }

    OCI_ConnPool *pool = OCI_PoolCreate(database, usr, pwd, OCI_POOL_SESSION, OCI_SESSION_DEFAULT, min, max, inc);
    if(pool){
        Log::error("oracle OCI_PoolCreate failed");
        return false;
    }

    oracle_conn_pools.insert(make_pair(tag, pool));
    return true;
}

ludb_conn_t* ludb_oracle_pool_connect(const char *tag){
    if(!oracle_conn_pools.count(tag)){
        Log::error("oracle connect of tag[%s] is not exist", tag);
        return NULL;
    }
    OCI_ConnPool *pool = (OCI_ConnPool*)oracle_conn_pools[tag];
    if(pool){
        OCI_Connection *cn = OCI_PoolGetConnection(pool, NULL);
        if(cn){
            ludb_oracle_conn *conn = new ludb_oracle_conn();
            conn->from_pool = true;
            conn->type = ludb_db_oracle;
            conn->conn = (void *)cn;
            return conn;
        }
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_oracle_conn::ludb_oracle_conn() {
}

ludb_oracle_conn::~ludb_oracle_conn() {
    OCI_ConnectionFree((OCI_Connection *)conn);
}

ludb_stmt_t* ludb_oracle_conn::create_stmt() {
    OCI_Statement *sm = OCI_CreateStatement((OCI_Connection *)conn);
    if(sm){
        ludb_oracle_stmt *stmt = new ludb_oracle_stmt();
        stmt->type = ludb_db_oracle;
        stmt->conn = this;
        stmt->stmt = (void*)sm;
        return stmt;
    }
    return NULL;
}

bool ludb_oracle_conn::ludb_commit() {
    return OCI_Commit((OCI_Connection *)conn);
}

//////////////////////////////////////////////////////////////////////////

ludb_oracle_stmt::ludb_oracle_stmt() {

}

ludb_oracle_stmt::~ludb_oracle_stmt() {
    OCI_FreeStatement((OCI_Statement *)stmt);
}

bool ludb_oracle_stmt::execute(const char *sql) {
    return OCI_ExecuteStmt((OCI_Statement *)stmt, sql);
}

bool ludb_oracle_stmt::prepare(const char *sql) {
    return OCI_Prepare((OCI_Statement *)stmt, sql);
}

bool ludb_oracle_stmt::bind_int(const char *name, int *data) {
    return OCI_BindInt((OCI_Statement *)stmt, name, data);
}

bool ludb_oracle_stmt::bind_str(const char *name, const char *data, int len) {
    return OCI_BindString((OCI_Statement *)stmt, name, (otext*)data, len);
}

bool ludb_oracle_stmt::execute() {
    return OCI_Execute((OCI_Statement *)stmt);
}

uint32_t ludb_oracle_stmt::affected_rows() {
    return OCI_GetAffectedRows((OCI_Statement *)stmt);
}

ludb_rest_t* ludb_oracle_stmt::result() {
    OCI_Resultset *rs = OCI_GetResultset((OCI_Statement *)stmt);
    if(rs) {
        ludb_oracle_rest_t *rest = new ludb_oracle_rest_t();
        rest->type = ludb_db_oracle;
        rest->stmt = this;
        rest->conn = conn;
        rest->rest = (void*)rs;
        return rest;
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////

ludb_oracle_rest_t::ludb_oracle_rest_t(){

}

ludb_oracle_rest_t::~ludb_oracle_rest_t(){

}

bool ludb_oracle_rest_t::next(){
    return OCI_FetchNext((OCI_Resultset *)rest);
}

string ludb_oracle_rest_t::get_char(uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)rest;
    if(OCI_IsNull(rs, i))
        return "";
    char* pTmp = (char*)OCI_GetString(rs, i);
    if (pTmp)
        return pTmp;
    return "";
}

int ludb_oracle_rest_t::get_int(uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)rest;
    return OCI_IsNull(rs, i)?0:OCI_GetInt(rs, i);
}

string ludb_oracle_rest_t::get_date(uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)rest;
    if(OCI_IsNull(rs, i))
        return "";

    char buff[32] = {0};
    OCI_Date *dat = OCI_GetDate(rs, i);
    if(!OCI_DateToText(dat,"yyyymmddhh24miss",20,buff))
        return "";

    OCI_DateFree(dat);
    return buff;
}

string ludb_oracle_rest_t::get_blob(uint32_t i) {
    OCI_Resultset *rs = (OCI_Resultset *)rest;
    if(OCI_IsNull(rs, i))
        return "";

    OCI_Lob *lob = OCI_GetLob(rs,i);
    big_uint len = OCI_LobGetLength(lob)*2;
    string ret(len+1, 0);

    big_uint readlen = 0;
    big_uint allread = 0;
    do {
        char *buff = (char*)ret.c_str()+allread;
        readlen = OCI_LobRead(lob, buff, len);
        allread += readlen;
    } while (readlen);

    OCI_LobFree(lob);
    return ret;
}

//////////////////////////////////////////////////////////////////////////

/** 以下是批量处理方法 */

static void str_set(char *target, int index, int len, const char *source) {
    int offset = index * (len + 1);
    char *p = target + offset;
    const char *q = source;
    int count = 0;
    memset(p, 0, len + 1);
	p[0] = ' ';
    while (q && *q != '\0' && count < len){
        *p++ = *q++;
        ++count;
    }
}

static void bind_set_data(OCI_Bind *bind, int index, int len, string val, bool is_null /*= false*/){
    if(!is_null) {
        char *str = (char *)OCI_BindGetData(bind);
        str_set(str, index, len, val.c_str());
    } else {
        OCI_BindSetNullAtPos(bind, index + 1);
    }
}

static void bind_set_date(OCI_Bind *bind, int index, string val, char *date_fmt, bool is_null /*= false*/){
    if(!is_null){
        OCI_Date **dates = (OCI_Date **)OCI_BindGetData(bind);
        OCI_DateFromText(dates[index], val.c_str(), date_fmt);
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int16(OCI_Bind *bind, int index, int16_t val, bool is_null /*= false*/){
    if(!is_null) {
        int16_t *numbers = (int16_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int32(OCI_Bind *bind, int index, int32_t val, bool is_null /*= false*/){
    if(!is_null) {
        int32_t *numbers = (int32_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

static void bind_set_int64(OCI_Bind *bind, int index, int64_t val, bool is_null /*= false*/){
    if(!is_null) {
        int64_t *numbers = (int64_t *)OCI_BindGetData(bind);
        numbers[index] = val;
    } else {
        OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
    }
}

ludb_oracle_batch::ludb_oracle_batch(string Tag, string Sql, int RowNum, int Interval, bind_column_t* Binds)
    : ludb_batch_t(Tag, Sql, RowNum, Interval, Binds)
{
    int32_t colNum = binds.size();    // 列数
    int32_t nBindSize = colNum * pointLen;    //绑定区域的大小
    int32_t allSize = 0;    //数据区域的大小
    for(auto &col : binds) {
        int Length = col.max_len;
        if(col.type==column_type_char)
            Length++;
        allSize += Length * row_num;
    }

    //申请内存
    buff = (char*)calloc(1, nBindSize + allSize);
    CHECK_POINT_VOID(buff);

    //绑定区域
    char *pBindBegin = buff;
    //数据区域
    char *pDataBegin = pBindBegin + nBindSize;

    //将绑定数据地址指向数据区域
    int32_t nOffsetBind = 0;
    int32_t nOffsetData = 0;
    for (auto &col : binds) {
        int Length;
        char* tmp = pDataBegin + nOffsetData;
        memcpy(pBindBegin+nOffsetBind, &tmp, pointLen);

        if (col.type == column_type_blob) {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < row_num; i++) {
                blob[i] = OCI_LobCreate(NULL,OCI_BLOB);
            }
        } else if (col.type == column_type_date) {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < row_num; i++) {
                da[i] = OCI_DateCreate(NULL);
            }
        }

        nOffsetBind += pointLen;
        Length = col.type==column_type_char?col.max_len+1:col.max_len;
        nOffsetData += Length * row_num;
    }
}

ludb_oracle_batch::~ludb_oracle_batch() {
    char* pDataBegin = buff + binds.size() * pointLen;
    int32_t nOffsetData = 0;
    for(auto &col : binds) {
        int Length = col.max_len;
        if(col.type==column_type_char)
            Length++;
        if (col.type == column_type_blob) {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < row_num; i++) {
                OCI_LobFree(blob[i]);
            }
        } else if (col.type == column_type_date) {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            int i = 0;
            for (; i < row_num; i++) {
                OCI_DateFree(da[i]);
            }
        }
        nOffsetData += Length * row_num;
    }

    free(buff);
}

bool ludb_oracle_batch::insert() {
    int nRowSize = insts.size();
    if (nRowSize > row_num) 
        nRowSize = row_num; //正常不会走到这一步
    Log::debug("insert %d rows to db", nRowSize);

    // 获取数据库连接
    ludb_conn_t *conn = ludb_oracle_pool_connect(tag.c_str());
    if(!conn || !conn->conn){
        Log::error("fail to get connection: %s", tag.c_str());
        return false;
    }
    OCI_Connection *cn = (OCI_Connection*)conn->conn;
    OCI_Statement *st = OCI_CreateStatement(cn);

    if(binds.empty()){
        for(auto &row : insts){
            for(string &sql : row){
                OCI_ExecuteStmt(st, sql.c_str());
            }
        }
        OCI_Commit(cn);
    } else {

        //准备执行sql
        OCI_SetBindAllocation(st, OCI_BAM_EXTERNAL);
        OCI_Prepare(st, sql.c_str());
        OCI_BindArraySetSize(st, nRowSize);

        //绑定
        int nOffsetBind = 0;
        char* pBindBegin = buff;
        for(auto &col : binds){
            if (col.type == column_type_char) {
                char* pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfStrings(st, col.name.c_str(), pBuff, col.max_len, 0);
            } else if(col.type == column_type_int) {
                int32_t* pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfInts(st, col.name.c_str(), pBuff, 0);
            } else if(col.type == column_type_float) {
                double* pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfDoubles(st, col.name.c_str(), pBuff, 0);
            } else if(col.type == column_type_long) {
                big_int* pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfBigInts(st, col.name.c_str(), pBuff, 0);
            } else if(col.type == column_type_uint) {
                uint32_t* pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfUnsignedInts(st, col.name.c_str(), pBuff, 0);
            } else if(col.type == column_type_date) {
                OCI_Date** pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfDates(st, col.name.c_str(), pBuff, 0);
            } else if(col.type == column_type_blob) {
                OCI_Lob** pBuff;
                memcpy(&pBuff, pBindBegin+nOffsetBind, pointLen);
                OCI_BindArrayOfLobs(st, col.name.c_str(), pBuff, OCI_BLOB, 0);
            }
            nOffsetBind += pointLen;
        }

        int column_num = binds.size();
        for(int i = 0; i < nRowSize; ++i) {
            vector<string> &row = insts[i];
            int nLineColumnSize = row.size();
            for (int j = 0; j<column_num; ++j) {
                bind_column_pri_t &col_info = binds[j];
                string value = j<nLineColumnSize?row[j]:col_info.default_value;
                bool isnull = col_info.nullable?value.empty():false;

                if(col_info.type == column_type_char) {
                    bind_set_data(OCI_GetBind(st, j+1), i, col_info.max_len, value, isnull);
                } else if (col_info.type == column_type_int 
                    || col_info.type == column_type_uint
                    || col_info.type == column_type_float) {
                        int32_t n = value.empty()?0:stoi(value);
                        bind_set_int32(OCI_GetBind(st, j+1), i, n, isnull);
                } else if (col_info.type == column_type_long) {
                    int64_t n = value.empty()?0:_atoi64(value.c_str());
                    bind_set_int64(OCI_GetBind(st, j+1), i, n, isnull);
                } else if (col_info.type == column_type_date) {
                    string val = value.substr(0, 14);
                    bind_set_date(OCI_GetBind(st, j+1), i, val, "yyyymmddhh24miss", isnull);
                }
            }//for (int j = 0; j<column_num; ++j)
        }//for(int i = 0; i < nRowSize; ++i)

        boolean bExcute = OCI_Execute(st);
        boolean bCommit = OCI_Commit(cn);
        unsigned int count = OCI_GetAffectedRows(st);    //某一行插入失败，不会回滚所有数据，但是出错后count为0，这是ocilib的一个bug
        char buff[100]={0};
        if (!bExcute || !bCommit || 0 == count){
            Log::error("Execute %s fail bExcute: %d; bCommit: %d; count: %d" , tag.c_str(), bExcute, bCommit, count);
        } else {
            Log::debug("Execute %s sucess bExcute: %d; bCommit: %d; count: %d" ,tag.c_str(), bExcute, bCommit, count);
        }
    }
    OCI_FreeStatement(st);
    delete conn;
    return true;
}