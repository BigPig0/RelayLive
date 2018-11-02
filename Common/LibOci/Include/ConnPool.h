#pragma once
#include "ocilib.h"
#include "OciDefine.h"

/** 连接最大空闲时间，如果连接超出这个时间一直处于空闲状态，就将其移除 */
#define ORA_CONN_MAX_IDLE_TIME 60 * 10

/** 连接状态 */
enum cn_state
{
	cs_idle = 0,     // 空闲
	cs_busy = 1,     // 工作
	cs_inactive = 2  // 失效
};

/** 连接结构体 */
struct DBConnection
{
	OCI_Connection *cn;      //oci连接实例
	cn_state cstate;         //连接状态
	time_t idle_begin_time;  //累计空闲时间
};

/**
 * 一个会话地址的连接池
 */
class LIBOCI_API ConnPool
{
public:
    /**
     * 构造函数
     * @param db 数据库地址
     * @param user 登录用户名
     * @param psw 登录密码
     * @param min 最小连接数
     * @param max 最大连接数
     * @param inc 每次最多创建的连接数
     */
	ConnPool(std::string db, std::string user, std::string psw);
	ConnPool(std::string db, std::string user, std::string psw, int min, int max, int inc);
	~ConnPool(void);

	void run();

    /**
     * 根据ID获取连接实例对象
     * @param id 空闲连接的id
     * @return 连接实例
     */
	OCI_Connection *at(int id);

    /**
     * 从池子中获取有个空闲连接的id
     * @param timeout 执行的超时时间，单位毫秒。默认超时时间为2秒
     * @return 空闲连接的id，如果没获取到则为-1
     * @remark 成功获取后，池子中这个连接的状态将会从idle变成busy
     */
	int getConnection(int timeout=2000);

    /**
     * 将一个连接还回到池子中
     * @param id 需要归还的连接的id
     * @remark 归还后，池子中这个连接的状态将会从busy变成idle
     */
	void releaseConnection(int id);
	
    /**
     * 获取正在工作状态的连接的个数
     */
	int getBusyCount();

    /**
     * 获取当前总的连接个数
     */
	int getOpenedCount(); 
	
private:
    /**
     * 初始化时创建连接
     */
	void createPool();
    /**
     * 释放所有连接
     */
	void freePool();
    /**
     * 创建连接
     * @param num 连接个数
     * @return 成功传感的连接个数
     */
	int createConnection(int num);

    /**
     * 移除指定的连接，必须是空闲状态
     * @param id 指定连接id
     * @return 成功true，失败false
     */
	bool removeConnection(int id);  

    /**
     * 检测连接是否有效
     */
	bool checkConnection(OCI_Connection *cn);

    /**
     * 返回空闲且有效的连接ID
     */
	int findIdle();

    /**
     * 定时进行线程池维护，删除失效和超时连接
     */
	void maintain();

    /**
     * 检查所有失效和超时的连接
     * @param invalid_conns[out] 输出失效和超时连接的ID
     * @param max_idle_time[in] 超时时间，单位是毫秒
     * @return 无效或超时的连接的个数
     */
	int find_invalid_connections(std::vector<int>& invalid_conns, int max_idle_time);

private:
	std::string                 m_strDatabase;              //< 数据库地址
	std::string                 m_strUserName;              //< 登录用户名
	std::string                 m_strPassword;              //< 登录密码
	int                         m_nMinConnectNum;           //< 最小连接数
	int                         m_nMaxConnectNum;           //< 最大连接数
	int                         m_nIncreaseConnectNum;      //< 每次创建的连接数

	std::map<int, DBConnection> m_mapConnects;              //< 数据库连接池
	std::mutex                  m_mtxConnects;              //< 数据库连接池的锁
	int                         m_nNextConnectID;           //< 新建连接所采用的id
	int                         m_nMaxIdleTime;             //< 允许的空闲时间，单位为秒，超过则移除
	
	int                         m_nConnectNum;              //< 池中的总连接数，可能包含非活跃的连接
	int                         m_nBusyConnectNum;          //< 池中当前正在工作的连接数

	std::thread                 m_threadMaintain;           //< 连接池的维护线程
    bool                        m_bRunning;                 //< 线程在运行
};

typedef std::shared_ptr<ConnPool> conn_pool_ptr;

