function GetDevInfo()
    local ret = {}
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return ret
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "select t.RECORDER_CODE, t.NAME, t.STATUS, t.LAT, t.LON from recorder t")
    local rs = DBTOOL_GET_RES(stmt)
    while (DBTOOL_FETCH_NEXT(rs)) do
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
	local sta = 0
	if status then 
		sta = 1 
    end
	local sql = string.format("update recorder set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
    local sql2 = string.format("update enforcement_recorder set STATUS = %d where RECORDER_CODE = '%s'", sta, code)
    print(sql)
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, sql)
    DBTOOL_EXECUTE_STMT(stmt, sql2)
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
	local sql = string.format("update recorder set LAT = %s, LON = %s where RECORDER_CODE = '%s'", lat, lon, code)
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
		local status = "0"
		if(dev["Status"] == "ON") then
		    status = "1"
		end
        local row = {dev["DevID"], dev["Name"], dev["Latitude"], dev["Longitude"], status}
        DBTOOL_ADD_ROW(devHelp, row)
    return true
end

function DeleteDev()
    local con = DBTOOL_GET_CONN("DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = DBTOOL_CREATE_STMT(con)
    DBTOOL_EXECUTE_STMT(stmt, "TRUNCATE TABLE recorder")
	DBTOOL_COMMIT(con)
    DBTOOL_FREE_STMT(stmt)
    DBTOOL_FREE_CONN(con)
    return true
end

function Init()
    DBTOOL_POOL_CONN({tag="DB", dbpath="32.81.129.13/orcl", user="basic", pwd="123", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into recorder (RECORDER_CODE, NAME, LAT, LON, STATUS) values (:CODE, :NAME, :LAT, :LON, :STATUS)"
    devHelp = DBTOOL_HELP_INIT("DB", sql, 50, 10, {
        {bindname = "CODE",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "NAME",       coltype = DBTOOL_TYPE_CHR, maxlen = 64},
        {bindname = "LAT",        coltype = DBTOOL_TYPE_CHR, maxlen = 20},
        {bindname = "LON",        coltype = DBTOOL_TYPE_CHR, maxlen = 20},
        {bindname = "STATUS",     coltype = DBTOOL_TYPE_INT}
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
		Latitude = tostring(x),
		Longitude = tostring(y)
	}
    return ret
end
