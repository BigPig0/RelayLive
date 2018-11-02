#pragma once
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include "OciDefine.h"

/**
 * 数据记录缓存区，当数据量达到一定数量或间隔一定时间时，通过回调方法批量处理
 */
class LIBOCI_API RowCollector
{
public:
    /**
     * 构造函数
     * @param num_rows 缓存满几行数据进行提交
     * @param interval 间隔几秒时间进行一次提交
     */
	RowCollector(void);
	RowCollector(std::size_t num_rows, std::size_t interval);
	~RowCollector(void);

    /**
     * 设置回调参数
     */
    void set_info(std::size_t num_rows, std::size_t interval);

    /**
     * 插入一行数据
     */
	void add_row(vector<string> row);

    /**
     * 添加回调方法
     */
	void add_insertion_handler(function<void(std::vector<vector<string>>)> handler);
	
private:

    /**
     * 定时扫描任务线程，接收区数据不足但超时也要处理
     * @call move_rows()
     * @call insert()
     */
	void collect();
    /**
     * 从接收区将指定数量的数据移动到待处理区
     */
	int move_rows();

    /**
     * 调用回调方法处理数据
     */
	void insert();

private:
	std::queue<vector<string>>   m_queReciveRows;               //接收到的数据
	std::vector<vector<string>>  m_vecInsertRows;               //待入库的数据
	std::size_t     m_nIntervalSeconds;            //入库间隔，单位为秒
	std::size_t     m_nMaxRows;                    //缓存的最大行数

	std::thread     m_thread;
    std::mutex      m_mtx;           //add_row()和collect()两个线程之间互斥
    bool            m_bRun;

	std::chrono::system_clock::time_point   m_tpInsertTime;     //< 上一次回调入库的时间
	vector<function<void(std::vector<vector<string>>)>>  m_vecCallbackFun;   //< 回调方法
};
