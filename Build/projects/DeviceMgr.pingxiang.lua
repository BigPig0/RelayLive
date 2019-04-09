function GetDevInfo()
    local ret = {}
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return ret
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "select t.RECORDER_CODE, t.RECORDER_NAME, t.STATUS from ENFORCEMENT_RECORDER t")
    local rs = DBTOOL_GET_RES(stmt)
    while (DBTOOL_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = DBTOOL_GET_STR(rs, 1)
        row["Name"]   = DBTOOL_GET_STR(rs, 2)
        row["Status"] = DBTOOL_GET_INT(rs, 3)
        table.insert(ret, row)
    end
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return ret
end

function UpdateStatus(code, status)
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_PREPARE(stmt, "update ENFORCEMENT_RECORDER set STATUS = :status where RECORDER_CODE = :code")
    DBTOOL_BIND_INT(stmt, ":status", status)
    DBTOOL_BIND_STRING(stmt, ":code", code, 30)
    DBTOOL_EXECUTE(stmt)
    local count = DBTOOL_GET_AFFECT(stmt)
    DBTOOL_COMMIT(stmt)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function UpdatePos(code, lat, lon)
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_PREPARE(stmt, "update ENFORCEMENT_RECORDER set LAT = :lat, LON = :lon where RECORDER_CODE = :code")
    DBTOOL_BIND_STRING(stmt, ":lat", lat, 10)
    DBTOOL_BIND_STRING(stmt, ":lon", lon, 10)
    DBTOOL_BIND_STRING(stmt, ":code", code, 30)
    DBTOOL_EXECUTE(stmt)
    local count = DBTOOL_GET_AFFECT(stmt)
    DBTOOL_COMMIT(stmt)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function InsertDev(dev)
    --DBTOOL_ADD_ROW(rows, dev)
    return true
end

function Init()
    --[[
    DBTOOL_POOL_CONN({tag="DB", dbpath="172.31.7.7/pxzhjt", user="basic", pwd="123", max=5, min=1, inc=2})
    rows = DBTOOL_ROWER_INIT(50, 10)
    ins = DBTOOL_INSERT_INIT("DB", "ENFORCEMENT_RECORDER", 10, {
        {colname = "RECORDER_CODE", coltype = DBTOOL_TYPE_CHR, maxlen = 20},
        {colname = "STATUS", coltype = DBTOOL_TYPE_INT},
        {colname = "LON", coltype = DBTOOL_TYPE_FLT},
        {colname = "LAT", coltype = DBTOOL_TYPE_FLT}
    })
    DBTOOL_ROW_INS(ins, rows)
	]]--
    return true
end

function Cleanup()
    rows = nil
    ins = nil
    return true
end
