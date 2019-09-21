function GetDevInfo()
    devtb = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return devtb
    end
    --部门
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "select t.DEPARTMEN_ID, t.DEPARTMENT_NAME, t.PARENT_ID from VIDEODEPART t")
    local rs = LUDB_GET_RES(stmt)
    while (LUDB_FETCH_NEXT(rs)) do
        local row = {}
        row["DevID"]  = LUDB_GET_STR(rs, 1)
        row["Name"]   = LUDB_GET_STR(rs, 2)
        row["BusinessGroupID"]= LUDB_GET_STR(rs, 3)
        --table.insert(devtb, row)
	    devtb[row["DevID"]] = row
    end
    LUDB_FREE_STMT(stmt)
    --设备
    local stmt2 = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt2, "select t.GUID, t.NAME, t.STATUS, t.LATITUDE, t.LONGITUDE, t.DEPARTMENT from VIDEODEVICE t where t.TYPE = 5")
    local rs2 = LUDB_GET_RES(stmt2)
    while (LUDB_FETCH_NEXT(rs2)) do
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
		row["ParentID"]   = LUDB_GET_STR(rs, 6)
        --table.insert(devtb, row)
	    devtb[row["DevID"]] = row
    end
    LUDB_FREE_STMT(stmt2)
    LUDB_FREE_CONN(con)
    return devtb
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
    local date=os.date("%Y%m%d%H%M%S")
	local sql = string.format("update VIDEODEVICE set STATUS = %d, RESETTIME = '%s' where GUID = '%s'", sta, date, code)
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
	if string.len(lat)>9 then
	    lat = string.sub(lat, 0, 9)
	end
	if string.len(lon)>9 then
	    lon = string.sub(lon, 0, 9)
	end
	local sql = string.format("update VIDEODEVICE set LATITUDE = %s, LONGITUDE = %s where GUID = '%s'", lat, lon, code)
	print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
	
	if devtb[code] ~= nil and devtb[code]["ParentID"] == '32048100002160100008' then
        local row = devtb[code]
        local now = os.date("%Y%m%d%H%M%S",os.time())
        sql = string.format("insert into CRUISER_GPS_HIS (VEHICLENO, GPSTIME, LONGITUDE, LATITUDE) values ('%s', '%s', %s, %s)", row["Name"], now, lon, lat)
        print(sql)
        local stmt = LUDB_CREATE_STMT(con)
        LUDB_CREATE_STMT(stmt, sql)
        LUDB_COMMIT(con)
        LUDB_FREE_STMT(stmt)
	end
	
    LUDB_FREE_CONN(con)
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
		local lat = "0"
		if(dev["Latitude"] ~= nil) then
		    lat = dev["Latitude"]
		end
		local lon = "0"
		if(dev["Longitude"] ~= nil) then
		    lon = dev["Longitude"]
		end
        local row = {dev["DevID"], dev["Name"], dev["DevID"], dev["DevID"], "5", lat, lon, status, date, dp}
        --print("dev:"..dev["DevID"].." name:"..dev["Name"].." Pos:"..lat.." "..lon)
        LUDB_ADD_ROW(devHelp, row)
    else
        --记录插入部门表
		if(dev["BusinessGroupID"] ~= nil) then
			if(dev["ParentID"] ~= nil) then
				dp = dev["ParentID"]
			else
				dp = dev["BusinessGroupID"]
			end
		end
        local row = {dev["DevID"], dev["Name"], dp}
        LUDB_ADD_ROW(departHelp, row)
    end
    return true
end

function DeleteDev()
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "DELETE FROM VIDEODEVICE WHERE TYPE = 5")
    LUDB_CREATE_STMT(stmt, "TRUNCATE TABLE VIDEODEPART")
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="D:/app/Administrator/product/instantclient_11_2"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="10.9.0.10/ETL", user="lyzhjt", pwd="zt123", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into VIDEODEVICE (GUID, NAME, SERVERGUID, OBJID, TYPE, LATITUDE, LONGITUDE, STATUS, UPDATETIME, DEPARTMENT) values (:GUID, :NAME, :SERVERGUID, :OBJID, :TYPE, :LAT, :LON, :STATUS, :UPTIME, :DPART)"
    devHelp = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "GUID",       coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "NAME",       coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "SERVERGUID", coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "OBJID",      coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "TYPE",       coltype = LUDB_TYPE_INT},
		{bindname = "LAT",        coltype = LUDB_TYPE_CHR, 10},
		{bindname = "LON",        coltype = LUDB_TYPE_CHR, 10},
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT},
        {bindname = "UPTIME",     coltype = LUDB_TYPE_CHR, maxlen = 14},
		{bindname = "DPART",      coltype = LUDB_TYPE_CHR, maxlen = 50}
    })
    --部门表插入工具
    local sql2 = "insert into VIDEODEPART (GUID, DEPARTMEN_ID, DEPARTMENT_NAME, PARENT_ID) values (:id, :id, :name, :pid)"
    departHelp = LUDB_BATCH_INIT("oracle", "DB", sql2, 50, 10, {
        {bindname = "id",       coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "name",     coltype = LUDB_TYPE_CHR, maxlen = 64},
        {bindname = "pid",      coltype = LUDB_TYPE_CHR, maxlen = 64},
    })
	--更新操作执行工具
	updateHelp = LUDB_BATCH_INIT("oracle", "DB", "", 50, 10, {})
    return true
end

function Cleanup()
    rows = nil
    ins = nil
	LUDB_CLEAN()
    return true
end

function transformLat(x, y)
    local ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * math.sqrt(math.abs(x))
    ret = (20.0 * math.sin(6.0 * x * math.pi) + 20.0 * math.sin(2.0 * x * math.pi)) * 2.0 / 3.0 + ret
    ret = (20.0 * math.sin(y * math.pi) + 40.0 * math.sin(y / 3.0 * math.pi)) * 2.0 / 3.0 + ret
    ret = (160.0 * math.sin(y / 12.0 * math.pi) + 320.0 * math.sin(y * math.pi / 30.0)) * 2.0 / 3.0 + ret
    return ret
end

function transformLon(x, y)
    local ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * math.sqrt(math.abs(x))
    ret = (20.0 * math.sin(6.0 * x * math.pi) + 20.0 * math.sin(2.0 * x * math.pi)) * 2.0 / 3.0 + ret
    ret = (20.0 * math.sin(x * math.pi) + 40.0 * math.sin(x / 3.0 * math.pi)) * 2.0 / 3.0 + ret
    ret = (150.0 * math.sin(x / 12.0 * math.pi) + 300.0 * math.sin(x / 30.0 * math.pi)) * 2.0 / 3.0 + ret
    return ret
end

a = 6378245.0
ee = 0.006693421622965943

function Gps84_To_Gcj(lat, lon)
    local dLat = transformLat(lon - 105.0, lat - 35.0)
    local dLon = transformLon(lon - 105.0, lat - 35.0)
    local radLat = lat / 180.0 * math.pi
    local magic = math.sin(radLat)
    magic = 1.0 - (ee * magic * magic)
    local sqrtMagic = math.sqrt(magic)
    dLat = dLat * 180.0 / a * (1.0 - ee) / magic * sqrtMagic * math.pi
    dLon = dLon * 180.0 / a / sqrtMagic * math.cos(radLat) * math.pi
    local mgLat = lat + dLat
    local mgLon = lon + dLon

    return mgLat, mgLon
end

function bd_To_Gcj(bd_lat, bd_lon)
    local x = bd_lon - 0.0065
    local y = bd_lat - 0.006
    local z = math.sqrt(x * x + y * y) - (2e-005 * math.sin(y * math.pi))
    local theta = math.atan2(y, x) - (3e-006 * math.cos(x * math.pi))
    local gg_lon = z * math.cos(theta)
    local gg_lat = z * math.sin(theta)
    return gg_lat, gg_lon
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
	if(lat < 1 or lon < 1) then
		return {Latitude = "0", Longitude = "0"}
	end

    --print(lat.."  "..lon)
    local x = 0
    local y = 0
    if(dev["ParentID"] ~= nil and dev["ParentID"] == "32048100002160943362") then
        x, y = Gps84_To_Gcj(tonumber(lat), tonumber(lon))
    else
        x, y = bd_To_Gcj(tonumber(lat), tonumber(lon))
    end
    --print(x.."  "..y)
    
	local ret = {
		Latitude = tostring(x),
		Longitude = tostring(y)
	}
    return ret
end
