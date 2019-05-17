#include "stdafx.h"
#include "dbHelp.h"
#include "dbTool.h"

namespace dbTool
{
    int pointLen = sizeof(void*); //< 指针的长度，32位平台4字节，64位平台8字节

    struct bindInfo
    {
        string      bindName;    //< 绑定标记名称
        uint32_t    columnType;  //< 列类型
        uint32_t    maxLen;      //< 最大长度，类型是字符串时必须设置
        bool        nullable;    //< 是否可为空
        string      default;     //< 默认值，偶尔需要
    };

    struct helpHandle
    {
        string                  tag;      //< 连接标记
        string                  sql;      //< 执行插入的sql语句
        int                     rowNum;   //< 一次提交的数据量
        int                     interval; //< 入库间隔，单位为秒
        vector<bindInfo>        binds;    //< 绑定的标记信息
        char*                   buff;     //< 内存区域

        queue<vector<string>>   recvQueue;     //接收到的数据
        vector<vector<string>>  instRows;      //待入库的数据
        std::mutex              mtx;           //上面两个变量线程安全锁

        std::thread             thrdRun;
        bool                    run;

        std::chrono::system_clock::time_point   instTime;     //< 上一次回调入库的时间
    };

    static void str_set(char *target, int index, int len, const char *source)
    {
        int offset = index * (len + 1);
        char *p = target + offset;
        const char *q = source;
        memset(p, 0, len + 1);
        int count = 0;
        while (q && *q != '\0' && count < len)
        {
            *p++ = *q++;
            ++count;
        }
    }

    static void bind_set_data(OCI_Bind *bind, int index, int len, string val, bool is_null /*= false*/)
    {
        if(!is_null)
        {
            char *str = (char *)OCI_BindGetData(bind);
            str_set(str, index, len, val.c_str());
        }
        else
        {
            OCI_BindSetNullAtPos(bind, index + 1);
        }
    }

    static void bind_set_data(OCI_Bind *bind, int index, string val, string date_fmt, bool is_null /*= false*/)
    {
        if(!is_null)
        {
            OCI_Date **dates = (OCI_Date **)OCI_BindGetData(bind);
            OCI_DateFromText(dates[index], val.c_str(), date_fmt.c_str());
        }
        else
        {
            OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
        }
    }

    static void bind_set_data(OCI_Bind *bind, int index, int16_t val, bool is_null /*= false*/)
    {
        if(!is_null)
        {
            int16_t *numbers = (int16_t *)OCI_BindGetData(bind);
            numbers[index] = val;
        }
        else
        {
            OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
        }
    }

    static void bind_set_data(OCI_Bind *bind, int index, int32_t val, bool is_null /*= false*/)
    {
        if(!is_null)
        {
            int32_t *numbers = (int32_t *)OCI_BindGetData(bind);
            numbers[index] = val;
        }
        else
        {
            OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
        }
    }

    static void bind_set_data(OCI_Bind *bind, int index, int64_t val, bool is_null /*= false*/)
    {
        if(!is_null)
        {
            int64_t *numbers = (int64_t *)OCI_BindGetData(bind);
            numbers[index] = val;
        }
        else
        {
            OCI_BindSetNullAtPos(bind, index + 1); //下标从1开始
        }
    }

    /** 从接收队列中将数据移到插入队列，到插入队列插满为止 */
    static int move_rows(helpHandle* h)
    {
        int count = 0;
        while (!h->recvQueue.empty() && h->instRows.size() < (size_t)h->rowNum)
        {
            h->instRows.push_back(h->recvQueue.front());
            h->recvQueue.pop();
            ++count;
        }
        return count;
    }

    /** 将插入队列中的数据插入数据库 */
    static bool insert(helpHandle* h)
    {
        if(h->instRows.empty())
            return true;
        
        int nRowSize = h->instRows.size();
        if (nRowSize > h->rowNum) nRowSize = h->rowNum; //正常不会走到这一步
        Log::debug("insert %d rows to db", nRowSize);

        // 获取数据库连接
        OCI_Connection *cn = DBTOOL_GET_CONNECT(h->tag);
        if(!cn){
            Log::error("fail to get connection: %s", h->tag.c_str());
            return false;
        }
        OCI_Statement *st = OCI_CreateStatement(cn);

        if(h->binds.empty()){
            for (int i=0; i<nRowSize; ++i)
            {
                for (auto sql:h->instRows[i])
                {
                    OCI_ExecuteStmt(st, sql.c_str());
                }
            }
            OCI_Commit(cn);
        } else {
            //准备执行sql
            OCI_SetBindAllocation(st, OCI_BAM_EXTERNAL);
            OCI_Prepare(st, h->sql.c_str());
            OCI_BindArraySetSize(st, nRowSize);

            //绑定
            int column_num = h->binds.size();
            int nOffsetBind = 0;
            char* pBindBegin = h->buff;
            for (auto& col:h->binds)
            {
                switch (col.columnType)
                {
                case SQLT_CHR:
                    {
                        char* pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfStrings(st, col.bindName.c_str(), pBuff, col.maxLen, 0);
                    }
                    break;
                case SQLT_INT:
                    {
                        int32_t* pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfInts(st, col.bindName.c_str(), pBuff, 0);
                    }
                    break;
                case SQLT_FLT:
                    {
                        double* pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfDoubles(st, col.bindName.c_str(), pBuff, 0);
                    }
                    break;
                case SQLT_LNG:
                    {
                        big_int* pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfBigInts(st, col.bindName.c_str(), pBuff, 0);
                    }
                    break;
                case SQLT_UIN:
                    {
                        uint32_t* pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfUnsignedInts(st, col.bindName.c_str(), pBuff, 0);
                    }
                    break;
                case SQLT_ODT:
                    {
                        OCI_Date** pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfDates(st, col.bindName.c_str(),pBuff, 0);
                    }
                    break;
                case SQLT_BLOB:
                    {
                        OCI_Lob** pBuff;
                        memcpy_s(&pBuff, pointLen, pBindBegin+nOffsetBind, pointLen);
                        OCI_BindArrayOfLobs(st, col.bindName.c_str(), pBuff, OCI_BLOB, 0);
                    }
                    break;
                }
                nOffsetBind += pointLen;
            }
            for (int i=0; i<nRowSize; ++i)
            {
                vector<string>& row = h->instRows[i];
                int nLineColumnSize = row.size();
                for (int j=0; j<column_num; ++j)
                {
                    string value = j<nLineColumnSize ? row[j] : h->binds[j].default;
                    bool isnull = h->binds[j].nullable ? value.empty() : false;

                    if(h->binds[j].columnType == SQLT_CHR)
                    {
                        bind_set_data(OCI_GetBind(st, j+1), i, h->binds[j].maxLen, value, isnull);
                    }
                    else if (h->binds[j].columnType == SQLT_INT 
                        || h->binds[j].columnType == SQLT_UIN)
                    {
                        int32_t n = value.empty()?0:stoi(value);
                        bind_set_data(OCI_GetBind(st, j+1), i, n, isnull);
                    }
                    else if (h->binds[j].columnType == SQLT_LNG)
                    {
                        int64_t n = value.empty()?0:_atoi64(value.c_str());
                        bind_set_data(OCI_GetBind(st, j+1), i, n, isnull);
                    }
                    else if (h->binds[j].columnType == SQLT_ODT)
                    {
                        bind_set_data(OCI_GetBind(st, j+1), i, value, "yyyymmddhh24miss", isnull);
                    }
                }
            }

            boolean bExcute = OCI_Execute(st);
            boolean bCommit = OCI_Commit(cn);
            unsigned int count = OCI_GetAffectedRows(st);    //某一行插入失败，不会回滚所有数据，但是出错后count为0，这是ocilib的一个bug
            if (!bExcute || !bCommit || 0 == count)
            {
                Log::warning("Execute %s fail bExcute: %d; bCommit: %d; count: %d" ,h->tag.c_str(), bExcute, bCommit, count);
            }
            else
            {
                Log::warning("Execute %s sucess bExcute: %d; bCommit: %d; count: %d" ,h->tag.c_str(), bExcute, bCommit, count);
            }
        }
        OCI_FreeStatement(st);
        OCI_ConnectionFree(cn);

        h->instRows.clear();
        return true;
    }

    static void collect(helpHandle* h)
    {
        Log::debug("dbHelp collect start running");
        std::chrono::milliseconds interval(h->interval * 1000LL);
        while (h->run) {
            {
                std::unique_lock<std::mutex> lock(h->mtx);
                int move_count = move_rows(h);
                //Log::debug("move %d rows",move_count);
                std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                if (std::chrono::time_point_cast<std::chrono::milliseconds>(now) 
                    - std::chrono::time_point_cast<std::chrono::milliseconds>(h->instTime)
                    > interval) {
                    //Log::debug("interval to insert");
                    insert(h);
                    h->instTime = now; //重置计时器
                }
                //Log::debug("catch rows:%d/%d",m_vecInsertRows.size(),m_nMaxRows);
                if (h->instRows.size() >= (size_t)h->rowNum) {
                    //Log::debug("size to insert");
                    insert(h);
                    h->instTime = now; //重置计时器
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    helpHandle* CreateHelp(string tag, string sql, int rowNum, int interval, bindParam* binds){
        helpHandle* ret = new helpHandle;
        ret->tag        = tag;
		ret->sql        = sql;
        ret->rowNum     = rowNum;
        ret->interval   = interval;
        ret->run        = true;
        ret->instTime   = std::chrono::system_clock::now();
        ret->thrdRun    = thread(collect, ret);

        // 设置绑定的参数
        for (int i = 0; binds[i].bindName != NULL; i++)
        {
            int maxlen = binds[i].maxLen;
            switch (binds[i].columnType)
            {
            case SQLT_CHR:
                if (maxlen == 0)
                    maxlen = 16;
                else
                    maxlen++;
                break;
            case SQLT_INT:
                maxlen = sizeof(int32_t);
                break;
            case SQLT_FLT:
                maxlen = sizeof(double);
                break;
            case SQLT_LNG:
                maxlen = sizeof(uint64_t);
                break;
            case SQLT_UIN:
                maxlen = sizeof(uint32_t);
                break;
            case SQLT_BLOB:
            case SQLT_ODT:
                maxlen = pointLen;
                break;
            }
            bindInfo info = {
                binds[i].bindName,  //绑定名称
                binds[i].columnType,//列类型
                maxlen,             //列最大长度
                binds[i].nullable,  //列允许为空
                binds[i].default?binds[i].default:""    //默认值
            };
            ret->binds.push_back(info);
        }

        // 列数
        int32_t colNum = ret->binds.size();
        //绑定区域的大小
        int32_t nBindSize = colNum * pointLen;
        //数据区域的大小
        int32_t allSize = 0;
        for (auto& col:ret->binds)
        {
            int Length = col.columnType==SQLT_CHR?col.maxLen+1:col.maxLen;
            allSize += Length * ret->rowNum;
        }

        //申请内存
        ret->buff = new char[nBindSize + allSize];
        CHECK_POINT_NULLPTR(ret->buff);
        memset(ret->buff,0,nBindSize + allSize);

        //绑定区域
        char* pBindBegin = ret->buff;
        //数据区域
        char* pDataBegin = pBindBegin + nBindSize;

        //将绑定数据地址指向数据区域
        int32_t nOffsetBind = 0, nOffsetData = 0;
        for (auto& col:ret->binds)
        {
            char* tmp = pDataBegin + nOffsetData;
            memcpy_s(pBindBegin+nOffsetBind, pointLen, &tmp, pointLen);

            if (col.columnType == SQLT_BLOB)
            {
                OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
                for (int i = 0; i < ret->rowNum; i++)
                {
                    blob[i] = OCI_LobCreate(NULL,OCI_BLOB);
                }
            }
            else if (col.columnType == SQLT_ODT)
            {
                OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
                for (int i = 0; i < ret->rowNum; i++)
                {
                    da[i] = OCI_DateCreate(NULL);
                }
            }

            nOffsetBind += pointLen;
            int Length = col.columnType==SQLT_CHR?col.maxLen+1:col.maxLen;
            nOffsetData += Length * ret->rowNum;
        }

        return ret;
    }

    void FreeHelp(helpHandle* h, bool clean){
        h->run = false;
        h->thrdRun.join();

        char* pDataBegin = h->buff + h->binds.size() * sizeof(void*);
        int32_t nOffsetData = 0;
        for (auto& col:h->binds)
        {
            if (col.columnType == SQLT_BLOB)
            {
                OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
                for (int i = 0; i < h->rowNum; i++)
                {
                    OCI_LobFree(blob[i]);
                }
            }
            else if (col.columnType == SQLT_ODT)
            {
                OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
                for (int i = 0; i < h->rowNum; i++)
                {
                    OCI_DateFree(da[i]);
                }
            }
            int Length = col.columnType==SQLT_CHR?col.maxLen+1:col.maxLen;
            nOffsetData += Length * h->rowNum;
        }
        SAFE_DELETE_ARRAY(h->buff);

        delete h;
    }

    void AddRow(helpHandle* h, vector<string> row)
    {
        std::unique_lock<std::mutex> lock(h->mtx);
        h->recvQueue.push(row);
    }

}