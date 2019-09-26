--永嘉对接海康8600脚本
--大写字母开头的函数是导出给执行程序调用，小写字母开头的函数是脚本内部使用

--得到redis中key名称中的类型字符
function getDevType(id)
    --print(id)
    local t = devtype[id]
	--print(t)
    if(t==0) then
	    return "car"  --移动小车
	elseif t==1 then
		return "sold"  --移动单兵
	elseif t==2 then
	    return "interphone" --移动对讲
	elseif t==3 then
	    return "ar"  --球机
	elseif t==4 then
	    return "monitor" --固定枪机
	end
	return ""
end

--得到数据库中的类型值
function getDevType2(id)
    --print(id)
    local t = devtype[id]
	--print(t)
    if(t==0) then
	    return "0"  --移动小车
	elseif t==1 then
		return "1"  --移动单兵
	elseif t==2 then
	    return "2" --移动对讲
	elseif t==3 then
	    return "3"  --球机
	elseif t==4 then
		return "4" --固定枪机
	end
	return ""
end

--将设备状态值转为数据库和redis中的值
function getDevSatus(s)
    if s == "ON" then
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
	    local id = LUDB_GET_STR(rs, 1)
		local type = LUDB_GET_INT(rs, 2)
		--print("devid: ",id," devtype: ",type)
		devtype[id] = type
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
    LUDB_EXECUTE_STMT(stmt, "select t.device_id, t.device_name, t.STATUS, t.LAT, t.LON from GB28181_Hik t")
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
        devtb[row["DevID"]] = row
    end
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return devtb
end

function UpdateStatus(code, status)
    local dev = devtb[code];
	if dev == nil then
	    return false
	end
	local sta = 0
	if(status) then
		row["Status"] = "ON";
		sta = 1 
	else
		row["Status"] = "OFF";
	end
	--国标设备实时表更新
	row = {sta, code}
	LUDB_ADD_ROW(gbstate, row)
	--redis
	local typedir = getDevType(dev["DevID"])
	if typedir == "" then
	    return false
	end
    local con = LUDB_CONN({dbtype="redis", dbpath="192.168.2.110:6379", user="0", pwd=""})
    if (type(con)=="nil") then
        return false
    end
	local sql = string.format("HSET \"gps:%s:last:%s\" \"isOnline\" %d", typedir, code, sta)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function UpdatePos(code, lat, lon)
    local dev = devtb[code]
	if dev == nil then
	    return false
	end
	if getDevType(dev["DevID"]) == "" then
	    --不是需要的设备类型
	    return false
	end
	dev["Latitude"] = lat;
	dev["Longitude"] = lon;
	--gps历史表插入
    local row = {dev["DevID"], dev["Latitude"], dev["Longitude"], os.date("%Y%m%d%H%M%S"), getDevType2(dev["DevID"]), os.date("%Y%m%d%H")}
    LUDB_ADD_ROW(gpshis, row)
	--国标设备实时表更新
	row = {dev["Latitude"], dev["Longitude"], dev["DevID"]}
	LUDB_ADD_ROW(gbgps, row)
	--redis
	local typedir = getDevType(dev["DevID"])
	if typedir == "" then
	    return false
	end
	local con = LUDB_CONN({dbtype="redis", dbpath="192.168.2.110:6379", user="0", pwd=""})
	local stmt = LUDB_CREATE_STMT(con)
	local sql = string.format("HMSET \"gps:%s:last:%s\" \"lat\" %s \"lon\" %s", typedir, code, lat, lon)
	print(sql)
    LUDB_EXECUTE_STMT(stmt, sql)
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
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
	devtb[dev["DevID"]] = dev
	--国标设备插入
	local row = {dev["DevID"], dev["Name"], dev["Latitude"], dev["Longitude"], getDevType2(dev["DevID"]), getDevSatus(dev["Status"])}
    LUDB_ADD_ROW(gbdev, row)
	--redis
	local typedir = getDevType(dev["DevID"])
	if typedir == "" then
	    --不是需要的设备类型
	    return false
	end
	local dep = devtb[dev["ParentID"]]
	if dep == nil then
	    return false
	end
	local con = LUDB_CONN({dbtype="redis", dbpath="192.168.2.110:6379", user="0", pwd=""})
	local stmt = LUDB_CREATE_STMT(con)
	local sql = string.format("HMSET \"gps:%s:last:%s\" \"deviceId\" \"%s\" \"deviceName\" \"%s\" \"lat\" %s \"lon\" %s \"isOnline\" %s \"deptCode\" \"%s\" \"deptName\" \"%s\"", typedir, dev["DevID"], dev["DevID"], dev["Name"], dev["Latitude"], dev["Longitude"], getDevSatus(dev["Status"]), dev["ParentID"], dep["Name"])
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

function DeleteDev()
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return false
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "TRUNCATE TABLE recorder")
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
	LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="F:\\Downloads\\Orical\\instantclient_11_2"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="192.168.2.110/jwt", user="yj_accident", pwd="123", max=5, min=1, inc=2})
	--gps历史表删除两天前的数据
	delGpsHis()
    --GPS历史表插入
    local sql = "insert into gps_history (DEVICE_ID, LAT, LON, GPSTIME, TYPE, GPS_FORMAT_TIME) values (:DEVICE_ID, :LAT, :LON, :GPSTIME, :TYPE, :GPS_FORMAT_TIME)"
    gpshis = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "DEVICE_ID",  coltype = LUDB_TYPE_CHR, maxlen = 30},
        {bindname = "LAT",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "GPSTIME",    coltype = LUDB_TYPE_ODT},
        {bindname = "TYPE",       coltype = LUDB_TYPE_INT},
        {bindname = "GPS_FORMAT_TIME", coltype = LUDB_TYPE_INT}
    })
	--国标设备表插入
	sql = "insert into GB28181_HIK (DEVICE_ID, DEVICE_NAME, LAT, LON, TYPE, STATUS) values (:DEVICE_ID, :DEVICE_NAME, :LAT, :LON, :TYPE, :STATUS)"
    gbdev = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "DEVICE_ID",  coltype = LUDB_TYPE_CHR, maxlen = 30},
        {bindname = "DEVICE_NAME",coltype = LUDB_TYPE_CHR, maxlen = 30},
        {bindname = "LAT",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "TYPE",       coltype = LUDB_TYPE_INT},
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT}
    })
	--国标设备表更新GPS
	sql = "update GB28181_HIK set LAT=:LAT, LON=:LON where DEVICE_ID = :DEVICE_ID"
	gbgps = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
		{bindname = "LAT",        coltype = LUDB_TYPE_CHR, maxlen = 20},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, maxlen = 20},
		{bindname = "DEVICE_ID",  coltype = LUDB_TYPE_CHR, maxlen = 30},
	})
	--国标设备表更新状态
	sql = "update GB28181_HIK set STATUS=:STATUS where DEVICE_ID = :DEVICE_ID"
	gbstate = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT},
		{bindname = "DEVICE_ID",  coltype = LUDB_TYPE_CHR, maxlen = 30},
	})
	
	--读取设备类型映射关系
	getDevTypeMap();
    return true
end

function Cleanup()
    rows = nil
    ins = nil
	LUDB_CLEAN("oracle")
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
