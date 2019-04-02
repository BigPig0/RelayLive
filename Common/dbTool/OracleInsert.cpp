#include "stdafx.h"
#include "OracleInsert.h"
#include "dbTool.h"

OracleInsert::OracleInsert()
    : m_buff(nullptr)
    , m_nPointLen(sizeof(void*))
{

}

OracleInsert::OracleInsert(string strDB, string strTable, int nRowSize)
    : m_strDataBase(strDB)
    , m_strTableName(strTable)
    , m_buff(nullptr)
    , m_nPointLen(sizeof(void*))
    , m_nRowSize(nRowSize)
{
}


OracleInsert::~OracleInsert(void)
{
    char* pDataBegin = m_buff + m_vecTableRow.size() * m_nPointLen;
    int32_t nOffsetData = 0;
    for (auto& col:m_vecTableRow)
    {
        if (col.nColumnType == SQLT_BLOB)
        {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            for (int i = 0; i < m_nRowSize; i++)
            {
                OCI_LobFree(blob[i]);
            }
        }
        else if (col.nColumnType == SQLT_ODT)
        {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            for (int i = 0; i < m_nRowSize; i++)
            {
                 OCI_DateFree(da[i]);
            }
        }
        int Length = col.nColumnType==SQLT_CHR?col.nMaxLength+1:col.nMaxLength;
        nOffsetData += Length * m_nRowSize;
    }
    SAFE_DELETE_ARRAY(m_buff);
}

void OracleInsert::Init(string strDB, string strTable, int nRowSize)
{
    m_strDataBase = strDB;
    m_strTableName = strTable;
    m_nRowSize = nRowSize;
}

void OracleInsert::AddCloumn(string strColumnName, 
                               uint32_t nColumnType, 
                               uint32_t nMaxLength /*= 0*/, 
                               bool bNullable /*= false*/, 
                               string strDefault /*= ""*/)
{
    switch (nColumnType)
    {
    case SQLT_CHR:
        if (nMaxLength == 0)
            nMaxLength = 16;
        else
            nMaxLength++;
        break;
    case SQLT_INT:
        nMaxLength = sizeof(int32_t);
        break;
    case SQLT_FLT:
        nMaxLength = sizeof(double);
        break;
    case SQLT_LNG:
        nMaxLength = sizeof(uint64_t);
        break;
    case SQLT_UIN:
        nMaxLength = sizeof(uint32_t);
        break;
    case SQLT_BLOB:
        nMaxLength = m_nPointLen;
        break;
    case SQLT_ODT:
        nMaxLength = m_nPointLen;
        break;
    }

    oci_column col;
    col.strColumnName = strColumnName;
    col.nMaxLength  = nMaxLength;
    col.nColumnType = nColumnType;
    col.bNullable   = bNullable;
    col.strDefault  = strDefault;
    m_vecTableRow.push_back(col);
}

bool OracleInsert::Prepair()
{
    // 生成sql
    stringstream ss;
    ss << "insert into " << m_strTableName << " (";
    int column_num = m_vecTableRow.size();
    for (int i = 0; i < column_num; ++i)
    {
        if(i>0)
            ss << ", ";
        ss << m_vecTableRow[i].strColumnName;
    }
    ss << ") values (";
    for (int i = 0; i < column_num; ++i)
    {
        if(i>0)
            ss << ", ";
        ss << ":" << m_vecTableRow[i].strColumnName;
    }
    ss << ")";
    m_strSql = ss.str();
    Log::debug("make sql:%s", m_strSql.c_str());

    // 列数
    int32_t colNum = m_vecTableRow.size();
    //绑定区域的大小
    int32_t nBindSize = colNum * m_nPointLen;
    //数据区域的大小
    int32_t allSize = 0;
    for (auto& col:m_vecTableRow)
    {
        int Length = col.nColumnType==SQLT_CHR?col.nMaxLength+1:col.nMaxLength;
        allSize += Length * m_nRowSize;
    }

    //申请内存
    m_buff = new char[nBindSize + allSize];
    CHECK_POINT(m_buff);
    memset(m_buff,0,nBindSize + allSize);

    //绑定区域
    char* pBindBegin = m_buff;
    //数据区域
    char* pDataBegin = pBindBegin + nBindSize;

    //将绑定数据地址指向数据区域
    int32_t nOffsetBind = 0, nOffsetData = 0;
    for (auto& col:m_vecTableRow)
    {
        char* tmp = pDataBegin + nOffsetData;
        memcpy_s(pBindBegin+nOffsetBind, m_nPointLen, &tmp, m_nPointLen);

        if (col.nColumnType == SQLT_BLOB)
        {
            OCI_Lob **blob = (OCI_Lob **)(pDataBegin+nOffsetData);
            for (int i = 0; i < m_nRowSize; i++)
            {
                blob[i] = OCI_LobCreate(NULL,OCI_BLOB);
            }
        }
        else if (col.nColumnType == SQLT_ODT)
        {
            OCI_Date **da = (OCI_Date **)(pDataBegin+nOffsetData);
            for (int i = 0; i < m_nRowSize; i++)
            {
                da[i] = OCI_DateCreate(NULL);
            }
        }

        nOffsetBind += m_nPointLen;
        int Length = col.nColumnType==SQLT_CHR?col.nMaxLength+1:col.nMaxLength;
        nOffsetData += Length * m_nRowSize;
    }
    Log::debug("creat data buffer");

    return true;
}

bool OracleInsert::Insert(vector<vector<string>> rows)
{
    if (rows.empty())
        return false;

    int nRowSize = rows.size();
    if (nRowSize > m_nRowSize) nRowSize = m_nRowSize;
    Log::debug("insert %d rows to db", nRowSize);

    // 获取数据库连接
    OCI_Connection *cn = OCI_GET_CONNECT(m_strDataBase);
    if(!cn){
        Log::error("fail to get connection: %s", m_strDataBase.c_str());
        return false;
    }

    OCI_Statement *st = OCI_CreateStatement(cn);


    //准备执行sql
    OCI_SetBindAllocation(st, OCI_BAM_EXTERNAL);
    OCI_Prepare(st, m_strSql.c_str());
    OCI_BindArraySetSize(st, nRowSize);

    //绑定
    int column_num = m_vecTableRow.size();
    int nOffsetBind = 0;
    char* pBindBegin = m_buff;
    for (auto& col:m_vecTableRow)
    {
        switch (col.nColumnType)
        {
        case SQLT_CHR:
            {
                char* pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfStrings(st, col.strColumnName.c_str(), pBuff, col.nMaxLength, 0);
            }
            break;
        case SQLT_INT:
            {
                int32_t* pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfInts(st, col.strColumnName.c_str(), pBuff, 0);
            }
            break;
        case SQLT_FLT:
            {
                double* pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfDoubles(st, col.strColumnName.c_str(), pBuff, 0);
            }
            break;
        case SQLT_LNG:
            {
                big_int* pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfBigInts(st, col.strColumnName.c_str(), pBuff, 0);
            }
            break;
        case SQLT_UIN:
            {
                uint32_t* pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfUnsignedInts(st, col.strColumnName.c_str(), pBuff, 0);
            }
            break;
        case SQLT_ODT:
            {
                OCI_Date** pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfDates(st, col.strColumnName.c_str(),pBuff, 0);
            }
            break;
        case SQLT_BLOB:
            {
                OCI_Lob** pBuff;
                memcpy_s(&pBuff, m_nPointLen, pBindBegin+nOffsetBind, m_nPointLen);
                OCI_BindArrayOfLobs(st, col.strColumnName.c_str(), pBuff, OCI_BLOB, 0);
            }
            break;
        }
        nOffsetBind += m_nPointLen;
    }
    for (int i=0; i<nRowSize; ++i)
    {
        vector<string>& row = rows[i];
        int nLineColumnSize = row.size();
        for (int j=0; j<column_num; ++j)
        {
            string value = j<nLineColumnSize ? row[j] : m_vecTableRow[j].strDefault;
            bool isnull = m_vecTableRow[j].bNullable ? value.empty() : false;

            if(m_vecTableRow[j].nColumnType == SQLT_CHR)
            {
                bind_set_data(OCI_GetBind(st, j+1), i, m_vecTableRow[j].nMaxLength, value, isnull);
            }
            else if (m_vecTableRow[j].nColumnType == SQLT_INT 
                || m_vecTableRow[j].nColumnType == SQLT_UIN)
            {
                int32_t n = value.empty()?0:stoi(value);
                bind_set_data(OCI_GetBind(st, j+1), i, n, isnull);
            }
            else if (m_vecTableRow[j].nColumnType == SQLT_LNG)
            {
                int64_t n = value.empty()?0:_atoi64(value.c_str());
                bind_set_data(OCI_GetBind(st, j+1), i, n, isnull);
            }
            else if (m_vecTableRow[j].nColumnType == SQLT_ODT)
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
        Log::warning("insert %s fail bExcute: %d; bCommit: %d; count: %d" ,m_strTableName.c_str(), bExcute, bCommit, count);
    }
    else
    {
        Log::warning("insert %s sucess bExcute: %d; bCommit: %d; count: %d" ,m_strTableName.c_str(), bExcute, bCommit, count);
    }
    OCI_FreeStatement(st);
    OCI_ConnectionFree(cn);
    return true;
}

void OracleInsert::str_set(char *target, int index, int len, const char *source)
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

void OracleInsert::bind_set_data(OCI_Bind *bind, int index, int len, string val, bool is_null /*= false*/)
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

void OracleInsert::bind_set_data(OCI_Bind *bind, int index, string val, string date_fmt, bool is_null /*= false*/)
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

void OracleInsert::bind_set_data(OCI_Bind *bind, int index, int16_t val, bool is_null /*= false*/)
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

void OracleInsert::bind_set_data(OCI_Bind *bind, int index, int32_t val, bool is_null /*= false*/)
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

void OracleInsert::bind_set_data(OCI_Bind *bind, int index, int64_t val, bool is_null /*= false*/)
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
