function GetDevInfo()
    local ret = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return ret
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "select t.RECORDER_CODE, t.RECORDER_NAME, t.STATUS, t.LAT, t.LON from ENFORCEMENT_RECORDER t")
    local rs = LUDB_GET_RES(stmt)
    while (LUDB_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = LUDB_GET_STR(rs, 1)
        row["Name"]   = LUDB_GET_STR(rs, 2)
        row["Status"] = LUDB_GET_INT(rs, 3)
        row["Latitude"] = LUDB_GET_STR(rs, 4)
        row["Longitude"]= LUDB_GET_STR(rs, 5)
        table.insert(ret, row)
    end
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return ret
end

function UpdateStatus(code, status)
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
	local sta = 0
	if status then 
		sta = 1 
	end
	local sql = string.format("update ENFORCEMENT_RECORDER set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
	print(sql)
    LUDB_CREATE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function UpdatePos(code, lat, lon)
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
	if string.len(lat)>9 then
	    lat = string.sub(lat, 0, 9)
	end
	if string.len(lon)>9 then
	    lon = string.sub(lon, 0, 9)
	end
	local sql = string.format("update ENFORCEMENT_RECORDER set LAT = %s, LON = %s where RECORDER_CODE = '%s'", lat, lon, code)
	print(sql)
    LUDB_CREATE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function InsertDev(dev)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="E:/instantclient_11_2"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="172.31.7.7/pxzhjt", user="basic", pwd="123", max=5, min=1, inc=2})
    return true
end

function Cleanup()
    rows = nil
    ins = nil
	LUDB_CLEAN()
    return true
end
