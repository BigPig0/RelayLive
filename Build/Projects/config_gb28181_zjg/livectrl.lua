function GetDevInfo()
    local ret = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return ret
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "select t.RECORDER_CODE, t.NAME, t.STATUS, t.LAT, t.LON from recorder t")
    local rs = LUDB_GET_RES(stmt)
    while (LUDB_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = LUDB_GET_STR(rs, 1)
        row["Name"]   = LUDB_GET_STR(rs, 2)
        local status  = LUDB_GET_INT(rs, 3)
        if(status == 1) then
            row["Status"] = "ON";
        else
            row["Status"] = "OFF";
        end
        row["Latitude"]   = LUDB_GET_STR(rs, 4)
        row["Longitude"]  = LUDB_GET_STR(rs, 5)
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
    local sta = 0
    if status then 
        sta = 1 
    end
    local sql = string.format("update recorder set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
    local sql2 = string.format("update enforcement_recorder set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, sql)
    LUDB_EXECUTE_STMT(stmt, sql2)
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
    if string.len(lat)>9 then
        lat = string.sub(lat, 0, 9)
    end
    if string.len(lon)>9 then
        lon = string.sub(lon, 0, 9)
    end
    local sql = string.format("update recorder set LAT = %s, LON = %s where RECORDER_CODE = '%s'", lat, lon, code)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, sql)
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function InsertDev(dev)
    local status = "0"
    if(dev["Status"] == "ON") then
        status = "1"
    end
    local row = {dev["DevID"], dev["Name"], dev["Latitude"], dev["Longitude"], status}
    LUDB_ADD_ROW(devHelp, row)
    return true
end

function DeleteDev(hour)
    if hour ~= 1 then
        return true
    end
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "TRUNCATE TABLE recorder")
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="C:/instantclient_11_2_64"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="32.81.129.13/orcl", user="basic", pwd="123", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into recorder (RECORDER_CODE, NAME, LAT, LON, STATUS) values (:CODE, :NAME, :LAT, :LON, :STATUS)"
    devHelp = LUDB_BATCH_INIT("DB", sql, 50, 10, {
        {bindname = "CODE",       coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "NAME",       coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "LAT",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT}
    })
    return true
end

function Cleanup()
    rows = nil
    ins = nil
	LUDB_CLEAN()
    return true
end

function TransDevPos(dev)
    local lat = 0
    if(dev["Latitude"] ~= nil) then
        lat = tonumber(dev["Latitude"])
    end
    local lon = 0
    if(dev["Longitude"] ~= nil) then
        lon = tonumber(dev["Longitude"])
    end
	local ret = {
		Latitude = tostring(lat),
		Longitude = tostring(lon)
	}
    return ret
end
