#pragma once
#include "uvipc.h"
#include <string>

namespace IPC {
    /**
     * ��ʼ��IPC
     * @name ipc�ͻ������ͣ���������ipc�ͻ�������
     * @port ��Ƶ��������˿ڣ���������ipc�ͻ�������
     */
    uv_ipc_handle_t* Init(char *type, int port, uv_ipc_recv_cb cb = NULL);

    /** ����IPC */
    void Cleanup();

    /**
     * ��ctrl server���͵�ǰ���ӵ�������Ϣ
     */
    void SendClients(std::string info);

};