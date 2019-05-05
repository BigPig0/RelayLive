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
        row["BusinessGroupID"]= DBTOOL_GET_STR(rs, 3)
        table.insert(ret, row)
    end
    DBTOOL_FREE_STMT(stmt)
    --设备
    local stmt2 = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt2, "select t.GUID, t.NAME, t.STATUS, t.LATITUDE, t.LONGITUDE, t.DEPARTMENT from VIDEODEVICE t")
    local rs2 = DBTOOL_GET_RES(stmt2)
    while (DBTOOL_FETCH_NEXT(rs2)) do
        local row = {}
        row["DevID"]  = DBTOOL_GET_STR(rs, 1)
        row["Name"]   = DBTOOL_GET_STR(rs, 2)
        local status  = DBTOOL_GET_INT(rs, 3)
		if(status == 1) then
			row["Status"] = "ON";
		else
			row["Status"] = "OFF";
		end
        row["Latitude"]   = DBTOOL_GET_STR(rs, 4)
        row["Longitude"]  = DBTOOL_GET_STR(rs, 5)
		row["ParentID"]   = DBTOOL_GET_STR(rs, 6)
        table.insert(ret, row)
    end
    DBTOOL_FREE_STMT(stmt2)
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
	local sql = string.format("update VIDEODEVICE set STATUS = %d, RESETTIME = '%s' where GUID = '%s'", sta, date, code)
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
	local dp = ""
    if checkTableKey(dev, "Status") then
        --记录插入设备表
		if(dev["ParentID"] ~= nil) then
			dp = dev["ParentID"]
		end
        local date=os.date("%Y%m%d%H%M%S")
		local status = "0"
		if(dev["Status"] == "ON") then
		    status = "1"
		end
        local row = {dev["DevID"], dev["Name"], dev["DevID"], "5", status, date, dp}
        DBTOOL_ADD_ROW(devHelp, row)
    else
        --记录插入部门表
		if(dev["BusinessGroupID"] ~= nil) then
			dp = dev["BusinessGroupID"]
		end
        local row = {dev["DevID"], dev["Name"], dp}
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
    local sql = "insert into VIDEODEVICE (GUID, NAME, SERVERGUID, TYPE, STATUS, UPDATETIME, DEPARTMENT) values (:GUID, :NAME, :SERVERGUID, :TYPE, :STATUS, :UPTIME, :DPART)"
    devHelp = DBTOOL_HELP_INIT("DB", sql, 50, 10, {
        {bindname = "GUID",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "NAME",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "SERVERGUID", coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "TYPE",       coltype = DBTOOL_TYPE_INT},
        {bindname = "STATUS",     coltype = DBTOOL_TYPE_INT},
        {bindname = "UPTIME",     coltype = DBTOOL_TYPE_CHR, maxlen = 14},
		{bindname = "DPART",      coltype = DBTOOL_TYPE_CHR, maxlen = 50}
    })
    --部门表插入工具
    local sql2 = "insert into VIDEODEPART (GUID, DEPARTMEN_ID, DEPARTMENT_NAME, PARENT_ID) values (:id, :id, :name, :pid)"
    departHelp = DBTOOL_HELP_INIT("DB", sql2, 50, 10, {
        {bindname = "id",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "name",     coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "pid",      coltype = DBTOOL_TYPE_CHR, maxlen = 64},
    })
	--更新操作执行工具
	updateHelp = DBTOOL_HELP_INIT("DB", "", 50, 10, {})
    return true
end

function Cleanup()
    rows = nil
    ins = nil
    return true
end
