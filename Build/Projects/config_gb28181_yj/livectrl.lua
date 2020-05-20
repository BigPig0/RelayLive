
--得到redis中key名称中的类型字符
function getDevType(id)
    --print(id)
	if devtype[id] == nil then
	    return ""
	end
    local t = devtype[id]["type"]
	--print(t)
    if(t==0) then
	    return "car"  --移动小车
	elseif t==1 then
		return "sold"  --移动单兵
	elseif t==2 then
	    return "interphone" --移动对讲
	elseif t==3 then
	    return "ar"  --鹰眼
	elseif t==4 then
	    return "monitor" --固定枪机
	end
	return ""
end

--得到数据库中的类型值
function getDevType2(id)
    --print(id)
	if devtype[id] == nil then
	    return ""
	end
    local t = devtype[id]["type"]
	--print(t)
    if(t==0) then
	    return "0"  --移动小车
	elseif t==1 then
		return "1"  --移动单兵
	elseif t==2 then
	    return "2" --移动对讲
	elseif t==3 then
	    return "3"  --鹰眼
	elseif t==4 then
		return "4" --固定枪机
	end
	return ""
end

--将设备状态值转为数据库和redis中的值
function getDevSatus(s)
    if s == "ON" or s == "1" then
	    return "1"
	end
	return "0"
end

--获取设备类型映射关系
function getDevTypeMap()
    devtype = {}
	local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
	local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "select * from GB28181_DEVTYPE")
	local rs = LUDB_GET_RES(stmt)
    while (LUDB_FETCH_NEXT(rs)) do
	    local devinfo = {}
	    devinfo["id"] = LUDB_GET_STR(rs, 1)
		devinfo["type"] = LUDB_GET_INT(rs, 2)
		devinfo["lon"] = LUDB_GET_STR(rs, 3)
		devinfo["lat"] = LUDB_GET_STR(rs, 4)
		devinfo["dep"] = LUDB_GET_STR(rs, 5)
		--print(devinfo["id"], devinfo["type"], devinfo["lon"], devinfo["lat"])
		devtype[devinfo["id"]] = devinfo
	end
	LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
	return true
end

function GetDevInfo()
    devtb = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return devtb
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "select t.RECORDER_CODE, t.NAME, t.STATUS, t.LAT, t.LON from recorder t")
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
        --table.insert(devtb, row)
        devtb[row["DevID"]] = row
    end
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return devtb
end

function UpdateStatus(code, status)
    return true
end

function UpdatePos(code, lat, lon)
    return true
end

function InsertDev(dev)
	--devtb[dev["DevID"]] = dev
	
	--sdk获取的设备插入数据库
	local row = {dev["DevID"], dev["Name"]}
	LUDB_ADD_ROW(gbdev, row)
	
	--从静态库取gps
	local lon = dev["Longitude"]
	local lat = dev["Latitude"]
	if devtype[dev["DevID"]] ~= nil then
	    lon = devtype[dev["DevID"]]["lon"]
		lat = devtype[dev["DevID"]]["lat"]
	end
	
	--redis
	local typedir = getDevType(dev["DevID"])
	if typedir == "" then
	    --不是需要的设备类型
	    return false
	end
	local devname = GBK2UTF8(string.gsub(dev["Name"],' ','_'))
	local depname = "empty"
	--local dep = devtb[dev["ParentID"]]
	--if dep ~= nil then
	--    depname = GBK2UTF8(string.gsub(dep["Name"],' ','_'))
	--end
	if devtype[dev["DevID"]] ~= nil then
	    depname = GBK2UTF8(string.gsub(devtype[dev["DevID"]]["dep"],' ','_'))
	end
	local con = LUDB_CONN({dbtype="redis", dbpath="41.215.241.141:6379", user="9", pwd=""})
	local stmt = LUDB_CREATE_STMT(con)
	local sql = string.format("HMSET \"gps:%s:last:%s\" \"deviceId\" \"%s\" \"deviceName\" \"%s\" \"isOnline\" %s \"deptCode\" \"%s\" \"deptName\" \"%s\"", typedir, dev["DevID"], dev["DevID"], devname, getDevSatus(dev["Status"]), dev["ParentID"], depname)
	if typedir=="ar" or typedir == "monitor" then
	    if lon == "" then lon = "0" end
		if lat == "" then lat = "0" end
		local gps = string.format(" \"lat\" %s \"lon\" %s", lat, lon)
		sql = sql..gps
	end
	if typedir=="ar" then
	    sql = sql.." \"url\" \"http://www.baidu.com\""
	end
	print(sql)
    LUDB_EXECUTE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
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
    LUDB_CREATE_STMT(stmt, "TRUNCATE TABLE gb28181_tmp")
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

--gps历史表删除两天前的数据
function delGpsHis()
	local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "delete from gps_history where GPSTIME < sysdate-2")
    LUDB_FREE_STMT(stmt)
	stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "delete from gb28181_tmp")
    LUDB_FREE_STMT(stmt)
    LUDB_COMMIT(con)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="C:/instantclient_11_2_64"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="41.215.241.143:1521/orcl", user="yj_accident", pwd="123", max=5, min=1, inc=2})
	--gps历史表删除两天前的数据
    delGpsHis()
    --设备信息保存到数据库
    local sql = "insert into GB28181_TMP (DEVICE_ID, DEVICE_NAME) values (:DEVICE_ID, :DEVICE_NAME)"
    gbdev = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "DEVICE_ID",  coltype = LUDB_TYPE_CHR, maxlen = 30},
        {bindname = "DEVICE_NAME",coltype = LUDB_TYPE_CHR, maxlen = 50}
    })
	--读取设备类型映射关系
    getDevTypeMap();
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
