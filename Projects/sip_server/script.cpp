#include "util.h"
#include "ludb.h"
#include "luapp.hpp"
#include "script.h"
#include "EncodeConvert.h"
#include "SipPrivate.h"
#include "SipServer.h"

namespace Script {

// lua环境句柄
lua::State<>           _lua;

// 以下为lua脚本所定义的函数
lua::GlobalFunction<lua::Bool()>
    luafInit;                 // 初始化
lua::GlobalFunction<lua::Bool()>
    luafCleanup;              // 清理
lua::GlobalFunction<lua::Table()> 
    luafGetDevInfo;           // 从数据库获取以前存下的设备信息
lua::GlobalFunction<lua::Bool(lua::Str, lua::Str)> 
    luafUpdateStatus;         // 更新设备在线状态
lua::GlobalFunction<lua::Bool(lua::Str, lua::Str, lua::Str)> 
    luafUpdatePos;            // 更新设备gps
lua::GlobalFunction<lua::Bool(lua::Table)> 
    luafInsertDev;            // 新增设备
lua::GlobalFunction<lua::Bool(lua::Int)>
    luafDeleteDev;            // 删除数据库中的设备
lua::GlobalFunction<lua::Table(lua::Table)>
    luafTransDevPos;          // gps转换

//一下函数为设置到lua脚本的函数
lua::Str luaGBK2UTF8(lua::Str s) {
    return EncodeConvert::AtoUTF8(s);
}
lua::Bool luaLogDebug(lua::Str s) {
    Log::debug(s.c_str());
    return true;
}
lua::Bool luaLogError(lua::Str s) {
    Log::error(s.c_str());
    return true;
}

// 设备信息转为lua格式
static lua::Table DevInfo2Table(SipServer::DevInfo* dev) {
    lua::Table tb;
    if(!dev->strDevID.empty())
        tb["DevID"] = dev->strDevID;
    if(!dev->strName.empty())
        tb["Name"] = dev->strName;
    if(!dev->strManuf.empty())
        tb["Manuf"] = dev->strManuf;
    if(!dev->strModel.empty())
        tb["Model"] = dev->strModel;
    if(!dev->strOwner.empty())
        tb["Owner"] = dev->strOwner;
    if(!dev->strCivilCode.empty())
        tb["CivilCode"] = dev->strCivilCode;
    if(!dev->strBlock.empty())
        tb["Block"] = dev->strBlock;
    if(!dev->strAddress.empty())
        tb["Address"] = dev->strAddress;
    if(!dev->strParental.empty())
        tb["Parental"] = dev->strParental;
    if(!dev->strParentID.empty())
        tb["ParentID"] = dev->strParentID;
    if(!dev->strSafetyWay.empty())
        tb["SafetyWay"] = dev->strSafetyWay;
    if(!dev->strRegisterWay.empty())
        tb["RegisterWay"] = dev->strRegisterWay;
    if(!dev->strCertNum.empty())
        tb["CertNum"] = dev->strCertNum;
    if(!dev->strCertifiable.empty())
        tb["Certifiable"] = dev->strCertifiable;
    if(!dev->strErrCode.empty())
        tb["ErrCode"] = dev->strErrCode;
    if(!dev->strEndTime.empty())
        tb["EndTime"] = dev->strEndTime;
    if(!dev->strSecrecy.empty())
        tb["Secrecy"] = dev->strSecrecy;
    if(!dev->strIPAddress.empty())
        tb["IPAddress"] = dev->strIPAddress;
    if(!dev->strPort.empty())
        tb["Port"] = dev->strPort;
    if(!dev->strPassword.empty())
        tb["Password"] = dev->strPassword;
    if(!dev->strStatus.empty())
        tb["Status"] = dev->strStatus;
    if(!dev->strLatitude.empty())
        tb["Latitude"] = dev->strLatitude;
    if(!dev->strLongitude.empty())
        tb["Longitude"] = dev->strLongitude;

    if(!dev->strPTZType.empty())
        tb["PTZType"] = dev->strPTZType;
    if(!dev->strPositionType.empty())
        tb["PositionType"] = dev->strPositionType;
    if(!dev->strRoomType.empty())
        tb["RoomType"] = dev->strRoomType;
    if(!dev->strUseType.empty())
        tb["UseType"] = dev->strUseType;
    if(!dev->strSupplyLightType.empty())
        tb["SupplyLightType"] = dev->strSupplyLightType;
    if(!dev->strDirectionType.empty())
        tb["DirectionType"] = dev->strDirectionType;
    if(!dev->strResolution.empty())
        tb["Resolution"] = dev->strResolution;
    if(!dev->strBusinessGroupID.empty())
        tb["BusinessGroupID"] = dev->strBusinessGroupID;
    if(!dev->strDownloadSpeed.empty())
        tb["DownloadSpeed"] = dev->strDownloadSpeed;
    if(!dev->strSVCSpaceSupportType.empty())
        tb["SVCSpaceSupportType"] = dev->strSVCSpaceSupportType;
    if(!dev->strSVCTimeSupportType.empty())
        tb["SVCTimeSupportType"] = dev->strSVCTimeSupportType;

    return tb;
}

// 执行lua脚本从数据库获取以前存下的设备信息
static void GetDevInfo() {
    lua::Table rows = luafGetDevInfo();
    MutexLock lock(&g_csDevs);
    for(auto it = rows.getBegin(); !it.isEnd(); it++){
        lua::Var k,v;
        it.getKeyValue(&k, &v);
        if(!lua::VarType<lua::Table>(v))
            continue;
        SipServer::DevInfo *dev = new SipServer::DevInfo();
        lua::Table row = lua::VarCast<lua::Table>(v);
        for(auto rit = row.getBegin(); !rit.isEnd(); rit++){
            lua::Var k2,v2;
            rit.getKeyValue(&k2, &v2);
            lua::Str key = lua::VarCast<lua::Str>(k2);
            if(key == "DevID"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strDevID = value;
            } else if(key == "Name"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strName = value;
            } else if(key == "Manuf"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strManuf = value;
            } else if(key == "Model"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strModel = value;
            } else if(key == "Owner"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strOwner = value;
            } else if(key == "CivilCode"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strCivilCode = value;
            } else if(key == "Block"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strBlock = value;
            } else if(key == "Address"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strAddress = value;
            } else if(key == "Parental"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strParental = value;
            } else if(key == "ParentID"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strParentID = value;
            } else if(key == "SafetyWay"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strSafetyWay = value;
            } else if(key == "RegisterWay"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strRegisterWay = value;
            } else if(key == "CertNum"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strCertNum = value;
            } else if(key == "Certifiable"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strCertifiable = value;
            } else if(key == "ErrCode"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strErrCode = value;
            } else if(key == "EndTime"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strEndTime = value;
            } else if(key == "Secrecy"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strSecrecy = value;
            } else if(key == "IPAddress"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strIPAddress = value;
            } else if(key == "Port"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strPort = value;
            } else if(key == "Password"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strPassword = value;
            } else if(key == "Status"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strStatus = value;
            } else if(key == "Latitude"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strLatitude = value;
            } else if(key == "Longitude"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strLongitude = value;
            } else if(key == "PTZType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strPTZType = value;
            } else if(key == "PositionType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strPositionType = value;
            } else if(key == "RoomType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strRoomType = value;
            } else if(key == "UseType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strUseType = value;
            } else if(key == "SupplyLightType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strSupplyLightType = value;
            } else if(key == "DirectionType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strDirectionType = value;
            } else if(key == "Resolution"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strResolution = value;
            } else if(key == "BusinessGroupID"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strBusinessGroupID = value;
            } else if(key == "DownloadSpeed"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strDownloadSpeed = value;
            } else if(key == "SVCSpaceSupportType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strSVCSpaceSupportType = value;
            } else if(key == "SVCTimeSupportType"){
                lua::Str value = lua::VarCast<lua::Str>(v2);
                dev->strSVCTimeSupportType = value;
            }
        }
        g_mapDevs.insert(make_pair(dev->strDevID, dev));
    }
}

void Init() {
    // 将函数映射到脚本里
    _lua.setFunc("GBK2UTF8", &luaGBK2UTF8);
    _lua.setFunc("LOGDEBUG", &luaLogDebug);
    _lua.setFunc("LOGERROR", &luaLogError);

    // ludb模块注册lua句柄，并将ludb中的一些函数映射到脚本里
    ludb_use_lua((void*)&_lua);

    // 启动脚本，并将一些脚本里面的函数映射出来
    string script = Settings::getValue("Script", "path", "livectrl.lua");
    _lua.run(".", script);
    _lua.getFunc("Init",        &luafInit);
    _lua.getFunc("Cleanup",     &luafCleanup);
    _lua.getFunc("GetDevInfo",  &luafGetDevInfo);
    _lua.getFunc("UpdateStatus",&luafUpdateStatus);
    _lua.getFunc("UpdatePos",   &luafUpdatePos);
    _lua.getFunc("InsertDev",   &luafInsertDev);
    _lua.getFunc("DeleteDev",   &luafDeleteDev);
    _lua.getFunc("TransDevPos", &luafTransDevPos);

    // 调用脚本进行初始化
    if(!luafInit()){
        Log::error("lua script init failed");
        return;
    }

    // 脚本从数据库读取设备
    GetDevInfo();
}

void Cleanup() {
    luafCleanup();
}

bool UpdateStatus(string code, string online) {
    return luafUpdateStatus(code, online);
}

bool UpdatePos(string code, string lat, string lon) {
    return luafUpdatePos(code, lat, lon);
}

bool InsertDev(SipServer::DevInfo* dev) {
    return luafInsertDev(DevInfo2Table(dev));
}

bool CleanDev(int64_t t) {
    return luafDeleteDev(t);
}

}