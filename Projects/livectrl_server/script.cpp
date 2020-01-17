#include "util.h"
#include "ludb.h"
#include "luapp.hpp"
#include "script.h"
#include "EncodeConvert.h"
#include "SipServer.h"

namespace Script {

lua::State<>           m_lua;
lua::GlobalFunction<lua::Bool()> luafInit;
lua::GlobalFunction<lua::Bool()> luafCleanup;
lua::GlobalFunction<lua::Table()> 
    luafGetDevInfo;
lua::GlobalFunction<lua::Bool(lua::Str, lua::Str)> 
    luafUpdateStatus;
lua::GlobalFunction<lua::Bool(lua::Str, lua::Str, lua::Str)> 
    luafUpdatePos;
lua::GlobalFunction<lua::Bool(lua::Table)> 
    luafInsertDev;
lua::GlobalFunction<lua::Bool()>
    luafDeleteDev;
lua::GlobalFunction<lua::Table(lua::Table)>
    luafTransDevPos;

lua::Str luaGBK2UTF8(lua::Str s) {
    return EncodeConvert::AtoUTF8(s);
}

lua::Table DevInfo2Table(SipServer::DevInfo* dev) {
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

vector<SipServer::DevInfo*> GetDevInfo()
{
    vector<SipServer::DevInfo*> vecRet;
    lua::Table rows = luafGetDevInfo();
    for(auto it = rows.getBegin(); !it.isEnd(); it++){
        lua::Var k,v;
        it.getKeyValue(&k, &v);
        if(!lua::VarType<lua::Table>(v))
            continue;
        DevInfo *dev = new DevInfo;
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
        vecRet.push_back(dev);
    }
    return vecRet;
}

void Init() {
    m_lua.setFunc("GBK2UTF8", &luaGBK2UTF8);

    ludb_use_lua((void*)&m_lua);

    m_lua.run(".","livectrl.lua");
    m_lua.getFunc("Init",        &luafInit);
    m_lua.getFunc("Cleanup",     &luafCleanup);
    m_lua.getFunc("GetDevInfo",  &luafGetDevInfo);
    m_lua.getFunc("UpdateStatus",&luafUpdateStatus);
    m_lua.getFunc("UpdatePos",   &luafUpdatePos);
    m_lua.getFunc("InsertDev",   &luafInsertDev);
    m_lua.getFunc("DeleteDev",   &luafDeleteDev);
    m_lua.getFunc("TransDevPos", &luafTransDevPos);

    // 调用脚本进行初始化
    if(!luafInit()){
        Log::error("data base init failed");
        return;
    }

    // 脚本从数据库读取设备
    vector<SipServer::DevInfo*> devInfo = GetDevInfo();
    for(auto &dev:devInfo) {

    }
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

bool CleanDev() {
    return luafDeleteDev();
}

}