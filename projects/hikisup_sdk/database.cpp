#include "util.h"
#include "easylog.h"
#include "database.h"
#include "ludb.h"
#include "uv.h"
#include "easylog.h"


namespace DbTsk {

    // 设备id与账号id映射
    static map<string, uint64_t> g_mapDevice;
    static CriticalSection g_csDevice;
    // 账号id与任务映射
    static map<uint64_t, VipTask> g_mapTask;
    static CriticalSection g_csTask;


static void updateDb(void *arg) {
    while (true) {
        string db = util::Settings::getValue("VipDB", "path");
        string usr = util::Settings::getValue("VipDB", "usr");
        string pwd = util::Settings::getValue("VipDB", "pwd");
        ludb_conn_t* conn = ludb_connect(ludb_db_type_t::ludb_db_oracle, db.c_str(), usr.c_str(), pwd.c_str());
        if(NULL == conn){
            Log::error("connect to vip db failed");
            sleep(10);
            continue;
        }


        string sql;
        ludb_stmt_t *stmt = NULL;
        ludb_rest_t *rs = NULL;

        sql = "select TASK_ID, ACCOUNT_ID from VIP_TASK t where t.Account_Id != 0 and t.Task_Status = 1";
        stmt = ludb_create_stmt(conn);
        ludb_execute_stmt(stmt, sql.c_str());
        rs = ludb_result(stmt);
        if(rs == NULL) {
            ludb_free_stmt(stmt);
            Log::error("query vip task db failed");
            sleep(10);
            continue;
        }
        g_mapTask.clear();
        while (ludb_result_next(rs)){
            VipTask tsk;
            tsk.taskId    = ludb_rest_get_int64(rs, 1);
            tsk.accountId = ludb_rest_get_int64(rs, 2);
            g_mapTask.insert(make_pair(tsk.accountId, tsk));
        }
        ludb_free_result(rs);
        ludb_free_stmt(stmt);
        Log::debug("all task num %d", g_mapTask.size());

        sql = "select Account_id, bind_code from VIP_ACCOUNT t";
        stmt = ludb_create_stmt(conn);
        ludb_execute_stmt(stmt, sql.c_str());
        rs = ludb_result(stmt);
        if(rs == NULL) {
            ludb_free_stmt(stmt);
            Log::error("query vip account db failed");
            sleep(10);
            continue;
        }
        g_mapDevice.clear();
        while (ludb_result_next(rs)){
            uint64_t accountId = ludb_rest_get_int64(rs, 1);
            string deviceCode = ludb_rest_get_char(rs, 2);
            g_mapDevice.insert(make_pair(deviceCode, accountId));
        }
        ludb_free_result(rs);
        ludb_free_stmt(stmt);
        Log::debug("all account num %d", g_mapDevice.size());

        ludb_free_conn(conn);
        sleep(10);
    }
}

/** 初始化 */
bool Init() {
    //初始化数据库
    string path = util::Settings::getValue("DataBase", "Path");
    if(!ludb_init_oracle(path.c_str())){
        Log::error("data base init failed");
        return false;
    }

    uv_thread_t t;
    uv_thread_create(&t, updateDb, NULL);
	return true;
}

bool CheckDeviceId(string &devid, VipTask &tskInfo) {
    bool ret = false;
    g_csDevice.lock();
    if(g_mapDevice.count(devid) == 0) {
        g_csDevice.unlock();
        Log::debug("GPS info: DeviceID:%s", devid.c_str());
    } else {
        uint64_t accountId = g_mapDevice[devid];
        g_csDevice.unlock();
        g_csTask.lock();
        if(g_mapTask.count(accountId) == 0) {
            g_csTask.unlock();
            Log::debug("GPS info: DeviceID:%s, Account:%llu", devid.c_str(), accountId);
        } else {
            tskInfo = g_mapTask[accountId];
            g_csTask.unlock();
            ret = true;
        }
    }
    return ret;
}
}