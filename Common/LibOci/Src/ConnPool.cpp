#include "stdafx.h"
#include "ConnPool.h"
#include <chrono>

using namespace std;
using namespace std::chrono;
typedef std::chrono::time_point<std::chrono::system_clock,milliseconds> millisecond_point;

ConnPool::ConnPool(string db, string user, string psw) :
	m_strDatabase(db), 
	m_strUserName(user), 
	m_strPassword(psw), 
	m_nNextConnectID(1), 
	m_nMaxIdleTime(ORA_CONN_MAX_IDLE_TIME),
	m_nMinConnectNum(2), 
	m_nMaxConnectNum(8), 
	m_nIncreaseConnectNum(1), 
	m_nConnectNum(0), 
	m_nBusyConnectNum(0),
    m_bRunning(false)
{
	createPool();
}

ConnPool::ConnPool(string db, string user, string psw, int min, int max, int inc) :
	m_strDatabase(db), 
	m_strUserName(user), 
	m_strPassword(psw), 
	m_nNextConnectID(1), 
	m_nMaxIdleTime(ORA_CONN_MAX_IDLE_TIME),
	m_nMinConnectNum(min), 
	m_nMaxConnectNum(max), 
	m_nIncreaseConnectNum(inc), 
	m_nConnectNum(0), 
	m_nBusyConnectNum(0),
    m_bRunning(false)
{
	createPool();
}

ConnPool::~ConnPool(void)
{
	m_bRunning = false;
    m_threadMaintain.join();
	freePool();
}

void ConnPool::run()
{
    //Log::debug("run");
    m_bRunning = true;
	m_threadMaintain = std::thread(&ConnPool::maintain, this);
    //maintain_thread.detach();
}

void ConnPool::createPool()
{
	if (m_nMinConnectNum > 0)
	{
		createConnection(m_nMinConnectNum);
	}
}

void ConnPool::freePool()
{
	if (m_nBusyConnectNum == 0)
	{
		//关闭所有连接，并释放资源
		std::map<int, DBConnection>::iterator it = m_mapConnects.begin();
		while (it != m_mapConnects.end())
		{
			OCI_ConnectionFree(it->second.cn);
			it++;
		}
		m_mapConnects.clear();
		m_nConnectNum = 0;
	}
}

OCI_Connection* ConnPool::at(int id)
{
	std::lock_guard<std::mutex> lock(m_mtxConnects);
	std::map<int, DBConnection>::iterator it = m_mapConnects.find(id);
	if (it != m_mapConnects.end())
		return it->second.cn;
	return 0;
}

int ConnPool::getConnection(int timeout)
{
	int id = -1;
	bool isTimeout = true;
    millisecond_point tpBegin = time_point_cast<milliseconds>(system_clock::now());
    milliseconds max_wait_time(timeout);
	while (time_point_cast<milliseconds>(system_clock::now())-tpBegin < max_wait_time)
	{
		m_mtxConnects.lock();
		if ((id = findIdle()) != -1)
		{
			if (checkConnection(m_mapConnects[id].cn))
			{
				m_mapConnects[id].cstate = cs_busy;
				m_nBusyConnectNum++;
				m_mtxConnects.unlock();
				isTimeout = false;
				break; //成功获取可用连接
			}
			else
			{
				//该连接已失效，重新获取
				m_mapConnects[id].cstate = cs_inactive;
				m_mtxConnects.unlock();
			}
		}
		else if (m_nConnectNum < m_nMaxConnectNum)
		{
			//总连接数未达上限，新建连接
			int num = (m_nMaxConnectNum - m_nConnectNum) < m_nIncreaseConnectNum ? 
				(m_nMaxConnectNum - m_nConnectNum) : m_nIncreaseConnectNum;
			createConnection(num);
			m_mtxConnects.unlock();
		}
		else
		{
			//等待其他线程释放连接
			m_mtxConnects.unlock();
			this_thread::sleep_for(chrono::milliseconds(100));
		}
	}
	if (isTimeout)
	{
		id = -1;
		Log::warning("timed out while getting connection");
	}
	return id;
}

void ConnPool::releaseConnection(int id)
{
	std::unique_lock<std::mutex> lock(m_mtxConnects);
	std::map<int, DBConnection>::iterator it = m_mapConnects.find(id);
	if (it != m_mapConnects.end())
	{
		if (checkConnection(it->second.cn))
		{
			it->second.cstate = cs_idle;
			it->second.idle_begin_time = system_clock::to_time_t(system_clock::now());  //重置空闲时间
		}
		else
		{
			it->second.cstate = cs_inactive;
		}
		m_nBusyConnectNum--;
	}
}

bool ConnPool::removeConnection(int id)
{
	map<int, DBConnection>::iterator it = m_mapConnects.find(id);
	if (it != m_mapConnects.end())
	{
		DBConnection conn = it->second;
		//只允许移除空闲的连接
		if (conn.cstate != cs_busy)
		{
			try
			{
				OCI_ConnectionFree(conn.cn);
			}
			catch(...)
			{
				OCI_Error* err = OCI_GetLastError();
				Log::error("OCI_ConnectionFree failed");
			}
			m_mapConnects.erase(it);
			m_nConnectNum--;
			return true;
		}
	}
	return false;
}

bool ConnPool::checkConnection(OCI_Connection *cn)
{
	if (OCI_IsConnected(cn) && OCI_Ping(cn))
	{
		return true;
	}
	return false;
}

int ConnPool::createConnection(int num)
{
    //Log::debug("createConnection num:%d",num);
	int num_created = 0;
	for (int i = 0; i < num; i++)
	{
		OCI_Connection *cn = OCI_ConnectionCreate(
			m_strDatabase.c_str(), m_strUserName.c_str(), m_strPassword.c_str(), OCI_SESSION_DEFAULT);
		//新建连接成功
		if (cn)
		{
            //Log::debug("createConnection sucess ");
			DBConnection conn;
			conn.cn = cn;
			conn.cstate = cs_idle;
            conn.idle_begin_time = system_clock::to_time_t(system_clock::now());
			m_mapConnects[m_nNextConnectID++] = conn; //连接编号从1开始
			m_nConnectNum++;
			num_created++;
		}
	}
    //Log::debug("createConnection sucess num:%d",num_created);
	return num_created;
}

int ConnPool::findIdle()
{
	int id = -1;
	for(auto cn : m_mapConnects)
	{
		if (cn.second.cstate == cs_idle)
		{
			id = cn.first;
			break;
		}
	}
    //Log::debug("id:%d",id);
	return id;
}

void ConnPool::maintain()
{
    //Log::debug("maintain");
	const int interval = 10;  //秒
	while (m_bRunning)
	{
		vector<int> invalid_conns;
		m_mtxConnects.lock();
		if (find_invalid_connections(invalid_conns, m_nMaxIdleTime*1000) > 0)
		{
			for(int conn_id : invalid_conns)
			{
				removeConnection(conn_id);
			}
			if (m_nConnectNum < m_nMinConnectNum)
			{
				createConnection(m_nMinConnectNum - m_nConnectNum);
			}
		}
		m_mtxConnects.unlock();
        for(int i=0; i<interval&&m_bRunning; ++i)
		    this_thread::sleep_for(chrono::seconds(1));
	}
}

int ConnPool::find_invalid_connections(vector<int> &invalid_conns, 
									   int max_idle_time)
{
    milliseconds max_idle_duration(max_idle_time);
	for(auto cn : m_mapConnects)
	{
		//超过最大空闲时间
		if (cn.second.cstate == cs_idle 
			&& time_point_cast<milliseconds>(system_clock::now()) - time_point_cast<milliseconds>(system_clock::from_time_t(cn.second.idle_begin_time)) > max_idle_duration)
		{
			invalid_conns.push_back(cn.first);
		}
		//删除无效连接
		if (cn.second.cstate == cs_inactive)
		{
			invalid_conns.push_back(cn.first);
		}
	}
    //Log::debug("find_invalid_connections:%d",invalid_conns.size());
	return invalid_conns.size();
}

int ConnPool::getBusyCount()
{
	return m_nBusyConnectNum;
}

int ConnPool::getOpenedCount()
{
	return m_nConnectNum;
}