#pragma once
#include <string>
#include <vector>
#include <map>

namespace DB {
    struct CameraInfo {
        std::string category_id;    // ���ID��0-ǹ��/1-����/2-����/3-��̨��
        std::string device_id;      // �豸ID
        std::string device_name;    // ����ͷ����
        std::string device_type;    // �豸���ͣ�����ͷ�̶�Ϊ0x3
        std::string inport;         // ͨ����
        std::string ConnectType;    // �������ͣ�0-TCP/1-UDP/2-MCAST/3-RTP��
        std::string controlunit_id; // ������������ID
        std::string parent_device_id; //���豸ID����������ID
        std::string region_id;      // ��������ID
        std::string camera_id;      // ��ص�ID
        std::string user_index_code; // �û��Զ�����
    };

    struct DeviceInfo {
        std::string device_id;      // �豸ID
        std::string device_name;    // �豸����
        std::string device_type;    // �豸����
        std::string device_state;   // �豸״̬ 0-���� 1-����
        std::string device_talk;    // �Խ�ͨ����
        std::string device_chan;    // ͨ������
        std::string ip_address;     // �豸IP��ַ
        std::string device_port;    // �豸�˿�
        std::string cell_id;        // ��������ID
        std::map<std::string,CameraInfo*> cameras;  //ͨ����-�����Ϣ
    };

    extern std::map<std::string, DeviceInfo*> _devs;
    extern std::map<std::string, CameraInfo*> _cams;

    void DevInfo2DB();
    void Init();
};