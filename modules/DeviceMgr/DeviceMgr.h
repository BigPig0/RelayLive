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
     * ƽ̨ע��
     * @param[in] pPlatform ע�������ƽ̨��Ϣ
     * @return �ɹ�true��ʧ��false
     */
    bool RegistPlatform(PlatFormInfo platform);

    /**
     * ƽ̨�������
     * @return �ɹ�true��ʧ��false
     */
    bool KeepAlivePlatform();

    /**
     * ƽ̨�Ƿ�����
     * @return ����true������false
     */
    bool IsPlatformLive();

    /**
     * ���ݱ����ȡƽ̨��Ϣ
     * @return ƽ̨��Ϣ���������ڻ����ߣ���Ϊnullptr
     */
    PlatFormInfo* GetPlatformInfo();

    /**
     * �����豸
     * @param[in] vecDevInfo �豸��Ϣ�������Ƕ��е�ʵ�����ⲿ���룬��ģ������ͷ�
     * @return �ɹ�true��ʧ��false
     * @note ���¼�ƽ̨��ѯ�豸״̬��Ϣ���¼�ƽ̨���͹�������Ϣ
     */
    bool AddDevice(vector<DevInfo*> vecDevInfo, bool bUpdate = true);

    /**
     * �޸��豸״̬
     * @param[in] pDev �豸��Ϣ�������Ƕ��е�ʵ�����ⲿ���룬��ģ������ͷ�
     * @return �ɹ�true��ʧ��false
     */
    bool UpdateDevice(DevInfo* pDev);

    /**
     * ��ȡƽ̨�������豸��Ϣ
     * @return �豸��Ϣ
     */
    vector<DevInfo*> GetDeviceInfo();

    /**
     * ���ݱ����ȡ�豸��Ϣ
     * @param[in] strDevCode �豸����
     * @return �豸��Ϣ���������ڻ����ߣ���Ϊnullptr
     */
    DevInfo* GetDeviceInfo(string strDevCode);

    /**
     * ����ĳ��ƽ̨�µ��豸
     * @param[in] strPlatformCode ƽ̨����
     * @return �ɹ�true��ʧ��false
     */
    bool CleanPlatform();
}