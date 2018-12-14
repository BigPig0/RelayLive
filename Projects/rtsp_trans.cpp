// sever.cpp : 定义控制台应用程序的入口点。
//
#include "common.h"
#include "MiniDump.h"
#include "uvIpc.h"
#include "utilc_api.h"
#include "rtsp.h"
#include "libOci.h"
#include <stdio.h>

//////////////////////////////////////////////////////////////////////////
// 数据库获取设备列表

static OracleClient* client;
static string _db_connect_name = "DB";
static map<string,RTSP_REQUEST> _map_devs;

static void init_db(){
    auto oci_error_handler = [](OCI_Error *err){
        OCI_Connection * conn = OCI_ErrorGetConnection(err);
        Log::error("Error[ORA-%05d] - msg[%s] - database[%s] - user[%s] - sql[%s]"
            , OCI_ErrorGetOCICode(err)
            , OCI_ErrorGetString(err)
            , OCI_GetDatabase(conn)
            , OCI_GetUserName(conn)
            , OCI_GetSql(OCI_ErrorGetStatement(err)));
    };
    string path = Settings::getValue("DataBase", "Path");

    if(!OCI_Initialize(oci_error_handler, path.c_str(), OCI_ENV_THREADED))
        return;

    ConPool_Setting dbset;
    dbset.database = Settings::getValue("DataBase","Addr");
    dbset.username = Settings::getValue("DataBase","User");
    dbset.password = Settings::getValue("DataBase","PassWord");
    dbset.max_conns = 5;
    dbset.min_conns = 2;
    dbset.inc_conns = 2;
    client = OracleClient::GetInstance();
    client->connect(_db_connect_name, dbset);
}

static bool get_devs_from_db()
{
    Log::debug("loadDevicesFromOracle begin");
    conn_pool_ptr pool = OracleClient::GetInstance()->get_conn_pool(_db_connect_name);
    if (pool == NULL) {
        Log::error("fail to get pool: basic");
        return false;
    }
    int index = pool->getConnection();
    if (index == -1) {
        Log::error("fail to get connection: basic");
        return false;
    }
    OCI_Connection *cn = pool->at(index);
    OCI_Statement *st = OCI_CreateStatement(cn);

    //查询设备信息
    const char *sql = "select t.FACILITY_ID,t.ACCESS_TYPE,t.IP_ADDRESS,"
        "t.PORT,t.USER_NAME,t.PASSWORD,t.CHANEL from VIDEO_INFO t"
        " where t.IP_ADDRESS is not null and t.IS_USABLE = 1 and t.ACCESS_TYPE = 1"
        " order by t.FACILITY_ID";

    OCI_ExecuteStmt(st, sql);
    OCI_Resultset *rs = OCI_GetResultset(st);
    while (OCI_FetchNext(rs))
    {
        RTSP_REQUEST dev;
        dev.ssid      = OCI_GET_STRING(rs,1);
        dev.dev_type  = (devType)stoi(OCI_GET_STRING(rs,2));
        dev.ip        = OCI_GET_STRING(rs,3);
        dev.port      = 554;//stoi(OCI_GET_STRING(rs,4));
        dev.user_name = OCI_GET_STRING(rs,5);
        dev.password  = OCI_GET_STRING(rs,6);
        dev.channel   = stoi(OCI_GET_STRING(rs,7));
        dev.stream    = stoi(Settings::getValue("DataBase","Mode"));
        _map_devs.insert(make_pair(dev.ssid, dev));

        stringstream ss;
        ss << "ssid:" << dev.ssid <<" devtype:" << dev.dev_type << " ip:" << dev.ip
            << " port:" << dev.port << " user:" << dev.user_name << " pwd:" << dev.password
            << " chanel:" << dev.channel << " stream:" << dev.stream;
        Log::debug(ss.str().c_str());
    }

    OCI_FreeStatement(st);
    pool->releaseConnection(index);
    Log::debug("get devs from db finish successful");
    return true;
}

static RTSP_REQUEST find_dev(string ssid) {
    auto it = _map_devs.find(ssid);
    if (it != _map_devs.end())
    {
        return it->second;
    }
    RTSP_REQUEST ret;
    ret.ssid = "err";
    return ret;
}

//////////////////////////////////////////////////////////////////////////

uv_ipc_handle_t* h = NULL;

static string strfind(char* src, char* begin, char* end){
    char *p1, *p2;
    p1 = strstr(src, begin);
    if(!p1) return "";
    p1 += strlen(begin);
    p2 = strstr(p1, end);
    if(p2) return string(p1, p2-p1);
    else return string(p1);
}

static void on_play_cb(string ssid, int status){
    Log::debug("rtsp play back %d", status);
    // ssid=123&ret=0&error=XXXX
    stringstream ss;
    ss << "ssid=" << ssid << "&ret=" << status << "&error=" << rtsp_strerr(status);
    string str = ss.str();
    uv_ipc_send(h, "liveDest", "live_play_answer", (char*)str.c_str(), str.size());
}

/**
 * @param h ipc句柄
 * @param user 用户自定义数据
 * @param name 发送者名称
 * @param msg 消息名称
 * @param data 接收到的数据
 * @param len 接收到的数据长度
 */
static void on_ipc_recv(uv_ipc_handle_t* h, void* user, char* name, char* msg, char* data, int len)
{
    if (!strcmp(msg,"live_play")) {
        // ssid=123&rtpip=1.1.1.1&rtpport=50000
		data[len] = 0;
        string ssid = strfind(data, "ssid=", "&");
        string port = strfind(data, "rtpport=", "&");
        RTSP_REQUEST req = find_dev(ssid);
        if(req.ssid != ssid) {
            stringstream ss;
            ss << "ssid=" << ssid << "&ret=-1&error=device is not exist";
            string str = ss.str();
            uv_ipc_send(h, "liveDest", "live_play_answer", (char*)str.c_str(), str.size());
        }
        req.rtp_port = stoi(port);
        rtsp_play(req, on_play_cb);
    } else if(!strcmp(msg,"stop_play")) {
		string ssid(data, len);
		RTSP_REQUEST req = find_dev(ssid);
        if(req.ssid != ssid) {
            stringstream ss;
            ss << "ssid=" << ssid << "&ret=-1&error=device is not exist";
            string str = ss.str();
            uv_ipc_send(h, "liveDest", "live_play_answer", (char*)str.c_str(), str.size());
        }
		rtsp_stop(ssid);
	}
}

int main()
{
    /** Dump设置 */
    CMiniDump dump("rtsp_trans.dmp");

    /** 进程间通信 */
    int ret = uv_ipc_client(&h, "relay_live", NULL, "liveSrc", on_ipc_recv, NULL);
    if(ret < 0) {
        printf("ipc server err: %s\n", uv_ipc_strerr(ret));
    }

    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\rtspTrans.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    /** 数据库模块 */
    init_db();
    get_devs_from_db();
    SAFE_DELETE(client);

	set_uv(NULL);

    //test
    /*
    Log::debug("begin test");
    set_uv(NULL);
    RTSP_REQUEST req = {
        "192.168.241.11",
        554,
        "admin",
        "12345",
        1,
        0,
        8888,
        DEV_HIK
    };

    ret = rtsp_play(req, on_play_cb);
    Log::debug("rtsp play %d", ret);

    Sleep(5000);
    ret = rtsp_stop(req);
    Log::debug("rtsp stop %d", ret);
    Sleep(5000);
    stop_uv();
    Log::error("test end");
    */
    sleep(INFINITE);
    return 0;
}