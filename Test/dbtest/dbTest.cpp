#include "common.h"
#include "DeviceMgr.h"

int main()
{
    /** 创建日志文件 */
    char path[MAX_PATH];
    sprintf_s(path, MAX_PATH, ".\\log\\test.txt");
    Log::open(Log::Print::both, Log::Level::debug, path);
    Log::debug("version: %s %s", __DATE__, __TIME__);

    /** 加载配置文件 */
    if (!Settings::loadFromProfile(".\\config.txt"))
    {
        Log::error("配置文件错误");
        return -1;
    }
    Log::debug("Settings::loadFromProfile ok");

    /** 初始化设备模块 */
    if (!DeviceMgr::Init())
    {
        Log::error("DeviceManagerInstance init failed");
        return -1;
    }
    Log::debug("DeviceMgr::Init ok");


    double lat = 23.123456;
    double lon = 128.123456;
    vector<DevInfo *> vecDevInfo;
    for(int i=0; i<10; i++){
        DevInfo* dev = new DevInfo();
        char devid[20]={0};
        sprintf(devid,"%06d", i);
        dev->strDevID = devid;
        dev->strName = "test" + std::to_string(i);
        dev->strStatus = "ON";
        dev->strLatitude = to_string(lat+0.01*i);
        dev->strLongitude = to_string(lon+0.01*i);
        dev->strPTZType = std::to_string(i%4);
        dev->strParentID = "000000";
        vecDevInfo.push_back(dev);
    }
    DeviceMgr::AddDevice(vecDevInfo);

    Sleep(1000);

    for(int x=0; x<100; x++) {
        Log::debug("time %d", x);
        for(int i=0; i<10; i++){
            DevInfo *dev = new DevInfo();
            char devid[20]={0};
            sprintf(devid,"%06d", i);
            dev->strDevID = devid;
            dev->strName = "test" + std::to_string(i);
            dev->strStatus = "ON";
            dev->strLatitude = to_string(lat+0.01*i+0.00001*x);
            dev->strLongitude = to_string(lon+0.01*i+0.00001*x);
            dev->strPTZType = std::to_string(i%4);
            dev->strParentID = "000000";
            //Log::debug("begin update");
            DeviceMgr::UpdateDevice(dev);
            //Log::debug("finish update");
        }
        Sleep(1000);
    }


    Sleep(INFINITE);
    return 0;
}