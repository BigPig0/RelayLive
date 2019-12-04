#pragma once
#include <string>
#include <vector>
#include <map>

namespace DB {
    struct CameraInfo {
        std::string category_id;    // 类别ID（0-枪机/1-半球/2-快球/3-云台）
        std::string device_id;      // 设备ID
        std::string device_name;    // 摄像头名称
        std::string device_type;    // 设备类型，摄像头固定为0x3
        std::string inport;         // 通道号
        std::string ConnectType;    // 连接类型（0-TCP/1-UDP/2-MCAST/3-RTP）
        std::string controlunit_id; // 所属控制中心ID
        std::string parent_device_id; //父设备ID，即编码器ID
        std::string region_id;      // 所属区域ID
        std::string camera_id;      // 监控点ID
        std::string user_index_code; // 用户自定义编号
    };

    struct DeviceInfo {
        std::string device_id;      // 设备ID
        std::string device_name;    // 设备名称
        std::string device_type;    // 设备类型
        std::string device_state;   // 设备状态 0-在线 1-断线
        std::string device_talk;    // 对讲通道数
        std::string device_chan;    // 通道个数
        std::string ip_address;     // 设备IP地址
        std::string device_port;    // 设备端口
        std::string cell_id;        // 所属区域ID
        std::map<std::string,CameraInfo*> cameras;  //通道号-相机信息
    };

    extern std::map<std::string, DeviceInfo*> _devs;
    extern std::map<std::string, CameraInfo*> _cams;

    void DevInfo2DB();
    void Init();
};