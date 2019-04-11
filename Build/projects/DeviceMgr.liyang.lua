function GetDevInfo()
    local ret = {}
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return ret
    end
    --部门
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "select t.DEPARTMEN_ID, t.DEPARTMENT_NAME, t.PARENT_ID from VIDEODEPART t")
    local rs = DBTOOL_GET_RES(stmt)
    while (DBTOOL_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = DBTOOL_GET_STR(rs, 1)
        row["Name"]   = DBTOOL_GET_STR(rs, 2)
        row["ParentID"]= DBTOOL_GET_STR(rs, 3)
        table.insert(ret, row)
    end
    DBTOOL_FREE_STMT(stmt)
    --设备
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "select t.GUID, t.NAME, t.STATUS, t.LATITUDE, t.LONGITUDE from VIDEODEVICE t")
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
    local date=os.date("%Y%m%d%H%M%S")
	local sql = string.format("update VIDEODEVICE set STATUS = %d, RESETTIME = '%s' where GUID = '%s'", sta, code, date)
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
	local sql = string.format("update VIDEODEVICE set LATITUDE = %s, LONGITUDE = %s where GUID = '%s'", lat, lon, code)
	print(sql)
    DBTOOL_EXECUTE_STMT(stmt, sql)
	DBTOOL_COMMIT(con)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function checkTableKey(tbl, key)
    for k,v in pairs(tbl) do
        if k == key then
            return true
        end
    end
    return false
end

function InsertDev(dev)
    if checkTableKey(dev, "Status") then
        --记录插入设备表
        local date=os.date("%Y%m%d%H%M%S")
        local row = {dev["DevID"], dev["Name"], dev["DevID"], dev["PTZType"], dev["Status"], date}
        DBTOOL_ADD_ROW(devHelp, row)
    else
        --记录插入部门表
        local row = {dev["DevID"], dev["Name"], dev["ParentID"]}
        DBTOOL_ADD_ROW(departHelp, row)
    end
    return true
end

function DeleteDev()
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "TRUNCATE TABLE VIDEODEVICE")
    DBTOOL_EXECUTE_STMT(stmt, "TRUNCATE TABLE VIDEODEPART")
	DBTOOL_COMMIT(con)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function Init()
    DBTOOL_POOL_CONN({tag="DB", dbpath="10.9.0.7/ETL", user="lyzhjt", pwd="zt123", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into VIDEODEVICE (GUID, NAME, SERVERGUID, TYPE, STATUS, UPDATETIME) values (:GUID, :NAME, :SERVERGUID, :TYPE, :STATUS, :UPTIME)"
    devHelp = luaHelpInit("DB", sql, 50, 10, {
        {bindname = "GUID",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "NAME",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "SERVERGUID", coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "TYPE",       coltype = DBTOOL_TYPE_INT},
        {bindname = "STATUS",     coltype = DBTOOL_TYPE_INT},
        {bindname = "UPTIME",     coltype = DBTOOL_TYPE_CHR, maxlen = 14}
    })
    --部门表插入工具
    local sql2 = "insert into VIDEODEPART (GUID, DEPARTMEN_ID, DEPARTMENT_NAME, PARENT_ID) values (:id, :id, :name, :pid)"
    departHelp = luaHelpInit("DB", sql2, 50, 10, {
        {bindname = "id",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "name",     coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "pid",      coltype = DBTOOL_TYPE_CHR, maxlen = 64},
    })
    return true
end

function Cleanup()
    rows = nil
    ins = nil
    return true
end
