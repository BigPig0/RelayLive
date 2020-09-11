#include "util.h"
#include "db.h"
#include "ludb.h"
#include "luapp.hpp"
#include "util_encode_conv.h"

lua::Str luaGBK2UTF8(lua::Str s) {
    return EncodeConvert::AtoUTF8(s);
}

namespace DB {
    std::map<std::string, DeviceInfo*> _devs;    //设备ID-设备信息
    std::map<std::string, CameraInfo*> _cams;    //相机ID-相机信息
    static lua::State<>           m_lua;
    static lua::GlobalFunction<lua::Bool(lua::Table)> luafInsertDev;

    void DevInfo2DB() {
        //for(auto dev:_devs) {
        //    lua::Table tb;
        //    if(dev.second->cameras.empty())
        //        continue;
        //    tb["DevID"] = dev.second->cameras.begin()->first;
        //    if(!dev.second->device_name.empty())
        //        tb["Name"] = dev.second->device_name;
        //    if(!dev.second->device_type.empty())
        //        tb["PTZType"] = dev.second->device_type;
        //    if(dev.second->device_state == "0")
        //        tb["Status"] = "1";
        //    else
        //        tb["Status"] = "0";
        //    if(!dev.second->cell_id.empty())
        //        tb["ParentID"] = dev.second->cell_id;
        //    luafInsertDev(tb);
        //}
		for(auto dev:_cams) {
            lua::Table tb;
            tb["DevID"] = dev.first;
            if(!dev.second->device_name.empty())
                tb["Name"] = dev.second->device_name;
            if(!dev.second->device_type.empty())
                tb["PTZType"] = dev.second->device_type;
            tb["Status"] = "1";
            if(!dev.second->parent_device_id.empty())
                tb["ParentID"] = dev.second->parent_device_id;
            luafInsertDev(tb);
        }
    }

    void Init()
    {
        m_lua.setFunc("GBK2UTF8", &luaGBK2UTF8);

        ludb_use_lua((void*)&m_lua);

        m_lua.run(".","hikctrl.lua");
        lua::GlobalFunction<lua::Bool()> luafInit;
        m_lua.getFunc("Init",        &luafInit);
        m_lua.getFunc("InsertDev",   &luafInsertDev);
        if(!luafInit()){
            Log::error("data base init failed");
        }
    }
}