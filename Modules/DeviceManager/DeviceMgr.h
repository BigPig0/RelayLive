#pragma once
#include "PublicDefine.h"

#ifdef DEVICEMANAGER_EXPORTS
#define DEVICE_API __declspec(dllexport)
#else
#define DEVICE_API
#endif

namespace DeviceMgr
{
    bool Init();

    bool Cleanup();

    /**
     * 平台注册
     * @param[in] pPlatform 注册过来的平台信息
     * @return 成功true，失败false
     */
    bool RegistPlatform(PlatFormInfo platform);

    /**
     * 平台保活更新
     * @return 成功true，失败false
     */
    bool KeepAlivePlatform();

    /**
     * 平台是否在线
     * @return 在线true，离线false
     */
    bool IsPlatformLive();

    /**
     * 根据编码获取平台信息
     * @return 平台信息，若不存在或离线，则为nullptr
     */
    PlatFormInfo* GetPlatformInfo();

    /**
     * 增加设备
     * @param[in] vecDevInfo 设备信息。必须是堆中的实例，外部申请，本模块进行释放
     * @return 成功true，失败false
     * @note 向下级平台查询设备状态信息后，下级平台推送过来的信息
     */
    bool AddDevice(vector<DevInfo*> vecDevInfo, bool bUpdate = true);

    /**
     * 修改设备状态
     * @param[in] pDev 设备信息。必须是堆中的实例，外部申请，本模块进行释放
     * @return 成功true，失败false
     */
    bool UpdateDevice(DevInfo* pDev);

    /**
     * 获取平台下所有设备信息
     * @return 设备信息
     */
    vector<DevInfo*> GetDeviceInfo();

    /**
     * 根据编码获取设备信息
     * @param[in] strDevCode 设备编码
     * @return 设备信息，若不存在或离线，则为nullptr
     */
    DevInfo* GetDeviceInfo(string strDevCode);

    /**
     * 清理某个平台下的设备
     * @param[in] strPlatformCode 平台编码
     * @return 成功true，失败false
     */
    bool CleanPlatform();
}