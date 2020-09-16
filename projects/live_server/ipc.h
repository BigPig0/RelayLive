#pragma once
#include <stdint.h>
#include <string>

namespace IPC {
    struct PlayRequest {
        uint32_t    id;    //����ID
        std::string code;  //��Ҫ���ŵ��豸����
        int         ret;   //��������0ʧ�ܣ� 1�ɹ�
        std::string info;  //Ӧ�����Ϣ
        bool        finish;//�Ƿ��յ�Ӧ��Ĭ��false���յ�Ӧ����Ϊtrue
        uint32_t    port;  //Ӧ��ָ���ı��ؽ��ն˿�
    };

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

    /**
     * ֪ͨsip server׼���������󣬵õ����λỰʹ�õı��ض˿�
     * @param code ���󲥷ŵ��豸����
     */
    PlayRequest* CreateReal(std::string code);

    /**
     * ֪ͨsip server��������
     */
    void RealPlay(PlayRequest* req);

    /**
     * ɾ������ʵ��
     */
    void DestoryRequest(PlayRequest *req);

	void Stop(uint32_t port);
};