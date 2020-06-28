function GetDevInfo()
    devtb = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return devtb
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "select t.CODE, t.NAME, t.STATUS, t.LAT, t.LON, t.PARENT from GB28181_DEV t")
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
		row["ParentID"]   = LUDB_GET_STR(rs, 6)
        devtb[row["DevID"]] = row
    end
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return devtb
end

function UpdateStatus(code, status)
    if devtb[code] ~= nil then
	    if devtb[code]["Status"] == status then
		    return true;
		end
		devtb[code]["Status"] = status
	end
	
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local sta = 0
    if status then 
        sta = 1 
    end
    local sql = string.format("update GB28181_DEV set STATUS = %d where CODE = '%s'", sta, code)
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
    if devtb[code] ~= nil then
	    if devtb[code]["Latitude"] == lat and devtb[code]["Longitude"] == lon then
		    return true;
		end
		devtb[code]["Latitude"] = lat
		devtb[code]["Longitude"] = lon
	end
	
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
    local sql = string.format("update GB28181_DEV set LAT = %s, LON = %s where CODE = '%s'", lat, lon, code)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, sql)
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function InsertDev(dev)
	local code = ""
	if dev["DevID"] ~= nil then
        code = dev["DevID"]
    end
	local name = ""
	if dev["Name"] ~= nil then
	    name = dev["Name"]
	end
	local lat = ""
	if dev["Latitude"] ~= nil then
	    lat = dev["Latitude"]
	end
	local lon = ""
	if dev["Longitude"] ~= nil then
	    lon = dev["Longitude"]
	end
    local status = "0"
    if(dev["Status"] == "ON") then
        status = "1"
    end
	local parent = ""
	if dev["ParentID"] ~= nil then
	    parent = dev["ParentID"]
	end
    local row = {code, name, lat, lon, status, parent}
    LUDB_ADD_ROW(devHelp, row)
    return true
end

function HourEvent(hour)
    if hour ~= 1 then
        return true
    end
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_CREATE_STMT(stmt, "TRUNCATE TABLE GB28181_DEV")
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="C:/instantclient_11_2"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="32.81.129.13/orcl", user="basic", pwd="123", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into GB28181_DEV(CODE, NAME, LAT, LON, STATUS, PARENT) values (:CODE, :NAME, :LAT, :LON, :STATUS, :PARENT)"
    devHelp = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "CODE",       coltype = LUDB_TYPE_CHR, maxlen = 30},
        {bindname = "NAME",       coltype = LUDB_TYPE_CHR, maxlen = 100},
        {bindname = "LAT",        coltype = LUDB_TYPE_CHR, maxlen = 10},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, maxlen = 10},
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT},
        {bindname = "PARENT",     coltype = LUDB_TYPE_CHR, maxlen = 30}
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
    local lat = ""
    if(dev["Latitude"] ~= nil) then
        lat = dev["Latitude"]
    end
    local lon = ""
    if(dev["Longitude"] ~= nil) then
        lon = dev["Longitude"]
    end
	local ret = {
		Latitude = lat,
		Longitude = lon
	}
    return ret
end
