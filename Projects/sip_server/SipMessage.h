#pragma once

namespace SipServer {

/**
 * 处理接收的消息，解析及应答。发送message请求(查询目录、云台控制)
 */
class CSipMessage
{
public:

    /**
     * 处理Message事件
     */
    void OnMessage(eXosip_event_t *osipEvent);

    /**
     * 目录查询
     */
    void QueryDirtionary();

    /**
     * 设备状态查询
     * @param devID[in] 设备id
     */
    void QueryDeviceStatus(string devID);

    /**
     * 设备信息查询请求
     * @param devID[in] 设备id
     */
    void QueryDeviceInfo(string devID);

    /**
     * 文件目录检索请求
     * @param devID[in] 设备id
     */
    void QueryRecordInfo(string devID, string strStartTime, string strEndTime);

    /**
     * 移动设备位置查询
     * @param devID[in] 设备id
     */
    void QueryMobilePosition(string devID);

    /**
     * 云台控制
     * @param strDevCode[in] 设备编码
     * @param nInOut[in]     镜头放大缩小 0:停止 1:缩小 2:放大
     * @param nUpDown[in]    镜头上移下移 0:停止 1:上移 2:下移
     * @param nLeftRight[in] 镜头左移右移 0:停止 1:左移 2:右移
     * @param cMoveSpeed[in] 镜头缩放速度
     * @param cMoveSpeed[in] 镜头移动速度
     */
    void DeviceControl(string strDevCode,
        int nInOut = 0, int nUpDown = 0, int nLeftRight = 0, 
        uint8_t cInOutSpeed = 0X1, uint8_t cMoveSpeed = 0XFF);

};

};