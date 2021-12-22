function GetDevInfo()
    devtb = {}
    local con = LUDB_POOL_CONN("oracle", "DB")
    if (type(con)=="nil") then
        return devtb
    end
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, "select t.dev_guid, t.dev_name, t.STATUS, t.LATITUDE, t.LONGITUDE from GB28181_DEVICE t where t.dev_source = 0")
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
    local date=os.date("%Y%m%d%H%M%S")
    local sql = string.format("update GB28181_DEVICE set STATUS = %d, updatetime = to_date('%s','yyyymmddhh24miss') where dev_source = 0 and dev_guid = '%s'", sta, date, code)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
    LUDB_EXECUTE_STMT(stmt, sql)
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function UpdatePos(code, lat, lon)
    --lat, lon = Gps84_To_Gcj(lat, lon)
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
    local date=os.date("%Y%m%d%H%M%S")
    local sql = string.format("update GB28181_DEVICE set LATITUDE = %s, LONGITUDE = %s, updatetime = to_date('%s','yyyymmddhh24miss') where dev_source = 0 and dev_guid = '%s'", lat, lon, date, code)
    print(sql)
    local stmt = LUDB_CREATE_STMT(con)
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
    --记录插入设备表
    local code = ""
    if dev["DevID"] ~= nil then
        code = dev["DevID"]
    end
    if string.len(code)<8 then
        return true
    end
    local name = ""
    if dev["Name"] ~= nil then
        name = dev["Name"]
    end
    local lat = "0"
    if dev["Latitude"] ~= nil then
        lat = dev["Latitude"]
    end
    local lon = "0"
    if dev["Longitude"] ~= nil then
        lon = dev["Longitude"]
    end
    --lat, lon = Gps84_To_Gcj(lat, lon)
    local status = "0"
    if(dev["Status"] == "ON") then
        status = "1"
    end
    local parent = ""
    if string.len(code)>8 then
        parent = string.sub(code, 0, 8)
    end
    local row = {code, name, lat, lon, status, parent}
    --print("dev:"..dev["DevID"].." name:"..dev["Name"].." Pos:"..lat.." "..lon)
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
    LUDB_CREATE_STMT(stmt, "TRUNCATE TABLE GB28181_DEVICE")
    LUDB_COMMIT(con)
    LUDB_FREE_STMT(stmt)
    LUDB_FREE_CONN(con)
    return true
end

function Init()
    LUDB_INIT({dbtype="oracle", path="D:\\jszt\\instantclient_11_2_64"})
    LUDB_CREAT_POOL({dbtype="oracle", tag="DB", dbpath="44.59.86.1:1521/orcl", user="roadnetwork_ch", pwd="123456", max=5, min=1, inc=2})
    --设备表插入工具
    local sql = "insert into GB28181_DEVICE (dev_guid, dev_name, LATITUDE, LONGITUDE, STATUS, dev_source, PARENT_ID) values (:GUID, :NAME, :LAT, :LON, :STATUS, 0, :PARENT)"
    devHelp = LUDB_BATCH_INIT("oracle", "DB", sql, 50, 10, {
        {bindname = "GUID",       coltype = LUDB_TYPE_CHR, maxlen = 40},
        {bindname = "NAME",       coltype = LUDB_TYPE_CHR, maxlen = 40},
        {bindname = "LAT",        coltype = LUDB_TYPE_CHR, 10},
        {bindname = "LON",        coltype = LUDB_TYPE_CHR, 10},
        {bindname = "STATUS",     coltype = LUDB_TYPE_INT},
        {bindname = "PARENT",     coltype = LUDB_TYPE_CHR, maxlen = 40}
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
    dLat = dLat * 180.0 / ((a * (1.0 - ee)) / magic * sqrtMagic * math.pi)
    dLon = dLon * 180.0 / (a / sqrtMagic * math.cos(radLat) * math.pi)
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
    x, y = Gps84_To_Gcj(tonumber(lat), tonumber(lon))
    --print(x.."  "..y)
    
    local ret = {
        Latitude = tostring(x),
        Longitude = tostring(y)
    }
    return ret
end
