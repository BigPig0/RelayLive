#pragma once
#include <string>

namespace IPC {
    /**
     * ��ʼ��IPC
     * @port liveserver��������˿ڣ���������ipc�ͻ�������
     */
    bool Init(int port);

    /** ����IPC */
    void Cleanup();

    /**
     * ��livectrl server���͵�ǰ���ӵ�������Ϣ
     */
    void SendClients(std::string info);
};