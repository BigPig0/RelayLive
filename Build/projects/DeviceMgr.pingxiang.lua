function GetDevInfo()
    local ret = {}
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return ret
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "select t.RECORDER_CODE, t.RECORDER_NAME, t.STATUS, t.LAT, t.LON from ENFORCEMENT_RECORDER t")
    local rs = DBTOOL_GET_RES(stmt)
    while (DBTOOL_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = DBTOOL_GET_STR(rs, 1)
        row["Name"]   = DBTOOL_GET_STR(rs, 2)
        row["Status"] = DBTOOL_GET_INT(rs, 3)
        row["Latitude"] = DBTOOL_GET_STR(rs, 4)
        row["Longitude"]= DBTOOL_GET_STR(rs, 5)
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
	local sta = 0
	if status then 
		sta = 1 
	end
	local sql = string.format("update ENFORCEMENT_RECORDER set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
	print(sql)
    DBTOOL_EXECUTE_STMT(stmt, sql)
	DBTOOL_COMMIT(con)
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
	if string.len(lat)>9 then
	    lat = string.sub(lat, 0, 9)
	end
	if string.len(lon)>9 then
	    lon = string.sub(lon, 0, 9)
	end
	local sql = string.format("update ENFORCEMENT_RECORDER set LAT = %s, LON = %s where RECORDER_CODE = '%s'", lat, lon, code)
	print(sql)
    DBTOOL_EXECUTE_STMT(stmt, sql)
	DBTOOL_COMMIT(con)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function InsertDev(dev)
    return true
end

function Init()
    DBTOOL_POOL_CONN({tag="DB", dbpath="172.31.7.7/pxzhjt", user="basic", pwd="123", max=5, min=1, inc=2})
    return true
end

function Cleanup()
    rows = nil
    ins = nil
    return true
end
