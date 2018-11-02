#pragma once

////////////////////////////////////////////////////////////////////////////////
namespace Oci
{

inline
char *uuid(char buf[33])
{
    GUID guid;
    if (CoCreateGuid(&guid) != S_OK)
        return NULL;

    sprintf_s(buf, 33, "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x" /*"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"*/
              , guid.Data1, guid.Data2, guid.Data3
              , guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3]
              , guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    return buf;
}

extern OCIEnv *env;

inline
bool init()
{
    // 创建OCI环境句柄，用于多线程环境
    return OCIEnvCreate(&env, OCI_THREADED | OCI_OBJECT, NULL, NULL, NULL, NULL, 0, NULL) == OCI_SUCCESS;
}

inline
void cleanup()
{
    if (env)
        OCIHandleFree(env, OCI_HTYPE_ENV);
    OCITerminate(OCI_THREADED | OCI_OBJECT);
}

inline
bool CheckLastError(OCIError *err, sword status, int *errCode, char errbuf[])
{
    switch (status)
    {
    case OCI_SUCCESS:
        return true;
    case OCI_SUCCESS_WITH_INFO:
    case OCI_ERROR:
    {
        if (errbuf)
            OCIErrorGet(err, 1, NULL, errCode, (OraText *)errbuf, OCI_ERROR_MAXMSG_SIZE2, OCI_HTYPE_ERROR);
        else
            OCIErrorGet(err, 1, NULL, errCode, NULL, 0, OCI_HTYPE_ERROR);
    }
    break;
    case OCI_NO_DATA:
    case OCI_INVALID_HANDLE:
    case OCI_NEED_DATA:
    case OCI_STILL_EXECUTING:
        *errCode = status;
        break;
    }
    return false;
}

struct Define
{
    explicit Define(int num) : buf(NULL), colPos(0)
    {
        columns = (Col *)calloc(num, sizeof(Col));
        if (nullptr == columns)
        {
            Log::error("calloc columns failed");
        }
    }

    ~Define()
    {
        if (columns)
            free(columns);
        if (buf)
            free(buf);
    }

    struct Col
    {
        sb4 maxalen;
        ub2 type;
        sb2 *ind;
        ub2 *rlen, *rcode;
        OCIDefine *handle;
        ub4 offset;
    };

    void setCol(ub2 type, sb4 maxalen = 0)
    {
        Col *col = &columns[colPos++];
        switch (type)
        {
        case SQLT_CHR:
            if (maxalen == 0)
                maxalen = 16;
            break;
        case SQLT_ODT:
            maxalen = sizeof(OCIDate) + 16 * sizeof(oratext);
            break;
        case SQLT_INT:
            maxalen = sizeof(long);
            break;
        case SQLT_FLT:
            maxalen = sizeof(double);
            break;
        case SQLT_UIN:
            maxalen = sizeof(ulong);
            break;
        case SQLT_BLOB:
            maxalen = sizeof(OCILobLocator *);
            break;
        }
        col->maxalen = maxalen;
        col->type = type;
    }

    void createRow(int num)
    {
        rowNum = num;
        rowSize = 0;
        for (int i = 0; i <  colPos; i++)
        {
            Col *col = &columns[i];
            col->offset = rowSize;
            rowSize += col->maxalen + sizeof(sb2) + sizeof(ub2) + sizeof(ub2);
        }
        buf = (char *)malloc(rowSize * rowNum);
        if (nullptr == buf)
        {
            Log::error("malloc buf failed");
        }
    }

    Col *columns;
    int colPos, rowSize, rowNum;
    char *buf;
};

struct Bind
{
    explicit Bind(int colNum) : buf(NULL), colPos(0)
    {
        columns = (Col *)calloc(colNum, sizeof(Col));
        if (nullptr == columns)
        {
            Log::error("calloc columns failed");
        }
    }

    ~Bind()
    {
        if (columns)
            free(columns);
        if (buf)
            free(buf);
    }

    struct Col
    {
        char name[32];
        sb4 maxalen;
        ub2 type;
        sb2 *ind;
        ub2 *alen, *rcode;
        OCIBind *handle;
        ub4 offset;
    };

    void setCol(ub2 type, sb4 maxalen = 0)
    {
        Col *col = &columns[colPos++];
        switch (type)
        {
        case SQLT_CHR:
            if (maxalen == 0)
                maxalen = 16;
            break;
        case SQLT_ODT:
            maxalen = sizeof(OCIDate);
            break;
        case SQLT_INT:
            maxalen = sizeof(long);
            break;
        case SQLT_FLT:
            maxalen = sizeof(double);
            break;
        case SQLT_UIN:
            maxalen = sizeof(ulong);
            break;
        case SQLT_BLOB:
            maxalen = sizeof(OCILobLocator *);
            break;
        }
        col->maxalen = maxalen;
        col->type = type;
    }

    void setCol(const char *name, ub2 type, sb4 maxalen = 0)
    {
        byNameToByPos.insert(make_pair(name, colPos));
        Col *col = &columns[colPos++];
        strcpy_s(col->name, 32, name);
        switch (type)
        {
        case SQLT_CHR:
            if (maxalen == 0)
                maxalen = 16;
            break;
        case SQLT_ODT:
            maxalen = sizeof(OCIDate);
            break;
        case SQLT_INT:
            maxalen = sizeof(long);
            break;
        case SQLT_FLT:
            maxalen = sizeof(double);
            break;
        case SQLT_UIN:
            maxalen = sizeof(ulong);
            break;
        case SQLT_BLOB:
            maxalen = sizeof(OCILobLocator *);
            break;
        }
        col->maxalen = maxalen;
        col->type = type;
    }

    void createRow(int num)
    {
        rowNum = num;
        rowSize = 0;
        for (int i = 0; i < colPos; i++)
        {
            Col *col = &columns[i];
            col->offset = rowSize;
            rowSize += col->maxalen + sizeof(sb2) + sizeof(ub2) + sizeof(ub2);
        }
        buf = (char *)malloc(rowSize * rowNum);
        if (nullptr == buf)
        {
            Log::error("malloc buf failed");
        }
    }

    Col *columns;
    int colPos, rowSize, rowNum;
    HashMap<string, int> byNameToByPos;
    char *buf;
};

}

////////////////////////////////////////////////////////////////////////////////
class OciConnection;
class OciSession;
class OciStmt;
class OciResultSet;

class OciConnection : public LastError
{
public:
    explicit OciConnection(const char *ipAndInstance);
    ~OciConnection();

    void createSession(int num);
    OciSession *newSession(const char *user, const char *password);

private:
    OCIError *m_err;
    OCIServer *m_srv;
    OciSession **m_sess;
    int m_sessPos;
};

class OciSession : public LastError
{
public:
    void createStmt(int num);
    OciStmt *prepare(const char *sql, int dmlNum, bool autoCommit = true);

private:
    explicit OciSession(OCIError *err, OCISvcCtx *svc, OCISession *sess);
    ~OciSession();

private:
    OCIError *m_err;
    OCISvcCtx *m_svc;
    OCISession *m_usr;
    OciStmt **m_stmt;
    int m_stmtPos;
    friend class OciConnection;
};

class OciStmt : public LastError
{
public:
    const char *errbuf();
    const char *sql();
    bool define(Oci::Define *def);
    bool bind(Oci::Bind *bind);

    // isnull = -1
    // normal = 0
    void setStr(int col, const char *v, bool isnull = false);
    void setDate(int col, const char *v, const char *fmt = "YYYYMMDDHH24MISS", bool isnull = false);
    void setInt(int col, long v, bool isnull = false);
    void setUInt(int col, unsigned long v, bool isnull = false);
    void setDouble(int col, double v, bool isnull = false);
    bool writeBlob(int row, int col, ulong64 *offset, void *buf, ulong64 *len);
    ulong64 getBindBlobSize(int row, int col);

    void setStr(const char *name, const char *v, bool isnull = false);
    void setDate(const char *name, const char *v, const char *fmt = "YYYYMMDDHH24MISS", bool isnull = false);
    void setInt(const char *name, long v, bool isnull = false);
    void setUInt(const char *name, unsigned long v, bool isnull = false);
    void setDouble(const char *name, double v, bool isnull = false);
    bool writeBlob(int row, const char *name, ulong64 *offset, void *buf, ulong64 *len);
    ulong64 getBindBlobSize(int row, const char *name);

    OciResultSet *query();
    void closeResultSet(OciResultSet *rs);
    int affectedRowCount();
    int m_dmlPos;
    bool update();

    bool commit();
    bool rollback();

private:
    explicit OciStmt(OCIError *err, OCISvcCtx *svc, OCIStmt *stmt, int dmlNum, bool autoCommit);
    ~OciStmt();

private:
    OCIError *m_err;
    OCISvcCtx *m_svc;
    OCIStmt *m_stmt;
    int m_dmlNum;
    bool m_autoCommit;

    Oci::Define *m_def;
    Oci::Bind *m_bind;
    char m_errbuf[OCI_ERROR_MAXMSG_SIZE2];
    friend class OciSession;
};

class OciResultSet : public LastError
{
public:
    bool next();

    const char *getStr(int col);
    const char *getDate(int col, const char *fmt = "YYYYMMDDHH24MISS");
    long getInt(int col);
    ulong getUInt(int col);
    double getDouble(int col);
    bool readBlob(int col, ulong64 *offset, void *buf, ulong64 *len);
    ulong64 getDefineBlobSize(int col);

    bool isNull(int col);
    ub2 actualLen(int col);
    ub2 returnCode(int col);

private:
    explicit OciResultSet(OCIError *err
                          , OCISvcCtx *svc
                          , OCIStmt *stmt
                          , OciStmt *stmtClass
                          , int maxRow
                          , bool finish
                          , Oci::Define *def);
private:
    OCIError *m_err;
    OCISvcCtx *m_svc;
    OCIStmt *m_stmt;
    OciStmt *m_stmtClass;
    int m_maxRow, m_row, m_fetchedRow;
    bool m_finish;

    Oci::Define *m_def;
    friend class OciStmt;
};

////////////////////////////////////////////////////////////////////////////////
inline
OciConnection::OciConnection(const char *ipAndInstance)
    : m_err(NULL)
    , m_srv(NULL)
    , m_sess(NULL)
    , m_sessPos(0)
{
    OCIHandleAlloc(Oci::env, (void **)&m_err, OCI_HTYPE_ERROR, 0, NULL);
    OCIHandleAlloc(Oci::env, (void **)&m_srv, OCI_HTYPE_SERVER, 0, NULL);

    sword status = OCIServerAttach(m_srv, m_err, (const OraText *)ipAndInstance, (int)strlen(ipAndInstance), OCI_DEFAULT);
    if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
        goto failed;
failed:
    ;
}

inline
OciConnection::~OciConnection()
{
    if (m_sess)
    {
        for (int i = 0; i < m_sessPos; i++)
        {
            if (m_sess[i])
                delete m_sess[i];
        }
        free(m_sess);
    }
    if (m_srv && m_err)
        OCIServerDetach(m_srv, m_err, OCI_DEFAULT);
    if (m_srv)
        OCIHandleFree(m_srv, OCI_HTYPE_SERVER);
    if (m_err)
        OCIHandleFree(m_err, OCI_HTYPE_ERROR);
}

inline
void OciConnection::createSession(int num)
{
    m_sess = (OciSession **)calloc(num, sizeof(OciSession *));
    if (nullptr == m_sess)
    {
        Log::error("calloc m_sess failed");
    }
}

inline
OciSession *OciConnection::newSession(const char *user, const char *password)
{
    OCISvcCtx *svc;
    OCISession *sess;
    OCIHandleAlloc(Oci::env, (void **)&svc, OCI_HTYPE_SVCCTX, 0, NULL);
    OCIAttrSet(svc, OCI_HTYPE_SVCCTX, m_srv, 0, OCI_ATTR_SERVER, m_err);
    OCIHandleAlloc(Oci::env, (void **)&sess, OCI_HTYPE_SESSION, 0, NULL);

    OCIAttrSet(sess, OCI_HTYPE_SESSION, (char *)user, (int)strlen(user), OCI_ATTR_USERNAME, m_err);
    OCIAttrSet(sess, OCI_HTYPE_SESSION, (char *)password, (int)strlen(password), OCI_ATTR_PASSWORD, m_err);

    sword status = OCISessionBegin(svc, m_err, sess, OCI_CRED_RDBMS, OCI_DEFAULT);
    if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
        goto failed;
    OCIAttrSet(svc, OCI_HTYPE_SVCCTX, sess, 0, OCI_ATTR_SESSION, m_err);
    return m_sess[m_sessPos++] = new OciSession(m_err, svc, sess);
failed:
    OCIHandleFree(svc, OCI_HTYPE_SVCCTX);
    OCIHandleFree(sess, OCI_HTYPE_SESSION);
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
inline
void OciSession::createStmt(int num)
{
    m_stmt = (OciStmt **)calloc(num, sizeof(OciStmt *));
    if (nullptr == m_stmt)
    {
        Log::error("calloc m_sess failed");
    }
}

inline
OciStmt *OciSession::prepare(const char *sql, int dmlNum, bool autoCommit)
{
    OCIStmt *stmt;
    OCIHandleAlloc(Oci::env, (void **)&stmt, OCI_HTYPE_STMT, 0, NULL);

    sword status = OCIStmtPrepare(stmt, m_err, (const OraText *)sql, (int)strlen(sql), OCI_NTV_SYNTAX, OCI_DEFAULT);
    if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
        goto failed;

    ub2 stmtType;
    OCIAttrGet(stmt, OCI_HTYPE_STMT, &stmtType, NULL, OCI_ATTR_STMT_TYPE, m_err);
    if (stmtType == OCI_STMT_SELECT)
        OCIAttrSet(stmt, OCI_HTYPE_STMT, &dmlNum, sizeof(dmlNum), OCI_ATTR_PREFETCH_ROWS, m_err);
    return m_stmt[m_stmtPos++] = new OciStmt(m_err, m_svc, stmt, dmlNum, autoCommit);
failed:
    OCIHandleFree(stmt, OCI_HTYPE_STMT);
    return NULL;
}

inline
OciSession::OciSession(OCIError *err, OCISvcCtx *svc, OCISession *sess)
    : m_err(err)
    , m_svc(svc)
    , m_usr(sess)
    , m_stmt(NULL)
    , m_stmtPos(0)
{
}

inline
OciSession::~OciSession()
{
    if (m_stmt)
    {
        for (int i = 0; i < m_stmtPos; i++)
        {
            if (m_stmt[i])
                delete m_stmt[i];
        }
        free(m_stmt);
    }
    if (m_svc && m_err && m_usr)
        OCISessionEnd(m_svc, m_err, m_usr, OCI_DEFAULT);
    if (m_svc)
        OCIHandleFree(m_svc, OCI_HTYPE_SVCCTX);
    if (m_usr)
        OCIHandleFree(m_usr, OCI_HTYPE_SESSION);
}

////////////////////////////////////////////////////////////////////////////////
inline
const char *OciStmt::errbuf()
{
    return m_errbuf;
}

inline
const char *OciStmt::sql()
{
    text *sql;
    ub4 len;
    OCIAttrGet(m_stmt, OCI_HTYPE_STMT, &sql, &len, OCI_ATTR_STATEMENT, m_err);
    return (const char *)sql;
}

inline
bool OciStmt::define(Oci::Define *def)
{
    m_def = def;

    ub4 pos = 1;
    sword status;
    char *p;
    sb4 maxalen;
    for (int i = 0; i < m_def->colPos; i++)
    {
        Oci::Define::Col *col = &m_def->columns[i];
        p = m_def->buf + col->offset;
        maxalen = col->maxalen;
        if (col->type == SQLT_BLOB)
        {
            maxalen = 0;
            for (int j = 0; j < m_def->rowNum; j++)
            {
                OCILobLocator *lobp;
                OCIDescriptorAlloc(Oci::env, (void **)&lobp, OCI_DTYPE_LOB, 0, NULL);
                OCILobLocator **lobpp = (OCILobLocator **)(p + m_def->rowSize * j);
                *lobpp = lobp;
            }
        }
        col->handle = NULL;
        status = OCIDefineByPos(m_stmt, &col->handle, m_err, pos++,
                                p, maxalen, col->type, p + col->maxalen,
                                (ub2 *)(p + col->maxalen + sizeof(sb2)),
                                (ub2 *)(p + col->maxalen + sizeof(sb2) + sizeof(ub2)), OCI_DEFAULT);
        if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
            return false;
        OCIDefineArrayOfStruct(col->handle, m_err, m_def->rowSize,
                               m_def->rowSize, m_def->rowSize, m_def->rowSize);
    }
    return true;
}

inline
bool OciStmt::bind(Oci::Bind *bind)
{
    m_bind = bind;

    ub4 pos = 1;
    sword status;
    char *p;
    sb4 maxalen;
    for (int i = 0; i < m_bind->colPos; i++)
    {
        Oci::Bind::Col *col = &m_bind->columns[i];
        p = m_bind->buf + col->offset;
        maxalen = col->maxalen;
        if (col->type == SQLT_BLOB)
        {
            maxalen = 0;
            for (int j = 0; j < m_bind->rowNum; j++)
            {
                OCILobLocator *lobp;
                OCIDescriptorAlloc(Oci::env, (void **)&lobp, OCI_DTYPE_LOB, 0, NULL);
                OCILobLocator **lobpp = (OCILobLocator **)(p + m_bind->rowSize * j);
                *lobpp = lobp;
            }
        }
        col->handle = NULL;
        if (strlen(col->name) == 0)
        {
            status = OCIBindByPos(m_stmt, &col->handle, m_err, pos++,
                                  p, maxalen, col->type, p + col->maxalen,
                                  (ub2 *)(p + col->maxalen + sizeof(sb2)),
                                  (ub2 *)(p + col->maxalen + sizeof(sb2) + sizeof(ub2)),
                                  0, NULL, OCI_DEFAULT);
        }
        else
        {
            status = OCIBindByName(m_stmt, &col->handle, m_err, (const OraText *)col->name, -1,
                                   p, maxalen, col->type, p + col->maxalen,
                                   (ub2 *)(p + col->maxalen + sizeof(sb2)),
                                   (ub2 *)(p + col->maxalen + sizeof(sb2) + sizeof(ub2)),
                                   0, NULL, OCI_DEFAULT);
        }
        if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
            return false;
        OCIBindArrayOfStruct(col->handle, m_err, m_bind->rowSize,
                             m_bind->rowSize, m_bind->rowSize, m_bind->rowSize);
    }
    return true;
}

inline
void OciStmt::setStr(int col, const char *v, bool isnull)
{
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        strcpy_s(p, maxalen, v);
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = (ub2)strlen(v);
    }
}

inline
void OciStmt::setDate(int col, const char *v, const char *fmt, bool isnull)
{
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        OCIDateFromText(m_err, (const oratext *)v, (ub4)strlen(v),
                        (const oratext *)fmt, (ub1)strlen(fmt),
                        NULL, 0, (OCIDate *)p);
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = (ub2)strlen(v);
    }
}

inline
void OciStmt::setInt(int col, long v, bool isnull)
{
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(long *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(long);
    }
}

inline
void OciStmt::setUInt(int col, ulong v, bool isnull)
{
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(ulong *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(ulong);
    }
}

inline
void OciStmt::setDouble(int col, double v, bool isnull)
{
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(double *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(double);
    }
}

inline
bool OciStmt::writeBlob(int row, int col, ulong64 *offset, void *buf, ulong64 *len)
{
    int rowSize = m_bind->rowSize;
    ub4 colOffset = m_bind->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_bind->buf[row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;
    sword status = OCILobWrite2(m_svc, m_err, lobp, len, NULL,
                                *offset, buf, *len, OCI_ONE_PIECE, NULL, NULL, 0, 0);
    if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
        return false;
    *offset += *len;
    return true;
}

inline
ulong64 OciStmt::getBindBlobSize(int row, int col)
{
    int rowSize = m_bind->rowSize;
    ub4 colOffset = m_bind->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_bind->buf[row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;
    oraub8 rlen;
    OCILobGetLength2(m_svc, m_err, lobp, &rlen);
    return rlen;
}

inline
void OciStmt::setStr(const char *name, const char *v, bool isnull)
{
    int col = m_bind->byNameToByPos.at(name);
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        strcpy_s(p, maxalen, v);
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = (ub2)strlen(v);
    }
}

inline
void OciStmt::setDate(const char *name, const char *v, const char *fmt, bool isnull)
{
    int col = m_bind->byNameToByPos.at(name);
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        OCIDateFromText(m_err, (const oratext *)v, (ub4)strlen(v),
                        (const oratext *)fmt, (ub1)strlen(fmt),
                        NULL, 0UL, (OCIDate *)p);
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = (ub2)strlen(v);
    }
}

inline
void OciStmt::setInt(const char *name, long v, bool isnull)
{
    int col = m_bind->byNameToByPos.at(name);
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(long *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(long);
    }
}

inline
void OciStmt::setUInt(const char *name, ulong v, bool isnull)
{
    int col = m_bind->byNameToByPos.at(name);
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(ulong *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(ulong);
    }
}

inline
void OciStmt::setDouble(const char *name, double v, bool isnull)
{
    int col = m_bind->byNameToByPos.at(name);
    sb4 maxalen = m_bind->columns[col].maxalen;
    int rowSize = m_bind->rowSize;
    ub4 offset = m_bind->columns[col].offset;
    char *p = &m_bind->buf[m_dmlPos * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    *indp = isnull ? -1 : 0;
    if (!isnull)
    {
        *(double *)p = v;
        ub2 *alenp = (ub2 *)(p + maxalen + sizeof(sb2));
        *alenp = sizeof(double);
    }
}

inline
bool OciStmt::writeBlob(int row, const char *name, ulong64 *offset, void *buf, ulong64 *len)
{
    int col = m_bind->byNameToByPos.at(name);
    int rowSize = m_bind->rowSize;
    ub4 colOffset = m_bind->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_bind->buf[row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;

    sword status = OCILobWrite2(m_svc, m_err, lobp, len, NULL,
                                *offset, buf, *len, OCI_ONE_PIECE, NULL, NULL, 0, 0);
    if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
        return false;
    *offset += *len;
    return true;
}

inline
ulong64 OciStmt::getBindBlobSize(int row, const char *name)
{
    int col = m_bind->byNameToByPos.at(name);
    int rowSize = m_bind->rowSize;
    ub4 colOffset = m_bind->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_bind->buf[row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;
    oraub8 rlen;
    OCILobGetLength2(m_svc, m_err, lobp, &rlen);
    return rlen;
}

inline
OciResultSet *OciStmt::query()
{
    sword status = OCIStmtExecute(m_svc, m_stmt, m_err, m_dmlNum, 0, NULL, NULL, OCI_DEFAULT);
    if (status == OCI_NO_DATA)
        return new OciResultSet(m_err, m_svc, m_stmt, this, m_dmlNum, true, m_def);
    else if (!Oci::CheckLastError(m_err, status, &m_errCode, m_errbuf))
        return NULL;
    return new OciResultSet(m_err, m_svc, m_stmt, this, m_dmlNum, false, m_def);
}

inline
void OciStmt::closeResultSet(OciResultSet *rs)
{
    delete rs;
}

inline
int OciStmt::affectedRowCount()
{
    int rows;
    OCIAttrGet(m_stmt, OCI_HTYPE_STMT, &rows, (ub4 *)sizeof(rows), OCI_ATTR_ROW_COUNT, m_err);
    return rows;
}

inline
bool OciStmt::update()
{
    if (++m_dmlPos < m_dmlNum)
        return true;
    sword status = OCIStmtExecute(m_svc, m_stmt, m_err, m_dmlNum, 0, NULL, NULL,
                                  (m_autoCommit ? OCI_COMMIT_ON_SUCCESS : 0));
    if (!Oci::CheckLastError(m_err, status, &m_errCode, m_errbuf))
        return false;
    m_dmlPos = 0;
    return true;
}

inline
bool OciStmt::commit()
{
    sword status = OCITransCommit(m_svc, m_err, OCI_DEFAULT);
    return Oci::CheckLastError(m_err, status, &m_errCode, NULL);
}

inline
bool OciStmt::rollback()
{
    sword status = OCITransRollback(m_svc, m_err, OCI_DEFAULT);
    return Oci::CheckLastError(m_err, status, &m_errCode, NULL);
}

inline
OciStmt::OciStmt(OCIError *err, OCISvcCtx *svc, OCIStmt *stmt, int dmlNum, bool autoCommit)
    : m_err(err)
    , m_svc(svc)
    , m_stmt(stmt)
    , m_dmlNum(dmlNum)
    , m_dmlPos(0)
    , m_autoCommit(autoCommit)
    , m_def(NULL)
    , m_bind(NULL)
{
}

inline
OciStmt::~OciStmt()
{
    char *p;
    if (m_def)
    {
        for (int i = 0; i < m_def->colPos; i++)
        {
            Oci::Define::Col *col = &m_def->columns[i];
            p = m_def->buf + col->offset;
            if (col->type == SQLT_BLOB)
            {
                for (int j = 0; j < m_def->rowNum; j++)
                {
                    OCILobLocator **lobpp = (OCILobLocator **)(p + m_def->rowSize * j);
                    OCIDescriptorFree(*lobpp, OCI_DTYPE_LOB);
                }
            }
        }
    }
    if (m_bind)
    {
        for (int i = 0; i < m_bind->colPos; i++)
        {
            Oci::Bind::Col *col = &m_bind->columns[i];
            p = m_bind->buf + col->offset;
            if (col->type == SQLT_BLOB)
            {
                for (int j = 0; j < m_bind->rowNum; j++)
                {
                    OCILobLocator **lobpp = (OCILobLocator **)(p + m_bind->rowSize * j);
                    OCIDescriptorFree(*lobpp, OCI_DTYPE_LOB);
                }
            }
        }
    }
    OCIHandleFree(m_stmt, OCI_HTYPE_STMT);
}

////////////////////////////////////////////////////////////////////////////////
inline
bool OciResultSet::next()
{
    if (m_row + 1 >= m_maxRow)
    {
        if (m_finish)
            return false;
        m_row = -1;

        sword status = OCIStmtFetch2(m_stmt, m_err, m_maxRow, OCI_FETCH_NEXT, 1, OCI_DEFAULT);
        if (status == OCI_NO_DATA)
        {
            m_finish = true;
            m_maxRow = m_stmtClass->affectedRowCount() - m_fetchedRow;
            if (m_maxRow == 0)
                return false;
        }
        else if (!Oci::CheckLastError(m_err, status, &m_errCode, NULL))
            return false;
        m_fetchedRow += m_maxRow;
    }
    m_row++;
    return true;
}

inline
const char *OciResultSet::getStr(int col)
{
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    char *p = &m_def->buf[m_row * rowSize + offset];
    p[actualLen(col)] = '\0';
    return p;
}

inline
const char *OciResultSet::getDate(int col, const char *fmt)
{
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    OCIDate *p = (OCIDate *)&m_def->buf[m_row * rowSize + offset];
    oratext *date = (oratext *)((char *)p + sizeof(OCIDate));
    ub4 rlen = 16;
    OCIDateToText(m_err, p, (const oratext *)fmt,
                  16, NULL, 0, &rlen, date);
    return (const char *)date;
}

inline
long OciResultSet::getInt(int col)
{
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    long *p = (long *)&m_def->buf[m_row * rowSize + offset];
    return *p;
}

inline
ulong OciResultSet::getUInt(int col)
{
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    ulong *p = (ulong *)&m_def->buf[m_row * rowSize + offset];
    return *p;
}

inline
double OciResultSet::getDouble(int col)
{
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    double *p = (double *)&m_def->buf[m_row * rowSize + offset];
    return *p;
}

inline
bool OciResultSet::readBlob(int col, ulong64 *offset, void *buf, ulong64 *len)
{
    int rowSize = m_def->rowSize;
    ub4 colOffset = m_def->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_def->buf[m_row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;

    sword status = OCILobRead2(m_svc, m_err, lobp, len, NULL,
                               *offset, buf, *len, OCI_ONE_PIECE, NULL, NULL, 0, 0);
    if (status == OCI_SUCCESS || status == OCI_NEED_DATA)
    {
        *offset += *len;
        return true;
    }
    Oci::CheckLastError(m_err, status, &m_errCode, NULL);
    return false;
}

inline
ulong64 OciResultSet::getDefineBlobSize(int col)
{
    int rowSize = m_def->rowSize;
    ub4 colOffset = m_def->columns[col].offset;
    OCILobLocator **lobpp = (OCILobLocator **)&m_def->buf[m_row * rowSize + colOffset];
    OCILobLocator *lobp = *lobpp;
    oraub8 rlen;
    OCILobGetLength2(m_svc, m_err, lobp, &rlen);
    return rlen;
}

inline
bool OciResultSet::isNull(int col)
{
    sb4 maxalen = m_def->columns[col].maxalen;
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    char *p = &m_def->buf[m_row * rowSize + offset];
    sb2 *indp = (sb2 *)(p + maxalen);
    return *indp == -1 ? true : false;
}

inline
ub2 OciResultSet::actualLen(int col)
{
    sb4 maxalen = m_def->columns[col].maxalen;
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    char *p = &m_def->buf[m_row * rowSize + offset];
    ub2 *rlen = (ub2 *)(p + maxalen + sizeof(sb2));
    return *rlen;
}

inline
ub2 OciResultSet::returnCode(int col)
{
    sb4 maxalen = m_def->columns[col].maxalen;
    int rowSize = m_def->rowSize;
    ub4 offset = m_def->columns[col].offset;
    char *p = &m_def->buf[m_row * rowSize + offset];
    ub2 *rc = (ub2 *)(p + maxalen + sizeof(sb2) + sizeof(ub2));
    return *rc;
}

inline
OciResultSet::OciResultSet(OCIError *err
                           , OCISvcCtx *svc
                           , OCIStmt *stmt
                           , OciStmt *stmtClass
                           , int maxRow
                           , bool finish
                           , Oci::Define *def)
    : m_err(err), m_svc(svc), m_stmt(stmt), m_stmtClass(stmtClass)
    , m_row(-1), m_finish(finish), m_def(def)
{
    if (m_finish)
        m_maxRow = m_stmtClass->affectedRowCount();
    else
        m_maxRow = maxRow;
    m_fetchedRow = m_maxRow;
}
