#pragma once
#include <string>

namespace IPC {
    /**
     * ��ʼ��IPC
     * @name ipc�ͻ������ͣ���������ipc�ͻ�������
     * @port ��Ƶ��������˿ڣ���������ipc�ͻ�������
     */
    bool Init(char *type, int port);

    /** ����IPC */
    void Cleanup();

    /**
     * ��ctrl server���͵�ǰ���ӵ�������Ϣ
     */
    void SendClients(std::string info);

};