#include "stdafx.h"
#include "RowCollector.h"


RowCollector::RowCollector()
    : m_nMaxRows(100)
    , m_nIntervalSeconds(10)
    , m_bRun(true)
    , m_thread(&RowCollector::collect, this)
{
    m_tpInsertTime = std::chrono::system_clock::now();
}

RowCollector::RowCollector(std::size_t num_rows, std::size_t interval) 
    : m_nMaxRows(num_rows)
    , m_nIntervalSeconds(interval)
    , m_bRun(true)
    , m_thread(&RowCollector::collect, this)
{
    m_tpInsertTime = std::chrono::system_clock::now();
}

void RowCollector::set_info(std::size_t num_rows, std::size_t interval)
{
    m_nMaxRows = num_rows;
    m_nIntervalSeconds = interval;
}

RowCollector::~RowCollector(void)
{
    m_bRun = false;
    m_thread.join();
}

void RowCollector::add_row(vector<string> row)
{
    std::unique_lock<std::mutex> lock(m_mtx);
    m_queReciveRows.push(row);
}

// called by collect()
int RowCollector::move_rows()
{
    int count = 0;
    while (!m_queReciveRows.empty() && m_vecInsertRows.size() < m_nMaxRows)
    {
        m_vecInsertRows.push_back(m_queReciveRows.front());
        m_queReciveRows.pop();
        ++count;
    }
    return count;
}

void RowCollector::add_insertion_handler(function<void(std::vector<vector<string>>)> handler)
{
    m_vecCallbackFun.push_back(handler);
}

// called by collect()
void RowCollector::insert()
{
    if (m_vecInsertRows.size() > 0)
    {
        for(auto fuc : m_vecCallbackFun)
        {
            Log::debug("call back func");
            if(!fuc._Empty())
            {
                Log::debug("can call back");
                fuc(m_vecInsertRows);
            }
        }
        m_vecInsertRows.clear();
    }
}

void RowCollector::collect()
{
    Log::debug("RowCollector start running");
    std::chrono::milliseconds interval(m_nIntervalSeconds * 1000LL);
    while (m_bRun)
    {
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            int move_count = move_rows();
            //Log::debug("move %d rows",move_count);
            std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
            if (std::chrono::time_point_cast<std::chrono::milliseconds>(now) 
                - std::chrono::time_point_cast<std::chrono::milliseconds>(m_tpInsertTime)
                > interval)
            {
                //Log::debug("interval to insert");
                insert();
                m_tpInsertTime = now; //重置计时器
            }
            //Log::debug("catch rows:%d/%d",m_vecInsertRows.size(),m_nMaxRows);
            if (m_vecInsertRows.size() >= m_nMaxRows)
            {
                //Log::debug("size to insert");
                insert();
                m_tpInsertTime = now; //重置计时器
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
