#include "uv.h"
#include "util.h"
#include "db.h"
#include "tmcp_interface_sdk.h"
#include <sstream>

#pragma comment(lib,"tmcp_interface_sdk.lib")

using namespace Platform ;

namespace HikPlat {

	static int _loginHandle = 0;
    static uv_mutex_t _mutex;

	bool Init() {
        // 初始化SDK
		int ret = Plat_Init();
		if(ret < 0) {
			Log::error("SDK init failed");
			return false;
		}

        //登陆平台
        string strIP = Settings::getValue("HikSDK", "IP");
        string strUsr = Settings::getValue("HikSDK", "User");
        string strPwd = Settings::getValue("HikSDK", "Pwd");
        string strPort= Settings::getValue("HikSDK", "Port");
		_loginHandle = Plat_LoginCMS(strIP.c_str(), strUsr.c_str(), strPwd.c_str(), strPort.c_str());
		if(_loginHandle <= 0) {
			Log::error("login failed %d", Plat_GetLastError());
			return false;
		}

		ret = Plat_QueryResource(_CONTROLUNIT, _loginHandle);
		if(ret < 0) {
			Log::debug("query camera failed %d", Plat_GetLastError());
			return false;
		}

        ////查询设备信息
        //ret = Plat_QueryResource(_DEVICE, _loginHandle);
        //if(ret < 0) {
        //    Log::error("query device failed %d", Plat_GetLastError());
        //    //return false;
        //}
		//
        //do {
        //    DB::DeviceInfo *dev = new DB::DeviceInfo();
        //    dev->device_id   = Plat_GetValueStr(Device::device_id, _loginHandle);
        //    dev->device_name = Plat_GetValueStr(Device::device_name, _loginHandle);
        //    dev->device_type = Plat_GetValueStr(Device::device_type, _loginHandle);
        //    dev->device_state= Plat_GetValueStr(Device::device_state, _loginHandle);
        //    dev->device_talk = Plat_GetValueStr(Device::device_talk, _loginHandle);
        //    dev->device_chan = Plat_GetValueStr(Device::device_chan, _loginHandle);
        //    dev->ip_address  = Plat_GetValueStr(Device::ip_address, _loginHandle);
        //    dev->device_port = Plat_GetValueStr(Device::device_port, _loginHandle);
        //    dev->cell_id     = Plat_GetValueStr(Device::cell_id, _loginHandle);
        //    if(!DB::_devs.count(dev->device_id))
        //        DB::_devs.insert(make_pair(dev->device_id, dev));
        //} while (Plat_MoveNext(_loginHandle)==0);
		//
        //Log::debug("device num %d", DB::_devs.size());

        //查询相机信息
        ret = Plat_QueryResource(_CAMERA, _loginHandle);
        if(ret < 0) {
            Log::debug("query camera failed");
            return false;
        }

        do {
            DB::CameraInfo *ca = new DB::CameraInfo();
            ca->category_id   = Plat_GetValueStr(Camera::category_id, _loginHandle);
            ca->device_id     = Plat_GetValueStr(Camera::device_id, _loginHandle);
            ca->device_name   = Plat_GetValueStr(Camera::device_name, _loginHandle);
            ca->device_type   = Plat_GetValueStr(Camera::device_type, _loginHandle);
            ca->inport        = Plat_GetValueStr(Camera::inport, _loginHandle);
            ca->ConnectType   = Plat_GetValueStr(Camera::ConnectType, _loginHandle);
            ca->controlunit_id = Plat_GetValueStr(Camera::controlunit_id, _loginHandle);
            ca->parent_device_id = Plat_GetValueStr(Camera::parent_device_id, _loginHandle);
            ca->region_id     = Plat_GetValueStr(Camera::region_id, _loginHandle);
            ca->camera_id     = Plat_GetValueStr(Camera::camera_id, _loginHandle);
            ca->user_index_code = Plat_GetValueStr(Camera::user_index_code, _loginHandle);
            //if(DB::_devs.count(ca->device_id)){
            //    if(!DB::_devs[ca->device_id]->cameras.count(ca->camera_id)) {
            //        DB::_devs[ca->device_id]->cameras.insert(make_pair(ca->camera_id, ca));
            //    }
            //}
            if(!DB::_cams.count(ca->camera_id))
                DB::_cams.insert(make_pair(ca->camera_id, ca));
        } while (Plat_MoveNext(_loginHandle)==0);

        Log::debug("camera num %d", DB::_cams.size());

        DB::DevInfo2DB();

        uv_mutex_init(&_mutex);
        
		return true;
	}

	void Cleanup() {
		if(_loginHandle > 0) {
			Plat_LogoutCMS(_loginHandle);
			_loginHandle = 0;
		}

		Plat_Free();

        uv_mutex_destroy(&_mutex);
	}

    string GetDevicesJson() {
        stringstream ss;
        ss << "{\"root\":[";
        bool bfirst = true;
        //for (auto dev:DB::_devs) {
        //    if(dev.second->cameras.empty())
        //        continue;
        //    if(!bfirst) {
        //        ss << ",";
        //    } else {
			//	bfirst = false;
			//}
        //    ss << "{\"DeviceID\":\"" << dev.second->cameras.begin()->first
        //        << "\",\"Name\":\"" << EncodeConvert::AtoUTF8(dev.second->device_name)
        //        << "\",\"Status\":\"";
        //    if(dev.second->device_state == "0")
        //        ss << "1";
        //    else
        //        ss << "0";
        //    ss << "\"}";
        //}
		for (auto dev:DB::_cams) {
			if(!bfirst) {
				ss << ",";
			} else {
				bfirst = false;
			}
			ss << "{\"DeviceID\":\"" << dev.first
                << "\",\"Name\":\"" << EncodeConvert::AtoUTF8(dev.second->device_name)
                << "\",\"Status\":\""
                << "1";
            ss << "\"}";
		}
        ss << "]}";

        return ss.str();
    }

    void DeviceControl(string strDev, int nInOut = 0, int nUpDown = 0, int nLeftRight = 0) {

    }
}